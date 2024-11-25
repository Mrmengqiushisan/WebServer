#pragma once
#include <pthread.h>
#include <iostream>
#include <cstdio>
#include <list>
#include "Locker.h"
//线程池类 定义成模板类为了代码的复用 模板参数T是任务类
template<typename T>
class Threadpool{
public:
	Threadpool(int thread_number = 8, int max_requests = 10000);
	~Threadpool();
	bool append(T* request);
private:
	//线程的数量
	int m_thread_number;
	//线程池数组，大小为m_thread_number
	pthread_t* m_threads;
	//请求队列中最多允许的等待处理的请求数量
	int m_max_requests;
	//请求队列
	std::list<T*>m_workqueue;
	//互斥锁
	Locker m_queuelocker;
	//信号量用来判断是否有任务需要处理
	Sem m_queuestat;
	//是否结束线程
	bool m_stop;
private:
	static void* worker(void* arg);
	void run();
};

template<typename T>
inline Threadpool<T>::Threadpool(int thread_number, int max_requests)
	:m_thread_number(thread_number),m_max_requests(max_requests),m_stop(false),m_threads(NULL){
	if (thread_number <= 0 || max_requests <= 0) {
		throw std::exception();
	}
	m_threads = new pthread_t[m_thread_number];
	if (!m_threads) {
		throw std::exception();
	}
	for (int i = 0; i < thread_number; i++) {
		printf("create the %dth thread\n", i);
		if (pthread_create(m_threads + i, NULL, worker, this) != 0) {
			delete[] m_threads;
			throw std::exception();
		}
		if (pthread_detach(m_threads[i])) {
			delete[] m_threads;
			throw std::exception();
		}
	}
}

template<typename T>
inline Threadpool<T>::~Threadpool()
{
	delete[] m_threads;
	m_stop = true;
}

template<typename T>
inline bool Threadpool<T>::append(T* request){
	m_queuelocker.Lock();
	if (m_workqueue.size() > m_max_requests) {
		m_queuelocker.unLock();
		return false;
	}
	m_workqueue.push_back(request);
	m_queuelocker.unLock();
	m_queuestat.Post();
	return true;
}

template<typename T>
inline void* Threadpool<T>::worker(void* arg){
	Threadpool* thiz = (Threadpool*)arg;
	thiz->run();
	return nullptr;
}

template<typename T>
inline void Threadpool<T>::run(){
	while (!m_stop) {
		m_queuestat.Wait();
		m_queuelocker.Lock();
		if (m_workqueue.empty()) {
			m_queuelocker.unLock();
			continue;
		}
		T* request = m_workqueue.front();
		m_workqueue.pop_front();
		m_queuelocker.unLock();
		if (!request) {
			continue;
		}
		request->process();
	}
}
