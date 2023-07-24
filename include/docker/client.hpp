#pragma once

#include "answer.hpp"
#include "http.hpp"
#include "request_creator.hpp"
#include "request_params.hpp"
#include "response_processor.hpp"

namespace docker {

class Client {
public:
    Client();

    explicit Client(std::string host);

    answer::Version dockerVersion();

    answer::InspectContainer inspectContainer(std::string const & id);

    answer::ListContainers listContainers(request_params::ListContainers const & params);

    answer::CreateContainer createContainer(request_params::CreateContainer const & params);

    answer::RunContainer runContainer(request_params::RunContainer const & params);

    answer::PutArchive putArchive(request_params::PutArchive const & params);

    answer::ExecCreate execCreate(request_params::ExecCreate const & params);

    answer::ExecStart execStart(request_params::ExecStart const & params);

    answer::Exec exec(request_params::ExecCreate const & params);

    answer::StartContainer startContainer(std::string const & id);

    answer::KillContainer killContainer(request_params::KillContainer const & params);

    answer::DeleteContainer deleteContainer(request_params::RemoveContainer const & params);

private:
    curl::Http m_http;
    json::RequestCreator m_jsonCreator;
    json::ResponseProcessor m_jsonParser;
};
}  // namespace docker
