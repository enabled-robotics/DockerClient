#pragma once

#include "request_params.hpp"

#include <memory>
#include <string>

// Create body for post requests
namespace docker::json {

class RequestCreator {
public:
    RequestCreator();
    ~RequestCreator();

    std::string createContainer(request_params::CreateContainer const & params);
    std::string execCreate(request_params::ExecCreate const & params);
    std::string execStart(request_params::ExecStart const & params);

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace docker::json
