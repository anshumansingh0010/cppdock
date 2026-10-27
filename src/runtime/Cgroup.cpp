#include <unistd.h>
#include <thread>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <stdexcept>
#include <linux/magic.h>
#include <runtime/Cgroup.hpp>

#ifndef CGROUP2_SUPER_MAGIC
#define CGROUP2_SUPER_MAGIC 0x63677270
#endif

namespace cppdock::runtime {

CGroup::CGroup(std::string name) : name_(std::move(name)) {
  path_ = std::string(CGROUP_ROOT) + "/" + name_;
}

CGroup::~CGroup() {
  try {
    remove();
  } catch (const std::exception &e) {
    std::cerr << "Error in cgroup destructor for " << name_ << ": " << e.what()<< std::endl;
  }
}

void CGroup::create() {
  struct statfs fs_stat;
  if (statfs(CGROUP_MOUNT, &fs_stat) != 0 ||
      fs_stat.f_type != CGROUP2_SUPER_MAGIC) {
    throw std::runtime_error("cgroup v2 unified hierarchy is not mounted at " + std::string(CGROUP_MOUNT));
  }
  std::string root_control =std::string(CGROUP_MOUNT) + "/cgroup.subtree_control";
  std::ofstream root_file(root_control);
  if (root_file.is_open()) {
    root_file << "+cpu +memory";
  }

  if (mkdir(CGROUP_ROOT, 0755) == -1) {
    if (errno != EEXIST) {
      throw std::runtime_error("Failed to create cgroup root folder " + std::string(CGROUP_ROOT));
    }
  }

  std::string cppdock_control =
      std::string(CGROUP_ROOT) + "/cgroup.subtree_control";
  std::ofstream cppdock_file(cppdock_control);
  if (cppdock_file.is_open()) {
    cppdock_file << "+cpu +memory";
  }

  if (mkdir(path_.c_str(), 0755) == -1) {
    if (errno != EEXIST) {
      throw std::runtime_error("Failed to create cgroup " + path_);
    }
  }
}

void CGroup::set_resources(const CGroupResources &resources) {
  if (!resources.cpu_max.empty()) {
    write_file("cpu.max", resources.cpu_max);
  }
  if (!resources.memory_max.empty()) {
    write_file("memory.max", resources.memory_max);
  }
}

void CGroup::add_process(pid_t pid) {
  write_file("cgroup.procs", std::to_string(pid));
}

void CGroup::remove() {
  constexpr int MAX_RETRIES = 5;
  constexpr int RETRY_DELAY_MS = 20;

  for (int i = 0; i < MAX_RETRIES; ++i) {
    if (rmdir(path_.c_str()) == 0) {
      return;
    }
    if (errno == ENOENT) {
      return;
    }
    if (errno == EBUSY) {
      if (i < MAX_RETRIES - 1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_DELAY_MS));
        continue;
      }
      std::cerr << "CGroup::remove: EBUSY while removing cgroup " << path_ << " (processes may still be attached)"<<std::endl;
      throw std::runtime_error("Failed to remove cgroup (EBUSY): " + path_);
    }
    throw std::runtime_error("Failed to remove cgroup " + path_ + ": " + std::strerror(errno));
  }
}

void CGroup::write_file(const std::string &filename, const std::string &value) {
  std::string file_path = path_ + "/" + filename;
  std::ofstream file(file_path);

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open cgroup file " + file_path);
  }
  if (!(file << value << "\n")) {
    throw std::runtime_error("Failed to write in cgroup file " + file_path);
  }
}
} // namespace cppdock::runtime