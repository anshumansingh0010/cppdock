#include <ContainerConfig.hpp>
#include <common/Logger.hpp>
#include <runtime/Container.hpp>

using namespace cppdock::runtime;

int main() {
    LOG_INIT_DEFAULT();
    try {
        const ContainerConfigs config =
            ContainerConfigsBuilder()
                .add_flags(CLONE_NEWNET | CLONE_NEWIPC | CLONE_NEWCGROUP)
                .with_id("new_id")
                .with_rootfs(PROJECT_ROOT_DIR "/rootfs")
                .with_command("sh")
                // .with_args({"mkdir", "-p", "test"})
                .with_resources({"50000 100000", "524288000"})
                .build();
        LOG_INFO(PROJECT_ROOT_DIR "/rootfs");
        Container new_con = Container(config);
        new_con.start();
        new_con.wait();
    } catch (const std::exception &e) {
        LOG_CRITICAL("Fatal container error: " , e.what());
        return 1;
    }
    return 0;
}