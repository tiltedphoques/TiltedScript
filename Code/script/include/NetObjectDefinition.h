#pragma once

#include <NetObject.h>

struct ScriptContext;

struct NetObjectDefinition
{
    struct Property
    {
        String Name;
        sol::object Default;
        sol::function OnRep;
    };

    struct RemoteProcedure
    {
        uint32_t Id{ 0 };
        String Name;
        sol::function OnCall;
    };

    NetObjectDefinition(ScriptContext& aContext, sol::table& aTable, String aClassname, NetObject::IListener* apNetObjectListener);
    ~NetObjectDefinition() noexcept = default;

    NetObjectDefinition(const NetObjectDefinition&) = default;
    NetObjectDefinition& operator=(const NetObjectDefinition&) = default;

    [[nodiscard]] const String& GetNamespace() const noexcept;
    [[nodiscard]] const String& GetClassName() const noexcept;
    [[nodiscard]] const String& GetDisplayName() const noexcept;

    [[nodiscard]] const Vector<Property>& GetReplicatedProperties() const noexcept;
    [[nodiscard]] const Property& GetReplicatedProperty(uint32_t aId) const noexcept;

    [[nodiscard]] const Map<std::string, RemoteProcedure>& GetRemoteProcedures() const noexcept;
    [[nodiscard]] Map<std::string, RemoteProcedure>& GetRemoteProcedures() noexcept;

    [[nodiscard]] NetObject::IListener* GetListener() const noexcept;

protected:

    [[nodiscard]] NetObject::Pointer Create();

    void ParseTable(sol::table& aTable, ScriptContext& aContext);
    void ParseProperty(const sol::object& aKey, const sol::object& aValue);
    void ParseRPC(const sol::object& aKey, const sol::object& aValue, ScriptContext& aContext);

    void AssignIds();

    void DefineScriptType(ScriptContext& aContext, sol::table& aTable);

private:

    friend struct NetProperties;

    Vector<Property> m_replicatedProperties;
    Vector<Property> m_localProperties;
    Map<std::string, RemoteProcedure> m_remoteProcedures;
    String m_className;
    String m_namespace;
    String m_displayName;
    NetObject::IListener* m_pListener;
};
