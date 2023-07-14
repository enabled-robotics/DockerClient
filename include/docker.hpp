#pragma once

#include "curl.hpp"
#include "docker_request_creator.hpp"
#include "docker_response_processor.hpp"
#include "request_params.hpp"
#include "returns.hpp"

#include <vector>

namespace docker {

struct Response {
    uint16_t httpCode;
    std::string data;
};

// TODO think about param functions, refactor
std::string param(const std::string & param_name, const std::string & param_value);

std::string param(const std::string & param_name, const char * param_value);

std::string param(const std::string & param_name, bool param_value);

std::string param(const std::string & param_name, int param_value);

class Docker {
public:
    Docker();

    explicit Docker(std::string host);

    Response system_info();

    returns::Version dockerVersion();

    returns::Images list_images();

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
};
}  // namespace docker
