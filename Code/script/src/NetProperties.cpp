#include <NetObjectDefinition.h>
#include <NetProperties.h>

NetProperties::NetProperties(NetObjectDefinition& aNetObject)
{
    uint32_t id = 0;

    m_replicatedProperties.reserve(aNetObject.m_replicatedProperties.size());

    for (auto& entry : aNetObject.m_replicatedProperties)
    {
        auto& property = m_replicatedProperties.emplace_back(id++, entry.Default);

        m_replicatedPropertiesNameLookup.emplace(entry.Name, &property);
    }

    for (auto& entry : aNetObject.m_localProperties)
    {
        m_localProperties.emplace(entry.Name, entry.Default);
    }
}

sol::object NetProperties::Get(const std::string& aKey, sol::this_state aState)
{
    {
        const auto it = m_replicatedPropertiesNameLookup.find(aKey);
        if (it != std::end(m_replicatedPropertiesNameLookup))
        {
            return it->second->Get();
        }
    }

    {
        const auto it = m_localProperties.find(aKey);
        if (it != std::end(m_localProperties))
        {
            return it->second;
        }
    }

    return make_object(aState.L, sol::nil);
}

void NetProperties::Set(const std::string& aKey, sol::stack_object aValue, sol::this_state)
{
    {
        auto it = m_replicatedPropertiesNameLookup.find(aKey);
        if (it != std::end(m_replicatedPropertiesNameLookup))
        {
            const sol::object value = aValue;
            it->second->Set(value);
            return;
        }
    }

    {
        auto it = m_localProperties.find(aKey);
        if (it != std::end(m_localProperties))
        {
            const sol::object value = aValue;
            it->second = value;
        }
    }
}

bool NetProperties::HasReplicatedProperties() const noexcept
{
    return !m_replicatedProperties.empty();
}

void NetProperties::Visit(const std::function<void(const NetProperty*)>& acFunctor) const noexcept
{
    for(auto& entry : m_replicatedProperties)
    {
        acFunctor(&entry);
    }
}

NetProperty* NetProperties::GetById(uint32_t aId) noexcept
{
    if (m_replicatedProperties.size() > aId) [[likely]]
        return &m_replicatedProperties[aId];

    return nullptr;
}

