#include "MyBuffer.h"

#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>

ssize_t mymuduo::MyBuffer::readFd(int fd, int *savedErrno)
{
    char extrabuf[65536] = {0};
    struct iovec vec[2];
    const size_t writable = writableBytes();
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writable;

    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;

    const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);
    if (n < 0)
    {
        *savedErrno = errno;
    }
    else if (static_cast<size_t>(n) <= writable)
    {
        writerIndex_ += n;
    }
    else
    {
        writerIndex_ = buffer_.size();
        append(extrabuf, n - writable);
    }

    return n;
}

ssize_t mymuduo::MyBuffer::writeFd(int fd, int* savedErrno)
{
    ssize_t n = 0;
    size_t toWrite = readableBytes();

    while (toWrite > 0)
    {
        n = ::write(fd, peek(), toWrite);

        if (n > 0) {
            readerIndex_ += n;
            toWrite -= n;
        } else {
            if (errno == EINTR)
                continue;

            if (errno == EAGAIN) {
                *savedErrno = EAGAIN;
                break;          // 等待 EPOLLOUT 下一次触发
            }

            *savedErrno = errno;
            return -1;         // 其他错误
        }
    }

    return n;
}
