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

struct DeleteContainer : Base {
    std::string message;  // empty, if success
};

struct ExecCreate : Base {
    std::string execId;
};

using KillContainer = DeleteContainer;
using StartContainer = DeleteContainer;
using PutArchive = DeleteContainer;
using ExecStart = DeleteContainer;
using Exec = ExecStart;

using RunContainer = CreateContainer;

}  // namespace docker::returns
