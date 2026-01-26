#include <runtime/Namespace.hpp>
#include <runtime/Process.hpp>
#include <sys/mman.h>
#include <sys/wait.h>
#include <system_error>

namespace cppdock::runtime {

Process::Process(std::function<int()> target) : target_(std::move(target)) {
  stack_ = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);

  if (stack_ == MAP_FAILED) {
    throw std::system_error(errno, std::generic_category(),
                            "Stack allocation failed");
  }
}

Process::~Process() {
  if (pid_ > 0) {
    int status;
    waitpid(pid_, &status, WNOHANG);
  }
  if (stack_) {
    munmap(stack_, STACK_SIZE);
  }
}

void Process::start(Namespace &ns) {
  // Stacks grow downward on x86_64; point to the end of the region
  void *stack_top = static_cast<char *>(stack_) + STACK_SIZE;

  pid_ = ns.spawn(target_, stack_top);
}

int Process::wait() {
  int status;
  if (waitpid(pid_, &status, 0) == -1) {
    throw std::system_error(errno, std::generic_category(), "Waitpid failed");
  }
  return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

pid_t Process::pid() const { return pid_; }

} // namespace cppdock::runtime
