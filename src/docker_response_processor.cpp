#include "docker_response_processor.hpp"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

namespace docker::json {

class ResponseProcessor::Impl {
public:
    std::string createContainer(std::string const & json) const;
    std::string killContainer(std::string const & json) const;
    std::string startContainer(std::string const & json) const;
    std::string execCreate(std::string const & json) const;
};

std::string ResponseProcessor::Impl::createContainer(std::string const & json) const {
    rapidjson::Document document;
    document.Parse(json);
    if (document.HasParseError()) {
        return {};
    }

    if (!document.IsObject()) {
        return {};
    }

    std::string_view constexpr kId = "Id";
    if (!document.GetObject().HasMember(kId.data())) {
        return {};
    }

    if (!document.GetObject()[kId.data()].IsString()) {
        return {};
    }

    auto id = document.GetObject()[kId.data()].GetString();
    return id;
}

std::string ResponseProcessor::Impl::killContainer(std::string const & json) const {
    rapidjson::Document document;
    document.Parse(json);

    if (document.HasParseError()) {
        return {};
    }

    if (!document.IsObject()) {
        return {};
    }

    std::string_view constexpr kMessage = "message";
    if (!document.GetObject().HasMember(kMessage.data())) {
        return {};
    }

    if (!document.GetObject()[kMessage.data()].IsString()) {
        return {};
    }

    auto message = document.GetObject()[kMessage.data()].GetString();
    return message;
}

std::string ResponseProcessor::Impl::startContainer(std::string const & json) const {
    return killContainer(json);
}

std::string ResponseProcessor::Impl::execCreate(std::string const & json) const {
    return createContainer(json);
}

// ResponseProcessor

ResponseProcessor::ResponseProcessor()
    : m_impl(std::make_unique<ResponseProcessor::Impl>()) {}

ResponseProcessor::~ResponseProcessor() {}

std::string ResponseProcessor::createContainer(std::string const & json) const {
    return m_impl->createContainer(json);
}

std::string ResponseProcessor::killContainer(std::string const & json) const {
    return m_impl->killContainer(json);
}

std::string ResponseProcessor::startContainer(std::string const & json) const {
    return m_impl->startContainer(json);
}

std::string ResponseProcessor::execCreate(std::string const & json) const {
    return m_impl->execCreate(json);
}

}  // namespace docker::json
