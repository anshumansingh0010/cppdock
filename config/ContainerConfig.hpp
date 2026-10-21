#ifndef CONTAINER_CONFIGS_HPP
#define CONTAINER_CONFIGS_HPP

#include<runtime/Cgroup.hpp>
#include <string>
#include <vector>
#include <sched.h>

namespace cppdock::runtime {
  struct ContainerConfigs{
    int flags=CLONE_NEWPID | CLONE_NEWUTS | CLONE_NEWNS;
    std::string container_id="";
    std::string rootfs_path="";
    std::string command="";
    std::vector<std::string> args={};
    CGroupResources resources;
  };
  class ContainerConfigsBuilder {
    public:
      ContainerConfigsBuilder() {};

      ContainerConfigsBuilder& with_flags(int flags){
        config.flags=flags;
        return *this;
      }

      ContainerConfigsBuilder& add_flags(int flags){
        config.flags|=flags;
        return *this;
      }

      ContainerConfigsBuilder& with_id(const std::string& id){
        config.container_id=id;
        return *this;
      }

      ContainerConfigsBuilder& with_rootfs(const std::string& path){
        config.rootfs_path=path;
        return *this;
      }

      ContainerConfigsBuilder& with_command(const std::string& cmd){
        config.command=cmd;
        return *this;
      }

      ContainerConfigsBuilder& with_args(const std::vector<std::string>& args){
        config.args=args;
        return *this;
      }

      ContainerConfigsBuilder& with_resources(const CGroupResources& res){
        config.resources=res;
        return *this;
      }

      ContainerConfigs build() {
        return config;
      }

    private:
      ContainerConfigs config;
  };
}

#endif