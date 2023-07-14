#include "docker.hpp"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

#include <sstream>
#include <vector>

namespace docker {

std::string const kDefaultHost = "http:/v1.41";
uint16_t constexpr kHttpGetSuccess = 200;
uint16_t constexpr kHttpDeleteSuccess = 204;

uint16_t constexpr kPutSuccess = 200;
uint16_t constexpr kCreateContainerSuccess = 201;
uint16_t constexpr kExecStartSuccess = 200;
uint16_t constexpr kExecCreateSuccess = 201;
uint16_t constexpr kStartContainerSuccess = 204;
uint16_t constexpr kKillContainerSuccess = 204;

Docker::Docker()
    : m_http() {}

Docker::Docker(std::string host)
    : m_http(host) {}

Response Docker::system_info() {
    std::string const endpoint = "/info";
    auto r = m_http.get(endpoint);
    return {r.httpCode, r.data};
}

returns::Version Docker::dockerVersion() {
    std::string const endpoint = "/version";
    auto r = m_http.get(endpoint);
    return {r.httpCode == kHttpGetSuccess, r.data};
}

/*
 * Images
 */
returns::Images Docker::list_images() {
    std::string const endpoint = "/images/json";
    auto r = m_http.get(endpoint);

    rapidjson::Document document;
    document.Parse(r.data);
    if (document.HasParseError()) {
        return {};
    }

    if (!document.IsArray()) {
        return {};
    }

    std::vector<std::string> images;
    for (auto const & element : document.GetArray()) {
        std::string_view constexpr kTags = "RepoTags";
        if (!element.HasMember(kTags.data())) {
            return {};
        }

        if (!element[kTags.data()].IsArray()) {
            return {};
        }
        for (auto const & tag : element[kTags.data()].GetArray()) {
            images.push_back(tag.GetString());
        }
    }

    return {r.httpCode == kHttpGetSuccess, std::move(images)};
}

/*
 * Containers
 */

returns::CreateContainer Docker::create_container(request_params::CreateContainer const & params) {
    std::string endpoint = "/containers/create";
    std::string body = m_requestCreator.createContainer(params);
    auto r = m_http.post(endpoint, curl::Body{curl::DataType::Json, body});
    std::string id = m_responseProcessor.createContainer(r.data);

    return {r.httpCode == kCreateContainerSuccess, id};
}

returns::StartContainer Docker::start_container(std::string const & id) {
    std::string endpoint = "/containers/" + id + "/start";
    auto r = m_http.post(endpoint);
    if (r.httpCode == kStartContainerSuccess) {
        return {true, {}};
    }

    std::string message = m_responseProcessor.startContainer(r.data);
    return {false, message};
}

returns::KillContainer Docker::kill_container(request_params::KillContainer const & params) {
    std::string query = "/containers/" + params.containerId + "/kill?";
    query += param("signal", params.signal);

    auto r = m_http.post(query);
    if (r.httpCode == kKillContainerSuccess) {
        return {true, {}};
    }

    std::string message = m_responseProcessor.killContainer(r.data);
    return {false, message};
}

returns::DeleteContainer Docker::delete_container(request_params::RemoveContainer const & params) {
    std::string path = "/containers/" + params.containerId + "?";
    path += param("v", params.volume);
    path += param("force", params.force);

    auto r = m_http.del(path);
    return {r.httpCode == kHttpDeleteSuccess, r.data};
}

returns::RunContainer Docker::run_container(request_params::RunContainer const & params) {
    auto result = create_container(params);
    if (!result.success) {
        // log
        return {};
    }

    auto r = start_container(result.containerId);
    if (!r.success) {
        // log r.message
        return {};
    }

    return {true, result.containerId};
}

returns::PutArchive Docker::put_archive(request_params::PutArchive const & params) {
    std::string path = "/containers/" + params.containerId + "/archive?path=" + params.path;
    auto r = m_http.put(path, {curl::DataType::Tar, params.archive});
    return {r.httpCode == kPutSuccess, r.data};
}

returns::ExecCreate Docker::execCreate(request_params::ExecCreate const & params) {
    std::string path = "/containers/";
    path += params.containerId + "/exec";
    auto body = m_requestCreator.execCreate(params);
    auto r = m_http.post(path, curl::Body{curl::DataType::Json, std::move(body)});
    auto id = m_responseProcessor.execCreate(r.data);
    return {r.httpCode == kExecCreateSuccess, id};
}

returns::ExecStart Docker::execStart(request_params::ExecStart const & params) {
    std::string path = "/exec/" + params.execId + "/start";
    auto body = m_requestCreator.execStart(params);
    auto r = m_http.post(path, curl::Body{curl::DataType::Json, std::move(body)});
    return {r.httpCode == kExecStartSuccess, r.data};
}

returns::Exec Docker::exec(request_params::ExecCreate const & params) {
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

std::string param(const std::string & param_name, const std::string & param_value) {
    if (!param_value.empty()) {
        return "&" + param_name + "=" + param_value;
    }
    return {""};
}

std::string param(const std::string & param_name, const char * param_value) {
    if (param_value != nullptr) {
        return "&" + param_name + "=" + param_value;
    }
    return {""};
}

std::string param(const std::string & param_name, bool param_value) {
    std::string ret;
    ret = "&" + param_name + "=";
    if (param_value) {
        return ret + "true";
    }
    return {ret + "false"};
}

std::string param(const std::string & param_name, int param_value) {
    if (param_value == -1) {
        return "";
    }

    std::ostringstream convert;
    convert << param_value;
    return "&" + param_name + "=" + convert.str();
}

}  // namespace docker
