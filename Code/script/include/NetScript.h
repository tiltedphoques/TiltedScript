#pragma once

#include <NetObjectDefinition.h>

using String = TiltedPhoques::String;

struct NetScript
{
    NetScript(TiltedPhoques::UniquePtr<NetObjectDefinition> apDef) 
        : Definition(std::move(apDef))
    {
        
    }

    String Filename;
    String Content;
    TiltedPhoques::UniquePtr<NetObjectDefinition> Definition;
};
