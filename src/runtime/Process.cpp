#include <cstring>
#include <iostream>
#include <runtime/Namespace.hpp>
#include <runtime/Process.hpp>
#include <signal.h>
#include <sys/eventfd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <system_error>

namespace cppdock::runtime {

Process::Process(std::function<int()> target) : target_(std::move(target)) {
  stack_ = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);

  if (stack_ == MAP_FAILED) {
    throw std::system_error(errno, std::generic_category(), "Stack allocation failed");
  }
}

Process::~Process() {
  if (pid_ > 0) {
    int status = 0;
    pid_t ret = waitpid(pid_, &status, WNOHANG);
    if (ret == 0) {
      kill(pid_, SIGKILL);
      waitpid(pid_, nullptr, 0);
    }
    pid_ = -1;
  }
  if (sync_fd_ != -1) {
    close(sync_fd_);
    sync_fd_ = -1;
  }
  if (stack_) {
    munmap(stack_, STACK_SIZE);
    stack_ = nullptr;
  }
}

void Process::start(Namespace &ns) {
  // Stacks grow downward on x86_64; point to the end of the region
  void *stack_top = static_cast<char *>(stack_) + STACK_SIZE;

  sync_fd_=eventfd(0,EFD_CLOEXEC);

  if(sync_fd_==-1){
    throw std::system_error(errno, std::generic_category(), "eventfd creation failed");
  }
  try{
    pid_ = ns.spawn(target_, stack_top,sync_fd_);
  }
  catch(...){
    close(sync_fd_);
    sync_fd_=-1;
    throw;
  }
}

void Process::resume(){
  if(sync_fd_!=-1){
    uint64_t val=1;
    if(write(sync_fd_,&val,sizeof(val))==-1){
      int err = errno;
      close(sync_fd_);
      sync_fd_=-1;
      throw std::system_error(err, std::generic_category(), "Failed to unblock child");
    }
    close(sync_fd_);
    sync_fd_=-1;
  }
}

int Process::wait() {
  if(pid_<=0) return 0;

  int status=0;
  pid_t current_pid=pid_;

  pid_t ret;
  while ((ret = waitpid(pid_, &status, 0)) == -1 && errno == EINTR) {}

  if (ret == -1) {
    if(errno==ECHILD){
      std::cout<<"Process "<<pid_<<" was reaped by kernel"<<std::endl;
      pid_=-1;
      return 0;
    }
    throw std::system_error(errno, std::generic_category(), "Waitpid failed");
  }

  pid_=-1;
  if(WIFEXITED(status)){
    int exit_code=WEXITSTATUS(status);
    std::cout<<"Container process "<<current_pid<<" exited normally with status "<<exit_code<<std::endl;
    return exit_code;
  }
  if(WIFSIGNALED(status)){
    int sig=WTERMSIG(status);
    std::cout<<"Container process "<<current_pid<<" was killed by signal "<<sig<<" ("<< strsignal(sig) << ")"
#ifdef WCOREDUMP
    << (WCOREDUMP(status) ? " (core dumped)" : "")
#endif
    << std::endl;
    return 128+sig;
  }
  return -1;
}

pid_t Process::pid() const { return pid_; }

} // namespace cppdock::runtime
