#pragma once

#define NOMINMAX

#include <Platform.hpp>
#include <cstdint>

#include <StackAllocator.hpp>
#include <ScratchAllocator.hpp>
#include <Stl.hpp>
#include <Outcome.hpp>
#include <Vector3.hpp>
#include <Filesystem.hpp>

#include <any>
#include <mutex>
#include <chrono>
#include <iostream>
#include <filesystem>
#include <codecvt>
#include <fstream>

extern "C"
{
#include <lua.h>
#include <lsqlite3.h>
}

#define SOL_ALL_SAFETIES_ON 1
#include <sol.hpp>

using namespace TiltedPhoques;

using namespace std::chrono_literals;

