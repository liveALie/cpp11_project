#ifndef _C11TEST_NONCOPYABLE_HPP_
#define _C11TEST_NONCOPYABLE_HPP_

class NonCopyable{
protected:
    NonCopyable() = default;
    ~NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};

#endif // _C11TEST_NONCOPYABLE_HPP_