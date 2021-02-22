#pragma once

#include <NetValue.h>

struct NetObject;

struct NetProperty final
{
    NetProperty(uint32_t aId, sol::object aDefault);
    ~NetProperty() = default;

    NetProperty(const NetProperty&) = default;
    NetProperty& operator=(const NetProperty&) = default;

    uint32_t GetId() const noexcept;
    bool IsDirty() const noexcept;
    bool IsNotDefault() const noexcept;
    void MarkDirty(bool aDirty) const;

    void Update(const NetValue& aValue, const TiltedPhoques::SharedPtr<NetObject>& aSelf);

    void Set(const sol::object& acObject) noexcept;
    sol::object Get() const noexcept;

    void Serialize(TiltedPhoques::Buffer::Writer& aWriter) const noexcept;
    void Deserialize(TiltedPhoques::Buffer::Reader& aReader) noexcept;

private:

    enum
    {
        IdBitSize = 30
    };

    uint32_t m_id : IdBitSize;
    mutable uint32_t m_changedOnce : 1;
    mutable uint32_t m_dirty : 1;
    sol::object m_object;
};
