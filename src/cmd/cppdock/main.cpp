#include <ContainerConfig.hpp>
#include <format>
#include <iostream>
#include <runtime/Container.hpp>
using namespace cppdock::runtime;

int main() {
  try {
    const ContainerConfigs config{CLONE_NEWPID | CLONE_NEWUTS | CLONE_NEWNS |
                                      CLONE_NEWNET | CLONE_NEWIPC |
                                      CLONE_NEWCGROUP,
                                  "jay",
                                  std::format("{}/test", PROJECT_ROOT_DIR),
                                  "mkdir",
                                  {"mkdir", "-p", "test"},
                                  {"50000 100000", "500M"}};
    std::cout << std::format("{}/test", PROJECT_ROOT_DIR) << std::endl;
    Container my_first = Container(config);
    my_first.start();
    my_first.wait();
  } catch (const std::exception &e) {
    std::cerr << "Fatal container error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}