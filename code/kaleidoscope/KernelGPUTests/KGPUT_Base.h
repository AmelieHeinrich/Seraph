//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-29 19:14:46
//

#pragma once

#include "KGPUT_Test.h"

namespace KGPUT
{
    class BaseTest
    {
    public:
        BaseTest();
        ~BaseTest();

        KC::Array<uint8> Run();
    
    protected:
        virtual void Execute() = 0;
        virtual void Cleanup() = 0;
    
        void Begin();
        void Flush();
        void Readback();
    protected:
        TestData Data;
        KGPU::ICommandList* mCommandList = nullptr;
    };
}
