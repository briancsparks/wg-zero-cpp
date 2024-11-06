// src/url_impl.hpp (private header)
#pragma once
#include <string>
#include <unordered_map>
#include <regex>

namespace seedlib {

class URLImpl {
public:
    explicit URLImpl(const std::string& url);

    std::string scheme;
    std::string host;
    uint16_t port{0};
    std::string path;
    std::string query;
    std::string fragment;

    // Cache for parsed query parameters
    mutable std::unordered_map<std::string, std::string> query_params;
    mutable bool query_params_parsed{false};

private:
    void parse_query_params() const;
    static std::string decode_uri_component(const std::string& encoded);
};

// src/url.cpp
#include "seedlib/url.hpp"
#include "url_impl.hpp"
#include <regex>

namespace seedlib {

namespace {
    // RFC 3986 compliant URL regex
    const std::regex url_regex(
        R"(^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)",
        std::regex::ECMAScript
    );

    const std::regex authority_regex(
        R"(^(?:([^@:]*)(?::([^@]*))?@)?(\[[^]\]+\]|[^:]+)(?::(\d{1,5}))?$)",
        std::regex::ECMAScript
    );

    // Known schemes and their default ports
    const std::unordered_map<std::string, uint16_t> default_ports = {
        {"http", 80},
        {"https", 443},
        {"ws", 80},
        {"wss", 443},
        {"ftp", 21}
    };
}

URLImpl::URLImpl(const std::string& url) {
    std::smatch match;
    if (!std::regex_match(url, match, url_regex)) {
        throw URLParseError("Invalid URL format");
    }

    scheme = match[2];
    std::transform(scheme.begin(), scheme.end(), scheme.begin(), ::tolower);

    // Parse authority component (user:pass@host:port)
    std::string authority = match[4];
    if (!authority.empty()) {
        std::smatch auth_match;
        if (!std::regex_match(authority, auth_match, authority_regex)) {
            throw URLParseError("Invalid authority component");
        }

        host = auth_match[3];
        std::string port_str = auth_match[4];

        if (!port_str.empty()) {
            try {
                port = std::stoi(port_str);
                if (port > 65535) {
                    throw URLParseError("Port number out of range");
                }
            } catch (const std::exception&) {
                throw URLParseError("Invalid port number");
            }
        } else if (auto it = default_ports.find(scheme); it != default_ports.end()) {
            port = it->second;
        }
    }

    path = match[5].length() ? match[5] : "/";
    query = match[7];
    fragment = match[9];
}

void URLImpl::parse_query_params() const {
    if (query_params_parsed) return;

    std::stringstream ss(query);
    std::string pair;

    while (std::getline(ss, pair, '&')) {
        auto eq_pos = pair.find('=');
        if (eq_pos == std::string::npos) {
            query_params[decode_uri_component(pair)] = "";
        } else {
            auto key = pair.substr(0, eq_pos);
            auto value = pair.substr(eq_pos + 1);
            query_params[decode_uri_component(key)] = decode_uri_component(value);
        }
    }

    query_params_parsed = true;
}

std::string URLImpl::decode_uri_component(const std::string& encoded) {
    std::string result;
    result.reserve(encoded.length());

    for (size_t i = 0; i < encoded.length(); ++i) {
        if (encoded[i] == '%' && i + 2 < encoded.length()) {
            int value;
            std::stringstream ss;
            ss << std::hex << encoded.substr(i + 1, 2);
            ss >> value;
            result += static_cast<char>(value);
            i += 2;
        } else if (encoded[i] == '+') {
            result += ' ';
        } else {
            result += encoded[i];
        }
    }

    return result;
}

// URL class implementation
std::optional<URL> URL::parse(std::string_view url) {
    try {
        return URL(std::make_unique<URLImpl>(std::string(url)));
    } catch (const URLParseError&) {
        return std::nullopt;
    }
}

URL::ValidationResult URL::validate(std::string_view url) {
    try {
        URLImpl impl(std::string(url));
        return {true, ""};
    } catch (const URLParseError& e) {
        return {false, e.what()};
    }
}

// Constructor and destructor implementations
URL::URL(std::unique_ptr<URLImpl> impl) : impl_(std::move(impl)) {}
URL::~URL() = default;
URL::URL(URL&&) noexcept = default;
URL& URL::operator=(URL&&) noexcept = default;
URL::URL(const URL& other) : impl_(std::make_unique<URLImpl>(*other.impl_)) {}
URL& URL::operator=(const URL& other) {
    if (this != &other) {
        impl_ = std::make_unique<URLImpl>(*other.impl_);
    }
    return *this;
}

// Getter implementations
std::string_view URL::scheme() const { return impl_->scheme; }
std::string_view URL::host() const { return impl_->host; }
uint16_t URL::port() const { return impl_->port; }
std::string_view URL::path() const { return impl_->path; }
std::string_view URL::query() const { return impl_->query; }
std::string_view URL::fragment() const { return impl_->fragment; }

// Modifier implementations
void URL::set_scheme(std::string_view scheme) {
    std::string scheme_str(scheme);
    std::transform(scheme_str.begin(), scheme_str.end(),
                  scheme_str.begin(), ::tolower);

    if (scheme_str.empty() || !std::isalpha(scheme_str[0])) {
        throw URLValidationError("Invalid scheme format");
    }

    impl_->scheme = std::move(scheme_str);
}

void URL::set_port(uint16_t port) {
    if (port == 0) {
        throw URLValidationError("Port cannot be 0");
    }
    impl_->port = port;
}

std::string URL::to_string() const {
    std::stringstream ss;
    ss << impl_->scheme << "://";
    ss << impl_->host;

    if (impl_->port != 0) {
        auto it = default_ports.find(impl_->scheme);
        if (it == default_ports.end() || it->second != impl_->port) {
            ss << ":" << impl_->port;
        }
    }

    ss << impl_->path;

    if (!impl_->query.empty()) {
        ss << "?" << impl_->query;
    }

    if (!impl_->fragment.empty()) {
        ss << "#" << impl_->fragment;
    }

    return ss.str();
}

bool URL::is_secure() const {
    return impl_->scheme == "https" || impl_->scheme == "wss";
}

} // namespace seedlib
