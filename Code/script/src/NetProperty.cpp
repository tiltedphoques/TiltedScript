#include <TiltedScriptPCH.h>

#include <NetObjectDefinition.h>
#include <NetProperty.h>
#include <NetObject.h>
#include <NetState.h>

NetProperty::NetProperty(uint32_t aId, sol::object aDefault)
    : m_id(aId)
    , m_changedOnce(false)
    , m_dirty(false)
    , m_object(std::move(aDefault))
{
}

uint32_t NetProperty::GetId() const noexcept
{
    return m_id;
}

bool NetProperty::IsDirty() const noexcept
{
    return static_cast<bool>(m_dirty);
}

bool NetProperty::IsNotDefault() const noexcept
{
    return static_cast<bool>(m_changedOnce);
}

void NetProperty::MarkDirty(bool aDirty) const
{
    m_dirty = aDirty;
    m_changedOnce = true;
}

void NetProperty::Update(const NetValue& aValue, const TiltedPhoques::SharedPtr<NetObject>& aSelf)
{
    const auto type = m_object.get_type();

    const auto obj = aValue.AsObject(m_object.lua_state());

    if (obj.get_type() == type) [[likely]]
    {
        m_object = obj;

        if (aSelf)
        {
            auto& definition = aSelf->GetDefinition();
            auto& propertyDef = definition.GetReplicatedProperty(GetId());

            if (propertyDef.OnRep.valid())
                TP_UNUSED(propertyDef.OnRep(aSelf));
        }
    }
}

void NetProperty::Set(const sol::object& acObject) noexcept
{
    const auto cObjectType = acObject.get_type();
    const auto cCurrentObjectType = m_object.get_type();

    if (cObjectType != cCurrentObjectType)
        return;

    // Check if we need to replicate
    if(acObject != m_object)
    {
        MarkDirty(true);
    }
    // Set
    m_object = acObject;
}

sol::object NetProperty::Get() const noexcept
{
    return m_object;
}

void NetProperty::Serialize(TiltedPhoques::Buffer::Writer& aWriter) const noexcept
{
    const NetValue value(m_object);
    value.Serialize(aWriter);
}

void NetProperty::Deserialize(TiltedPhoques::Buffer::Reader& aReader) noexcept
{
    NetValue value(m_object);
    value.Deserialize(aReader);

    m_object = value.AsObject(m_object.lua_state());
}
