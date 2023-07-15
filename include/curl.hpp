#pragma once

#include <optional>
#include <string>

namespace curl {

struct Response {
    uint16_t httpCode;
    std::string data;  // json
};

enum class DataType { Json, Tar };

struct Body {
    DataType type;
    std::string data;
};

namespace detail {
struct Request {
    std::string_view method;
    std::string url;
    std::optional<Body> const & body;
};
}  // namespace detail

class Http {
public:
    Http();

    explicit Http(std::string url);
    ~Http();

    Response get(std::string const & url) const;
    Response post(std::string const & url, std::optional<Body> && body) const;
    Response post(std::string const & url) const;
    Response put(std::string const & url, Body const & body);
    Response del(std::string const & url) const;

private:
    Response performRequest(detail::Request && request) const;

    std::string const m_baseUrl;
    bool const m_isRemote{false};
};

}  // namespace curl
