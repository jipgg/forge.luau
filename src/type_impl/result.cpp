#include "common.hpp"
#include "named_atom.hpp"
#include "util/basic_type.hpp"

using type = basic_type<result, result::destructor>;
generate_type_trivial(result, type);

[[noreturn]] static void throw_bad_access(lua_State* L) {
    luaL_errorL(L, "bad result access");
}
static void expect_function(lua_State* L, int idx = 2) {
    if (not lua_isfunction(L, idx)) {
        luaL_typeerrorL(L, idx, "function");
    }
}

auto test_namecall_result(lua_State* L, result& self, int atom) -> std::optional<int> {
    auto push_self = [&] {
        lua_pushvalue(L, 1);
        return 1;
    };
    using named = named_atom;
    switch (static_cast<named>(atom)) {
        case named::and_then:
            if (not self.has_value) return push_self();
            expect_function(L, 2);
            self.push(L);
            lua_call(L, 1, 1);
            return 1;
        case named::or_else:
            if (self.has_value) return push_self();
            expect_function(L, 2);
            self.push(L);
            lua_call(L, 1, 1);
            return 1;
        case named::has_error: 
            return lua::push(L, not self.has_value);
        case named::has_value: 
            return lua::push(L, self.has_value);
        case named::value_or:
            if (self.has_value) self.push(L);
            else lua_pushvalue(L, 2);
            return 1;;
        case named::error_or:
            if (not self.has_value) self.push(L);
            else lua_pushvalue(L, 2);
            return 1;
        case named::value:
            if (not self.has_value) throw_bad_access(L);
            return self.push(L);
        case named::error:
            if (self.has_value) throw_bad_access(L);
            return self.push(L);
        default: return std::nullopt;
    }
}
static auto namecall(lua_State* L) -> int {
    auto& self = type::get(L, 1);
    auto const [atom, name] = lua::namecall_atom(L);
    auto pushed = test_namecall_result(L, self, atom);
    if (not pushed) luaL_errorL(L, "invalid namecall '%s'.", name);
    return *pushed;
}
auto result_create(lua_State* L) -> int {
    if (lua_isnoneornil(L, 2)) new_result(L, {
        .ref = lua_ref(L, 1),
        .has_value = true
    });
    else new_result(L, {
        .ref = lua_ref(L, 2),
        .has_value = false
        });
    return 1;
};

void register_result(lua_State* L) {
    luaL_Reg const metatable[] = {
        {"__namecall", namecall},
        {nullptr, nullptr}
    };
    type::setup(L, metatable);
}
