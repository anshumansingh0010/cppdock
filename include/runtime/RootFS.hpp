#ifndef ROOTFS_HPP
#define ROOTFS_HPP

#include <string>

namespace cppdock::runtime {

class RootFS {
public:
  explicit RootFS(const std::string &root_path);

  void prepare();
  void mount_essentials();

private:
  std::string root_path_;
};

} // namespace cppdock::runtime
#endif