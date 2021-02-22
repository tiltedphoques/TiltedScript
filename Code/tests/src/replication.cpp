
#include <catch2/catch.hpp>

// stl
#include <filesystem>
// end stl

// TiltedCore
#include <TiltedCore/Stl.hpp>
#include <TiltedCore/Outcome.hpp>
#include <TiltedCore/Buffer.hpp>
// end TiltedCore

#include <sol/sol.hpp>
#include <ScriptStore.h>

TEST_CASE("Script Store Load", "[replication.store]")
{
    ScriptStore masterStore(true);
   
    auto loadedScripts = masterStore.LoadFullScripts("../../test_scripts");

    REQUIRE(loadedScripts == 1);
    REQUIRE(masterStore.IsAuthority() == true);

    GIVEN("The script should load one net object")
    {
        uint32_t counter = 0;

        auto netState = masterStore.GetNetState();

        TiltedPhoques::Buffer buffer(1000);
        TiltedPhoques::Buffer::Writer writer(&buffer);
        TiltedPhoques::Buffer::Reader reader(&buffer);

        THEN(" serialize the definitions")
        {
            netState->SerializeDefinitions(writer);

            THEN(" load the definitions")
            {
                ScriptStore slaveStore(false);
                slaveStore.GetNetState()->LoadDefinitions(reader);

                THEN(" serialize the objects")
                {
                    // First call forces net objects to get ids and generates the snapshot with the newly added objects
                    REQUIRE(netState->GenerateDifferentialSnapshot(writer));
                    // Now that objects have been added to the world, we can process calls
                    REQUIRE(netState->GenerateDifferentialSnapshot(writer));
                    // Nothing should have changed so it doesn't have anything to send
                    REQUIRE(!netState->GenerateDifferentialSnapshot(writer));

                    // Will add the new object
                    slaveStore.GetNetState()->ApplyDifferentialSnapshot(reader);
                    // Will process calls
                    slaveStore.GetNetState()->ApplyDifferentialSnapshot(reader);

                    THEN(" attempt a full snapshot")
                    {
                        ScriptStore slaveStore2(false);
                        netState->SerializeDefinitions(writer);
                        slaveStore2.GetNetState()->LoadDefinitions(reader);

                        // Here we create a full snapshot containing all net objects to get up to speed
                        netState->GenerateFullSnapshot(writer);

                        THEN(" load the objects")
                        {
                            // After this we have the same state as the master, we can start applying diffs
                            slaveStore2.GetNetState()->LoadFullSnapshot(reader);
                        }
                    }
                }
            }
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
