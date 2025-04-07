#include "framework.hpp"
#include <httplib.h>

static auto abc(state_t L) -> int {
    using namespace httplib;
    Server svr;
    svr.bind_to_any_port("0.0.0.0");
    svr.Get("/hello", [](const Request& req, Response& res) {
        res.set_content("Hello world", "text/plain");
    });
    return 0;
}
