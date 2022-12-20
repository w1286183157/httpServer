#pragma once

#include <exception>
#include <pthread.h>
#include <semaphore.h>

/*封装信号量类*/
class sem
{
public:
    sem();
    ~sem();
    bool wait();
    bool post();

private:
    sem_t m_sem;
};

/*封装互斥锁的类*/

class locker
{
public:
    /*创建并初始化互斥锁*/
    locker();
    ~locker();
    bool lock();
    bool unlock();

private:
    pthread_mutex_t m_mutex;
};

/*封装条件变量*/
class cond
{
public:
    cond();
    ~cond();
    bool wait();
    bool signal();

private:
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
};