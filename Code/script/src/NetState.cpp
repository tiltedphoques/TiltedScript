#include "NetState.h"
#include "NetObject.h"

NetState::NetState(bool aIsAuthority)
    : m_authority(aIsAuthority)
{}

NetState::~NetState() noexcept
{
    m_replicatedObjects.clear();
}

void NetState::GenerateFullSnapshot(TiltedPhoques::Buffer::Writer& aWriter)
{
    Visit([&aWriter](NetObject* apObject)
    {
        TiltedPhoques::StackAllocator<1 << 14> stackAllocator;

        TiltedPhoques::Allocator::Push(stackAllocator);
        TiltedPhoques::Vector<const NetProperty*> properties;
        TiltedPhoques::Allocator::Pop();

        apObject->GetProperties().Visit([&properties](const NetProperty* apProperty)
        {
            if (apProperty->IsNotDefault())
            {
                properties.push_back(apProperty);
            }
        });

        const uint32_t id = apObject->GetId();
        const uint16_t propertyCount = properties.size();

        aWriter.WriteBytes(reinterpret_cast<const uint8_t*>(&id), sizeof(id));
        aWriter.WriteBytes(reinterpret_cast<const uint8_t*>(&propertyCount), sizeof(propertyCount));

        for(auto pProperty : properties)
        {
            pProperty->Serialize(aWriter);
        }
    });
}

void NetState::GenerateDifferentialSnapshot(TiltedPhoques::Buffer::Writer& aWriter)
{
    ProcessNewObjects([this](NetObject* apObject)
    {
        apObject->SetId(GenerateNetId());
    });

    /*Visit([&aWriter](NetObject* apObject)
    {
        TiltedPhoques::StackAllocator<1 << 16> stackAllocator;

        TiltedPhoques::Allocator::Push(stackAllocator);
        TiltedPhoques::Vector<const NetProperty*> properties;
        TiltedPhoques::Vector<NetRPCs::Call> calls;
        TiltedPhoques::Allocator::Pop();

        apObject->GetProperties().Visit([&properties](const NetProperty* apProperty)
        {
            if (apProperty->IsDirty())
            {
                properties.push_back(apProperty);

                apProperty->MarkDirty(false);
            }
        });

        apObject->GetRPCs().Process([&calls](const NetRPCs::Call& aCall)
        {
            calls.push_back(aCall);
        });

        const auto callSize = apObject->GetRPCs().Size();

        if (!properties.empty() || !calls.empty())
        {
            const uint32_t id = apObject->GetId();
            const uint16_t propertyCount = properties.size() & 0xFFFF;
            const uint16_t callCount = calls.size() & 0xFFFF;

            aWriter.WriteBytes(reinterpret_cast<const uint8_t*>(&id), sizeof(id));
            aWriter.WriteBytes(reinterpret_cast<const uint8_t*>(&propertyCount), sizeof(propertyCount));
            aWriter.WriteBytes(reinterpret_cast<const uint8_t*>(&callCount), sizeof(callCount));

            for (auto pProperty : properties)
            {
                pProperty->Serialize(aWriter);
            }

            for (auto& call : calls)
            {
                call.Serialize(aWriter);
            }
        }
    });*/
}

bool NetState::IsAuthority() const noexcept
{
    return m_authority;
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
