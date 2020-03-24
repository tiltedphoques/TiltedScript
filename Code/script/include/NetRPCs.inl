#include "NetObject.h"

template<class T>
void NetRPCs::Process(const T& aFunctor)
{
    if (m_calls.empty())
        return;

    const auto isAuthority = m_parent.GetDefinition().GetParentState()->IsAuthority();

    if (isAuthority)
    {
        for (auto& call : m_calls)
        {
            if (Execute(call))
                aFunctor(call);
        }
    }
    else
    {
        for (auto& call : m_calls)
        {
            aFunctor(call);
        }
    }

    m_calls.clear();
}
