set_languages("cxx17")

set_xmakever("2.5.1")

add_requires(
    "catch2",
    "tiltedcore",
    "hopscotch-map",
    "sqlite3",
    "sol2",
    "glm")

add_rules("mode.debug","mode.releasedbg", "mode.release")
add_rules("plugin.vsxmake.autoupdate")

if is_mode("release") then
    add_ldflags("/LTCG", "/OPT:REF")
    add_cxflags("/Ot", "/GL", "/Ob2", "/Oi", "/GS-")
    add_defines("NDEBUG")
    set_optimize("fastest")
end

target("TiltedScript")
    set_kind("static")
    set_group("Libraries")
    add_files("Code/script/src/*.cpp")
    add_includedirs("Code/script/include/", {public = true})
    add_headerfiles(
        "Code/script/include/*.hpp",
        "Code/script/include/*.h",
        "Code/script/include/*.inl",
    {prefixdir = "TiltedScript"})
    set_pcxxheader("Code/script/include/TiltedScriptPCH.h")
    add_packages("tiltedcore", "mimalloc", "hopscotch-map", "sqlite3", "lua", "sol2", "glm")

target("TiltedScript_Tests")
    set_kind("binary")
    set_group("Tests")
    add_files("Code/tests/src/*.cpp")
    add_includedirs("Code/script/include/")
    add_deps("TiltedScript")
    add_packages("catch2", "tiltedcore", "hopscotch-map", "sqlite3", "lua", "sol2", "glm")
