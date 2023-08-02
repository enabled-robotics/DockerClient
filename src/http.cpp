#include "docker/http.hpp"

#include <curl/curl.h>

#include <iostream>

namespace curl {

size_t WriteCallback(void * contents, size_t size, size_t nmemb, void * userp) {
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

std::string const kDefaultHost = "http:/v1.41";  // default docker host

std::string_view constexpr kGet = "GET";
std::string_view constexpr kPost = "POST";
std::string_view constexpr kDelete = "DELETE";
std::string_view constexpr kPut = "PUT";

Http::Http()
    : m_baseUrl(kDefaultHost) {
    curl_global_init(CURL_GLOBAL_ALL);
}

Http::Http(std::string url)
    : m_baseUrl(std::move(url))
    , m_isRemote(m_baseUrl != kDefaultHost) {
    curl_global_init(CURL_GLOBAL_ALL);
}

Http::~Http() { curl_global_cleanup(); }

Response Http::get(std::string const & url) const { return performRequest({kGet, url, {}}); }

Response Http::post(std::string const & url, std::optional<Body> && body) const {
    return performRequest({kPost, url, std::move(body)});
}

Response Http::post(std::string const & url) const { return post(url, std::nullopt); }

Response Http::put(std::string const & url, Body const & body) {
    return performRequest({kPut, url, body});
}

Response Http::del(std::string const & url) const {
    return performRequest({kDelete, url, std::nullopt});
}

Response Http::performRequest(detail::Request && request) const {
    curl_slist * headers = nullptr;

    CURL * curl = curl_easy_init();
    if (!curl) {
        std::cerr << "error while initiating curl" << std::endl;
        curl_global_cleanup();
        exit(1);
    }

    headers = curl_slist_append(headers, "Accept: application/json");

    if (request.body.has_value()) {
        if (request.body->type == DataType::Json) {
            headers = curl_slist_append(headers, "Content-Type: application/json");
        } else {
            headers = curl_slist_append(headers, "Content-Type: application/x-tar");
        }

        if (!request.body->data.empty()) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.body->data.data());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, request.body->data.size());
        }
    }

    std::string str;
    if (!m_isRemote) {
        curl_easy_setopt(curl, CURLOPT_UNIX_SOCKET_PATH, "/var/run/docker.sock");
    }

    std::string const full_url = m_baseUrl + request.url;
    curl_easy_setopt(curl, CURLOPT_URL, full_url.data());
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, request.method.data());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &str);

    // Timeout in seconds for communication with docker.
    // 10 seconds is limit for user code & tests execution,
    // 2 seconds are for HTTP overhead:
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10 + 2);

    auto res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform error: " << curl_easy_strerror(res) << std::endl;
        curl_global_cleanup();
        exit(1);
    }

    long status = 0;  // long type is required!
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_easy_cleanup(curl);

    headers = curl_slist_append(nullptr, "Expect:");
    curl_slist_free_all(headers);

    return {static_cast<uint16_t>(status), str};
}
}  // namespace curl
