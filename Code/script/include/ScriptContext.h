#pragma once

#include <NetObject.h>
#include <NetObjectDefinition.h>

struct ScriptContext : sol::state
{
    ScriptContext(String aNamespace, bool aIsAuthority, NetObject::IListener* apListener = nullptr);
    ~ScriptContext() noexcept = default;

    TP_NOCOPYMOVE(ScriptContext);

    [[nodiscard]] std::pair<bool, String> LoadScript(const std::filesystem::path& acFile);
    [[nodiscard]] Outcome<bool, String> LoadNetworkObject(const std::filesystem::path& acFile);

    [[nodiscard]] const String& GetNamespace() const noexcept;
    [[nodiscard]] NetObject::IListener* GetNetObjectListener() const noexcept ;
    [[nodiscard]] bool IsAuthority() const noexcept;

    NetObject::Pointer Create(const String& acName);

private:

    String m_namespace;
    Vector<UniquePtr<NetObjectDefinition>> m_netObjectDefinitions;
    NetObject::IListener* m_pListener;
    bool m_isAuthority;
};
