#pragma once

#include "http.hpp"
#include "request_creator.hpp"
#include "request_params.hpp"
#include "response_processor.hpp"
#include "returns.hpp"

namespace docker {

class Client {
public:
    Client();

    explicit Client(std::string host);

    returns::Version dockerVersion();

    returns::InspectContainer inspectContainer(std::string const & id);

    returns::ListContainers listContainers(request_params::ListContainers const & params);

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
    json::RequestCreator m_jsonCreator;
    json::ResponseProcessor m_jsonParser;
};
}  // namespace docker
