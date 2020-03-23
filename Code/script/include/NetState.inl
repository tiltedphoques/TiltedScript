template<class T>
void NetState::Visit(const T& acFunctor)
{
    for (auto& kvp : m_replicatedObjects)
    {
        acFunctor(kvp.second);
    }
}

template<class T>
void NetState::ProcessNewObjects(const T& acFunctor)
{
    auto it = std::begin(m_newReplicatedObjects);
    while (it != std::end(m_newReplicatedObjects) && !m_newReplicatedObjects.empty())
    {
        const auto pObject = *it;

        acFunctor(pObject);

        if (pObject->IsReplicated())
        {
            m_replicatedObjects[pObject->GetId()] = pObject;

            if(m_newReplicatedObjects.size() == 1)
            {
                m_newReplicatedObjects.clear();
                it = std::end(m_newReplicatedObjects);
            }
            else
            {
                std::iter_swap(it, std::end(m_newReplicatedObjects) - 1);
                m_newReplicatedObjects.pop_back();
            }
        }
        else
            ++it;
    }
}

template<class T>
void NetState::ProcessDeletedObjects(const T& acFunctor)
{
    for (auto id : m_deletedReplicatedObjects)
    {
        acFunctor(id);
    }

    m_deletedReplicatedObjects.clear();
}
