#include <runtime/Process.hpp>
#include <cerrno>
#include<runtime/Container.hpp>
#include<runtime/Namespace.hpp>
#include<ContainerConfig.hpp>
#include <sched.h>
#include <signal.h>
#include<iostream>


namespace cppdock::runtime {
  
  Container::Container(const ContainerConfigs &config): configs_(config), cgroup_(config.container_id),
   rootfs_(config.rootfs_path) {}

  Container::~Container(){
    stop();
  }

  void Container::start(){
    if (process_ && process_->pid() > 0) {
        throw std::runtime_error("Container is already running\n");
    }
    std::cout<<"Starting container "<<configs_.container_id<<std::endl;

    cgroup_.create();
    cgroup_.set_resources(configs_.resources);

    auto payload_fun=[this](){return this->payload();};
    
    Namespace ns(configs_.flags);

    process_=std::make_unique<Process>(payload_fun);
    process_->start(ns);

    cgroup_.add_process(process_->pid());
  }

  void Container::stop(){
    if(process_ && process_->pid()>0){
      std::cout<<"Stopping container "<<configs_.container_id<<std::endl;
      kill(process_->pid(),SIGKILL);
      process_->wait();
    }
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
    std::cout<<"Inside isolated namespace, setting up root fs\n";
    rootfs_.prepare();
    rootfs_.mount_essentials();

    std::vector<const char *> c_args;
    c_args.push_back(configs_.command.c_str());

    for(const auto &arg : configs_.args){
      c_args.push_back(arg.c_str());
    }
    c_args.push_back(nullptr);

    std::cout<<"Executing process \n";
    if(execvp(configs_.command.c_str(), const_cast<char**>(c_args.data()))==-1){
      std::cerr<<"Failed to execute "<<configs_.command<<"\nExecvp errno ( "<<errno<<" )\n";
      return -1;
    }
    return 0;
  }
}