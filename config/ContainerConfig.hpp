#ifndef CONTAINER_CONFIGS_HPP
#define CONTAINER_CONFIGS_HPP

#include<runtime/Cgroup.hpp>
#include <string>
#include <vector>

namespace cppdock::runtime {
  struct ContainerConfigs{
    int flags;
    std::string container_id;
    std::string rootfs_path;
    std::string command;
    std::vector<std::string> args;
    CGroupResources resources;
  };
}

#endif