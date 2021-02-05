#include <ScriptContext.h>
#include <NetObjectDefinition.h>
#include <NetProperties.h>
#include <NetRPCs.h>
#include <NetState.h>
#include <lsqlite3.h>

#include <TiltedCore/Filesystem.hpp>

ScriptContext::ScriptContext(String aNamespace, bool aIsAuthority, TiltedPhoques::SharedPtr<NetState> aNetState)
    : m_namespace(std::move(aNamespace))
    , m_state(aNetState)
    , m_isAuthority(aIsAuthority)
{
    new_usertype<NetProperties>("NetProperties", sol::no_constructor,
        sol::meta_function::index, &NetProperties::Get,
        sol::meta_function::new_index, &NetProperties::Set);

    new_usertype<NetRPCs>("NetRPCs", sol::no_constructor,
        sol::meta_function::index, &NetRPCs::Get);

    // Create the type
    new_usertype<NetObject>("NetObject", sol::no_constructor,
        sol::meta_function::index, &NetObject::Get,
        sol::meta_function::new_index, &NetObject::Set);

    auto IsAuthorityFunc = [this]() { return IsAuthority(); };

    registry()["IsAuthority"] = IsAuthorityFunc;

    open_libraries(sol::lib::base, sol::lib::coroutine, sol::lib::string, sol::lib::math, sol::lib::table, sol::lib::package, sol::lib::utf8, sol::lib::bit32);
    require("sqlite3", luaopen_lsqlite3);
}

std::pair<bool, String> ScriptContext::LoadScript(const std::filesystem::path& acFile)
{
    try
    {
        const auto result = script_file(acFile.string());

        if (!result.valid())
        {
            const sol::error error = result;
            return std::make_pair(false, String(error.what()));
        }
    }
    catch (std::exception & exception)
    {
        return std::make_pair(false, String(exception.what()));
    }

    return std::make_pair(true, String());
}

TiltedPhoques::Outcome<bool, String> ScriptContext::LoadNetworkObject(const std::filesystem::path& acFile, const std::string& acIdentifier)
{
    const auto classname = String(acFile.stem().string());
    const auto fullName = m_namespace + "_" + classname;

    // Create a new table that the script will affect and set it as a global named "OBJECT"
    sol::table elementTable = create_table();
    elementTable[sol::meta_function::index] = elementTable;

    globals()["OBJECT"] = elementTable;

    const auto result = LoadScript(acFile);

    // The script has been executed, cleanup the globals
    globals()["OBJECT"] = nullptr;

    if (!result.first)
        return result.second;

    auto& scriptVector = GetNetState()->m_replicatedScripts[GetNamespace()];
 
    auto pTmpDef = TiltedPhoques::MakeUnique<NetObjectDefinition>(*this, elementTable, classname, GetNetState(), static_cast<uint32_t>(scriptVector.size()));

    auto& replicatedObject = scriptVector.emplace_back(std::move(pTmpDef));
    replicatedObject.Filename = acIdentifier.c_str();
    replicatedObject.Content = std::move(TiltedPhoques::LoadFile(acFile));

    return true;
}

const String& ScriptContext::GetNamespace() const noexcept
{
    return m_namespace;
}

NetState::Pointer ScriptContext::GetNetState() const noexcept
{
    return m_state;
}

bool ScriptContext::IsAuthority() const noexcept
{
    return m_isAuthority;
}

NetObject::Pointer ScriptContext::Create(const String& acName)
{
    const auto result = script("return " + acName + ".new()");
    if (result.valid())
    {
        NetObject::Pointer& netObject = result;

        return netObject;
    }

    return nullptr;
}
