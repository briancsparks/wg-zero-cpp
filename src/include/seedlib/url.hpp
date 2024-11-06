// src/include/seedlib/url.hpp

#ifndef URL_HPP
#define URL_HPP

#include <string>
#include <string_view>
#include <optional>
#include <stdexcept>

namespace seedlib {

// Forward declaration of implementation
class URLImpl;

class URL {
public:
  // Constructors that might fail should use factory methods
  static std::optional<URL> parse(std::string_view url);

  // Validation with reason
  struct ValidationResult {
    bool valid;
    std::string reason;
  };
  static ValidationResult validate(std::string_view url);

  // Getters for components
  std::string_view scheme() const;
  std::string_view host() const;
  uint16_t port() const;  // Returns 0 if not specified
  std::string_view path() const;
  std::string_view query() const;
  std::string_view fragment() const;

  // Modification methods
  void set_scheme(std::string_view scheme);
  void set_port(uint16_t port);

  // Utility methods
  std::string to_string() const;
  bool is_secure() const;  // checks if scheme is https or wss

  // Rule of 5
  URL(URL&& other) noexcept;
  URL& operator=(URL&& other) noexcept;
  URL(const URL& other);
  URL& operator=(const URL& other);
  ~URL();

private:
  // Private constructor used by factory method
  explicit URL(std::unique_ptr<URLImpl> impl);
  std::unique_ptr<URLImpl> impl_;
};

// Domain-specific exceptions
class URLParseError : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

class URLValidationError : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

} // namespace seedlib

#endif
