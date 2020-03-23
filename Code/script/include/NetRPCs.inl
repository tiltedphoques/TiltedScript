template<class T>
void NetRPCs::Process(const T& aFunctor)
{
    for (auto& call : m_calls)
    {
        if(Execute(call))
            aFunctor(call);
    }

    m_calls.clear();
}
