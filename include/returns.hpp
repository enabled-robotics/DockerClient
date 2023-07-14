#include <string>
#include <vector>

namespace docker::returns {

struct Base {
    bool success{false};
};

struct Version : Base {
    std::string json;  // maybe exposed if needed
};

struct Images : Base {
    std::vector<std::string> repoTags;
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

using DeleteContainer = CommonCase;
using KillContainer = CommonCase;
using StartContainer = CommonCase;
using PutArchive = CommonCase;
using ExecStart = CommonCase;
using Exec = ExecStart;

using RunContainer = CreateContainer;

}  // namespace docker::returns
