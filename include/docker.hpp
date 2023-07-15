#pragma once

#include "curl.hpp"
#include "docker_request_creator.hpp"
#include "docker_response_processor.hpp"
#include "request_params.hpp"
#include "returns.hpp"

namespace docker {

// TODO think about param functions, refactor
std::string param(const std::string & param_name, const std::string & param_value);

std::string param(const std::string & param_name, const char * param_value);

std::string param(const std::string & param_name, bool param_value);

std::string param(const std::string & param_name, int param_value);

class Docker {
public:
    Docker();

    explicit Docker(std::string host);

    returns::Version dockerVersion();

    returns::CreateContainer createContainer(request_params::CreateContainer const & params);

    returns::RunContainer runContainer(request_params::RunContainer const & params);

    returns::PutArchive putArchive(request_params::PutArchive const & params);

    returns::ExecCreate execCreate(request_params::ExecCreate const & params);

    returns::ExecStart execStart(request_params::ExecStart const & params);

    returns::Exec exec(request_params::ExecCreate const & params);

    returns::StartContainer startContainer(std::string const & id);

    returns::KillContainer killContainer(request_params::KillContainer const & params);

    returns::DeleteContainer deleteContainer(request_params::RemoveContainer const & params);

private:
    curl::Http m_http;
    json::RequestCreator m_requestCreator;
    json::ResponseProcessor m_responseProcessor;
};
}  // namespace docker
