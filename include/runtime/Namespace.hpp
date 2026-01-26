#ifndef NAMESPACE_HPP
#define NAMESPACE_HPP

#include <sched.h>
#include <functional>

namespace cppdock::runtime {

class Namespace {
public:
    static constexpr int DEFAULT_FLAGS = 
        CLONE_NEWPID  | 
        CLONE_NEWUTS  | 
        CLONE_NEWNS   | 
        CLONE_NEWNET  | 
        CLONE_NEWIPC  | 
        CLONE_NEWCGROUP | 
        CLONE_NEWTIME;

    explicit Namespace(int flags = DEFAULT_FLAGS);

    int spawn(std::function<int()> child_func, void* stack_ptr);

private:
    int flags_;
};

}


#endif