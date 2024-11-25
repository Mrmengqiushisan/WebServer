#ifndef TIMER_LST_H
#define TIMER_LST_H
//二叉平衡树实现 定时器队列
#include <stdio.h>
#include <arpa/inet.h>
#include <time.h>
#include <assert.h>
#include <iostream>
#include <set>
#include <unordered_map>
#include <vector>
#define BUFFER_SIZE 64
class util_timer; 

class client_data{
public:
    sockaddr_in cliadr; //客户端地址
    int         sockfd; //客户端套接字文件描述符
    char        buf[BUFFER_SIZE];//读缓存区域
    util_timer* timer;  //用于实际的处理这个节点下的信息
};
//单个节点类定义
typedef void(*CALLBACK)(client_data*);//回调函数
class util_timer{
public:
    time_t          expire;     //任务超时事件 这里指的是绝对时间
    CALLBACK        cb_func;    //回调函数用于处理具体的客户端任务
    client_data*    user_data;  //该节点下的用户信息
    bool operator<(const util_timer& rhs)const{
        return this->expire<rhs.expire;
    }
};
//构建二叉平衡树的方法类
class timer_set{
public:
    timer_set(){
        s.clear();
        data.clear();
    }
    ~timer_set(){

    }
    void add_timer(const util_timer& timer){
        if(!data.count(timer.user_data->sockfd)){
            data.emplace(timer.user_data->sockfd,timer);
            s.insert(timer);
        }
    }
    void adjust_timer(const util_timer& timer){
        if(!data.count(timer.user_data->sockfd)){
            std::cout<<"SET NO DATA FROM:"<<timer.user_data->sockfd<<std::endl;
            return;
        }
        auto& it=data[timer.user_data->sockfd];
        it=timer;   //引用修改哈希表中的定时器值
        s.erase(it);//删除就版本
        s.insert(timer);
    }
    void del_timer(const util_timer& timer){
        if(!data.count(timer.user_data->sockfd))
            return;
        //同时删除哈希表以及平衡树中的值
        int fd=timer.user_data->sockfd;
        data.erase(fd);
        s.erase(timer);
    }
    //SIGALARM 信号每次被触发就在其信号处理函数中执行该函数
    void tick(){
        if(s.empty())return;
        std::cout<<"TIMER TICK\n";
        time_t curtime=time(nullptr);
        std::vector<util_timer>deldata;
        for(auto& tm:s){
            if(curtime<tm.expire)break;
            tm.cb_func(tm.user_data);
            deldata.push_back(tm);
        }
        while(!deldata.empty()){
            s.erase(deldata.front());
            deldata.erase(deldata.begin());
        }
    }
    void print(){
        for(auto& da:s){
            printf("ID: %d,TIMER: %ld\n",da.user_data->sockfd,da.expire);
        }
    }
private:
    std::set<util_timer>s;
    std::unordered_map<int,util_timer>data;
};

#endif //TIMER_LST_H