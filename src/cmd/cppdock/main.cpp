#include <ContainerConfig.hpp>
#include <format>
#include <iostream>
#include <runtime/Container.hpp>

using namespace cppdock::runtime;

int main() {
  try {
    const ContainerConfigs config=ContainerConfigsBuilder()
          .add_flags(CLONE_NEWNET | CLONE_NEWIPC | CLONE_NEWCGROUP)
          .with_id("new_id")
          .with_rootfs(std::format("{}/rootfs", PROJECT_ROOT_DIR))
          .with_command("sh")
          // .with_args({"mkdir", "-p", "test"})
          .with_resources({"50000 100000", "524288000"})
          .build();
    std::cout << std::format("{}/rootfs", PROJECT_ROOT_DIR) << std::endl;
    Container new_con = Container(config);
    new_con.start();
    new_con.wait();
  } catch (const std::exception &e) {
    std::cerr << "Fatal container error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}