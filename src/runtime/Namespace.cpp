#include <runtime/Namespace.hpp>
#include <sys/mount.h>
#include <sys/wait.h>
#include <system_error>
#include<sys/eventfd.h>
#include <unistd.h>

namespace cppdock::runtime {

Namespace::Namespace(int flags) : flags_(flags) {}

struct ChildArgs {
  std::function<int()> func;
  int sync_fd;
};

static int child_main(void *arg) {
  auto *args = static_cast<ChildArgs *>(arg);

  if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL) == -1) {
    return -1;
  }

  uint64_t val=0;
  if(read(args->sync_fd,&val,sizeof(val))==-1){
    return -1;
  }

  close(args->sync_fd);

  return args->func();
}

int Namespace::spawn(std::function<int()> child_func, void *stack_ptr, int sync_fd) {
  ChildArgs args{std::move(child_func),sync_fd};

  pid_t pid = clone(child_main, stack_ptr, flags_ | SIGCHLD, &args);

  if (pid == -1) {
    throw std::system_error(errno, std::generic_category(), "Clone failed");
  }

  return pid;
}

} // namespace cppdock::runtime
