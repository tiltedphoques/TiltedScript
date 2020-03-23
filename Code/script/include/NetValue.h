#pragma once

#include <Buffer.hpp>
#include <variant>

using NetValueParent = std::variant<double, std::string, bool>;

struct NetValue : NetValueParent
{
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

    void FromObject(sol::object aObject) noexcept;
    [[nodiscard]] sol::object AsObject(sol::this_state aState) const noexcept;
};

// GCC is such an unbelievable piece of crap, its std::variant is broken so we must inject stuff in std ! great !
namespace std
{
    template<>
    struct variant_size<NetValue> : variant_size<NetValueParent> {
    };

    template<std::size_t I>
    struct variant_alternative<I, NetValue> : variant_alternative<I, NetValueParent> {
    };
}
