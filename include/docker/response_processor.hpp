#pragma once

#include "returns.hpp"

#include <memory>
#include <string>

namespace docker::json {
class ResponseProcessor {
public:
    ResponseProcessor();
    ~ResponseProcessor();

    std::string createContainer(std::string const & json) const;
    std::string killContainer(std::string const & json) const;
    std::string startContainer(std::string const & json) const;
    std::string execCreate(std::string const & json) const;
    std::vector<returns::Container> listContainers(std::string const & json) const;

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace docker::json
