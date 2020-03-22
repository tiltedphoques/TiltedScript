#pragma once

#include <NetProperties.h>
#include <NetRPCs.h>

struct NetObjectDefinition;

struct NetObject : std::enable_shared_from_this<NetObject>
{
    using Pointer = TiltedPhoques::SharedPtr<NetObject>;

    struct IListener
    {
        virtual ~IListener() = default;
        virtual void OnCreate(NetObject* apObject) = 0;
        virtual void OnDelete(NetObject* apObject) = 0;
    };

    NetObject(NetObjectDefinition& aNetObjectDefinition);
    ~NetObject();

    void SetId(uint32_t aId) noexcept;
    [[nodiscard]] uint32_t GetId() const noexcept;

    // Returns true if the parent is assigned, otherwise it cannot be assigned
    [[nodiscard]] bool SetParentId(uint32_t aParentId);
    [[nodiscard]] uint32_t GetParentId() const;

    [[nodiscard]] bool IsReplicated() const noexcept;
    [[nodiscard]] bool NeedsReplication() noexcept;

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
    uint32_t m_id;
    uint32_t m_parentId;
};
