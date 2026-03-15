#include<runtime/Container.hpp>
#include<runtime/Namespace.hpp>
#include<ContainerConfig.hpp>


namespace cppdock::runtime {
  Container::Container(const ContainerConfigs &config): configs_(config), cgroup_(config.container_id),
   rootfs_(config.rootfs_path) {}
  Container::~Container(){
    
  }
  void Container::start(){
    cgroup_.create();
    cgroup_.set_resources(configs_.resources);
    auto payload_fun=[this](){return this->payload();};
    
  }
  void Container::stop(){

  }
  void Container::pause(){

  }
  void Container::resume(){

  }
  int Container::wait(){
    if(process_){
      return process_->wait();
    }
    return -1;
  }
}