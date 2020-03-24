#pragma once

#include <NetProperties.h>
#include <NetRPCs.h>

struct NetObjectDefinition;

struct NetObject : std::enable_shared_from_this<NetObject>
{
    using Pointer = TiltedPhoques::SharedPtr<NetObject>;

    NetObject(NetObjectDefinition& aNetObjectDefinition);
    ~NetObject();

    void SetId(uint32_t aId) noexcept;
    [[nodiscard]] uint32_t GetId() const noexcept;

    // Returns true if the parent is assigned, otherwise it cannot be assigned
    [[nodiscard]] bool SetParentId(uint32_t aParentId);
    [[nodiscard]] uint32_t GetParentId() const;

    [[nodiscard]] void SetOwnerId(uint32_t aOwnerId);
    [[nodiscard]] uint32_t GetOwnerId() const;

    [[nodiscard]] bool IsReplicated() const noexcept;
    [[nodiscard]] bool NeedsReplication() noexcept;

    sol::object Get(const std::string& aKey, sol::this_state aState);
    void Set(const std::string& key, sol::stack_object value, sol::this_state aState);

    [[nodiscard]] NetProperties& GetProperties() noexcept;
    [[nodiscard]] NetRPCs& GetRPCs() noexcept;
    [[nodiscard]] const NetObjectDefinition& GetDefinition() const noexcept;
    [[nodiscard]] NetObjectDefinition& GetDefinition() noexcept;

    TP_NOCOPYMOVE(NetObject);

private:

    friend struct NetProperties;

    NetObjectDefinition& m_netObjectDefinition;
    NetProperties m_properties;
    NetRPCs m_remoteProcedures;
    TiltedPhoques::Map<std::string, sol::object> m_metaTable;
    uint32_t m_id;
    uint32_t m_parentId;
    uint32_t m_ownerId;
};
