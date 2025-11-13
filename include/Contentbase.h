#pragma once
#include "MyBuffer.h"
#include "Timestamp.h"

class Contentbase {
public:
    Contentbase() = default;
    virtual ~Contentbase() = default;

    virtual bool parseContent(mymuduo::MyBuffer* buf, Timestamp receiveTime) = 0;
    virtual bool isFinished() const = 0;
};