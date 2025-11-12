#pragma once

#include <vector>
#include <string>
#include <cassert>
#include <algorithm>

class Buffer
{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;
    explicit Buffer()
        : buffer_(kCheapPrepend + kInitialSize),
          readerIndex_(kCheapPrepend),
          writerIndex_(kCheapPrepend) {}

    // 返回可读字节数、可写字节数、预留字节数
    size_t readableBytes() const { return writerIndex_ - readerIndex_; }
    size_t writableBytes() const { return buffer_.size() - writerIndex_; }
    size_t prependableBytes() const {return readerIndex_;}

    // 返回可读数据的起始地址
    const char *peek() const { return begin() + readerIndex_;}

    void retrieve(size_t len)
    {
        if (len < readableBytes())
        {
            readerIndex_ += len;
        }
        else
        {
            retrieveAll();
        }
    }

    void retrieveAll()
    {
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }

    void ensureWritableBytes(size_t len)
    {
        if (writableBytes() < len)
        {
            makeSpace(len);
        }
    }

    void makeSpace(size_t len)
    {
        if (writableBytes() + prependableBytes() < len + kCheapPrepend)
        {
            buffer_.resize(writerIndex_ + len);
        }
        else
        {
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_,
                      begin() + writerIndex_,
                      begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
    }

    std::string retrieveAsString(size_t len)
    {
        len = std::min(len, readableBytes());
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

    std::string retrieveAllAsString()
    {
        return retrieveAsString(readableBytes());
    }

    void append(const char *data, size_t len)
    {
        ensureWritableBytes(len);
        std::copy(data, data + len, beginWrite());
        writerIndex_ += len;
    }

    // 返回可写数据的起始地址
    char *beginWrite() { return begin() + writerIndex_; }
    const char *beginWrite() const {return begin() + writerIndex_;}

    //find\r\n
    const char* findCRLF() const
    {
        const char* crlf = std::search(peek(), begin() + writerIndex_,
                                       "\r\n", "\r\n" + 2);
        return crlf == begin() + writerIndex_ ? nullptr : crlf;
    }

    //find\r\n from start
    const char* findCRLF(const char* start) const
    {
        const char* crlf = std::search(start, begin() + writerIndex_,
                                       "\r\n", "\r\n" + 2);
        return crlf == begin() + writerIndex_ ? nullptr : crlf;
    }

    // retrieve until end
    void retrieveUntil(const char* end)
    {   
        assert(peek() <= end);
        assert(end <= beginWrite());
        retrieve(end - peek()); 
    }

    //读写文件描述符封装
    ssize_t readFd(int fd, int *savedErrno);
    ssize_t writeFd(int fd, int *savedErrno);

private:
    char *begin()
    {
        return &*buffer_.begin();
    }

    const char *begin() const
    {
        return &*buffer_.begin();
    }

    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;
};