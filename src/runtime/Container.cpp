#include <cerrno>
#include <iostream>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ContainerConfig.hpp>
#include <runtime/Container.hpp>
#include <runtime/Namespace.hpp>
#include <runtime/Process.hpp>

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 64
#endif

namespace cppdock::runtime {
  
  Container::Container(const ContainerConfigs &config): configs_(config), cgroup_(config.container_id),
   rootfs_(config.rootfs_path) {}

  Container::~Container(){
    try {
      stop();
    } catch (const std::exception& e) {
      std::cerr << "Error in ~Container: " << e.what() << std::endl;
    } catch (...) {
      std::cerr << "Unknown error in ~Container" << std::endl;
    }
  }

  void Container::start(){
    if (process_ && process_->pid() > 0) {
      throw std::runtime_error("Container is already running");
    }
    std::cout<<"Starting container "<<configs_.container_id<<std::endl;

    cgroup_.create();
    try {
      cgroup_.set_resources(configs_.resources);

      auto payload_fun=[this](){return this->payload();};
    
      Namespace ns(configs_.flags);

      process_=std::make_unique<Process>(payload_fun);
      process_->start(ns);
    
      cgroup_.add_process(process_->pid());
      process_->resume();
    } catch (...) {
      if (process_ && process_->pid() > 0) {
        kill(process_->pid(), SIGKILL);
        try { 
          process_->wait(); 
        } catch (const std::exception& e) {
          std::cerr << "Failed to wait for killed process: " << e.what() << std::endl;
        }
      }
      try { 
        cgroup_.remove(); 
      } catch (const std::exception& e) {
        std::cerr << "Failed to remove cgroup: " << e.what() << std::endl;
      }
      throw;
    }
  }

  void Container::stop(){
    if(process_ && process_->pid() > 0){
      std::cout << "Stopping container " << configs_.container_id << std::endl;
      kill(process_->pid(), SIGKILL);
      process_->wait();
    }
    cgroup_.remove();
  }

  void Container::pause(){
    if(process_ && process_->pid()>0){
      kill(process_->pid(), SIGSTOP);
    }
  }

  void Container::resume(){
    if(process_ && process_->pid()>0){
      kill(process_->pid(), SIGCONT);
    }
  }

  int Container::wait(){
    if(process_){
      return process_->wait();
    }
    return -1;
  }

  int Container::payload(){
    std::cout<<"Inside isolated namespace, setting up root fs"<<std::endl;
    rootfs_.prepare();
    rootfs_.mount_essentials();

    std::string hostname = configs_.container_id.substr(0, HOST_NAME_MAX);
    if(sethostname(hostname.c_str(), hostname.length()) == -1){
      std::cerr<<"Warning: sethostname failed (errno " << errno << ")"<<std::endl;
    }

    if(setsid() == -1 && errno != EPERM){
      std::cerr<<"Warning: setsid failed (errno "<<errno<<")"<<std::endl;
    }

    clearenv();
    setenv("PATH", "/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin", 1);
    setenv("HOME", "/root", 1);
    setenv("USER", "root", 1);
    setenv("TERM", "xterm", 1);

    std::vector<std::string> args = configs_.args;
    if(args.empty()){
      args.push_back(configs_.command);
    }

    std::vector<char *> c_args;
    c_args.reserve(args.size() + 1);
    for(auto &arg : args){
      c_args.push_back(arg.data());
    }
    c_args.push_back(nullptr);

    std::cout<<"Executing process "<<std::endl;
    if(execvp(configs_.command.c_str(), c_args.data()) == -1){
      std::cerr<<"Failed to execute "<<configs_.command<<"\nExecvp errno ( "<<errno<<" )"<<std::endl;
      return (errno == ENOENT) ? 127 : 126;
    }
    return 0;
  }
}