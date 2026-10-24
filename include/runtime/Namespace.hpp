#ifndef NAMESPACE_HPP
#define NAMESPACE_HPP

#include <sched.h>
#include <functional>

namespace cppdock::runtime {

class Namespace {
public:
    explicit Namespace(int flags);

    int spawn(std::function<int()> child_func, void* stack_ptr,int sync_fd);

private:
    int flags_;
};

}


#endif