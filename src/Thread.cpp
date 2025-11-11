#include "Thread.h"
#include "CurrentThread.h"
#include <semaphore.h>

std::atomic<int> Thread::numCreated_(0);

Thread::Thread(ThreadFunc fuc, const std::string &name) : started_(false),
                    joined_(false),
                    func_(std::move(fuc)),
                    name_(name),
                    tid_(0) {}

Thread::~Thread() {
    if (started_ && !joined_) {
        thread_->detach();
    }
}

void Thread::start() {
    started_ = true;
    sem_t  sem;
    sem_init(&sem, 0, 0); // 初始化信号量
    thread_ = std::make_shared<std::thread>([this,&sem]() {
    tid_ = CurrentThread::tid(); // 在新线程中获取线程 ID
    sem_post(&sem);          // 通知主线程线程 ID 已经获取
    func_();                     // 执行线程函数
    });
    sem_wait(&sem); // 等待新线程获取线程 ID
}

void Thread::join() {
    joined_ = true;
    thread_->join();
}