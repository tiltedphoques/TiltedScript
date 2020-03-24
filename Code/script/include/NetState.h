#pragma once

#include <NetScript.h>
#include <NetObject.h>
#include <Buffer.hpp>

struct ScriptStore;


struct NetState : std::enable_shared_from_this<NetState>
{
    using ReplicateScriptMap = TiltedPhoques::Map<TiltedPhoques::String, TiltedPhoques::Vector<NetScript>>;
    using Pointer = TiltedPhoques::SharedPtr<NetState>;
    using Weak = Pointer::weak_type;

    NetState(ScriptStore& aScriptStore);
    ~NetState() noexcept;

    TP_NOCOPYMOVE(NetState);

    void LoadDefinitions(TiltedPhoques::Buffer::Reader& aReader);
    void LoadFullSnapshot(TiltedPhoques::Buffer::Reader& aReader);
    void ApplyDifferentialSnapshot(TiltedPhoques::Buffer::Reader& aReader);
    void ProcessCallRequest(TiltedPhoques::Buffer::Reader& aReader, uint32_t aOwnerId);

    void SerializeDefinitions(TiltedPhoques::Buffer::Writer& aWriter);
    void GenerateFullSnapshot(TiltedPhoques::Buffer::Writer& aWriter);
    bool GenerateDifferentialSnapshot(TiltedPhoques::Buffer::Writer& aWriter);
    bool GenerateCallRequest(TiltedPhoques::Buffer::Writer& aWriter);

    [[nodiscard]] bool IsAuthority() const noexcept;
    [[nodiscard]] uint32_t Size() const noexcept;

    template<class T>
    void Visit(const T& acFunctor);
    
    [[nodiscard]] NetObject* GetById(uint32_t aId) const noexcept;

    void OnCreate(NetObject* apObject);
    void OnDelete(NetObject* apObject);

    uint32_t GenerateNetId() noexcept;

    void Reset() noexcept;

protected:

    template<class T>
    void ProcessNewObjects(const T& acFunctor);

    template<class T>
    void ProcessDeletedObjects(const T& acFunctor);

    static void SerializeFullObject(TiltedPhoques::Buffer::Writer& aWriter, NetObject* apObject, bool aMarkClean = false);
    static bool SerializeUpdateObject(TiltedPhoques::Buffer::Writer& aWriter, NetObject* apObject);

    void DeserializeUpdate(TiltedPhoques::Buffer::Reader& aReader);
    void DeserializeFullObject(TiltedPhoques::Buffer::Reader& aReader);
    void DeserializeDelete(TiltedPhoques::Buffer::Reader& aReader);

private:

    enum DiffType
    {
        kAdd = 1,
        kUpdate = 2,
        kDelete = 3,
        kEnd = 0x7F
    };

    friend struct ScriptContext;

    ReplicateScriptMap m_replicatedScripts;
    TiltedPhoques::Map<uint32_t, NetObject*> m_replicatedObjects;
    TiltedPhoques::Vector<NetObject*> m_newReplicatedObjects;
    TiltedPhoques::Vector<uint32_t> m_deletedReplicatedObjects;
    TiltedPhoques::Vector<NetObject::Pointer> m_remoteObjects;

    uint32_t m_objectId{ 1 };
    uint32_t m_netId{ 1 };

    ScriptStore& m_scriptStore;
};

#include "NetState.inl"
