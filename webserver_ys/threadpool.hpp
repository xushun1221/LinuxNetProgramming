/*
@Filename : threadpool.hpp
@Description : threadpool
@Datatime : 2022/06/24 16:30:44
@Author : xushun
*/
#ifndef __THREADPOOL_H_
#define __THREADPOOL_H_

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "locker.hpp"

// 线程池类 模板参数是任务类
template<typename T>
class threadpool {
public:
    // 创建并初始化线程池
    // thread_number 线程池中线程的数量 max_requests 请求队列中最多允许的等待处理的请求数量
    threadpool(int thread_number = 8, int max_requests = 10000);
    ~threadpool();
    // 向任务队列中添加任务
    bool append(T* request);
private:
    // 工作线程的执行函数 不断地从任务队列中取出任务并执行
    // 在c++程序中使用pthread_create函数时 第3个参数必须指向一个静态成员
    // 所以将类的对象作为参数传递给静态函数 然后在静态函数中引用这个对象 调用其动态方法
    static void* worker(void* arg);
    void run();
private:
    int m_thread_number;        // 线程池中的线程数
    int m_max_requests;         // 请求队列中允许的最大请求数
    pthread_t* m_threads;       // 描述线程池的数组 大小为 m_thread_number
    std::list<T*> m_workqueue;  // 请求队列
    locker m_queuelocker;       // 保护请求队列的互斥锁
    sem m_queuestat;            // 是否有任务需要处理
    bool m_stop;                // 是否结束线程
};

template<typename T>
threadpool<T>::threadpool(int thread_number, int max_requests) : 
        m_thread_number(thread_number), m_max_requests(max_requests), 
        m_stop(false), m_threads(NULL) {
    if ((thread_number <= 0) || (max_requests <= 0)) {
        std::exception();
    }
    m_threads = new pthread_t[thread_number];
    if (!m_threads) {
        std::exception();
    }
    // 创建thread_number个线程 并将其设置为分离线程
    for (int i = 0; i < thread_number; ++ i) {
        printf("create the %dth thread\n", i);
        if (pthread_create(m_threads + i, NULL, worker, this) != 0) {
            delete [] m_threads;
            throw std::exception();
        }
        if (pthread_detach(m_threads[i]) != 0) {
            delete [] m_threads;
            throw std::exception();
        }
    }
}

template<typename T>
threadpool<T>::~threadpool() {
    delete [] m_threads;
    m_stop = true;
}

template<typename T>
bool threadpool<T>::append(T* request) {
    // 操作请求队列的时候需要加锁 因为它被所有线程共享
    m_queuelocker.lock();
    if (m_workqueue.size() > m_max_requests) {
        m_queuelocker.unlock();
        return false;
    }
    m_workqueue.push_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();
    return true;
}

template<typename T>
void* threadpool<T>::worker(void* arg) {
    threadpool* pool = (threadpool*)arg;
    pool -> run();
    return pool;
}

template<typename T>
void threadpool<T>::run() {
    while (!m_stop) {
        m_queuestat.wait();
        m_queuelocker.lock();
        if (m_workqueue.empty()) {
            m_queuelocker.unlock();
            continue;
        }
        T* request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();
        if (!request) {
            continue;
        }
        request -> process();
    }
}

#endif