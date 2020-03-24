#pragma once

#include <NetObject.h>

struct ScriptContext;

struct NetState;

struct NetObjectDefinition
{
    struct Property
    {
        TiltedPhoques::String Name;
        sol::object Default;
        sol::function OnRep;
    };

    struct RemoteProcedure
    {
        uint32_t Id{ 0 };
        TiltedPhoques::String Name;
        sol::function OnCall;
    };

    NetObjectDefinition(ScriptContext& aContext, sol::table& aTable, TiltedPhoques::String aClassname, TiltedPhoques::SharedPtr<NetState> aParentState, uint32_t aId);
    ~NetObjectDefinition() noexcept = default;

    NetObjectDefinition(const NetObjectDefinition&) = default;
    NetObjectDefinition& operator=(const NetObjectDefinition&) = default;

    [[nodiscard]] const TiltedPhoques::String& GetNamespace() const noexcept;
    [[nodiscard]] const TiltedPhoques::String& GetClassName() const noexcept;
    [[nodiscard]] const TiltedPhoques::String& GetDisplayName() const noexcept;
    [[nodiscard]] uint32_t GetId() const noexcept;

    [[nodiscard]] const TiltedPhoques::Vector<Property>& GetReplicatedProperties() const noexcept;
    [[nodiscard]] const Property& GetReplicatedProperty(uint32_t aId) const noexcept;

    [[nodiscard]] const TiltedPhoques::Map<std::string, RemoteProcedure>& GetRemoteProcedures() const noexcept;
    [[nodiscard]] TiltedPhoques::Map<std::string, RemoteProcedure>& GetRemoteProcedures() noexcept;

    [[nodiscard]] TiltedPhoques::SharedPtr<NetState> GetParentState() const noexcept;
    [[nodiscard]] const TiltedPhoques::Map<std::string, sol::object>& GetDefaultTable() const noexcept;

    [[nodiscard]] NetObject::Pointer Create();
    [[nodiscard]] NetObject::Pointer CreateLua();

protected:

    void ParseTable(sol::table& aTable, ScriptContext& aContext);
    void ParseProperty(const sol::object& aKey, const sol::object& aValue);
    void ParseRPC(const sol::object& aKey, const sol::object& aValue, ScriptContext& aContext);

    void AssignIds();

    void DefineScriptType(ScriptContext& aContext, sol::table& aTable);

private:

    friend struct NetProperties;

    TiltedPhoques::Vector<Property> m_replicatedProperties;
    TiltedPhoques::Vector<Property> m_localProperties;
    TiltedPhoques::Map<std::string, RemoteProcedure> m_remoteProcedures;
    TiltedPhoques::String m_className;
    TiltedPhoques::String m_namespace;
    TiltedPhoques::String m_displayName;
    uint32_t m_id;
    TiltedPhoques::Map<std::string, sol::object> m_defaultTable;
    TiltedPhoques::SharedPtr<NetState> m_parentState;
    sol::table m_metatable;
};
