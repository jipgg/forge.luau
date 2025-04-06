#ifndef FRAMEWORK_HPP
#define FRAMEWORK_HPP

#include <memory>
#include <expected>
#include <filesystem>
#include <lua.h>
using path_t = std::filesystem::path;
using state_t = lua_State*;
using state_owner_t = std::unique_ptr<lua_State, decltype(&lua_close)>;

namespace fw {
auto setup_state() -> state_owner_t;
auto load_script(state_t L, const path_t& path) -> std::expected<state_t, std::string>;
}

#endif
