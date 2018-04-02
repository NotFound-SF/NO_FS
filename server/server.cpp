
#include "common.h"
#include <pthread.h>
#include <iostream>
#include <queue>
#include <sys/resource.h>
#include <map>

using namespace std;


struct workinfo{
	pthread_cond_t    qready;                 // 每个任务的条件变量
	pthread_mutex_t   qlock;                  // 每个任务的队列的访问锁
	queue<int> workq;                         // 待处理工作队列,直接接受文件描述符号
};
 
static unsigned  int     num = 0;             // 调试相关的变量，检测服务器的连接数    
static int               epollfd;             // epoll文件描述符号

static pthread_mutex_t   elock = PTHREAD_MUTEX_INITIALIZER;
static struct workinfo   *worklist;           // 线程工作列表 

map<int, fdinfo>    fd_info_map;              // 从文件描述符号查找ID等信息 
map<uint32_t, int>  id_fd_map;                // 重ID查询到文件描述符信息



void *time_thread(void *arg);                 // 脉搏线程声明 
void *handle_thread(void *arg);               // 任务处理线程


int main(int argc, char *argv[]) 
{
	int        clientfd;                      // 暂存客户连接的文件描述符
	int        nready;                        // epoll事件就绪数
	
	socklen_t  clilen;                        // 存储连接地址的长度
	pthread_t  tid;                           // 线程标号
	
	struct     sockaddr_in  cliaddr;
		

    // 根据输入参数建立处理线程的数量

    int        thread_count;                                        // 要创建的线程数

	if (1 == argc) 
		thread_count = THREAD_COUNT;                                // 输入参数不存在就建立  THREAD_COUNT个处理线程
	else if(2 == argc) {
		thread_count = atoi(argv[1]);                               // 将命令行传入的第一参数作为创建线程的数目
	} else {
	    cerr << "argv error" <<endl;                                // 输入参数出错就直接退出
		exit(1);
	}


	// 修改进程能打开的文件数目限制，以保证有更多客户可以连接
	//
	struct     rlimit       flimit;
	
	flimit.rlim_cur = CLIENT_MAX;
	flimit.rlim_max = CLIENT_MAX;
	if (setrlimit(RLIMIT_NOFILE, &flimit) < 0)                     // 修改失败就直接打印能打开的最大文件描述符个数
	{
		perror("setrlimit");
        cout << "MAXFILE: " << sysconf(_SC_OPEN_MAX) << endl;
	}


    // 确保不会产生僵尸进程
    
	if (SIG_ERR == signal(SIGCHLD, SIG_IGN))
		ERR_EXIT("signal");
    

	// 创建服务器地址，设置端口等操作

    struct     sockaddr_in  servaddr;
    
	bzero(&servaddr, sizeof(servaddr));                          // 必须先清空
	servaddr.sin_family = AF_INET;                    
	servaddr.sin_port   = htons(MyPort);                          // 设置监听端口
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);                 // 监听所有的网卡


	// 初始化服务器，创建Socket并且绑定到设置的地址处，使用TCP协议并且设置监听的长度

    int        listenfd;                                          // socket的文件描述符
	
	listenfd = initServer(SOCK_STREAM, (struct sockaddr*)&servaddr,
						  sizeof(servaddr), SOMAXCONN);
    
	if (listenfd < 0)                                             // 表示创建服务器失败
		ERR_EXIT("initSever");
    
	set_fl(listenfd, O_NONBLOCK);                                 // 设置为非阻塞，使用epoll边缘触发必须设置


	// 创建epoll实例，并且申请能存放epoll活跃事件的空间 

    
	if ((epollfd = epoll_create1(EPOLL_CLOEXEC)) < 0)
		ERR_EXIT("epoll_create");
	
	// 初始化event关联listenfd的输入事件,并且将socket文件描述符号绑定到epollfd实例

    struct  epoll_event   event;                                  // 事件结构体
	
	event.events = EPOLLIN;                                       // 监听输入事件
	event.data.fd = listenfd;                                     // 将穿件的socket文件绑定epoll实例
	
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &event) < 0)  // 将设置添加到	epollfd
		ERR_EXIT("epoll_ctl");


	// 在内存空间申请存放被epoll更新的事件集合的内存空间

    struct     epoll_event  *epevents;
    
	epevents = (struct epoll_event*)malloc(sizeof(struct epoll_event)*EPOLL_SIZE);
	if (NULL == epevents) {
        cerr <<  "malloc failed" << endl;
		exit(1);
	}
    

	// 申请空间，存放每个处理线程相关的数据
	
	worklist = (struct workinfo*)malloc(thread_count*sizeof(struct workinfo));
    
	if (NULL == worklist) {
        cerr << "malloc error for worklist" << endl;
		exit(1);
	}

    // 初始化每个现存相关的数据
    
	for (int index = 0; index < thread_count; index++) {
		pthread_mutex_init(&worklist[index].qlock, NULL);
		pthread_cond_init(&worklist[index].qready, NULL);
		new (&worklist[index].workq) queue<int>();                   // 初始化队列

		// 创建响应的线程，并且给响应线程编号

		int *t_num = new int;                                        // 必须从内存申请空间

		*t_num = index;
		pthread_create(&tid, NULL, &handle_thread, t_num);           // 创建响应的线程
	}


	// 创建超时检测线程
	
	pthread_create(&tid, NULL, &time_thread, NULL);                  // 该线程会释放资源


	// 大循环等待epoll接受到输入事件返回，根据需要插入到响应的map

    struct     fdinfo       newfd;                                   // 新的文件链接
    int        which;                                                // 用于标识就绪任务的线程
    int        new_work;                                             // 处理线程的新任务
	
	while (1) {
        
		nready = epoll_wait(epollfd, epevents, EPOLL_SIZE, -1);      // 阻塞等待，输入事件到来	
		
		if (-1 == nready)
			ERR_EXIT("epoll_wait");                                    // 失败直接退出

		for (int index = 0; index < nready; index++) {
			if (epevents[index].data.fd == listenfd) {                 // 表示有新的连接上来
				clilen = sizeof(cliaddr);
				clientfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);
				set_fl(clientfd, O_NONBLOCK);                          // 设置为非阻塞
				event.events = EPOLLIN|EPOLLET;                        // 边缘触发方式
				event.data.fd = clientfd;
				newfd.DID = INITDID;                                   // 初始化为无效
				newfd.timeout = TIMECOUNT;                             // 初始化倒计时
				
				pthread_mutex_lock(&elock);
				epoll_ctl(epollfd, EPOLL_CTL_ADD, clientfd, &event);   // 监听新的连接
                fd_info_map.insert(make_pair(clientfd, newfd));        // 将新链接的客户插入到fd-key表
                                
				num++;
                
				pthread_mutex_unlock(&elock);

				printf("From:%s Port:%d\n", inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
				printf("Client num:%u\n", num);
			} else {                                                // 表示客户端有数据交付

                // 将新的任务加入待处理队列
                
				which = epevents[index].data.fd % thread_count;     // 选择队列
				new_work = epevents[index].data.fd;                 // 初始化要加入队列的数据
				
				pthread_mutex_lock(&worklist[which].qlock);
				if (worklist[which].workq.empty()) {                // 待处理任务队列为空发信号
					worklist[which].workq.push(new_work);
					pthread_cond_signal(&worklist[which].qready);   //线程需要唤醒,可能多余		
				} else {
					worklist[which].workq.push(new_work);
				}
				pthread_mutex_unlock(&worklist[which].qlock);
			}
		}
	}

	close(listenfd);
	close(epollfd);
	free(epevents);

	printf("Done\n");

	exit(0);
}



// 处理线程
void *handle_thread(void *arg)
{
	int               mytid;
	int               tagfd;
	int               str_len;
	struct datformat  buf;
	int               my_work;

	mytid = *(int*)arg;                                                               // 取得自身的编号
	delete (int*)arg;                                                                 // 释放掉主线程申请的空间


	while(1) {

        // 阻塞等待新任务到来
        
		pthread_mutex_lock(&worklist[mytid].qlock);                                   // 锁定就绪队列		
		while(worklist[mytid].workq.empty())        
			pthread_cond_wait(&worklist[mytid].qready, &worklist[mytid].qlock);       // 等待条件成立
		my_work = worklist[mytid].workq.front();                                      // 从就绪队列取出一个任务
		worklist[mytid].workq.pop();                                                  // 从队列删除一个取出的任务
		pthread_mutex_unlock(&worklist[mytid].qlock);                                 // 锁定就绪队列		

		// 处理主任务发来的任务
		while (1) {
            
			str_len = recv(my_work, &buf, sizeof(struct datformat), 0);           // 读取数据
            
			if (0 == str_len) {                                                       // 表示客户端断啦
                pthread_mutex_lock(&elock);
				// 标记为删除 0表示将要删除
				fd_info_map[my_work].timeout = 0;                                     // 0标记该节点被删除
				pthread_mutex_unlock(&elock);
				break;
			} else if (str_len < 0){                        
				if (EAGAIN == errno)                                                  // 表示缓冲区无数据可读
					break;
			} else {                                                                  // 表示有真实数据

                // 刷新超时参数,取出通信目标fd
			
				pthread_mutex_lock(&elock);
				fd_info_map[my_work].timeout = TIMECOUNT;                              // 刷线超时参数
				if (INITDID == fd_info_map[my_work].DID) {                             // 该链接第一次真实通信添加key->dat
					fd_info_map[my_work].DID = buf.selfID;	                           // 在main线程已经插入
                    id_fd_map.insert(make_pair(buf.selfID, my_work));                  // 即使有第二个同样的ID也不会打断第一个
					
					printf("DID : %d\n", buf.selfID);
				}

				// 通过目标ID找到指定指定文件描述符,0表示目标设备未上线 
				
                auto itr_temp = id_fd_map.find(buf.targetID);

                if (itr_temp == id_fd_map.end())
                    tagfd = 0;
                else
                    tagfd = itr_temp->second;
				
				pthread_mutex_unlock(&elock);

				if (0 != buf.targetID) {
					if (0 != tagfd) {
						send(tagfd, &buf, sizeof(datformat), 0);                       // 去掉了目标和头信息需要修改
					}else {                                                            // 表示设备断线
						//send(my_work.cfd, "outlin", sizeof(datformat), 0);           // 提示自己
					}
				} else {                                                               // 脉搏信息不处理
					//printf("ID: %d\talive\n", buf.selfID);
				}
			}
     	}
	}

	return 0;
}



// 该线程定时递减文件描述符中的标志，以判断设备是否掉线
// 以及释放只连接占用资源的恶意用户资源
void *time_thread(void *arg)
{
	int            count;
	uint32_t       tmpID;
	int            tmpfd;
	
	arg = arg;

	while (1) {
		// 遍历所有连接的超时时间
		pthread_mutex_lock(&elock);
		for(auto itr = fd_info_map.begin(); itr != fd_info_map.end(); count++) {       // count计数变量个数
			
			tmpID = itr->second.DID;                                                   // 取得设备ID
			itr -> second.timeout--;                                                   // 超时递减	

            // 调试代码
 //           cout << "fd: " << id_fd_map.find(tmpID)->second;
 //           cout << "  ID: " << tmpID << endl;  



			if (itr->second.timeout <= 0 ) {                                           // 表示超时或者客户放弃连接            
				tmpfd = itr->first;                                                    // 取得文件描述符号
				epoll_ctl(epollfd, EPOLL_CTL_DEL, tmpfd, NULL);                        // 从epoll中移除
				//删除队列中的标记
				id_fd_map.erase(tmpID);                                                 // 通过key删除
				itr = fd_info_map.erase(itr);                                               
				num--;
				close(tmpfd);
				printf("Client num:%u\n", num);
			} else {
				itr++;
			}
			// 减小加锁粒度，不让处理线程长时间挂起
			if (count>GRANULARITY){
				count = 0;
				pthread_mutex_unlock(&elock);                                          // 释放锁
				usleep(100);                                                           // 切换一次线程，让处理线程执行
				pthread_mutex_lock(&elock);                                            // 重新获取锁
			}
		}
		pthread_mutex_unlock(&elock);                                                  // 遍历完一次解锁
		
		sleep(TIMECYCLE);                                                              // 睡眠该线程
	}

	return 0;
}



