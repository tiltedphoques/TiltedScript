#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <TiltedCore/Platform.hpp>
#include <cstdint>

#include <TiltedCore/StackAllocator.hpp>
#include <TiltedCore/ScratchAllocator.hpp>
#include <TiltedCore/Stl.hpp>
#include <TiltedCore/Outcome.hpp>
#include <TiltedCore/Vector3.hpp>
#include <TiltedCore/Filesystem.hpp>

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

