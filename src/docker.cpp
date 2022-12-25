/****
 Dependency
 - libcurl
 - [RapidJSON](https://github.com/Tencent/rapidjson/)



 Basic Usage

#include "docker.h"
int main(){
    Docker client = Docker();

    JSON_DOCUMENT doc = client.list_containers(true);
    std::cout << jsonToString(doc) << std::endl;

    return 0;
} 



 Return Type for Each Methods

Object o; (rapidjson::Document)
 - success        [bool]                  : if succeeded to request
 - data           [Object/Array/string]   : actual data by server (data type depends on API, but it would be Object if 'success' is false)
 - code(optional) [int]                   : http status code if 'success' is false

e.g.
{
  "success": false,
  "code": 404,
  "data": {
      "message": "No such container: 5d271b3a52263330348b71948bd25cda455a49f7e7d69cfc73e6b2f3b5b41a4c" 
  }
} 


{
  "success": true ,
  "data": {
      "Architecture": "x86_64",
          ...
      "SystemTime": "2018-05-23T19:26:54.357768797+09:00" 
  }
}

****/

#include "docker.h"
#include <utility>
#include <sstream>
#include <fstream>
#include <vector>

/*
*  
* START Docker Implementation
* 
*/
JSON_DOCUMENT Docker::emptyDoc = JSON_DOCUMENT();

Docker::Docker() : host_uri("http:/v1.41") {
    curl_global_init(CURL_GLOBAL_ALL);
    is_remote = false;
}

Docker::Docker(std::string host) : host_uri(std::move(host)) {
    curl_global_init(CURL_GLOBAL_ALL);
    is_remote = true;
}

Docker::~Docker() {
    curl_global_cleanup();
}


/*
*  
* Public Methods
* 
*/

/*
* System
*/
JSON_DOCUMENT Docker::system_info() {
    std::string path = "/info";
    return requestAndParseJson(Method::GET, path);
}

JSON_DOCUMENT Docker::docker_version() {
    std::string path = "/version";
    return requestAndParseJson(Method::GET, path);
}

/*
* Images
*/
JSON_DOCUMENT Docker::list_images() {
    std::string path = "/images/json";
    return requestAndParseJson(Method::GET, path);
}

/*
* Containers
*/
JSON_DOCUMENT Docker::list_containers(bool all, int limit, const std::string &since, const std::string &before,
                                      int size, JSON_DOCUMENT &filters) {
    std::string path = "/containers/json?";
    path += param("all", all);
    path += param("limit", limit);
    path += param("since", since);
    path += param("before", before);
    path += param("size", size);
    path += param("filters", filters);
    return requestAndParseJson(Method::GET, path);
}

JSON_DOCUMENT Docker::inspect_container(const std::string &container_id) {
    std::string path = "/containers/" + container_id + "/json";
    return requestAndParseJson(Method::GET, path);
}

JSON_DOCUMENT Docker::top_container(const std::string &container_id) {
    std::string path = "/containers/" + container_id + "/top";
    return requestAndParseJson(Method::GET, path);
}

JSON_DOCUMENT Docker::logs_container(const std::string &container_id, bool follow, bool o_stdout, bool o_stderr,
                                     bool timestamps, const std::string &tail) {
    std::string path = "/containers/" + container_id + "/logs?";
    path += param("follow", follow);
    path += param("stdout", o_stdout);
    path += param("stderr", o_stderr);
    path += param("timestamps", timestamps);
    path += param("tail", tail);
    return requestAndParse(Method::GET, path, 101);
}

JSON_DOCUMENT Docker::create_container(const JSON_DOCUMENT &parameters, const std::string &name) {
    std::string path = "/containers/create";
    path += not name.empty() ? "?name=" + name : "";
    return requestAndParseJson(Method::POST, path, 201, parameters);
}

JSON_DOCUMENT Docker::start_container(const std::string &container_id) {
    std::string path = "/containers/" + container_id + "/start";
    return requestAndParse(Method::POST, path, 204);
}

JSON_DOCUMENT Docker::get_container_changes(const std::string &container_id) {
    std::string path = "/containers/" + container_id + "/changes";
    return requestAndParseJson(Method::GET, path);
}

JSON_DOCUMENT Docker::stop_container(const std::string &container_id, int delay) {
    std::string path = "/containers/" + container_id + "/stop?";
    path += param("t", delay);
    return requestAndParse(Method::POST, path, 204);
}

JSON_DOCUMENT Docker::kill_container(const std::string &container_id, int signal) {
    std::string path = "/containers/" + container_id + "/kill?";
    path += param("signal", signal);
    return requestAndParse(Method::POST, path, 204);
}

JSON_DOCUMENT Docker::pause_container(const std::string &container_id) {
    std::string path = "/containers/" + container_id + "/pause";
    return requestAndParse(Method::POST, path, 204);
}

JSON_DOCUMENT Docker::wait_container(const std::string &container_id) {
    std::string path = "/containers/" + container_id + "/wait";
    return requestAndParseJson(Method::POST, path);
}

JSON_DOCUMENT Docker::delete_container(const std::string &container_id, bool v, bool force) {
    std::string path = "/containers/" + container_id + "?";
    path += param("v", v);
    path += param("force", force);
    return requestAndParse(Method::DELETE, path, 204);
}

JSON_DOCUMENT Docker::unpause_container(const std::string &container_id) {
    std::string path = "/containers/" + container_id + "/unpause?";
    return requestAndParse(Method::POST, path, 204);
}

JSON_DOCUMENT Docker::restart_container(const std::string &container_id, int delay) {
    std::string path = "/containers/" + container_id + "/restart?";
    path += param("t", delay);
    return requestAndParse(Method::POST, path, 204);
}

JSON_DOCUMENT Docker::attach_to_container(const std::string &container_id, bool logs, bool stream, bool o_stdin,
                                          bool o_stdout, bool o_stderr) {
    std::string path = "/containers/" + container_id + "/attach?";
    path += param("logs", logs);
    path += param("stream", stream);
    path += param("stdin", o_stdin);
    path += param("stdout", o_stdout);
    path += param("stderr", o_stderr);

    return requestAndParse(Method::POST, path, 101);
}
//void Docker::copy_from_container(const std::string& container_id, const std::string& file_path, const std::string& dest_tar_file){}


/*
*  
* Private Methods
* 
*/

JSON_DOCUMENT Docker::requestAndParse(Method method, const std::string &path, unsigned success_code,
                                      const JSON_DOCUMENT &param, bool isReturnJson) {
    std::string paramString;
    std::string method_str;
    struct curl_slist *headers = nullptr;
    const char *paramChar;
    switch (method) {
        case Method::GET:
            method_str = "GET";
            break;
        case Method::POST:
            method_str = "POST";
            break;
        case Method::DELETE:
            method_str = "DELETE";
            break;
        case Method::PUT:
            method_str = "PUT";
            break;
        default:
            method_str = "GET";
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
    paramString = std::string(buffer.GetString());
    paramChar = paramString.c_str();
    if (isReturnJson)
        headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");

    std::vector<char> readBuffer;
    if (!is_remote)
        curl_easy_setopt(curl, CURLOPT_UNIX_SOCKET_PATH, "/var/run/docker.sock");
    curl_easy_setopt(curl, CURLOPT_URL, (host_uri + path).c_str());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method_str.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    if (method == Method::POST) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, paramChar);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(paramChar));
    }

    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
    unsigned status = 0;
    curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &status);
    curl_easy_cleanup(curl);

    std::string str;
    for (char c : readBuffer) {
        // TODO: attention! maybe this check should be in another place?
        if (c < '\x20') {
            continue;
        }
        str.push_back(c);
    }

    JSON_DOCUMENT doc(rapidjson::kObjectType);
    if (status == success_code || status == 200) {
        doc.AddMember("success", true, doc.GetAllocator());
        JSON_VALUE dataString;

        dataString.SetString(str, doc.GetAllocator());

        doc.AddMember("data", dataString, doc.GetAllocator());
    } else {
        JSON_DOCUMENT resp(&doc.GetAllocator());
        resp.Parse(str.c_str());

        doc.AddMember("success", false, doc.GetAllocator());
        doc.AddMember("code", status, doc.GetAllocator());
        doc.AddMember("data", resp, doc.GetAllocator());
    }
    headers = curl_slist_append(nullptr, "Expect:");
    curl_slist_free_all(headers);
    return doc;
}

JSON_DOCUMENT Docker::requestAndParseJson(Method method, const std::string &path, unsigned success_code,
                                          const JSON_DOCUMENT &param) {
    auto result_obj = requestAndParse(method, path, success_code, param, true);
    bool result = (result_obj.HasMember("success") && result_obj["success"].IsBool() &&
                   result_obj["success"].GetBool());
    if (result) {
        JSON_DOCUMENT doc(rapidjson::kObjectType);

        JSON_DOCUMENT data(&doc.GetAllocator());
        data.Parse(result_obj["data"].GetString());

        doc.AddMember("success", true, doc.GetAllocator());
        doc.AddMember("data", data, doc.GetAllocator());
        return doc;
    } else {
        return result_obj;
    }
}

JSON_DOCUMENT Docker::run_container(const rapidjson::Document &parameters, const std::string &name) {
    auto result = create_container(parameters, name);

    auto const data = result["data"].GetObject();
    start_container(data["Id"].GetString());

    return result;
}

JSON_DOCUMENT Docker::put_archive(const std::string &container_id, const std::string &pathInContainer,
                                  const std::string &pathToArchive) {
    std::string path = "/containers/";
    path += container_id + std::string{"/archive?path="} + pathInContainer;
    return requestAndParsePut(path, pathToArchive);
}

JSON_DOCUMENT Docker::requestAndParsePut(const std::string &path, const std::string &pathToArchive) {
    std::ifstream file(pathToArchive.c_str(), std::ios::binary | std::ios::ate);
    file.unsetf(std::ios::skipws);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer;
    buffer.reserve(size);
    buffer.insert(buffer.begin(),
                  std::istream_iterator<char>(file),
                  std::istream_iterator<char>());

    curl = curl_easy_init();
    if (!curl) {
        std::cout << "error while initiating curl" << std::endl;
        curl_global_cleanup();
        std::abort();
    }
    curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/x-tar");

    std::string readBuffer;
    if (!is_remote) {
        curl_easy_setopt(curl, CURLOPT_UNIX_SOCKET_PATH, "/var/run/docker.sock");
    }
    curl_easy_setopt(curl, CURLOPT_URL, (host_uri + path).c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buffer.data());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, buffer.size());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
    uint32_t status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_easy_cleanup(curl);

    const char *buf = readBuffer.c_str();
    JSON_DOCUMENT doc(rapidjson::kObjectType);
    if (status == 200) {
        doc.AddMember("success", true, doc.GetAllocator());

        JSON_VALUE dataString;
        dataString.SetString(readBuffer.c_str(), doc.GetAllocator());

        doc.AddMember("data", dataString, doc.GetAllocator());
    } else {
        JSON_DOCUMENT resp(&doc.GetAllocator());
        resp.Parse(buf);

        doc.AddMember("success", false, doc.GetAllocator());
        doc.AddMember("code", status, doc.GetAllocator());
        doc.AddMember("data", resp, doc.GetAllocator());
    }
    headers = curl_slist_append(nullptr, "Expect:");
    curl_slist_free_all(headers);
    return doc;
}

JSON_DOCUMENT Docker::exec(const JSON_DOCUMENT &createParameters, const JSON_DOCUMENT &startParameters,
                           const std::string &container_id) {
    auto const createResult = execCreate(createParameters, container_id);
    if (!createResult.HasMember("success") || !createResult["success"].GetBool() || !createResult.HasMember("data") ||
        !createResult["data"].IsObject() || !createResult["data"].GetObject().HasMember("Id") ||
        !createResult["data"].GetObject()["Id"].IsString()) {
        return {};
    }
    std::string const execId = createResult["data"].GetObject()["Id"].GetString();

    return execStart(startParameters, execId);
}

JSON_DOCUMENT Docker::execCreate(const JSON_DOCUMENT &parameters, const std::string &container_id) {
    std::string path = "/containers/";
    path += container_id + std::string{"/exec"};
    return requestAndParseJson(Method::POST, path, 201, parameters);
}

JSON_DOCUMENT Docker::execStart(const JSON_DOCUMENT &parameters, const std::string &exec_id) {
    std::string path = "/exec/" + exec_id + std::string{"/start"};
    return requestAndParse(Method::POST, path, 200, parameters);
}

/*
*  
* END Docker Implementation
* 
*/

std::string param(const std::string &param_name, const std::string &param_value) {
    if (!param_value.empty()) {
        return "&" + param_name + "=" + param_value;
    } else {
        return "";
    }
}

std::string param(const std::string &param_name, const char *param_value) {
    if (param_value != nullptr) {
        return "&" + param_name + "=" + param_value;
    } else {
        return "";
    }
}

std::string param(const std::string &param_name, bool param_value) {
    std::string ret;
    ret = "&" + param_name + "=";
    if (param_value) {
        return ret + "true";
    } else {
        return ret + "false";
    }
}

std::string param(const std::string &param_name, int param_value) {
    if (param_value != -1) {
        std::ostringstream convert;
        convert << param_value;
        return "&" + param_name + "=" + convert.str();
    } else {
        return "";
    }
}

std::string param(const std::string &param_name, JSON_DOCUMENT &param_value) {
    if (param_value.IsObject()) {
        std::string paramString;
        rapidjson::StringBuffer buffer;
        buffer.Clear();
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        param_value.Accept(writer);
        paramString = std::string(buffer.GetString());
        return "&" + param_name + "=" + paramString;
    } else {
        return "";
    }
}

std::string jsonToString(JSON_VALUE &doc) {
    rapidjson::StringBuffer buffer;
    buffer.Clear();
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    return {buffer.GetString()};
}
