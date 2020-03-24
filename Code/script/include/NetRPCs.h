#pragma once

#include <NetValue.h>

struct NetObjectDefinition;
struct NetObject;

struct NetRPCs
{
    struct Call
    {
        uint32_t RpcId;
        TiltedPhoques::Vector<NetValue> Args;

        void Serialize(TiltedPhoques::Buffer::Writer& aWriter) const;
        void Deserialize(TiltedPhoques::Buffer::Reader& aReader);
    };

    NetRPCs(NetObject& aNetObject);
    ~NetRPCs() noexcept = default;

    NetRPCs(const NetRPCs&) = delete;
    NetRPCs& operator=(const NetRPCs&) = delete;

    sol::object Get(const std::string& aKey, sol::this_state aState);

    template<class T>
    void Process(const T& aFunctor);

    [[nodiscard]] uint32_t Size() const noexcept;
    [[nodiscard]] bool Execute(const Call& aCall) const noexcept;

    void Queue(Call aCall) noexcept;

protected:

    void HandleCall(uint32_t aRpcId, const TiltedPhoques::String& aName, sol::variadic_args aArgs);

private:

    NetObject& m_parent;
    TiltedPhoques::Vector<Call> m_calls;
};

#include "NetRPCs.inl"
