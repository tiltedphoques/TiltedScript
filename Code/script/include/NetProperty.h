#pragma once

struct NetObject;

struct NetProperty
{
    NetProperty(uint32_t aId, sol::object aDefault);
    virtual ~NetProperty() = default;

    NetProperty(const NetProperty&) = default;
    NetProperty& operator=(const NetProperty&) = default;

    uint32_t GetId() const noexcept;
    bool IsDirty() const noexcept;
    bool IsNotDefault() const noexcept;
    void MarkDirty(bool aDirty) const;

    void Update(double aNumber, const TiltedPhoques::SharedPtr<NetObject>& aSelf);
    void Update(const std::string& acString, const TiltedPhoques::SharedPtr<NetObject>& aSelf);
    void Update(bool aBoolean, const TiltedPhoques::SharedPtr<NetObject>& aSelf);

    void Set(const sol::object& acObject) noexcept;
    sol::object Get() const noexcept;

private:

    uint32_t m_id : 30;
    mutable uint32_t m_changedOnce : 1;
    mutable uint32_t m_dirty : 1;
    sol::object m_object;
};
