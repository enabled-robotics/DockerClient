#include "request_params.hpp"

#include <string>


namespace docker::query {

std::string dockerVersion();

std::string listContainers(request_params::ListContainers const & params);

std::string createContainer();

std::string startContainer(std::string const & id);

std::string killContainer(request_params::KillContainer const & params);

std::string deleteContainer(request_params::RemoveContainer const & params);

std::string putArchive(request_params::PutArchive const & params);

std::string execCreate(request_params::ExecCreate const & params);

std::string execStart(request_params::ExecStart const & params);

}  // namespace query
