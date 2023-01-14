#pragma once

#include <cstdlib>
#include <cstring>
#include <curl/curl.h>
#include <iostream>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <vector>

using JSON_DOCUMENT = rapidjson::Document;
using JSON_VALUE = rapidjson::Value;

enum class Method { GET, POST, DELETE, PUT };

std::string param(const std::string &param_name,
                  const std::string &param_value);

std::string param(const std::string &param_name, const char *param_value);

std::string param(const std::string &param_name, bool param_value);

std::string param(const std::string &param_name, int param_value);

std::string param(const std::string &param_name, JSON_DOCUMENT &param_value);

std::string jsonToString(JSON_VALUE &doc);

class Docker {
public:
  Docker();

  explicit Docker(std::string host);

  ~Docker();

  /*
   * System
   */
  JSON_DOCUMENT system_info();

  JSON_DOCUMENT docker_version();

  /*
   * Images
   */
  JSON_DOCUMENT list_images();

  /*
   * Containers
   */
  JSON_DOCUMENT list_containers(bool all = false, int limit = -1,
                                const std::string &since = "",
                                const std::string &before = "", int size = -1,
                                JSON_DOCUMENT &filters = emptyDoc);

  JSON_DOCUMENT inspect_container(const std::string &container_id);

  JSON_DOCUMENT top_container(const std::string &container_id);

  JSON_DOCUMENT logs_container(const std::string &container_id,
                               bool follow = false, bool o_stdout = true,
                               bool o_stderr = false, bool timestamps = false,
                               const std::string &tail = "all");

  JSON_DOCUMENT create_container(const JSON_DOCUMENT &parameters,
                                 const std::string &name = "");

  JSON_DOCUMENT run_container(const JSON_DOCUMENT &parameters,
                              const std::string &name = "");

  JSON_DOCUMENT put_archive(const std::string &container_id,
                            const std::string &pathInContainer,
                            const std::string &pathToArchive);

  JSON_DOCUMENT exec(const JSON_DOCUMENT &createParameters,
                     const JSON_DOCUMENT &startParameters,
                     const std::string &container_id);

  JSON_DOCUMENT start_container(const std::string &container_id);

  JSON_DOCUMENT get_container_changes(const std::string &container_id);

  JSON_DOCUMENT stop_container(const std::string &container_id, int delay = -1);

  JSON_DOCUMENT kill_container(const std::string &container_id,
                               int signal = -1);

  JSON_DOCUMENT pause_container(const std::string &container_id);

  JSON_DOCUMENT wait_container(const std::string &container_id);

  JSON_DOCUMENT delete_container(const std::string &container_id,
                                 bool v = false, bool force = false);

  JSON_DOCUMENT unpause_container(const std::string &container_id);

  JSON_DOCUMENT restart_container(const std::string &container_id,
                                  int delay = -1);

  JSON_DOCUMENT attach_to_container(const std::string &container_id,
                                    bool logs = false, bool stream = false,
                                    bool o_stdin = false, bool o_stdout = false,
                                    bool o_stderr = false);

private:
  std::string host_uri;
  bool is_remote;
  CURL *curl{nullptr};
  CURLcode res{};

  static JSON_DOCUMENT emptyDoc;

  JSON_DOCUMENT requestAndParse(Method method, const std::string &path,
                                unsigned success_code = 200,
                                const JSON_DOCUMENT &param = emptyDoc,
                                bool isReturnJson = false);

  JSON_DOCUMENT requestAndParsePut(const std::string &path,
                                   const std::string &pathToArchive);

  JSON_DOCUMENT requestAndParseJson(Method method, const std::string &path,
                                    unsigned success_code = 200,
                                    const JSON_DOCUMENT &param = emptyDoc);

  JSON_DOCUMENT execCreate(const JSON_DOCUMENT &parameters,
                           const std::string &container_id);

  JSON_DOCUMENT execStart(const JSON_DOCUMENT &parameters,
                          const std::string &exec_id);

  static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                              void *userp) {
    std::vector<char> &data = *reinterpret_cast<std::vector<char> *>(userp);
    data.reserve(size * nmemb);
    char const *pData = static_cast<char const *>(contents);
    for (size_t index = 0; index < data.capacity(); ++index) {
      data.push_back(static_cast<char>(*(pData + index)));
    }

    return size * nmemb;
  }
};
