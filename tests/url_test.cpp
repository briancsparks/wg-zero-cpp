// tests/url_test.cpp
#include <catch2/catch_test_macros.hpp>
#include <seedlib/url.hpp>
#include <fstream>

using namespace seedlib;

TEST_CASE("URL parsing basic functionality", "[url]") {
    SECTION("Valid URLs are parsed correctly") {
        auto url = URL::parse("https://example.com:8080/path?query#fragment");
        REQUIRE(url.has_value());

        auto& u = url.value();
        CHECK(u.scheme() == "https");
        CHECK(u.host() == "example.com");
        CHECK(u.port() == 8080);
        CHECK(u.path() == "/path");
        CHECK(u.query() == "query");
        CHECK(u.fragment() == "fragment");
    }

    SECTION("Invalid URLs return empty optional") {
        CHECK_FALSE(URL::parse("not a url").has_value());
        CHECK_FALSE(URL::parse("http://").has_value());
        CHECK_FALSE(URL::parse("://example.com").has_value());
    }
}

TEST_CASE("URL validation provides meaningful errors", "[url]") {
    SECTION("Valid URLs pass validation") {
        auto result = URL::validate("https://example.com");
        CHECK(result.valid);
        CHECK(result.reason.empty());
    }

    SECTION("Invalid URLs provide clear error messages") {
        auto result = URL::validate("http://[invalid]");
        CHECK_FALSE(result.valid);
        CHECK(result.reason.find("Invalid host") != std::string::npos);
    }
}

TEST_CASE("URL golden value tests", "[url][golden]") {
    std::ifstream golden_file("tests/data/url_golden.json");
    REQUIRE(golden_file.is_open());

    // Read and parse golden file
    // Format: {"url": "...", "expected": {"scheme": "...", "host": "...", ...}}
    // Implementation left as exercise
}

TEST_CASE("URL modification maintains validity", "[url]") {
    auto url = URL::parse("http://example.com").value();

    SECTION("Setting valid scheme works") {
        url.set_scheme("https");
        CHECK(url.scheme() == "https");
        CHECK(url.is_secure());
    }

    SECTION("Setting invalid scheme throws") {
        CHECK_THROWS_AS(url.set_scheme("not-a-scheme"), URLValidationError);
    }

    SECTION("Setting valid port works") {
        url.set_port(8080);
        CHECK(url.port() == 8080);
    }

    SECTION("Setting invalid port throws") {
        CHECK_THROWS_AS(url.set_port(0), URLValidationError);
        CHECK_THROWS_AS(url.set_port(65536), URLValidationError);
    }
}

// Example of property-based testing
TEST_CASE("URL properties", "[url][property]") {
    SECTION("Parsing and toString are inverse operations") {
        auto url = URL::parse("https://example.com/path").value();
        auto str = url.to_string();
        auto reparsed = URL::parse(str).value();
        CHECK(url.to_string() == reparsed.to_string());
    }
}
