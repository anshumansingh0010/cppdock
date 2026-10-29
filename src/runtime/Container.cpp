#include <ContainerConfig.hpp>
#include <cerrno>
#include <runtime/Container.hpp>
#include <runtime/Namespace.hpp>
#include <runtime/Process.hpp>
#include <signal.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <common/Logger.hpp>

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 64
#endif

namespace cppdock::runtime {

Container::Container(const ContainerConfigs &config)
    : configs_(config), cgroup_(config.container_id),
      rootfs_(config.rootfs_path) {}

Container::~Container() {
  try {
    stop();
  } catch (const std::exception &e) {
    LOG_ERROR("Error in ~Container: " , e.what());
  } catch (...) {
    LOG_ERROR( "Unknown error in ~Container");
  }
}

void Container::start() {
  if (process_ && process_->pid() > 0) {
    throw std::runtime_error("Container is already running");
  }
  LOG_INFO("Starting container " , configs_.container_id);

  cgroup_.create();
  try {
    cgroup_.set_resources(configs_.resources);

    auto payload_fun = [this]() { return this->payload(); };

    Namespace ns(configs_.flags);

    process_ = std::make_unique<Process>(payload_fun);
    process_->start(ns);

    cgroup_.add_process(process_->pid());
    process_->resume();
  } catch (...) {
    if (process_ && process_->pid() > 0) {
      kill(process_->pid(), SIGKILL);
      try {
        process_->wait();
      } catch (const std::exception &e) {
        LOG_ERROR("Failed to wait for killed process: " , e.what());
      }
    }
    try {
      cgroup_.remove();
    } catch (const std::exception &e) {
      LOG_CRITICAL("Failed to remove cgroup: " , e.what());
    }
    throw;
  }
}

void Container::stop() {
  if (process_ && process_->pid() > 0) {
    LOG_WARN("Stopping container " , configs_.container_id);
    kill(process_->pid(), SIGKILL);
    process_->wait();
  }
  cgroup_.remove();
}

void Container::pause() {
  if (process_ && process_->pid() > 0) {
    kill(process_->pid(), SIGSTOP);
  }
}

void Container::resume() {
  if (process_ && process_->pid() > 0) {
    kill(process_->pid(), SIGCONT);
  }
}

int Container::wait() {
  if (process_) {
    return process_->wait();
  }
  return -1;
}

int Container::payload() {
  LOG_INFO("Inside isolated namespace, setting up root fs");
  rootfs_.prepare();
  rootfs_.mount_essentials();

  std::string hostname = configs_.container_id.substr(0, HOST_NAME_MAX);
  if (sethostname(hostname.c_str(), hostname.length()) == -1) {
    LOG_WARN("sethostname failed (errno " , errno , ")");
  }

  if (setsid() == -1 && errno != EPERM) {
    LOG_WARN("setsid failed (errno " , errno , ")");
  }

  clearenv();
  setenv("PATH", "/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin",
         1);
  setenv("HOME", "/root", 1);
  setenv("USER", "root", 1);
  setenv("TERM", "xterm", 1);

  std::vector<std::string> args = configs_.args;
  if (args.empty()) {
    args.push_back(configs_.command);
  }

  std::vector<char *> c_args;
  c_args.reserve(args.size() + 1);
  for (auto &arg : args) {
    c_args.push_back(arg.data());
  }
  c_args.push_back(nullptr);

  LOG_INFO("Executing process");
  if (execvp(configs_.command.c_str(), c_args.data()) == -1) {
    LOG_CRITICAL("Failed to execute " , configs_.command , "\nExecvp errno ( " , errno , ")");
    return (errno == ENOENT) ? 127 : 126;
  }
  return 0;
}
} // namespace cppdock::runtime