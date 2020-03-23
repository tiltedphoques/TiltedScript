#include <NetObjectDefinition.h>
#include <NetObject.h>
#include <ScriptContext.h>

NetObjectDefinition::NetObjectDefinition(ScriptContext& aContext, sol::table& aTable, String aClassname, NetState& aParentState)
    : m_className(std::move(aClassname))
    , m_namespace(aContext.GetNamespace())
    , m_parentState(aParentState)
{
    m_displayName = m_namespace + "::" + m_className;

    ParseTable(aTable, aContext);
    AssignIds();

    // We have read the Properties & NetRPCs now we remove them
    aTable["Properties"] = sol::lua_nil;
    aTable["NetRPCs"] = sol::lua_nil;

    DefineScriptType(aContext, aTable);
}

const String& NetObjectDefinition::GetNamespace() const noexcept
{
    return m_namespace;
}

const String& NetObjectDefinition::GetClassName() const noexcept
{
    return m_className;
}

const String& NetObjectDefinition::GetDisplayName() const noexcept
{
    return m_displayName;
}

const Vector<NetObjectDefinition::Property>& NetObjectDefinition::GetReplicatedProperties() const noexcept
{
    return m_replicatedProperties;
}

const NetObjectDefinition::Property& NetObjectDefinition::GetReplicatedProperty(uint32_t aId) const noexcept
{
    return m_replicatedProperties[aId];
}

const Map<std::string, NetObjectDefinition::RemoteProcedure>& NetObjectDefinition::GetRemoteProcedures() const noexcept
{
    return m_remoteProcedures;
}

Map<std::string, NetObjectDefinition::RemoteProcedure>& NetObjectDefinition::GetRemoteProcedures() noexcept
{
    return m_remoteProcedures;
}

NetState& NetObjectDefinition::GetParentState() const noexcept
{
    return m_parentState;
}

const TiltedPhoques::Map<std::string, sol::object>& NetObjectDefinition::GetDefaultTable() const noexcept
{
    return m_defaultTable;
}

NetObject::Pointer NetObjectDefinition::Create()
{
    return MakeShared<NetObject>(*this);
}

void NetObjectDefinition::ParseTable(sol::table& aTable, ScriptContext& aContext)
{
    sol::object properties = aTable["Properties"];
    sol::object rpcs = aTable["NetRPCs"];

    if (properties && properties.get_type() == sol::type::table)
    {
        sol::table elementProperties = properties.as<sol::table>();

        for (const auto& kv : elementProperties)
        {
            const auto key = kv.first;
            const auto value = kv.second;

            ParseProperty(key, value);
        }
    }

    if (rpcs && rpcs.get_type() == sol::type::table)
    {
        sol::table elementProperties = rpcs.as<sol::table>();

        for (const auto& kv : elementProperties)
        {
            const auto key = kv.first;
            const auto value = kv.second;

            ParseRPC(key, value, aContext);
        }
    }
}

void NetObjectDefinition::ParseProperty(const sol::object& aKey, const sol::object& aValue)
{
    if (aKey.get_type() == sol::type::string)
    {
        std::string str = aKey.as<std::string>();
        if (str[0] == '_' && str[1] == '_')
            return;

        if (aValue.get_type() == sol::type::string ||
            aValue.get_type() == sol::type::boolean ||
            aValue.get_type() == sol::type::number ||
            aValue.get_type() == sol::type::table)
        {
            if (aValue.get_type() == sol::type::table)
            {
                sol::table propertyTable = aValue;
                if (propertyTable["Replicates"].valid() && propertyTable["Default"].valid())
                {
                    auto& entry = m_replicatedProperties.emplace_back();
                    entry.Default = propertyTable["Default"];
                    entry.Name = str;

                    if (propertyTable["OnRep"].valid() && propertyTable["OnRep"].get_type() == sol::type::function)
                    {
                        auto onRep = propertyTable["OnRep"];
                        entry.OnRep = onRep.get<sol::function>();;
                    }
                    sol::state_view v(aValue.lua_state());

                    // spdlog::debug("\t\t> Replicated Property : {}", str);

                    return;
                }
            }
        }

        auto& entry = m_localProperties.emplace_back();
        entry.Name = str;
        entry.Default = aValue;
    }
}

void NetObjectDefinition::ParseRPC(const sol::object& aKey, const sol::object& aValue, ScriptContext& aContext)
{
    if (aKey.get_type() == sol::type::string && aValue.get_type() == sol::type::table)
    {
        std::string key = aKey.as<std::string>();
        sol::table rpcTable = aValue;

        sol::function callback = rpcTable["OnMaster"];
        if (!callback.valid())
        {
            // spdlog::error("{} RPC {} is missing OnMaster", GetDisplayName(), key);
            return;
        }

        auto& rpc = m_remoteProcedures[key];
        rpc.Name = key.c_str();

        if(aContext.IsAuthority())
        {
            rpc.OnCall = callback;
        }
        else
        {
            callback = rpcTable["OnProxy"];
            if(callback.valid())
            {
                rpc.OnCall = callback;
            }
        }
    }
}

void NetObjectDefinition::AssignIds()
{
    // TODO: Find a better solution to keep the order stable
    std::sort(std::begin(m_replicatedProperties), std::end(m_replicatedProperties), [](const Property& a, const Property& b)
    {
        return a.Name < b.Name;
    });

    // Sort by name for stable ids, we don't care if the order of RPCs is changed
    Vector<std::string> rpcNames;
    for (auto& rpc : m_remoteProcedures)
    {
        rpcNames.push_back(rpc.first);
    }

    std::sort(std::begin(rpcNames), std::end(rpcNames));
    uint32_t i = 0;
    for (auto& name : rpcNames)
    {
        m_remoteProcedures[name].Id = i++;
    }
}

void NetObjectDefinition::DefineScriptType(ScriptContext& aContext, sol::table& aTable)
{
    for (auto& kvp : aTable)
    {
        if(kvp.first.get_type() == sol::type::string)
        {
            m_defaultTable[kvp.first.as<std::string>()] = kvp.second;
        }
    }

    auto NewFunction = [this](sol::this_state aState)
    {
        auto obj = sol::make_object(aState.L, Create());
        return obj;
    };

    auto netObject = aContext.create_named_table(GetClassName().c_str());
    netObject["new"] = NewFunction;
}
