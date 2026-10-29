#ifndef UTILS_HPP
#define UTILS_HPP

#include <sstream>
#include <string>

namespace cppdock::common{
template <typename T> std::string toString(const T &val) {
    if constexpr (std::is_convertible_v<T, std::string>) {
        return std::string(val);
    } else if constexpr (std::is_arithmetic_v<T>) {
        return std::to_string(val);
    } else {
        std::ostringstream ss;
        ss << val;
        return ss.str();
    }
}
template <typename... Args> std::string formatMsg(Args &&...args) {
    std::string result;
    (result.append(toString(std::forward<Args>(args))), ...);
    return result;
}
}

#endif