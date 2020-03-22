#include <NetObjectDefinition.h>
#include <NetRPCs.h>

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

void NetRPCs::Visit(const std::function<void(Call*)>& aFunctor)
{
    for(auto& call : m_calls)
    {
        aFunctor(&call);
    }

    m_calls.clear();
}

bool NetRPCs::Execute(Call* apCall) noexcept
{
    auto& definition = m_parent.GetDefinition();
    auto& remoteProcedures = definition.GetRemoteProcedures();

    for(auto& kvp : remoteProcedures)
    {
        if(kvp.second.Id == apCall->RpcId && kvp.second.OnCall.valid())
        {
            auto result = kvp.second.OnCall(m_parent.shared_from_this(), sol::as_args(apCall->Args));
            if (result.valid())
            {
                const auto ret = result.get<std::optional<bool>>();
                if (ret)
                    return *ret;
            }
            /*
            else
            {
                sol::error err = result;
                spdlog::error("Error calling RPC {} with {}", kvp.first, err.what());
            }
            */
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
        }
    }
}
