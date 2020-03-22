#pragma once

struct NetObjectDefinition;
struct NetObject;

struct NetRPCs
{
    struct Call
    {
        uint32_t RpcId;
        Vector<std::variant<std::string, double, bool>> Args;
    };

    NetRPCs(NetObject& aNetObject);
    ~NetRPCs() noexcept = default;

    NetRPCs(const NetRPCs&) = default;
    NetRPCs& operator=(const NetRPCs&) = default;

    sol::object Get(const std::string& aKey, sol::this_state aState);

    void Visit(const std::function<void(Call*)>& aFunctor);

    bool Execute(Call* apCall) noexcept;

    void Queue(Call aCall) noexcept;

protected:

    void HandleCall(uint32_t aRpcId, const String& aName, sol::variadic_args aArgs);

private:

    NetObject& m_parent;
    Vector<Call> m_calls;
};
