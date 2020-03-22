#include <NetObjectDefinition.h>
#include <NetProperty.h>
#include <NetObject.h>

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
    return m_dirty == true;
}

bool NetProperty::IsNotDefault() const noexcept
{
    return (m_changedOnce == true);
}

void NetProperty::MarkDirty(bool aDirty) const
{
    m_dirty = aDirty;
    m_changedOnce = true;
}

void NetProperty::Update(double aNumber, const SharedPtr<NetObject>& aSelf)
{
    const auto type = m_object.get_type();

    if (type == sol::type::number) [[likely]]
    {
        auto& definition = aSelf->GetDefinition();
        auto& propertyDef = definition.GetReplicatedProperty(GetId());

        m_object = sol::make_object(m_object.lua_state(), aNumber);

        if (propertyDef.OnRep.valid())
            TP_UNUSED(propertyDef.OnRep(aSelf));
    }
    /*else
        spdlog::error("Bad update type {} {}", static_cast<int>(type), static_cast<int>(sol::type::number));
    */
}

void NetProperty::Update(const std::string& acString, const SharedPtr<NetObject>& aSelf)
{
    const auto type = m_object.get_type();

    if (type == sol::type::string) [[likely]]
    {
        auto& definition = aSelf->GetDefinition();
        auto& propertyDef = definition.GetReplicatedProperty(GetId());

        m_object = sol::make_object(m_object.lua_state(), acString);

        if (propertyDef.OnRep.valid())
            TP_UNUSED(propertyDef.OnRep(aSelf));
    }
    /*else
        spdlog::error("Bad update type {} {}", static_cast<int>(type), static_cast<int>(sol::type::number));
    */
}

void NetProperty::Update(bool aBoolean, const SharedPtr<NetObject>& aSelf)
{
    const auto type = m_object.get_type();

    if (type == sol::type::boolean) [[likely]]
    {
        auto& definition = aSelf->GetDefinition();
        auto& propertyDef = definition.GetReplicatedProperty(GetId());

        m_object = sol::make_object(m_object.lua_state(), aBoolean);

        if (propertyDef.OnRep.valid())
            TP_UNUSED(propertyDef.OnRep(aSelf));
    }
    /*else
        spdlog::error("Bad update type {} {}", static_cast<int>(type), static_cast<int>(sol::type::number));
    */
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
