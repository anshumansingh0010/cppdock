#ifndef NAMESPACE_HPP
#define NAMESPACE_HPP

#include <functional>
#include <sched.h>

namespace cppdock::runtime {

struct ChildArgs {
  std::function<int()> func;
  int sync_fd;
};

class Namespace {
public:
  explicit Namespace(int flags);

  int spawn(std::function<int()> child_func, void *stack_ptr, int sync_fd);

private:
  int flags_;
};

} // namespace cppdock::runtime

#endif