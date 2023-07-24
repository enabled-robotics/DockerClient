#pragma once
#include <string>

namespace docker {

struct Container {
    std::string id;
    std::string image;
};

struct InspectContainerInfo {
    bool isRunning{false};
    std::string image;
};

}  // namespace docker