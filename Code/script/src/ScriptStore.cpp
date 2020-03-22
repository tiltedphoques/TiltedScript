#include "ScriptStore.h"
#include "StaticFunctions.h"

ScriptStore::ScriptStore(bool aIsAuthority)
    : m_authority(aIsAuthority)
{}

ScriptStore::~ScriptStore() noexcept
{
    m_replicatedObjects.clear();
    m_contexts.clear();
}

void ScriptStore::LoadFullScripts(const std::filesystem::path& acBasePath) noexcept
{
    if (!is_directory(acBasePath))
        create_directory(acBasePath);

    if (!std::filesystem::is_directory("db"))
        std::filesystem::create_directory("db");

    for (auto& file : std::filesystem::directory_iterator(acBasePath))
    {
        if (!file.is_directory() && file.path().extension() == ".lua")
        {
            const auto modName = file.path().filename();

            const auto pContext = CreateContext(String(modName.stem().u8string()));

            auto registry = pContext->registry();
            registry["PLUGIN_NAME"] = modName.stem().string();

            // spdlog::info("Loading mod {}...", modName.string());

            if (m_authority)
                LoadNetObjects(acBasePath, pContext);

            auto result = pContext->LoadScript(file.path());

            /*
            if (!result.first)
            {
                spdlog::error("Failed to execute initialization file!\n{}", modName.string(), result.second.c_str());
            }
            else
            {
                spdlog::info("\t> Initialization executed", modName.string());
            }
            */
        }
    }
}

ScriptContext* ScriptStore::CreateContext(const String& acNamespace) noexcept
{
    auto it = m_contexts.find(acNamespace);
    if(it != std::end(m_contexts))
    {
        // spdlog::error("Creating a script context with namespace {} but already exists !", acNamespace);
        return it->second.get();
    }

    const auto result = m_contexts.insert(std::make_pair(acNamespace, MakeUnique<ScriptContext>(acNamespace, IsAuthority(), this)));
    it = result.first;
    if (it == std::end(m_contexts))
        return nullptr;

    auto& ctx = it->second;

    RegisterExtensions(*ctx);

    return ctx.get();
}

void ScriptStore::VisitNetObjects(const std::function<void(NetObject*)>& acFunctor)
{
    for (auto& kvp : m_replicatedObjects)
    {
        acFunctor(kvp.second);
    }
}

void ScriptStore::VisitNewNetObjects(const std::function<void(NetObject*)>& acFunctor)
{
    auto it = std::begin(m_newReplicatedObjects);
    while (it != std::end(m_newReplicatedObjects))
    {
        const auto pObject = *it;

        acFunctor(pObject);

        if (pObject->IsReplicated())
        {
            m_replicatedObjects[pObject->GetId()] = pObject;

            // Remove self from the vector
            std::iter_swap(it, std::end(m_newReplicatedObjects) - 1);
            m_newReplicatedObjects.pop_back();
        }
        else
            ++it;
    }
}

void ScriptStore::VisitDeletedObjects(const std::function<void(uint32_t)>& acFunctor)
{
    for(auto id : m_deletedReplicatedObjects)
    {
        acFunctor(id);
    }

    m_deletedReplicatedObjects.clear();
}

NetObject* ScriptStore::GetById(uint32_t aId) const noexcept
{
    const auto it = m_replicatedObjects.find(aId);
    if (it != std::end(m_replicatedObjects))
        return it->second;
        
    return nullptr;
}

void ScriptStore::OnCreate(NetObject* apObject)
{
    if (apObject->NeedsReplication())
        m_newReplicatedObjects.push_back(apObject);
}

void ScriptStore::OnDelete(NetObject* apObject)
{
    if (apObject->NeedsReplication()) [[likely]]
    {
        if (m_replicatedObjects.erase(apObject->GetId()) > 0) [[likely]]
        {
            // Only mark for deletion sync if the object was replicated
            m_deletedReplicatedObjects.push_back(apObject->GetId());
            return;
        }

        // Pop new object if it has been processed yet
        const auto it = std::find(std::begin(m_newReplicatedObjects), std::end(m_newReplicatedObjects), apObject);
        if(it != std::end(m_newReplicatedObjects))
        {
            std::iter_swap(it, std::end(m_newReplicatedObjects) - 1);
            m_newReplicatedObjects.pop_back();
        }
    }

    
}

void ScriptStore::Reset() noexcept
{
    m_replicatedObjects.clear();
    m_newReplicatedObjects.clear();
    m_deletedReplicatedObjects.clear();
    m_replicatedScripts.clear();
    m_contexts.clear();

    m_objectId = 1;
    m_netId = 1;
}

void ScriptStore::RegisterExtensions(ScriptContext& aContext)
{
    using TVec3 = Vector3<float>;

    auto vector3Type = aContext.new_usertype<TVec3>("Vec3", sol::constructors<TVec3(), TVec3(float, float, float), TVec3(const TVec3&)>(),
        sol::meta_function::addition, &TVec3::operator+,
        sol::meta_function::subtraction, &TVec3::operator-,
        sol::meta_function::multiplication, &TVec3::operator*,
        sol::meta_function::to_string, [](TVec3& v) { return std::string("x: ") + std::to_string(v.m_x) + " y: " + std::to_string(v.m_y) + " z: " + std::to_string(v.m_z); },
        "length", &TVec3::Length);

    vector3Type["x"] = &TVec3::m_x;
    vector3Type["y"] = &TVec3::m_y;
    vector3Type["z"] = &TVec3::m_z;

    aContext.set_function("print", Lua::Print);
}

void ScriptStore::LoadNetObjects(const std::filesystem::path& acBasePath, ScriptContext* apContext) noexcept
{
    const auto netObjectsPath = acBasePath / apContext->GetNamespace() / "net_objects";

    if (!is_directory(netObjectsPath))
        return;

    for (auto& netFile : std::filesystem::recursive_directory_iterator(netObjectsPath))
    {
        if (!netFile.is_directory() && netFile.path().extension() == ".lua")
        {
            const auto result = apContext->LoadNetworkObject(netFile.path());
            if (!result)
            {
                // spdlog::error("\t> Failed to load network object {}\n{}", netFile.path().string(), result.GetError().c_str());
                continue;
            }

            if (!IsAuthority())
                continue;

            auto& scriptVector = m_replicatedScripts[apContext->GetNamespace()];
            auto& replicatedObject = scriptVector.emplace_back();

            replicatedObject.Id = m_objectId++;
            replicatedObject.Filename = netFile.path().lexically_relative(acBasePath).string();
            replicatedObject.Content = std::move(LoadFile(netFile));
        }
    }
}
