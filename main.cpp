#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include "Locker.h"
#include "Threadpool.h"
#include "http_conn.h"
#include <iostream>
#include <signal.h>
#define MAX_FD 65535
#define MAX_EVENT_NUMBER 10000
using namespace std;
//添加信号捕捉
extern void addsig(int sig,void(*handler)(int));
//添加文件描述符到epoll中
extern void addfd(int epollfd,int fd,bool one_shot);
//从epoll中删除文件描述符
extern void removefd(int epollfd,int fd);
//修改文件描述符
extern void mdfd(int epollfd,int fd,int ev);

int main(int argc, char* argv[]) {
	if(argc<=1){
		//basename 知识获取其基础的名字
		printf("按照如下格式运行：%s port_number\n",basename(argv[0]));
		exit(-1);
	}
	//获取端口号
	int port=atoi(argv[1]);
	//对SIGPIPE信号进行处理
	addsig(SIGPIPE,SIG_IGN);//SIG_IGN 忽略改信号 SIG_DFL 返回默认行为 SIG_ERR 返回错误 
	//创建线程池 初始化线程池
	Threadpool<http_conn>*pool=nullptr;
	try{
		pool=new Threadpool<http_conn>();
	}
	catch(...){
		exit(-1);
	}
	//创建一个数组用于保存所有的客户端信息
	http_conn* users=new http_conn[MAX_FD];
	//创建套接字
	int listenfd=socket(PF_INET,SOCK_STREAM,0);
	if(listenfd==-1){
		perror("socket");
		exit(-1);
	}
	//设置端口复用
	int opt=1;
	setsockopt(listenfd,SOL_SOCKET,SO_REUSEPORT,&opt,sizeof(opt));
	//绑定
	sockaddr_in seraddr;
	seraddr.sin_family=AF_INET;
	seraddr.sin_addr.s_addr=INADDR_ANY;
	seraddr.sin_port=htons(port);
	if(bind(listenfd,(sockaddr*)&seraddr,sizeof(seraddr))==-1){
		perror("bind");
		exit(-1);
	}
	//监听
	if(listen(listenfd,5)==-1){
		perror("listen");
		exit(-1);
	}
	//创建epoll对象 时间数组添加
	epoll_event events[MAX_EVENT_NUMBER];
	int epollfd=epoll_create(1);
	//将监听的文件描述符添加到epoll对象中
	addfd(epollfd,listenfd,false); 
	http_conn::m_epollfd=epollfd;
	while(1){
		int num=epoll_wait(epollfd,events,MAX_EVENT_NUMBER,-1);
		if(num<0&&errno!=EINTR){
			printf("epoll failure\n");
			break;
		}
		//循环遍历事件数组
		for(int i=0;i<num;i++){
			int sockfd=events[i].data.fd;
			if(sockfd==listenfd){
				//有客户端连接进来
				sockaddr_in cliaddr;
				socklen_t cliaddrLen=sizeof(cliaddr);
				int connfd=accept(listenfd,(sockaddr*)&cliaddr,&cliaddrLen);
				if(connfd==-1){
					perror("accept");
					exit(-1);
				}
				if(http_conn::m_user_count>=MAX_FD){
					//目前连接数满了
					//给客户端写一个信息，服务器内部正忙
					close(connfd);
					continue;
				}
				//将新的客户的数据初始化 放在数组中
				users[connfd].init(connfd,cliaddr);
			}else if(events[i].events&(EPOLLRDHUP|EPOLLHUP|EPOLLERR)){
				//对方异常断开或者错误等事件
				users[sockfd].close_conn();
			}else if(events[i].events&EPOLLIN){
				if(users[sockfd].read()){
					//一次性把所有数据都写出来
					pool->append(users+sockfd);
				}else {
					users[sockfd].close_conn();
				}
			}else if(events[i].events&EPOLLOUT){
				if(!users[sockfd].write()){
					users[sockfd].close_conn();
				}
			}
		}
	}
	close(epollfd);
	close(listenfd);
	delete[] users;
	delete pool;
	return 0;
}