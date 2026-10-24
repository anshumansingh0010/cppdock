#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <functional>
#include <sys/types.h>

namespace cppdock::runtime {

class Process {
public:
  static constexpr size_t STACK_SIZE = 1024 * 1024;

  explicit Process(std::function<int()> target);
  ~Process();

  void start(class Namespace &ns);
  void resume();
  int wait();
  pid_t pid() const;

private:
  std::function<int()> target_;
  pid_t pid_ = -1;
  int sync_fd_=-1;
  void *stack_ = nullptr;
};

} // namespace cppdock::runtime
#endif