local M = {}

local multicast = require("./multicast")
export type multicast<Args...> = multicast.multicast<Args...>
M.multicast = multicast

return M
