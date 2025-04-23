#include "common.hpp"
#include "utility/luau.hpp"
using builder_t = writer_builder_t;

static auto write(std::ostream& os, std::span<char> data) -> decltype(os) {
    os.write(&data.front(), data.size());
    return os;
} 
auto try_writer_namecall(state_t L, writer_t self, int atom) -> std::optional<int> {
    switch (static_cast<method_name>(atom)) {
        case method_name::write:
            write(*self, luau::to_buffer(L, 2));
            return luau::none;
        case method_name::writeln:
            write(*self, luau::to_buffer(L, 2)) << '\n'; 
            return luau::none;
        case method_name::print:
            *self << luaL_checkstring(L, 2);
            return luau::none;
        case method_name::println:
            *self << luaL_checkstring(L, 2) << '\n';
            return luau::none;
        case method_name::eof:
            return luau::push(L, self->eof());
        default:
            return std::nullopt;
    }
}
static auto namecall(state_t L) -> int {
    auto& self = builder_t::self(L);
    auto const [atom, name] = luau::namecall_atom(L);
    if (auto r = try_writer_namecall(L, self, atom)) return *r;
    luaL_errorL(L, "invalid namecall '%s'", name);
}
void register_writer(state_t L) {
    luaL_Reg const metatable[] = {
        {"__namecall", namecall},
        {nullptr, nullptr}
    };
    builder_t::register_type(L, metatable);
}
