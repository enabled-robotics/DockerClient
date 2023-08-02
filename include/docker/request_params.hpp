#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Params for docker body
namespace docker::request_params {

struct CreateContainer {
    std::string image;
    bool tty{true};
    uint64_t memory{0};  // bytes
};

struct RemoveContainer {
    std::string containerId;
    bool volume{false};
    bool force{false};
};

struct KillContainer {
    std::string containerId;
    std::string signal;  // e.g. "SIGKILL"
};

using RunContainer = CreateContainer;

struct PutArchive {
    std::string containerId;
    std::string path;
    std::string archive;  // in memory tar archive
};

struct ExecCreate {
    std::string containerId;
    std::vector<std::string> cmd;
    bool attachStderr{true};
    bool attachStdout{true};
    bool detach{false};
    bool tty{true};
};

// extra options
struct ExecStart {
    std::string execId;
    bool detach{false};
    bool tty{true};
};

struct ListContainers {
    bool all{false};
    int32_t limit{-1};
    bool size{false};
    std::string filters;  // in json
};

}  // namespace docker::request_params
