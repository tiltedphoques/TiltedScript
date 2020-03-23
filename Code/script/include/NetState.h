#pragma once

#include <NetScript.h>
#include <Buffer.hpp>

struct NetObject;

struct NetState
{
    using ReplicateScriptMap = TiltedPhoques::Map<TiltedPhoques::String, TiltedPhoques::Vector<NetScript>>;

    NetState(bool aIsAuthority);
    ~NetState() noexcept;

    TP_NOCOPYMOVE(NetState);

    void GenerateFullSnapshot(TiltedPhoques::Buffer::Writer& aWriter);
    void GenerateDifferentialSnapshot(TiltedPhoques::Buffer::Writer& aWriter);

    [[nodiscard]] bool IsAuthority() const noexcept;

    template<class T>
    void Visit(const T& acFunctor);
    
    [[nodiscard]] NetObject* GetById(uint32_t aId) const noexcept;

    void OnCreate(NetObject* apObject);
    void OnDelete(NetObject* apObject);

    uint32_t GenerateNetId() noexcept;

protected:

    template<class T>
    void ProcessNewObjects(const T& acFunctor);

    template<class T>
    void ProcessDeletedObjects(const T& acFunctor);

private:

    friend struct ScriptContext;

    ReplicateScriptMap m_replicatedScripts;
    TiltedPhoques::Map<uint32_t, NetObject*> m_replicatedObjects;
    TiltedPhoques::Vector<NetObject*> m_newReplicatedObjects;
    TiltedPhoques::Vector<uint32_t> m_deletedReplicatedObjects;

    uint32_t m_objectId{ 1 };
    uint32_t m_netId{ 1 };
    uint32_t m_authority : 1;
};

#include "NetState.inl"
