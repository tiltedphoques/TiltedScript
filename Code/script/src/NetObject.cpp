#include <NetObject.h>
#include <NetObjectDefinition.h>

NetObject::NetObject(NetObjectDefinition& aNetObjectDefinition)
    : m_netObjectDefinition(aNetObjectDefinition)
    , m_properties(aNetObjectDefinition)
    , m_remoteProcedures(*this)
    , m_id(0)
    , m_parentId(std::numeric_limits<decltype(m_parentId)>::max())
{
    aNetObjectDefinition.GetListener()->OnCreate(this);
}

NetObject::~NetObject()
{
    m_netObjectDefinition.GetListener()->OnDelete(this);
}

void NetObject::SetId(uint32_t aId) noexcept
{
    if (m_id == 0) 
        m_id = aId;
}

uint32_t NetObject::GetId() const noexcept
{
    return m_id;
}

bool NetObject::SetParentId(uint32_t aParentId)
{
    // Can only set the parent before the object is replicated
    if (IsReplicated())
        return false;

    m_parentId = aParentId;

    return true;
}

uint32_t NetObject::GetParentId() const
{
    return m_parentId;
}

bool NetObject::IsReplicated() const noexcept
{
    return GetId() != 0;
}

bool NetObject::NeedsReplication() noexcept
{
    return GetProperties().HasReplicatedProperties();
}

NetProperties& NetObject::GetProperties() noexcept
{
    return m_properties;
}

NetRPCs& NetObject::GetRPCs() noexcept
{
    return m_remoteProcedures;
}

const NetObjectDefinition& NetObject::GetDefinition() const noexcept
{
    return m_netObjectDefinition;
}

NetObjectDefinition& NetObject::GetDefinition() noexcept
{
    return m_netObjectDefinition;
}
