#include "catch.hpp"

extern "C"
{
#include "lsqlite3.h"
#include "lauxlib.h"
}

// stl
#include <filesystem>
// end stl

// TiltedCore
#include "Stl.hpp"
#include "Outcome.hpp"
// end TiltedCore

#include "sol.hpp"
#include "ScriptStore.h"


TEST_CASE("Script Store Load", "[replication.store]")
{
    ScriptStore store(true);
    auto loadedScripts = store.LoadFullScripts("../../test_scripts");

    REQUIRE(loadedScripts == 1);
    REQUIRE(store.IsAuthority() == true);

    GIVEN("The script should load one net object")
    {
        uint32_t counter = 0;

        auto& netState = store.GetNetState();

        TiltedPhoques::Buffer buffer(1000);
        TiltedPhoques::Buffer::Writer writer(&buffer);

        netState.GenerateDifferentialSnapshot(writer);
        netState.GenerateFullSnapshot(writer);

        THEN(" can be serialized")
        {
            store.GetNetState();
        }
    }
}

TEST_CASE("Net Values", "[replication.value]")
{
    TiltedPhoques::Buffer buf(1000);
    TiltedPhoques::Buffer::Writer writer(&buf);
    TiltedPhoques::Buffer::Reader reader(&buf);

    {
        NetValue value(80.f);
        value.Serialize(writer);

        NetValue val(0.f);
        val.Deserialize(reader);

        REQUIRE(std::get<double>(val) == 80.f);
    }

    {
        NetValue value(std::string("test"));
        value.Serialize(writer);

        NetValue val(std::string(""));
        val.Deserialize(reader);

        REQUIRE(std::get<std::string>(val) == "test");
    }

    {
        NetValue value(true);
        value.Serialize(writer);

        NetValue val(false);
        val.Deserialize(reader);

        REQUIRE(std::get<bool>(val) == true);
    }
}

TEST_CASE("Properties", "[replication.properties]")
{
    ScriptStore store(true);
    auto pCtx = store.CreateContext("test");

    const sol::object def(pCtx->lua_state(), sol::in_place_type<double>, 42.0);

    NetProperty property(0, def);
    
    auto storedObj = property.Get();
    REQUIRE(storedObj.get_type() == sol::type::number);
    REQUIRE(storedObj.as<double>() == 42.0);
    REQUIRE(property.IsDirty() == false);
    REQUIRE(property.IsNotDefault() == false);

    property.Update(NetValue(43.0), nullptr);

    storedObj = property.Get();
    REQUIRE(storedObj.get_type() == sol::type::number);
    REQUIRE(storedObj.as<double>() == 43.0);

    // Assume we synced
    property.MarkDirty(false);

    property.Update(NetValue("test"), nullptr);

    storedObj = property.Get();
    REQUIRE(storedObj.get_type() == sol::type::number);
    REQUIRE(storedObj.as<double>() == 43.0);

    property.Set(sol::object(pCtx->lua_state(), sol::in_place_type<double>, 45.0));

    storedObj = property.Get();
    REQUIRE(storedObj.get_type() == sol::type::number);
    REQUIRE(storedObj.as<double>() == 45.0);

    NetValue v;
    v.FromObject(storedObj);

    REQUIRE(std::get<double>(v) == 45.0);
}
