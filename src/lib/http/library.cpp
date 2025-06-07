#include "export.hpp"
#include <httplib.h>
#include "lua/json.hpp"
#include "lua/lua.hpp"
#include <print>
#include <expected>
#include <regex>
#include "lua/typeutility.hpp"
using state = lua_State*;

struct parsed_url {
    std::string scheme;
    std::string host;
    int port;
    std::string path;
    auto host_port() {
        return std::format("{}://{}:{}", scheme, host, port);
    }
};
static auto parse_url(const std::string &url) -> std::expected<parsed_url, const char*> {
    std::regex url_regex(R"((http|https)://([^:/]+)(?::(\d+))?(/.*)?)");
    std::smatch match;
    if (not std::regex_match(url, match, url_regex)) {
        return std::unexpected("invalid url format");
    }
    auto scheme = match[1];
    auto port = match[3].matched
        ? std::stoi(match[3])
        : (scheme == "https" ? 443 : 80);

    return parsed_url {
        .scheme = scheme,
        .host = match[2],
        .port = port,
        .path = match[4].matched ? match[4].str() : "/"
    };
}
static auto urlinfo(state L) -> int {
    auto parsed = parse_url(luaL_checkstring(L, 1));
    if (!parsed) return lua::none;
    lua_newtable(L);
    using lua::set_field;
    set_field(L, "host", parsed->host);
    set_field(L, "port", parsed->port);
    set_field(L, "path", parsed->path);
    set_field(L, "scheme", parsed->scheme);
    return 1;
}
//library
static auto get(lua_State* L) -> int {
    auto parsed = parse_url(luaL_checkstring(L, 1));
    if (not parsed) luaL_errorL(L, parsed.error());
    auto client = httplib::Client{parsed->host_port()};
    auto r = client.Get(parsed->path);
    if (!r) return lua::push_tuple(L, lua::nil, std::format("error occurred ({})", static_cast<int>(r.error())));
    lua::type<lib::http::response>::make(L, std::move(*r));
    return 1;
}
static auto client(lua_State* L) -> int {
    lua::type<lib::http::client>::make(L, luaL_checkstring(L, 1));
    return 1;
}
static auto post(lua_State* L) -> int {
    auto parsed = parse_url(luaL_checkstring(L, 1));
    if (not parsed) luaL_errorL(L, parsed.error());
    auto client = httplib::Client{parsed->host_port()};

    std::string body{};
    httplib::Result result{};
    switch (lua_type(L, 2)) {
        case LUA_TTABLE:
            result = client.Post(
                parsed->path,
                lua::json::table_to_json(L, 2),
                "application/json"
            );
        case LUA_TNIL:
        case LUA_TNONE:
            result = client.Post(parsed->path);
            break;
        default:
            luaL_argerrorL(L, 2, nullptr);
    }
    if (result) {
        lua::push(L, result->status);
        if (not result->body.empty()) {
            lua::json::push_json_as_table(L, result->body);
        }
        return 2;
    }
    return lua::push_tuple(L, lua::nil, "request failed");
}
void lib::http::library(lua_State* L, int idx) {
    lua::set_functions(L, idx, std::to_array<luaL_Reg>({
        {"urlinfo", urlinfo},
        {"client", ::client},
        {"get", get},
        {"post", post},
    }));
}
