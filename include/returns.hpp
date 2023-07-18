#pragma once

#include <string>
#include <vector>

namespace docker::returns {

struct Base {
    bool success{false};
};

struct Version : Base {
    std::string json;  // maybe exposed if needed
};

struct CreateContainer : Base {
    std::string containerId;
};

struct CommonCase : Base {
    std::string message;
};

struct ExecCreate : Base {
    std::string execId;
};

struct Container {
    std::string id;
    std::string image;
};

struct ListContainers : Base {
    std::vector<Container> containers;
};

using DeleteContainer = CommonCase;
using KillContainer = CommonCase;
using StartContainer = CommonCase;
using PutArchive = CommonCase;
using ExecStart = CommonCase;
using Exec = ExecStart;

using RunContainer = CreateContainer;

}  // namespace docker::returns
