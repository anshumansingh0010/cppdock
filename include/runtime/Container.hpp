#ifndef CONTAINER_HPP
#define CONTAINER_HPP

#include <ContainerConfig.hpp>
#include <memory>
#include <runtime/Cgroup.hpp>
#include <runtime/Process.hpp>
#include <runtime/RootFS.hpp>
#include <sys/types.h>

namespace cppdock::runtime {

class Container {
public:
  explicit Container(const ContainerConfigs &config);
  ~Container();

  // to avoid for copying and moving container
  Container(const Container &) = delete;
  Container &operator=(const Container &) = delete;
  Container(Container &&) = delete;
  Container &operator=(Container &&) = delete;

  void start();
  void stop();
  void resume();
  void pause();
  int wait();

private:
  ContainerConfigs configs_;
  CGroup cgroup_;
  RootFS rootfs_;
  std::unique_ptr<Process> process_;
  int payload();
};

} // namespace cppdock::runtime

#endif