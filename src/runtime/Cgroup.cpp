#include<runtime/Cgroup.hpp>
#include <cerrno>
#include <stdexcept>
#include<sys/stat.h>
#include<fstream>
#include<iostream>
#include<unistd.h>

namespace cppdock::runtime {

  CGroup::CGroup(std::string name): name_(std::move(name)){
    path_=std::string(CGROUP_ROOT)+"/"+name_;
  }
  CGroup::~CGroup(){
    try{
      remove();
    }
    catch(const std::exception &e){
      std::cerr<<"Error in cgroup destructor for "<<name_<<"\n";
    }
  }
  // create paths
  void CGroup::create(){
    if(mkdir(CGROUP_ROOT,0755)==-1){
      if(errno!=EEXIST){
        throw std::runtime_error("Failed to create cgroup root folder "+std::string(CGROUP_ROOT));
      }
    }

    std::string root_control = std::string(CGROUP_ROOT) + "/cgroup.subtree_control";
    std::ofstream root_file(root_control);
    if(root_file.is_open()){
      root_file << "+cpu +memory";
    }

    if(mkdir(path_.c_str(),0755)==-1){
      if(errno!=EEXIST){
        throw std::runtime_error("Failed to create cgroup "+path_);
      }
    }
  }
  void CGroup::set_resources(const CGroupResources &resources){
    if(!resources.cpu_max.empty()){
      write_file("cpu.max", resources.cpu_max);
    }
    if(!resources.memory_max.empty()){
      write_file("memory.max",resources.memory_max);
    }
  }
  void CGroup::add_process(pid_t pid){
    write_file("cgroup.procs",std::to_string(pid));
  }
  void CGroup::remove(){
    if(rmdir(path_.c_str())==-1){
      if(errno!=ENOENT){
        throw std::runtime_error("Failed to remove cgroup "+path_);
      }
    }
  }
  void CGroup::write_file(const std::string &filename, const std::string &value){
    std::string file_path=path_+"/"+filename;
    std::ofstream file(file_path);
    if(!file.is_open()){
      throw std::runtime_error("Failed to open cgroup file "+file_path);
    }
    if(!(file<<value)){
      throw std::runtime_error("Failed to write in cgroup file "+file_path);
    }

  }
}