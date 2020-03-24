#pragma once

#include <Buffer.hpp>
#include <variant>

// Bool must be index 0 for microsoft not to fuck up
using NetValueParent = std::variant<bool, std::string, double>;

struct NetValue : NetValueParent
{
    enum Type
    {
        kNumber,
        kBoolean,
        kString,
        kCount
    };

    NetValue();
    explicit NetValue(sol::object aObject);
    NetValue(double aValue);
    NetValue(std::string aValue);
    explicit NetValue(bool aValue);
    ~NetValue() = default;

    NetValue(const NetValue&) = default;
    NetValue(NetValue&&) = default;
    NetValue& operator=(const NetValue&) = default;
    NetValue& operator=(NetValue&&) = default;

    void Serialize(TiltedPhoques::Buffer::Writer& aWriter) const noexcept;
    void Deserialize(TiltedPhoques::Buffer::Reader& aReader) noexcept;

    void SerializeFull(TiltedPhoques::Buffer::Writer& aWriter) const noexcept;
    void DeserializeFull(TiltedPhoques::Buffer::Reader& aReader) noexcept;

    void FromObject(sol::object aObject) noexcept;
    [[nodiscard]] sol::object AsObject(sol::this_state aState) const noexcept;
};
