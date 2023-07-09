#pragma once

#include "curl.hpp"
#include "docker_request_creator.hpp"
#include "docker_response_processor.hpp"
#include "request_params.hpp"
#include "returns.hpp"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>

namespace docker {

enum class Method { GET, POST, DELETE, PUT };

struct Response {
    uint16_t httpCode;
    std::string data;
};

using JSON_DOCUMENT = rapidjson::Document;
using JSON_VALUE = rapidjson::Value;

std::string param(const std::string & param_name, const std::string & param_value);

std::string param(const std::string & param_name, const char * param_value);

std::string param(const std::string & param_name, bool param_value);

std::string param(const std::string & param_name, int param_value);

std::string param(const std::string & param_name, JSON_DOCUMENT & param_value);

std::string jsonToString(const JSON_VALUE & doc);

class Docker {
public:
    Docker();

    explicit Docker(std::string host);

    ~Docker();

    /*
     * System
     */
    Response system_info();

    returns::Version dockerVersion();

    /*
     * Images
     */
    returns::Images list_images();

    /*
     * Containers
     */

    returns::CreateContainer create_container(request_params::CreateContainer const & params);

    returns::RunContainer run_container(request_params::RunContainer const & params);

    returns::PutArchive put_archive(request_params::PutArchive const & params);

    returns::ExecCreate execCreate(request_params::ExecCreate const & params);

    returns::ExecStart execStart(request_params::ExecStart const & params);

    returns::Exec exec(request_params::ExecCreate const & params);

    returns::StartContainer start_container(std::string const & id);

    returns::KillContainer kill_container(request_params::KillContainer const & params);

    returns::DeleteContainer delete_container(request_params::RemoveContainer const & params);

private:
    curl::Http m_http;
    json::RequestCreator m_requestCreator;
    json::ResponseProcessor m_responseProcessor;

    std::string host_uri;
    bool is_remote;
    CURL * curl{nullptr};
    CURLcode res{};

    static JSON_DOCUMENT emptyDoc;

    JSON_DOCUMENT requestAndParse(Method method, const std::string & path,
                                  unsigned success_code = 200,
                                  const JSON_DOCUMENT & param = emptyDoc,
                                  bool isReturnJson = false);

    JSON_DOCUMENT requestAndParseJson(Method method, const std::string & path,
                                      unsigned success_code = 200,
                                      const JSON_DOCUMENT & param = emptyDoc);

    JSON_DOCUMENT execCreate(const JSON_DOCUMENT & parameters, const std::string & container_id);

    JSON_DOCUMENT execStart(const JSON_DOCUMENT & parameters, const std::string & exec_id);

    static size_t WriteCallback(void * contents, size_t size, size_t nmemb, void * userp) {
        ((std::string *)userp)->append((char *)contents, size * nmemb);
        return size * nmemb;
    }
};
}  // namespace docker
