#include "AsyncLogging.h"
#include <fstream>
#include <unistd.h>

AsyncLogging::AsyncLogging(const std::string& file)
    : file_(file),
      running_(false),
      currentBuffer_(new Buffer()),
      nextBuffer_(new Buffer())
{
    currentBuffer_->ensureWritableBytes(4096);
    nextBuffer_->ensureWritableBytes(4096);
}

void AsyncLogging::start() {
    running_ = true;
    thread_ = std::thread(&AsyncLogging::threadFunc, this);
}

void AsyncLogging::stop() {
    running_ = false;
    cond_.notify_all();
    if (thread_.joinable()) thread_.join();
}

void AsyncLogging::append(const char* msg, size_t len)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (currentBuffer_->writableBytes() >= len + 1) {
        currentBuffer_->append(msg, len);
        currentBuffer_->append("\n", 1);
    } else {
        buffersToWrite_.push_back(currentBuffer_);

        if (nextBuffer_) {
            currentBuffer_ = nextBuffer_;
            nextBuffer_ = nullptr;
        } else {
            currentBuffer_ = new Buffer();
            currentBuffer_->ensureWritableBytes(4096);
        }

        currentBuffer_->append(msg, len);
        currentBuffer_->append("\n", 1);
        cond_.notify_one();
    }
}

void AsyncLogging::threadFunc()
{
    std::ofstream out(file_, std::ios::app);
    Buffer* newBuffer1 = new Buffer();
    Buffer* newBuffer2 = new Buffer();
    newBuffer1->ensureWritableBytes(4096);
    newBuffer2->ensureWritableBytes(4096);

    std::vector<Buffer*> buffers;

    while (running_) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (buffersToWrite_.empty()) {
                cond_.wait_for(lock, std::chrono::seconds(2));
            }

            buffersToWrite_.push_back(currentBuffer_);
            currentBuffer_ = newBuffer1;
            newBuffer1 = nullptr;

            buffers.swap(buffersToWrite_);
            if (!nextBuffer_) {
                nextBuffer_ = newBuffer2;
                newBuffer2 = nullptr;
            }
        }

        // 将 buffers 里的内容写入文件
        for (auto buf : buffers) {
            out.write(buf->peek(), buf->readableBytes());
            buf->retrieveAll();
        }
        out.flush();

        if (!newBuffer1) {
            newBuffer1 = buffers.back();
            newBuffer1->retrieveAll();
        }
        if (!newBuffer2) {
            newBuffer2 = buffers[buffers.size() - 2];
            newBuffer2->retrieveAll();
        }

        buffers.clear();
    }
}
