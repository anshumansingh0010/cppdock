#include <cerrno>
#include <iostream>
#include <runtime/RootFS.hpp>
#include <sys/sysmacros.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <system_error>
#include <unistd.h>
#include <vector>

namespace cppdock::runtime {

RootFS::RootFS(const std::string &root_path) : root_path_(root_path) {}

void RootFS::prepare() {
  if (mount(root_path_.c_str(), root_path_.c_str(), NULL, MS_BIND | MS_REC,
            NULL) == -1) {
    throw std::system_error(errno, std::generic_category(), "Bind mount rootfs failed");
  }

  std::string old_root = root_path_ + "/.old_root";
  mkdir(old_root.c_str(), 0700);

  if (syscall(SYS_pivot_root, root_path_.c_str(), old_root.c_str()) == -1) {
    throw std::system_error(errno, std::generic_category(), "pivot_root failed");
  }

  if (chdir("/") == -1) {
    throw std::system_error(errno, std::generic_category(), "chdir to new root failed");
  }

  if (umount2("/.old_root", MNT_DETACH) == -1) {
    throw std::system_error(errno, std::generic_category(), "umount old_root failed");
  }

  rmdir("/.old_root");
}

void RootFS::mount_essentials() {
  auto mount_req = [](const char* source, const char* target, const char* type, unsigned long flags, const void* data) {
    if (mount(source, target, type, flags, data) == -1) {
      throw std::system_error(errno, std::generic_category(),std::string("Failed to mount ") + target);
    }
  };

  try {
    mount_req("proc", "/proc", "proc", 0, NULL);
    mount_req("sysfs", "/sys", "sysfs", 0, NULL);
    mount_req("tmpfs", "/dev", "tmpfs", MS_NOSUID | MS_STRICTATIME, "mode=755");

    struct DevNode {
      const char* path;
      int major;
      int minor;
      mode_t mode;
    };
    const std::vector<DevNode> devices={
      {"/dev/null",1,3,0666},
      {"/dev/zero",1,5,0666},
      {"/dev/random",1,8,0666},
      {"/dev/urandom",1,9,0666},
      {"/dev/tty",5,0,0666},
    };

    for(const auto& dev:devices){
      if(mknod(dev.path,S_IFCHR|dev.mode,makedev(dev.major,dev.minor))==-1){
        if(errno!=EEXIST){
          std::cerr<<"Warning: failed to mknod "<<dev.path<<" errno ( "<<errno<<" )\n";
        }
      }
    }

    symlink("/proc/self/fd", "/dev/fd");
    symlink("/proc/self/fd/0", "/dev/stdin");
    symlink("/proc/self/fd/1", "/dev/stdout");
    symlink("/proc/self/fd/2", "/dev/stderr");
    
  } catch (const std::exception& e) {
    std::cerr << "Critical Error during RootFS setup: " << e.what() << std::endl;
    throw; 
  }
}

} // namespace cppdock::runtime
