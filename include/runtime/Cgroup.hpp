#ifndef CGROUP_HPP
#define CGROUP_HPP

#include <sys/types.h>
#include<string>

namespace cppdock::runtime {

  struct CGroupResources{
    std::string cpu_max;
    std::string memory_max;
  };

  class CGroup{
  public:  
    static constexpr const char *CGROUP_ROOT="/sys/fs/cgroup/cppdock";
    explicit CGroup(std::string name);
    ~CGroup();

    CGroup(const CGroup&)=delete;
    CGroup& operator=(const CGroup&)=delete;
    
    void create();
    void set_resources(const CGroupResources &resources);
    void remove();
    void add_process(pid_t pid);

  private:
    std::string name_;
    std::string path_;

    void write_file(const std::string &filename, const std::string &value);
  };

}

#endif