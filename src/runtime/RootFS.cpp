#include <runtime/RootFS.hpp>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <system_error>
#include <unistd.h>

namespace cppdock::runtime {

RootFS::RootFS(const std::string &root_path) : root_path_(root_path) {}

void RootFS::prepare() {
  if (mount(root_path_.c_str(), root_path_.c_str(), NULL, MS_BIND | MS_REC,
            NULL) == -1) {
    throw std::system_error(errno, std::generic_category(),
                            "Bind mount rootfs failed");
  }

  std::string old_root = root_path_ + "/.old_root";
  mkdir(old_root.c_str(), 0700);

  if (syscall(SYS_pivot_root, root_path_.c_str(), old_root.c_str()) == -1) {
    throw std::system_error(errno, std::generic_category(),
                            "pivot_root failed");
  }

  if (chdir("/") == -1) {
    throw std::system_error(errno, std::generic_category(),
                            "chdir to new root failed");
  }

  if (umount2("/.old_root", MNT_DETACH) == -1) {
    throw std::system_error(errno, std::generic_category(),
                            "umount old_root failed");
  }

  rmdir("/.old_root");
}

void RootFS::mount_essentials() {
  mount("proc", "/proc", "proc", 0, NULL);
  mount("sysfs", "/sys", "sysfs", 0, NULL);
  mount("tmpfs", "/dev", "tmpfs", MS_NOSUID | MS_STRICTATIME, "mode=755");
}

} // namespace cppdock::runtime
