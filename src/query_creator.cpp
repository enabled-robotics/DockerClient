#include "docker/query_creator.hpp"

std::string const kVersion = "/version";
std::string const kContainers = "/containers";
std::string const kCreate = "/create";
std::string const kStart = "/start";
std::string const kKill = "/kill";
std::string const kArchive = "/archive";
std::string const kExec = "/exec";
std::string const kJson = "/json";

std::string const kSlash = "/";
std::string const kSignal = "signal";
std::string const kForce = "force";
std::string const kVolume = "v";
std::string const kPath = "path";
std::string const kAll = "all";
std::string const kLimit = "limit";
std::string const kSize = "size";
std::string const kFilters = "filters";

std::string const kQuestionMark = "?";
std::string const kAmpersand = "&";

std::string const kTrue = "true";
std::string const kFalse = "false";

std::string const kContainersCreate = kContainers + kCreate;

namespace {

std::string makeSuffix(std::string const & paramName, bool value) {
    if (value) {
        return paramName + "=" + kTrue;
    } else {
        return paramName + "=" + kFalse;
    }
}

std::string makeSuffix(std::string const & paramName, std::string const & value) {
    return paramName + "=" + value;
}

std::string makeSuffix(std::string const & paramName, int32_t value) {
    return paramName + "=" + std::to_string(value);
}

}  // namespace

namespace docker::query {

std::string dockerVersion() { return kVersion; }

std::string listContainers(request_params::ListContainers const & params) {
    std::string query = kContainers + kJson;
    if (!params.all && params.limit == -1 && !params.size && params.filters.empty()) {
        return query;
    }

    query += kQuestionMark;

    query += makeSuffix(kAll, params.all);
    query += kAmpersand;

    query += makeSuffix(kLimit, params.limit);
    query += kAmpersand;

    query += makeSuffix(kSize, params.size);

    // todo process kFilters
    return query;
}

std::string createContainer() { return kContainersCreate; }

std::string startContainer(std::string const & id) { return kContainers + kSlash + id + kStart; }

std::string killContainer(request_params::KillContainer const & params) {
    std::string query = kContainers + kSlash + params.containerId + kKill;
    if (!params.signal.empty()) {
        query += kQuestionMark;
        query += makeSuffix(kSignal, params.signal);
    }

    return query;
}

std::string deleteContainer(request_params::RemoveContainer const & params) {
    std::string query = kContainers + kSlash + params.containerId;
    if (!params.force && !params.volume) {
        return query;
    }
    query += kQuestionMark;

    query += makeSuffix(kForce, params.force);
    query += kAmpersand;
    query += makeSuffix(kVolume, params.volume);

    return query;
}

std::string putArchive(request_params::PutArchive const & params) {
    std::string query = kContainers + kSlash + params.containerId + kArchive;
    query += kQuestionMark;
    query += makeSuffix(kPath, params.path);
    return query;
}

std::string execCreate(request_params::ExecCreate const & params) {
    std::string query = kContainers + kSlash + params.containerId + kExec;
    return query;
}

std::string execStart(request_params::ExecStart const & params) {
    std::string query = kExec + kSlash + params.execId + kStart;
    return query;
}

}  // namespace docker::query
