#pragma once

#include <ScriptContext.h>
#include <NetState.h>

struct ScriptStore
{
    using String = TiltedPhoques::String;

    struct ReplicatedScript
    {
        String Filename;
        uint32_t Id;
        String Content;
    };

    using ReplicateScriptMap = TiltedPhoques::Map<TiltedPhoques::String, TiltedPhoques::Vector<ReplicatedScript>>;

    ScriptStore(bool aIsAuthority);
    ~ScriptStore() noexcept;

    TP_NOCOPYMOVE(ScriptStore);

    [[nodiscard]] bool IsAuthority() const noexcept { return m_authority; }

    uint32_t LoadFullScripts(const std::filesystem::path& acBasePath) noexcept;
    void LoadNetObjects(const std::filesystem::path& acBasePath, ScriptContext* apContext) noexcept;

    [[nodiscard]] ScriptContext* CreateContext(const TiltedPhoques::String& acNamespace) noexcept;
    [[nodiscard]] NetState::Pointer GetNetState() noexcept;

    virtual void LogScript(const std::string& acLog);
    virtual void LogInfo(const std::string& acLog);
    virtual void LogError(const std::string& acLog);

protected:

    void Reset() noexcept;

    virtual void RegisterExtensions(ScriptContext& aContext);


    [[nodiscard]] uint32_t GenerateNetId() noexcept;

    using ContextMap = TiltedPhoques::Map<String, TiltedPhoques::UniquePtr<ScriptContext>>;

    ContextMap m_contexts;

private:

    void Print(sol::this_state aState);

    NetState::Pointer m_state;
    uint32_t m_netId{ 1 };
    uint32_t m_authority : 1;
};
