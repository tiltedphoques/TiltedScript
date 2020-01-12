premake.extensions.script = {}

function script_parent_path()
    local str = debug.getinfo(2, "S").source:sub(2)
    local dir =  str:match("(.*/)"):sub(0,-2)
    local index = string.find(dir, "/[^/]*$")
    return dir:sub(0, index)
end

function script_generate()
    if premake.extensions.script.generated == true then
        return
    end

    project ("Script")
        kind ("StaticLib")
        language ("C++")

        includedirs
        {
            premake.extensions.script.path .. "/Code/script/include/",
            premake.extensions.core.path .. "/Code/core/include/",
            premake.extensions.script.path .. "/ThirdParty/lua/",
            premake.extensions.script.path .. "/ThirdParty/sqlite3/",
        }

        files
        {
            premake.extensions.script.path .. "/Code/script/include/**.hpp",
            premake.extensions.script.path .. "/Code/script/include/**.h",
            premake.extensions.script.path .. "/Code/script/src/**.cpp",
            premake.extensions.script.path .. "/Code/script/src/**.c",
        }

        links
        {
            "Lua",
            "sqlite3"
        }

    premake.extensions.script.generated = true
end

function script_lua_generate()
    if premake.extensions.script.lua_generated == true then
        return
    end

    project ("Lua")
        kind ("StaticLib")
        language ("C")

        includedirs
        {
            premake.extensions.script.path .. "/ThirdParty/lua/",
        }

        files
        {
            premake.extensions.script.path .. "/ThirdParty/lua/**.h",
            premake.extensions.script.path .. "/ThirdParty/lua/**.c",
        }

    premake.extensions.script.lua_generated = true
end

function script_sqlite3_generate()
    if premake.extensions.script.sqlite3_generated == true then
        return
    end

    project ("sqlite3")
        kind ("StaticLib")
        language ("C")

        defines
        {
            "SQLITE_OMIT_LOAD_EXTENSION"
        }

        includedirs
        {
            premake.extensions.script.path .. "/ThirdParty/sqlite3/",
        }

        files
        {
            premake.extensions.script.path .. "/ThirdParty/sqlite3/**.h",
            premake.extensions.script.path .. "/ThirdParty/sqlite3/**.c",
        }

    premake.extensions.script.sqlite3_generated = true
end

function script_generate_all()
    
    group ("Libraries")
        script_generate()

    group ("ThirdParty")
        script_lua_generate()
        script_sqlite3_generate()

end

premake.extensions.script.path = script_parent_path()
premake.extensions.script.generate = script_generate_all