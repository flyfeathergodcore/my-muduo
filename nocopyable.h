#pragma once
/*
不可拷贝类
*/
class nocopyable
{
public:
    nocopyable(nocopyable &) = delete;
    nocopyable &operator=(const nocopyable &) = delete;

protected:
    nocopyable() = default;
    ~nocopyable() = default;
};