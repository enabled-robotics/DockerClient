#include "docker_request_creator.hpp"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

namespace docker::json {

std::string_view constexpr kImage = "Image";
std::string_view constexpr kTty = "Tty";
std::string_view constexpr kHostConfig = "HostConfig";
std::string_view constexpr kMemory = "Memory";

// ExecCreate
std::string_view constexpr kAttachStderr = "AttachStderr";
std::string_view constexpr kAttachStdout = "AttachStdout";
std::string_view constexpr kDetach = "Detach";
std::string_view constexpr kCmd = "Cmd";

class RequestCreator::Impl {
public:
    Impl();

    std::string createContainer(request_params::CreateContainer const & params);
    std::string execCreate(request_params::ExecCreate const & params);
    std::string execStart(request_params::ExecStart const & params);

private:
    rapidjson::StringBuffer m_stringBuffer;
    rapidjson::Writer<rapidjson::StringBuffer> m_writer;
};

RequestCreator::Impl::Impl()
    : m_writer(m_stringBuffer) {}

std::string RequestCreator::Impl::createContainer(request_params::CreateContainer const & params) {
    m_writer.StartObject();

    m_writer.Key(kImage.data());
    m_writer.String(params.image);

    m_writer.Key(kTty.data());
    m_writer.Bool(params.tty);

    m_writer.Key(kHostConfig.data());
    m_writer.StartObject();
    m_writer.Key(kMemory.data());
    m_writer.Uint64(params.memory);
    m_writer.EndObject();

    m_writer.EndObject();

    auto request = m_stringBuffer.GetString();

    m_stringBuffer.Clear();
    m_writer.Reset(m_stringBuffer);
    return request;
}

std::string RequestCreator::Impl::execCreate(request_params::ExecCreate const & params) {
    m_writer.StartObject();

    m_writer.Key(kAttachStderr.data());
    m_writer.Bool(params.attachStderr);

    m_writer.Key(kAttachStdout.data());
    m_writer.Bool(params.attachStdout);

    m_writer.Key(kDetach.data());
    m_writer.Bool(params.detach);

    m_writer.Key(kTty.data());
    m_writer.Bool(params.tty);

    m_writer.Key(kCmd.data());
    m_writer.StartArray();
    for (size_t index = 0; index < params.cmd.size(); ++index) {
        m_writer.String(std::move(params.cmd[index]));
    }
    m_writer.EndArray();

    m_writer.EndObject();

    auto request = m_stringBuffer.GetString();

    m_stringBuffer.Clear();
    m_writer.Reset(m_stringBuffer);
    return request;
}

std::string RequestCreator::Impl::execStart(request_params::ExecStart const & params) {
    m_writer.StartObject();

    m_writer.Key(kTty.data());
    m_writer.Bool(params.tty);

    m_writer.Key(kDetach.data());
    m_writer.Bool(params.detach);

    m_writer.EndObject();

    auto request = m_stringBuffer.GetString();

    m_stringBuffer.Clear();
    m_writer.Reset(m_stringBuffer);
    return request;
}

// RequestCreator

RequestCreator::RequestCreator()
    : m_impl(std::make_unique<RequestCreator::Impl>()) {}

RequestCreator::~RequestCreator() {}

std::string RequestCreator::createContainer(request_params::CreateContainer const & params) {
    return m_impl->createContainer(params);
}

std::string RequestCreator::execCreate(request_params::ExecCreate const & params) {
    return m_impl->execCreate(params);
}

std::string RequestCreator::execStart(request_params::ExecStart const & params) {
    return m_impl->execStart(params);
}

}  // namespace docker::json
