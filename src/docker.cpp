#include "docker.hpp"

#include <fstream>
#include <sstream>
#include <utility>
#include <vector>

namespace docker {

std::string const kDefaultHost = "http:/v1.41";
uint16_t constexpr kHttpGetSuccess = 200;
uint16_t constexpr kHttpDeleteSuccess = 204;

uint16_t constexpr kPutSuccess = 200;
uint16_t constexpr kCreateContainerSuccess = 201;
uint16_t constexpr kExecStartSuccess = 200;
uint16_t constexpr kExecCreateSuccess = 201;
uint16_t constexpr kStartContainerSuccess = 204;
uint16_t constexpr kKillContainerSuccess = 204;

JSON_DOCUMENT Docker::emptyDoc = JSON_DOCUMENT();

Docker::Docker()
    : m_http()
    , host_uri(kDefaultHost)
    , is_remote(false) {}

Docker::Docker(std::string host)
    : m_http(host)
    , host_uri(std::move(host)) {
    if (host_uri == kDefaultHost) {
        is_remote = false;
    } else {
        is_remote = true;
    }
}

Docker::~Docker() { curl_global_cleanup(); }

/*
 *
 * Public Methods
 *
 */

/*
 * System
 */
Response Docker::system_info() {
    std::string const endpoint = "/info";
    auto r = m_http.get(endpoint);
    return {r.httpCode, r.data};
}

returns::Version Docker::dockerVersion() {
    std::string const endpoint = "/version";
    auto r = m_http.get(endpoint);
    return {r.httpCode == kHttpGetSuccess, r.data};
}

/*
 * Images
 */
returns::Images Docker::list_images() {
    std::string const endpoint = "/images/json";
    auto r = m_http.get(endpoint);

    rapidjson::Document document;
    document.Parse(r.data);
    if (document.HasParseError()) {
        return {};
    }

    if (!document.IsArray()) {
        return {};
    }

    std::vector<std::string> images;
    for (auto const & element : document.GetArray()) {
        std::string_view constexpr kTags = "RepoTags";
        if (!element.HasMember(kTags.data())) {
            return {};
        }

        if (!element[kTags.data()].IsArray()) {
            return {};
        }
        for (auto const & tag : element[kTags.data()].GetArray()) {
            images.push_back(tag.GetString());
        }
    }

    return {r.httpCode == kHttpGetSuccess, std::move(images)};
}

/*
 * Containers
 */

returns::CreateContainer Docker::create_container(request_params::CreateContainer const & params) {
    std::string endpoint = "/containers/create";
    std::string body = m_requestCreator.createContainer(params);
    auto r = m_http.post(endpoint, curl::Body{curl::DataType::Json, body});
    std::string id = m_responseProcessor.createContainer(r.data);

    return {r.httpCode == kCreateContainerSuccess, id};
}

returns::StartContainer Docker::start_container(std::string const & id) {
    std::string endpoint = "/containers/" + id + "/start";
    auto r = m_http.post(endpoint);
    if (r.httpCode == kStartContainerSuccess) {
        return {true, {}};
    }

    std::string message = m_responseProcessor.startContainer(r.data);
    return {false, message};
}

returns::KillContainer Docker::kill_container(request_params::KillContainer const & params) {
    std::string query = "/containers/" + params.containerId + "/kill?";
    query += param("signal", params.signal);

    auto r = m_http.post(query);
    if (r.httpCode == kKillContainerSuccess) {
        return {true, {}};
    }

    std::string message = m_responseProcessor.killContainer(r.data);
    return {false, message};
}

returns::DeleteContainer Docker::delete_container(request_params::RemoveContainer const & params) {
    std::string path = "/containers/" + params.containerId + "?";
    path += param("v", params.volume);
    path += param("force", params.force);

    auto r = m_http.del(path);
    return {r.httpCode == kHttpDeleteSuccess, r.data};
}

// void Docker::copy_from_container(const std::string& container_id, const
// std::string& file_path, const std::string& dest_tar_file){}

/*
 *
 * Private Methods
 *
 */

JSON_DOCUMENT Docker::requestAndParse(Method method, const std::string & path,
                                      unsigned success_code, const JSON_DOCUMENT & param,
                                      bool isReturnJson) {
    std::string method_str;
    curl_slist * headers = nullptr;

    switch (method) {
    case Method::GET: method_str = "GET"; break;
    case Method::POST: method_str = "POST"; break;
    case Method::DELETE: method_str = "DELETE"; break;
    case Method::PUT: method_str = "PUT"; break;
    default: method_str = "GET";
    }

    curl = curl_easy_init();
    if (!curl) {
        std::cout << "error while initiating curl" << std::endl;
        curl_global_cleanup();
        exit(1);
    }
    rapidjson::StringBuffer buffer;
    buffer.Clear();
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    param.Accept(writer);
    const std::string paramString = std::string(buffer.GetString());

    if (isReturnJson) {
        headers = curl_slist_append(headers, "Accept: application/json");
    }
    headers = curl_slist_append(headers, "Content-Type: application/json");

    std::string str;
    if (!is_remote)
        curl_easy_setopt(curl, CURLOPT_UNIX_SOCKET_PATH, "/var/run/docker.sock");

    std::string const full_url = host_uri + path;
    curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method_str.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &str);

    // Timeout in seconds for communication with docker.
    // 10 seconds is limit for user code & tests execution,
    // 2 seconds are for HTTP overhead:
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10 + 2);

    if (method == Method::POST) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, paramString.c_str());
    }

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        JSON_DOCUMENT doc(rapidjson::kObjectType);
        return doc;
    }
    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_easy_cleanup(curl);

    JSON_DOCUMENT doc(rapidjson::kObjectType);
    if (status == success_code || status == 200) {
        doc.AddMember("success", true, doc.GetAllocator());
        JSON_VALUE dataString;

        dataString.SetString(str, doc.GetAllocator());
        doc.AddMember("data", dataString, doc.GetAllocator());
    } else {
        JSON_DOCUMENT resp(&doc.GetAllocator());
        resp.Parse(str);

        doc.AddMember("success", false, doc.GetAllocator());
        doc.AddMember("code", static_cast<int64_t>(status), doc.GetAllocator());
        doc.AddMember("data", resp, doc.GetAllocator());
    }
    headers = curl_slist_append(nullptr, "Expect:");
    curl_slist_free_all(headers);
    return doc;
}

JSON_DOCUMENT Docker::requestAndParseJson(Method method, const std::string & path,
                                          unsigned success_code, const JSON_DOCUMENT & param) {
    auto result_obj = requestAndParse(method, path, success_code, param, true);
    bool result = (result_obj.HasMember("success") && result_obj["success"].IsBool()
                   && result_obj["success"].GetBool());
    if (!result) {
        return result_obj;
    }

    JSON_DOCUMENT doc(rapidjson::kObjectType);
    JSON_DOCUMENT data(&doc.GetAllocator());
    data.Parse(result_obj["data"].GetString());

    doc.AddMember("success", true, doc.GetAllocator());
    doc.AddMember("data", data, doc.GetAllocator());
    return doc;
}

returns::RunContainer Docker::run_container(request_params::RunContainer const & params) {
    // TODO refactor
    auto result = create_container(params);
    if (!result.success) {
        // log
        return {};
    }

    auto r = start_container(result.containerId);
    if (!r.success) {
        // log r.message
        return {};
    }

    return {true, result.containerId};
}

returns::PutArchive Docker::put_archive(request_params::PutArchive const & params) {
    std::string path = "/containers/" + params.containerId + "/archive?path=" + params.path;
    auto r = m_http.put(path, {curl::DataType::Tar, params.archive});
    return {r.httpCode == kPutSuccess, r.data};
}

returns::ExecCreate Docker::execCreate(request_params::ExecCreate const & params) {
    std::string path = "/containers/";
    path += params.containerId + "/exec";
    auto body = m_requestCreator.execCreate(params);
    auto r = m_http.post(path, curl::Body{curl::DataType::Json, std::move(body)});
    auto id = m_responseProcessor.execCreate(r.data);
    return {r.httpCode == kExecCreateSuccess, id};
}

returns::ExecStart Docker::execStart(request_params::ExecStart const & params) {
    std::string path = "/exec/" + params.execId + "/start";
    auto body = m_requestCreator.execStart(params);
    auto r = m_http.post(path, curl::Body{curl::DataType::Json, std::move(body)});
    return {r.httpCode == kExecStartSuccess, r.data};
}

returns::Exec Docker::exec(request_params::ExecCreate const & params) {
    auto createResult = execCreate(params);
    if (!createResult.success) {
        return {};
    }

    auto createStart = execStart({createResult.execId});
    if (!createStart.success) {
        return {};
    }

    return createStart;
}

JSON_DOCUMENT Docker::execCreate(const JSON_DOCUMENT & parameters,
                                 const std::string & container_id) {
    std::string path = "/containers/";
    path += container_id + "/exec";
    return requestAndParseJson(Method::POST, path, 201, parameters);
}

JSON_DOCUMENT Docker::execStart(const JSON_DOCUMENT & parameters, const std::string & exec_id) {
    std::string path = "/exec/" + exec_id + "/start";
    return requestAndParse(Method::POST, path, 200, parameters);
}

/*
 *
 * END Docker Implementation
 *
 */

std::string param(const std::string & param_name, const std::string & param_value) {
    if (!param_value.empty()) {
        return "&" + param_name + "=" + param_value;
    }
    return {""};
}

std::string param(const std::string & param_name, const char * param_value) {
    if (param_value != nullptr) {
        return "&" + param_name + "=" + param_value;
    }
    return {""};
}

std::string param(const std::string & param_name, bool param_value) {
    std::string ret;
    ret = "&" + param_name + "=";
    if (param_value) {
        return ret + "true";
    }
    return {ret + "false"};
}

std::string param(const std::string & param_name, int param_value) {
    if (param_value == -1) {
        return "";
    }

    std::ostringstream convert;
    convert << param_value;
    return "&" + param_name + "=" + convert.str();
}

std::string param(const std::string & param_name, JSON_DOCUMENT & param_value) {
    if (!param_value.IsObject()) {
        return "";
    }

    std::string paramString;
    rapidjson::StringBuffer buffer;
    buffer.Clear();
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    param_value.Accept(writer);
    paramString = std::string(buffer.GetString());
    return "&" + param_name + "=" + paramString;
}

std::string jsonToString(const JSON_VALUE & doc) {
    if (doc.IsString()) {
        return doc.GetString();
    }

    rapidjson::StringBuffer buffer;
    buffer.Clear();
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    return {buffer.GetString()};
}
}  // namespace docker
