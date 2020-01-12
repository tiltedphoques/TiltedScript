#include "catch.hpp"
#include <iostream>

extern "C"
{
#include "lsqlite3.h"
#include "lauxlib.h"
}

#include "sol.hpp"


TEST_CASE("Basic script", "[script.basic]")
{
    sol::state lua;

    lua.open_libraries(sol::lib::base, sol::lib::package);

    int value = lua.script("return 54");
    REQUIRE(value == 54);
}

TEST_CASE("SQLite3", "[script.db]")
{
    sol::state lua;

    lua.open_libraries(sol::lib::base, sol::lib::package);
    auto result = lua.require("sqlite3", luaopen_lsqlite3);
    REQUIRE(result.valid());

    sol::table sqlite = lua["sqlite3"];

    // Unprotect meta
    sqlite[sol::metatable_key] = nullptr;

    sqlite["test"] = []() { std::cout << "TEST" << std::endl; };

    // Protect meta
    sqlite[sol::metatable_key] = sqlite;

    auto script = R"V0G0N(
    local sqlite3 = require("sqlite3")

    local db = sqlite3.open_memory()

    sqlite3.test()

    assert( db:exec "CREATE TABLE test (col1, col2)" )
    assert( db:exec "INSERT INTO test VALUES (1, 2)" )
    assert( db:exec "INSERT INTO test VALUES (2, 4)" )
    assert( db:exec "INSERT INTO test VALUES (3, 6)" )
    assert( db:exec "INSERT INTO test VALUES (4, 8)" )
    assert( db:exec "INSERT INTO test VALUES (5, 10)" )

    local sum = 0

    for row in db:nrows("SELECT * FROM test") do
      sum = sum + row.col2
    end

    return sum

    )V0G0N";

    REQUIRE(lua.script(script).get<int>() == 30);
}
