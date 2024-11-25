#pragma once

#include<pthread.h>
#include<exception>
#include<semaphore.h>
//线程同步机制封装类
//互斥锁类
class Locker{
public:
	Locker() {
		if (pthread_mutex_init(&m_mutex, NULL) != 0) {
			throw std::exception();
		}
	}
	//上锁
	bool Lock() {
		return pthread_mutex_lock(&m_mutex) == 0;
	}
	//解锁
	bool unLock() {
		return pthread_mutex_unlock(&m_mutex) == 0;
	}
	//获取互斥量
	pthread_mutex_t* Get() {
		return &m_mutex;
	}
	~Locker() {
		pthread_mutex_destroy(&m_mutex);
	}
private:
	pthread_mutex_t m_mutex;
};
//条件变量
class Cond{
public:
	Cond() {
		if (pthread_cond_init(&m_cond, NULL) != 0) {
			throw std::exception();
		}
	}
	//唤醒条件
	bool Wait(pthread_mutex_t* mutex) {
		return  pthread_cond_wait(&m_cond, mutex) == 0;
	}
	bool Wait(pthread_mutex_t* mutex, timespec t) {
		return  pthread_cond_timedwait(&m_cond, mutex, &t) == 0;
	}
	bool Signal() {
		return pthread_cond_signal(&m_cond) == 0;
	}
	bool BroadCast() {
		return pthread_cond_broadcast(&m_cond) == 0;
	}
	~Cond() {
		pthread_cond_destroy(&m_cond);
	}
private:
	pthread_cond_t m_cond;
};
//信号量类
class Sem {
public:
	Sem() {
		if (sem_init(&m_sem, 0, 0) != 0) {
			throw std::exception();
		}
	}
	Sem(int num) {
		if (sem_init(&m_sem, 0, num) != 0) {
			throw std::exception();
		}
	}
	bool Wait() {
		return sem_wait(&m_sem) == 0;
	}
	bool Post() {
		return sem_post(&m_sem) == 0;
	}
	~Sem() {
		sem_destroy(&m_sem);
	}
private:
	sem_t m_sem;
};
