#include <TiltedScriptPCH.h>

#include "ScriptStore.h"

ScriptStore::ScriptStore(bool aIsAuthority)
    : m_authority(aIsAuthority),
      m_state(TiltedPhoques::MakeShared<NetState>(*this)) {}

ScriptStore::~ScriptStore() noexcept
{
}

uint32_t ScriptStore::LoadFullScripts(const std::filesystem::path& acBasePath) noexcept
{
    if (!is_directory(acBasePath))
        create_directory(acBasePath);

    if (!std::filesystem::is_directory("db"))
        std::filesystem::create_directory("db");

    uint32_t counter = 0;

    for (auto& file : std::filesystem::directory_iterator(acBasePath))
    {
        if (!file.is_directory() && file.path().extension() == ".lua")
        {
            const auto modName = file.path().filename();

            const auto pContext = CreateContext(String(modName.stem().string()));

            if (m_authority)
                LoadNetObjects(acBasePath, pContext);

            auto result = pContext->LoadScript(file.path());

            std::ostringstream oss;

            if (!result.first)
            {
                oss << "Failed to execute initialization " << modName << std::endl << result.second.c_str();

                LogError(oss.str());
            }
            else
            {
                oss << "Initialization executed " << modName;

                counter++;

                LogInfo(oss.str());
            }
        }
    }

    return counter;
}

ScriptContext* ScriptStore::CreateContext(const String& acNamespace) noexcept
{
    auto it = m_contexts.find(acNamespace);
    if(it != std::end(m_contexts))
    {
        std::ostringstream oss;
        oss << "Creating a script context with namespace " << acNamespace << " but already exists !";
        LogError(oss.str());

        return it->second.get();
    }

    auto netState = GetNetState();
    const auto result = m_contexts.insert(
        std::make_pair(acNamespace, TiltedPhoques::MakeUnique<ScriptContext>(
                                        acNamespace, IsAuthority(), netState)));
    it = result.first;
    if (it == std::end(m_contexts))
        return nullptr;

    auto& ctx = it->second;

    auto registry = ctx->registry();
    registry["PLUGIN_NAME"] = std::string(acNamespace.c_str());

    RegisterExtensions(*ctx);

    return ctx.get();
}

NetState::Pointer ScriptStore::GetNetState() noexcept
{
    return m_state;
}

void ScriptStore::Reset() noexcept
{
    m_state->Reset();
    m_contexts.clear();

    m_netId = 1;
}

void ScriptStore::RegisterExtensions(ScriptContext& aContext)
{
    static auto Print = [this](sol::this_state aState)
    {
        return this->Print(aState);
    };

    using TVec3 = TiltedPhoques::Vector3<float>;

    auto vector3Type = aContext.new_usertype<TVec3>("Vec3", sol::constructors<TVec3(), TVec3(float, float, float), TVec3(const TVec3&)>(),
        sol::meta_function::addition, &TVec3::operator+,
        sol::meta_function::subtraction, &TVec3::operator-,
        sol::meta_function::multiplication, &TVec3::operator*,
        sol::meta_function::to_string, [](TVec3& v) { return std::string("x: ") + std::to_string(v.m_x) + " y: " + std::to_string(v.m_y) + " z: " + std::to_string(v.m_z); },
        "length", &TVec3::Length);

    vector3Type["x"] = &TVec3::m_x;
    vector3Type["y"] = &TVec3::m_y;
    vector3Type["z"] = &TVec3::m_z;

    aContext.set_function("print", Print);
}

void ScriptStore::LogScript(const std::string& acLog)
{
    std::cout << acLog << std::endl;
}

void ScriptStore::LogInfo(const std::string& acLog)
{
    std::cout << acLog << std::endl;
}

void ScriptStore::LogError(const std::string& acLog)
{
    std::cerr << acLog << std::endl;
}

uint32_t ScriptStore::GenerateNetId() noexcept
{
    return m_netId++;
}

void ScriptStore::Print(sol::this_state aState)
{
    const sol::state_view state(aState);

    std::ostringstream oss;
    oss << "[script:" << state.registry()["PLUGIN_NAME"].get<std::string>() << "] ";

    const auto L = aState.lua_state();

    const auto count = lua_gettop(L);

    lua_getglobal(L, "tostring");

    for (auto i = 1; i <= count; i++)
    {
        lua_pushvalue(L, -1);
        lua_pushvalue(L, i);

        lua_call(L, 1, 1);

        size_t l;
        const char* pStr = lua_tolstring(L, -1, &l);

        if (pStr == nullptr)
            return;

        if (i > 1)
            oss << '\t';

        oss << pStr;

        lua_pop(L, 1);
    }

    LogScript(oss.str());
}

void ScriptStore::LoadNetObjects(const std::filesystem::path& acBasePath, ScriptContext* apContext) noexcept
{
    const auto netObjectsPath = acBasePath / apContext->GetNamespace() / "net_objects";

    const auto path = acBasePath.parent_path();

    if (!is_directory(netObjectsPath))
        return;

    for (auto& netFile : std::filesystem::recursive_directory_iterator(netObjectsPath))
    {
        if (!netFile.is_directory() && netFile.path().extension() == ".lua")
        {
            const auto result = apContext->LoadNetworkObject(netFile.path(), netFile.path().lexically_relative(path).string());
            if (!result)
            {
                std::ostringstream oss;
                oss << "Failed to load network object " << netFile.path() << std::endl << result.GetError().c_str();
                LogError(oss.str());

                continue;
            }
        }
    }
}
