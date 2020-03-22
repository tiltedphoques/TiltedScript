#pragma once

#include <NetProperty.h>

struct NetObjectDefinition;
struct NetObject;

struct NetProperties
{
    NetProperties(NetObjectDefinition& aNetObject);
    ~NetProperties() noexcept = default;

    NetProperties(const NetProperties&) = default;
    NetProperties& operator=(const NetProperties&) = default;

    sol::object Get(const std::string& aKey, sol::this_state aState);
    void Set(const std::string& key, sol::stack_object value, sol::this_state aState);

    [[nodiscard]] bool HasReplicatedProperties() const noexcept;

    void Visit(const std::function<void(const NetProperty*)>& acFunctor) const noexcept;
    NetProperty* GetById(uint32_t aId) noexcept;

private:

    friend struct NetObject;

    // We store in a flat array and a lookup map to avoid iterating over one or the other depending on the key we want
    Vector<NetProperty> m_replicatedProperties;
    Map<std::string, NetProperty*> m_replicatedPropertiesNameLookup;
    // These do not have an id so just store in a map
    Map<std::string, sol::object> m_localProperties;
};
