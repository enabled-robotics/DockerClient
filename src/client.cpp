#include "docker/client.hpp"

#include "docker/query_creator.hpp"

#include <vector>

namespace docker {

std::string const kDefaultHost = "http:/v1.41";
uint16_t constexpr kHttpGetSuccess = 200;
uint16_t constexpr kHttpDeleteSuccess = 204;

uint16_t constexpr kPutSuccess = 200;
uint16_t constexpr kListContainersSuccess = 200;
uint16_t constexpr kCreateContainerSuccess = 201;
uint16_t constexpr kExecStartSuccess = 200;
uint16_t constexpr kExecCreateSuccess = 201;
uint16_t constexpr kStartContainerSuccess = 204;
uint16_t constexpr kKillContainerSuccess = 204;

Client::Client()
    : m_http() {}

Client::Client(std::string host)
    : m_http(std::move(host)) {}

answer::Version Client::dockerVersion() {
    auto r = m_http.get(query::dockerVersion());
    return {r.httpCode == kHttpGetSuccess, r.data};
}

answer::InspectContainer Client::inspectContainer(std::string const & id) {
    auto r = m_http.get(query::inspectContainer(id));
    if (r.httpCode != kHttpGetSuccess) {
        return {};
    }

    bool isRunning = m_jsonParser.inspectContainer(r.data);
    return {true, isRunning};
}
answer::ListContainers Client::listContainers(request_params::ListContainers const & params) {
    auto r = m_http.get(query::listContainers(params));
    if (r.httpCode != kListContainersSuccess) {
        return {};
    }

    auto containers = m_jsonParser.listContainers(r.data);
    return {true, containers};
}

answer::CreateContainer Client::createContainer(request_params::CreateContainer const & params) {
    std::string body = m_jsonCreator.createContainer(params);
    auto r = m_http.post(query::createContainer(), curl::Body{curl::DataType::Json, body});
    std::string id = m_jsonParser.createContainer(r.data);

    return {r.httpCode == kCreateContainerSuccess, id};
}

answer::StartContainer Client::startContainer(std::string const & id) {
    auto r = m_http.post(query::startContainer(id));
    if (r.httpCode == kStartContainerSuccess) {
        return {true, {}};
    }

    std::string message = m_jsonParser.startContainer(r.data);
    return {false, message};
}

answer::KillContainer Client::killContainer(request_params::KillContainer const & params) {
    auto r = m_http.post(query::killContainer(params));
    if (r.httpCode == kKillContainerSuccess) {
        return {true, {}};
    }

    std::string message = m_jsonParser.killContainer(r.data);
    return {false, message};
}

answer::DeleteContainer Client::deleteContainer(request_params::RemoveContainer const & params) {
    auto r = m_http.del(query::deleteContainer(params));
    return {r.httpCode == kHttpDeleteSuccess, r.data};
}

answer::RunContainer Client::runContainer(request_params::RunContainer const & params) {
    auto result = createContainer(params);
    if (!result.success) {
        // log
        return {};
    }

    auto r = startContainer(result.containerId);
    if (!r.success) {
        // log r.message
        return {};
    }

    return {true, result.containerId};
}

answer::PutArchive Client::putArchive(request_params::PutArchive const & params) {
    auto r = m_http.put(query::putArchive(params), {curl::DataType::Tar, params.archive});
    return {r.httpCode == kPutSuccess, r.data};
}

answer::ExecCreate Client::execCreate(request_params::ExecCreate const & params) {
    auto body = m_jsonCreator.execCreate(params);
    auto r =
        m_http.post(query::execCreate(params), curl::Body{curl::DataType::Json, std::move(body)});
    auto id = m_jsonParser.execCreate(r.data);
    return {r.httpCode == kExecCreateSuccess, id};
}

answer::ExecStart Client::execStart(request_params::ExecStart const & params) {
    auto body = m_jsonCreator.execStart(params);
    auto r =
        m_http.post(query::execStart(params), curl::Body{curl::DataType::Json, std::move(body)});
    return {r.httpCode == kExecStartSuccess, r.data};
}

answer::Exec Client::exec(request_params::ExecCreate const & params) {
    auto createResult = execCreate(params);
    if (!createResult.success) {
        return {};
    }

    auto createStart = execStart({createResult.execId});
    if (!createStart.success) {
        return {};
    }

    return createStart;
}

}  // namespace docker
