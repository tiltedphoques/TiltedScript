#pragma once

#include <NetObject.h>
#include <NetObjectDefinition.h>
#include <filesystem>

struct ScriptContext : sol::state
{
    ScriptContext(TiltedPhoques::String aNamespace, bool aIsAuthority, TiltedPhoques::SharedPtr<NetState> aNetState);
    ~ScriptContext() noexcept = default;

    TP_NOCOPYMOVE(ScriptContext);

    [[nodiscard]] std::pair<bool, TiltedPhoques::String> LoadScript(const std::filesystem::path& acFile);
    [[nodiscard]] TiltedPhoques::Outcome<bool, TiltedPhoques::String> LoadNetworkObject(const std::filesystem::path& acFile, const std::string& acIdentifier);

    [[nodiscard]] const TiltedPhoques::String& GetNamespace() const noexcept;
    [[nodiscard]] TiltedPhoques::SharedPtr<NetState> GetNetState() const noexcept ;
    [[nodiscard]] bool IsAuthority() const noexcept;

    NetObject::Pointer Create(const TiltedPhoques::String& acName);

private:

    TiltedPhoques::String m_namespace;
    TiltedPhoques::Vector<TiltedPhoques::UniquePtr<NetObjectDefinition>> m_netObjectDefinitions;
    TiltedPhoques::SharedPtr<NetState> m_state;
    bool m_isAuthority;
};
