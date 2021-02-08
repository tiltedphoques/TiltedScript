set_languages("cxx17")

set_xmakever("2.5.1")

add_requires("tiltedcore", "mimalloc", "hopscotch-map", {configs = {rltgenrandom = true }})
add_requires("catch2")

add_rules("mode.debug","mode.releasedbg", "mode.release")
add_rules("plugin.vsxmake.autoupdate")

if is_mode("release") then
    add_ldflags("/LTCG", "/OPT:REF")
    add_cxflags("/Ot", "/GL", "/Ob2", "/Oi", "/GS-")
    add_defines("NDEBUG")
    set_optimize("fastest")
end

target("lua")
    set_kind("static")
    set_group("Libraries")
    set_languages("c11")
    add_files("ThirdParty/lua/*.c")
    add_headerfiles("ThirdParty/lua/*.h")
    add_includedirs("ThirdParty/lua", {public = true})

target("sqlite3")
    set_kind("static")
    set_group("Libraries")
    set_languages("c11")
    add_files("ThirdParty/sqlite3/sqlite3.c")
    add_headerfiles("ThirdParty/sqlite3/sqlite3.h")
    add_includedirs("ThirdParty/sqlite3", {public = true})
    add_defines("SQLITE_OMIT_LOAD_EXTENSION")

target("TiltedScript")
    set_kind("static")
    set_group("Libraries")
    add_files("Code/script/src/*.cpp")
    add_includedirs("Code/script/include/", "ThirdParty/lua/", "ThirdParty/sqlite3", {public = true})
    add_headerfiles(
        "Code/script/include/*.hpp", 
        "Code/script/include/*.h", 
        "Code/script/include/*.inl", 
    {prefixdir = "TiltedScript"})
    add_deps("lua", "sqlite3")
    add_packages("tiltedcore", "mimalloc", "hopscotch-map")

target("TiltedScript_Tests")
    set_kind("binary")
    set_group("Tests")
    add_files("Code/tests/src/*.cpp")
    add_deps("TiltedScript")
    add_packages("tiltedcore", "catch2", "hopscotch-map")
