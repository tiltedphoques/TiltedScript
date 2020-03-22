#pragma once

#include "ScriptContext.h"

struct ScriptStore : NetObject::IListener
{
    struct ReplicatedScript
    {
        String Filename;
        uint32_t Id;
        String Content;
    };

    ScriptStore(bool aIsAuthority);
    ~ScriptStore() noexcept;

    TP_NOCOPYMOVE(ScriptStore);

    [[nodiscard]] bool IsAuthority() const noexcept { return m_authority; }

    void LoadFullScripts(const std::filesystem::path& acBasePath) noexcept;
    void LoadNetObjects(const std::filesystem::path& acBasePath, ScriptContext* apContext) noexcept;

    [[nodiscard]] ScriptContext* CreateContext(const String& acNamespace) noexcept;

    void VisitNetObjects(const std::function<void(NetObject*)>& acFunctor);
    void VisitNewNetObjects(const std::function<void(NetObject*)>& acFunctor);
    void VisitDeletedObjects(const std::function<void(uint32_t)>& acFunctor);

    void InitializeNewNetObjects();

    NetObject* GetById(uint32_t aId) const noexcept;

    void OnCreate(NetObject* apObject) override;
    void OnDelete(NetObject* apObject) override;

    uint32_t GenerateNetId() noexcept { return m_netId++; }

protected:

    void Reset() noexcept;

    virtual void RegisterExtensions(ScriptContext& aContext);

    const auto& GetReplicatedScripts() const noexcept { return m_replicatedScripts; }

    Map<String, UniquePtr<ScriptContext>> m_contexts;

private:

    Map<String, Vector<ReplicatedScript>> m_replicatedScripts;
    Map<uint32_t, NetObject*> m_replicatedObjects;
    Vector<NetObject*> m_newReplicatedObjects;
    Vector<uint32_t> m_deletedReplicatedObjects;

    uint32_t m_objectId{ 1 };
    uint32_t m_netId{ 1 };
    uint32_t m_authority : 1;
};
