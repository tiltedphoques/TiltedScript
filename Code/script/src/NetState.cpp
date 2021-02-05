#include "NetState.h"
#include "NetObject.h"
#include "NetObjectDefinition.h"
#include "TiltedCore/Serialization.hpp"
#include "ScriptStore.h"
#include "ScriptContext.h"

#include <fstream>

using TiltedPhoques::Serialization;
using TiltedPhoques::Buffer;
using TiltedPhoques::Vector;

NetState::NetState(ScriptStore& aScriptStore)
    : m_scriptStore(aScriptStore)
{}

NetState::~NetState() noexcept
{
}

void NetState::LoadDefinitions(TiltedPhoques::Buffer::Reader& aReader)
{
    auto rootPath = std::filesystem::temp_directory_path() / "TiltedScripts" / ".net_scripts";

    auto count = Serialization::ReadVarInt(aReader);
    for( ; count ; --count)
    {
        const auto name = Serialization::ReadString(aReader);

        auto pContext = m_scriptStore.CreateContext(name.c_str());

        auto scriptCount = Serialization::ReadVarInt(aReader);
        for( ; scriptCount ; scriptCount--)
        {
            const auto fileName = Serialization::ReadString(aReader);
            const auto content = Serialization::ReadString(aReader);

            auto path = rootPath / fileName;

            create_directories(path.parent_path());

            std::ofstream file(path, std::ios::binary);
            file.write(content.data(), content.size());
            file.close();

            auto result = pContext->LoadNetworkObject(path, path.filename().stem().string());
            if(result.HasError())
            {
                m_scriptStore.LogError(result.GetError().c_str());
            }
        }
    }

    remove_all(rootPath);
}

void NetState::DeserializeFullObject(TiltedPhoques::Buffer::Reader& aReader)
{
    const auto id = Serialization::ReadVarInt(aReader);
    const auto parentId = Serialization::ReadVarInt(aReader);
    const auto classNamespace = Serialization::ReadString(aReader);
    const auto classId = Serialization::ReadVarInt(aReader);

    auto& vec = m_replicatedScripts[classNamespace.c_str()];

    auto pObject = vec[classId].Definition->Create();
    const auto result = pObject->SetParentId(parentId);
    if (!result)
        m_scriptStore.LogError("Tried to assign a parent id but failed ?!");

    pObject->SetId(id);
    m_replicatedObjects[id] = pObject.get();

    m_remoteObjects.push_back(pObject);

    auto propertyCount = Serialization::ReadVarInt(aReader);

    for (; propertyCount; --propertyCount)
    {
        auto propId = Serialization::ReadVarInt(aReader);
        auto pProperty = pObject->GetProperties().GetById(propId);

        pProperty->Deserialize(aReader);
    }
}

void NetState::DeserializeDelete(TiltedPhoques::Buffer::Reader& aReader)
{
    auto id = Serialization::ReadVarInt(aReader);

    for(auto it = std::begin(m_remoteObjects); it != std::end(m_remoteObjects); ++it)
    {
        if((*it)->GetId() == id)
        {
            if (m_remoteObjects.size() == 1)
                m_remoteObjects.clear();
            else
            {
                std::iter_swap(it, std::end(m_remoteObjects) - 1);
                m_remoteObjects.pop_back();
            }

            return;
        }
    }
}

void NetState::LoadFullSnapshot(TiltedPhoques::Buffer::Reader& aReader)
{
    auto count = Serialization::ReadVarInt(aReader);
    for (; count; --count)
    {
        DeserializeFullObject(aReader);
    }
}

void NetState::ApplyDifferentialSnapshot(TiltedPhoques::Buffer::Reader& aReader)
{
    auto type = static_cast<DiffType>(Serialization::ReadVarInt(aReader));

    while(type != kEnd)
    {
        switch(type)
        {
        case kAdd:
            DeserializeFullObject(aReader);
            break;
        case kUpdate:
            DeserializeUpdate(aReader);
            break;
        case kDelete:
            DeserializeDelete(aReader);
            break;
        }

        type = static_cast<DiffType>(Serialization::ReadVarInt(aReader));
    }
}

void NetState::ProcessCallRequest(TiltedPhoques::Buffer::Reader& aReader, uint32_t aOwnerId)
{
    auto type = static_cast<DiffType>(Serialization::ReadVarInt(aReader));

    while (type == kUpdate)
    {
        const auto id = Serialization::ReadVarInt(aReader);
        auto count = Serialization::ReadVarInt(aReader);

        auto pObject = GetById(id);
        if (!pObject)
        {
            assert("Object did not exist !");
            return;
        }

        for(; count ; --count)
        {
            NetRPCs::Call call;
            call.Deserialize(aReader);

            // Don't let people make calls on stuff they don't own
            if(pObject->GetOwnerId() == aOwnerId)
                pObject->GetRPCs().Queue(call);
        }

        type = static_cast<DiffType>(Serialization::ReadVarInt(aReader));
    }
}

void NetState::SerializeDefinitions(Buffer::Writer& aWriter)
{
    Serialization::WriteVarInt(aWriter, m_replicatedScripts.size());

    for(auto& kvp : m_replicatedScripts)
    {
        const auto& name = kvp.first;
        const auto& vec = kvp.second;

        Serialization::WriteString(aWriter, name.c_str());
        Serialization::WriteVarInt(aWriter, vec.size());
        for(auto& script : vec)
        {
            Serialization::WriteString(aWriter, script.Filename.c_str());
            Serialization::WriteString(aWriter, script.Content.c_str());
        }
    }
}

void NetState::GenerateFullSnapshot(Buffer::Writer& aWriter)
{
    Serialization::WriteVarInt(aWriter, Size());

    Visit([&aWriter](NetObject* apObject)
        {
            SerializeFullObject(aWriter, apObject);
        });
}

bool NetState::GenerateDifferentialSnapshot(Buffer::Writer& aWriter)
{
    uint32_t count = 0;

    Visit([&aWriter, &count](NetObject* apObject)
        {
            if (SerializeUpdateObject(aWriter, apObject))
                ++count;
        });

    ProcessNewObjects([this, &aWriter, &count](NetObject* apObject)
        {
            apObject->SetId(GenerateNetId());

            ++count;

            Serialization::WriteVarInt(aWriter, kAdd);
            SerializeFullObject(aWriter, apObject, true);
        });

    ProcessDeletedObjects([&count, &aWriter](uint32_t aId)
        {
            ++count;
            Serialization::WriteVarInt(aWriter, kDelete);
            Serialization::WriteVarInt(aWriter, aId);
        });

    if(count)
    {
        // Mark EOF
        Serialization::WriteVarInt(aWriter, kEnd);
        return true;
    }

    return false;
}

bool NetState::GenerateCallRequest(TiltedPhoques::Buffer::Writer& aWriter)
{
    if(!IsAuthority())
    {
        uint32_t count = 0;

        Visit([&aWriter, &count](NetObject* apObject)
            {
                Vector<NetRPCs::Call> calls;

                apObject->GetRPCs().Process([&calls](const NetRPCs::Call& aCall)
                    {
                        calls.push_back(aCall);
                    });

                const uint32_t id = apObject->GetId();
                const uint16_t callCount = calls.size();

                if (callCount)
                {
                    ++count;

                    Serialization::WriteVarInt(aWriter, kUpdate);
                    Serialization::WriteVarInt(aWriter, id);
                    Serialization::WriteVarInt(aWriter, callCount);

                    for (auto& call : calls)
                    {
                        call.Serialize(aWriter);
                    }
                }
            });

        if(count)
        {
            Serialization::WriteVarInt(aWriter, kEnd);
            return true;
        }
    }

    return false;
}

bool NetState::IsAuthority() const noexcept
{
    return m_scriptStore.IsAuthority();
}

uint32_t NetState::Size() const noexcept
{
    return m_replicatedObjects.size();
}

NetObject* NetState::GetById(uint32_t aId) const noexcept
{
    const auto it = m_replicatedObjects.find(aId);
    if (it != std::end(m_replicatedObjects))
        return it->second;
        
    return nullptr;
}

void NetState::OnCreate(NetObject* apObject)
{
    if (apObject->NeedsReplication())
        m_newReplicatedObjects.push_back(apObject);
}

void NetState::OnDelete(NetObject* apObject)
{
    if (apObject->NeedsReplication()) [[likely]]
    {
        if (m_replicatedObjects.erase(apObject->GetId()) > 0) [[likely]]
        {
            // Only mark for deletion sync if the object was replicated
            m_deletedReplicatedObjects.push_back(apObject->GetId());
            return;
        }

        // Pop new object if it has been processed yet
        const auto it = std::find(std::begin(m_newReplicatedObjects), std::end(m_newReplicatedObjects), apObject);
        if(it != std::end(m_newReplicatedObjects))
        {
            std::iter_swap(it, std::end(m_newReplicatedObjects) - 1);
            m_newReplicatedObjects.pop_back();
        }
    }
}

uint32_t NetState::GenerateNetId() noexcept
{
    return m_netId++;
}

void NetState::Reset() noexcept
{
    m_objectId = 1;
    m_netId = 1;

    m_remoteObjects.clear();
    m_deletedReplicatedObjects.clear();
    m_newReplicatedObjects.clear();
    m_replicatedObjects.clear();
    m_replicatedScripts.clear();
}

void NetState::SerializeFullObject(TiltedPhoques::Buffer::Writer& aWriter, NetObject* apObject, bool aMarkClean)
{
    Vector<const NetProperty*> properties;

    apObject->GetProperties().Visit([&properties, aMarkClean](const NetProperty* apProperty)
        {
            if (apProperty->IsNotDefault())
            {
                properties.push_back(apProperty);
            }
        });

    const uint32_t id = apObject->GetId();
    const uint32_t parentId = apObject->GetParentId();
    const uint16_t propertyCount = properties.size();

    Serialization::WriteVarInt(aWriter, id);
    Serialization::WriteVarInt(aWriter, parentId);
    Serialization::WriteString(aWriter, apObject->GetDefinition().GetNamespace().c_str());
    Serialization::WriteVarInt(aWriter, apObject->GetDefinition().GetId());
    Serialization::WriteVarInt(aWriter, propertyCount);

    for (auto pProperty : properties)
    {
        Serialization::WriteVarInt(aWriter, pProperty->GetId());
        pProperty->Serialize(aWriter);

        if(aMarkClean)
            pProperty->MarkDirty(false);
    }
}

bool NetState::SerializeUpdateObject(TiltedPhoques::Buffer::Writer& aWriter, NetObject* apObject)
{
    Vector<const NetProperty*> properties;
    Vector<NetRPCs::Call> calls;

    apObject->GetProperties().Visit([&properties](const NetProperty* apProperty)
        {
            if (apProperty->IsDirty())
            {
                properties.push_back(apProperty);
            }
        });

    apObject->GetRPCs().Process([&calls](const NetRPCs::Call& aCall)
        {
            calls.push_back(aCall);
        });

    const uint32_t id = apObject->GetId();
    const uint16_t propertyCount = properties.size();
    const uint16_t callCount = calls.size();

    if (propertyCount || callCount)
    {
        Serialization::WriteVarInt(aWriter, kUpdate);
        Serialization::WriteVarInt(aWriter, id);
        Serialization::WriteVarInt(aWriter, propertyCount);
        Serialization::WriteVarInt(aWriter, callCount);

        for (auto pProperty : properties)
        {
            Serialization::WriteVarInt(aWriter, pProperty->GetId());
            pProperty->Serialize(aWriter);
            pProperty->MarkDirty(false);
        }

        for (auto& call : calls)
        {
            call.Serialize(aWriter);
        }

        return true;
    }

    return false;
}

void NetState::DeserializeUpdate(TiltedPhoques::Buffer::Reader& aReader)
{
    const auto id = Serialization::ReadVarInt(aReader);
    const auto propertyCount = Serialization::ReadVarInt(aReader);
    const auto callCount = Serialization::ReadVarInt(aReader);

    auto pObject = GetById(id);
    if(!pObject)
    {
        assert("Object did not exist !");
        return;
    }

    for(auto i = 0; i < propertyCount; ++i)
    {
        const auto propId = Serialization::ReadVarInt(aReader);
        auto pProperty = pObject->GetProperties().GetById(propId);
        if(!pProperty)
        {
            assert("Property did not exist !");
            return;
        }

        pProperty->Deserialize(aReader);
    }

    for(auto i = 0; i < callCount; ++i)
    {
        NetRPCs::Call call;
        call.Deserialize(aReader);

        auto result = pObject->GetRPCs().Execute(call);
        TP_UNUSED(result);
    }
}
