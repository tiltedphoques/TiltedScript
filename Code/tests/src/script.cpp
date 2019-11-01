#include "catch.hpp"
extern "C"
{
#include "lua.h"
}
#include "sol.hpp"

TEST_CASE("Basic script", "[script.basic]")
{
    sol::state lua;

    lua.open_libraries(sol::lib::base, sol::lib::package);

    int value = lua.script("return 54");
    REQUIRE(value == 54);
}
