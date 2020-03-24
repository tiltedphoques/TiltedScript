#include <NetObject.h>
#include <NetState.h>
#include <NetObjectDefinition.h>

NetObject::NetObject(NetObjectDefinition& aNetObjectDefinition)
    : m_netObjectDefinition(aNetObjectDefinition)
    , m_properties(aNetObjectDefinition)
    , m_remoteProcedures(*this)
    , m_metaTable(aNetObjectDefinition.GetDefaultTable())
    , m_id(0)
    , m_parentId(std::numeric_limits<decltype(m_parentId)>::max())
{
    aNetObjectDefinition.GetParentState()->OnCreate(this);
}

NetObject::~NetObject()
{
    m_netObjectDefinition.GetParentState()->OnDelete(this);
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

void NetObject::SetOwnerId(uint32_t aOwnerId)
{
    m_ownerId = aOwnerId;
}

uint32_t NetObject::GetOwnerId() const
{
    return m_ownerId;
}

bool NetObject::IsReplicated() const noexcept
{
    return GetId() != 0;
}

bool NetObject::NeedsReplication() noexcept
{
    return GetProperties().HasReplicatedProperties();
}

sol::object NetObject::Get(const std::string& aKey, sol::this_state aState)
{
    if (aKey == "Properties") return sol::make_object(aState.L, GetProperties());
    if (aKey == "NetRPCs") return sol::make_object(aState.L, &GetRPCs());

    return m_metaTable[aKey];
}

void NetObject::Set(const std::string& aKey, sol::stack_object value, sol::this_state aState)
{
    if (aKey == "Properties" || aKey == "NetRPCs") return;

    const sol::object obj = value;
    m_metaTable[aKey] = obj;
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
