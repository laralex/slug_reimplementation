#include "util.hpp"

auto readTextFile(std::string_view filepath) -> std::optional<std::string>  {
    std::ifstream textfile;
    textfile.open(filepath.data());
    if (textfile.is_open() == false) {
        return std::nullopt;
    }
    std::stringstream ss;
    ss << textfile.rdbuf();
    textfile.close();
    return ss.str();
}