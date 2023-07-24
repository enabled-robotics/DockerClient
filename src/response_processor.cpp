#include "docker/response_processor.hpp"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

namespace docker::json {

std::string const kConfig = "Config";
std::string const kId = "Id";
std::string const kImage = "Image";
std::string const kState = "State";
std::string const kRunning = "Running";

class ResponseProcessor::Impl {
public:
    std::string createContainer(std::string const & json) const;
    std::string killContainer(std::string const & json) const;
    std::string startContainer(std::string const & json) const;
    std::string execCreate(std::string const & json) const;
    std::vector<Container> listContainers(std::string const & json) const;
    InspectContainerInfo inspectContainer(std::string const & json) const;
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

std::vector<Container> ResponseProcessor::Impl::listContainers(std::string const & json) const {
    rapidjson::Document document;
    document.Parse(json);

    if (document.HasParseError()) {
        return {};
    }

    if (!document.IsArray()) {
        return {};
    }

    std::vector<Container> containers;
    for (auto const & object : document.GetArray()) {
        if (!object.HasMember(kId) || !object.HasMember(kImage)) {
            return {};
        }

        containers.push_back({object[kId].GetString(), object[kImage].GetString()});
    }

    return containers;
}

InspectContainerInfo ResponseProcessor::Impl::inspectContainer(std::string const & json) const {
    rapidjson::Document document;
    document.Parse(json);

    if (document.HasParseError()) {
        return {};
    }

    if (!document.IsObject()) {
        return {};
    }

    auto const & object = document.GetObject();

    if (!object.HasMember(kConfig)) {
        return {};
    }

    if (!object[kConfig].IsObject()) {
        return {};
    }

    auto const & config = object[kConfig].GetObject();
    if (!config.HasMember(kImage)) {
        return {};
    }

    if (!config[kImage].IsString()) {
        return {};
    }

    if (!object.HasMember(kImage)) {
        return {};
    }

    if (!object.HasMember(kState)) {
        return {};
    }

    auto const & state = object[kState];
    if (!state.IsObject()) {
        return {};
    }

    if (!state.HasMember(kRunning)) {
        return {};
    }

    auto const & running = state[kRunning];
    if (!running.IsBool()) {
        return {};
    }

    return {running.GetBool(), config[kImage].GetString()};
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

std::vector<Container> ResponseProcessor::listContainers(std::string const & json) const {
    return m_impl->listContainers(json);
}

InspectContainerInfo ResponseProcessor::inspectContainer(std::string const & json) const {
    return m_impl->inspectContainer(json);
}
}  // namespace docker::json
