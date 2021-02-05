#include <NetObjectDefinition.h>
#include <NetState.h>
#include <NetRPCs.h>
#include <TiltedCore/Serialization.hpp>
#include <iostream>

using TiltedPhoques::Serialization;

void NetRPCs::Call::Serialize(TiltedPhoques::Buffer::Writer& aWriter) const
{
    Serialization::WriteVarInt(aWriter, RpcId);
    Serialization::WriteVarInt(aWriter, Args.size());
    for(auto& arg : Args)
    {
        arg.SerializeFull(aWriter);
    }
}

void NetRPCs::Call::Deserialize(TiltedPhoques::Buffer::Reader& aReader)
{
    RpcId = Serialization::ReadVarInt(aReader);
    const auto count = Serialization::ReadVarInt(aReader);
    Args.resize(count);
    for(auto i = 0 ; i < count ; ++i)
    {
        Args[i].DeserializeFull(aReader);
    }
}

NetRPCs::NetRPCs(NetObject& aNetObject)
    : m_parent(aNetObject)
{
    
}

sol::object NetRPCs::Get(const std::string& aKey, sol::this_state aState)
{
    auto& definition = m_parent.GetDefinition();
    auto& remoteProcedures = definition.GetRemoteProcedures();

    auto it = remoteProcedures.find(aKey);
    if (it == std::end(remoteProcedures)) [[unlikely]]
        return sol::lua_nil;

    auto id = it->second.Id;
    auto name = it->second.Name;

    auto CallFunc = [this, id, name](sol::variadic_args args) { return HandleCall(id, name, args); };

    return sol::make_object(aState.L, CallFunc);
}

uint32_t NetRPCs::Size() const noexcept
{
    return m_calls.size();
}

bool NetRPCs::Execute(const Call& aCall) const noexcept
{
    auto& definition = m_parent.GetDefinition();
    auto& remoteProcedures = definition.GetRemoteProcedures();

    for(auto& kvp : remoteProcedures)
    {
        auto OnCall = kvp.second.OnCall;

        if(kvp.second.Id == aCall.RpcId && OnCall.valid())
        {
            TiltedPhoques::Vector<NetValueParent> args;
            for(auto& arg : aCall.Args)
            {
                args.emplace_back(arg);
            }

            auto result = OnCall(m_parent.shared_from_this(), sol::as_args(args));
            if (result.valid())
            {
                const auto ret = result.get<std::optional<bool>>();
                if (ret)
                    return *ret;
            }
            else
            {
                sol::error err = result;
                std::ostringstream oss;
                oss << "Error calling RPC " << kvp.first << " with " << err.what();
                std::cout << oss.str() << std::endl;
            }
        }
    }

    return false;
}

void NetRPCs::Queue(Call aCall) noexcept
{
    m_calls.emplace_back(std::move(aCall));
}

void NetRPCs::HandleCall(uint32_t aRpcId, const String& aName, sol::variadic_args aArgs)
{
    TP_UNUSED(aName);

    auto& call = m_calls.emplace_back();
    call.RpcId = aRpcId;
    call.Args.reserve(aArgs.size());

    for(auto arg : aArgs)
    {
        switch(arg.get_type())
        {
        case sol::type::number:
            call.Args.push_back(arg.as<double>());
            break;
        case sol::type::string:
            call.Args.push_back(arg.as<std::string>());
            break;
        case sol::type::boolean:
            call.Args.push_back(arg.as<bool>());
            break;
        default:
            break;
        }
    }
}
