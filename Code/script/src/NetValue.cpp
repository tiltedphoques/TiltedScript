#include <NetValue.h>
#include <Serialization.hpp>

using TiltedPhoques::Serialization;

NetValue::NetValue()
    : NetValueParent()
{
}

NetValue::NetValue(sol::object aObject)
{
    FromObject(aObject);
}

NetValue::NetValue(double aValue)
    : NetValueParent(aValue)
{
}

NetValue::NetValue(std::string aValue)
    : NetValueParent(aValue)
{
}

NetValue::NetValue(bool aValue)
    : NetValueParent(aValue)
{
}

void NetValue::Serialize(Buffer::Writer& aWriter) const noexcept
{
    const NetValueParent& parent = *this;

    std::visit([&aWriter](auto&& arg)
    {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::string>)
        {
            Serialization::WriteString(aWriter, arg);
        }
        else if constexpr (std::is_same_v<T, double>)
        {
            Serialization::WriteDouble(aWriter, arg);
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
            Serialization::WriteVarInt(aWriter, arg);
        }
    }, parent);
}

void NetValue::Deserialize(Buffer::Reader& aReader) noexcept
{
    NetValueParent& parent = *this;

    std::visit([&aReader, this](auto& arg)
    {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::string>)
        {
            arg = Serialization::ReadString(aReader);
        }
        else if constexpr (std::is_same_v<T, double>)
        {
            arg = Serialization::ReadDouble(aReader);
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
            arg = Serialization::ReadBool(aReader);
        }
    }, parent);
}

void NetValue::SerializeFull(TiltedPhoques::Buffer::Writer& aWriter) const noexcept
{
    const NetValueParent& parent = *this;

    // Use 7 bits to describe the type so that we get nice optimizations with booleans
    std::visit([&aWriter](auto&& arg)
        {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::string>)
            {
                aWriter.WriteBits(0, 7);
                Serialization::WriteString(aWriter, arg);
            }
            else if constexpr (std::is_same_v<T, double>)
            {
                aWriter.WriteBits(1, 7);
                Serialization::WriteDouble(aWriter, arg);
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
                aWriter.WriteBits(2, 7);
                Serialization::WriteVarInt(aWriter, arg);
            }
        }, parent);
}

void NetValue::DeserializeFull(TiltedPhoques::Buffer::Reader& aReader) noexcept
{
    auto id = Serialization::ReadVarInt(aReader);
    switch(id)
    {
    case 0:
        *this = Serialization::ReadString(aReader);
        break;
    case 1:
        *this = Serialization::ReadDouble(aReader);
        break;
    case 2:
        *this = Serialization::ReadBool(aReader);
        break;
    default:
        break;
    }
}

void NetValue::FromObject(sol::object aObject) noexcept
{
    const auto cType = aObject.get_type();

    switch(cType)
    {
    case sol::type::number:
        *this = aObject.as<double>(); break;
    case sol::type::string:
        *this = aObject.as<std::string>(); break;
    case sol::type::boolean:
        *this = aObject.as<bool>(); break;
    default:
        break;
    }
}

sol::object NetValue::AsObject(sol::this_state aState) const noexcept
{
    const NetValueParent& parent = *this;

    auto obj = std::visit([aState](auto&& arg)
    {
        sol::state_view lua(aState);

        using T = std::decay_t<decltype(arg)>;

        return sol::object(lua, sol::in_place_type<T>, arg);
    }, parent);

    return obj;
}