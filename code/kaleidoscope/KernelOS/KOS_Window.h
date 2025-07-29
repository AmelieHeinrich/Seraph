//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 18:12:07
//

#pragma once

#include "KOS_Common.h"

namespace KOS
{
    class IWindow
    {
    public:
        static IWindow* Create(uint width, uint height, const char* title);
        virtual ~IWindow() {}

        virtual bool IsOpen() = 0;
        virtual bool PollEvents(void** data) = 0;
        virtual void* GetGPUSurface() = 0;
        virtual void* GetHandle() = 0;

        virtual void GetSize(int& width, int& height) = 0;
        virtual void SetTitle(const char* title) = 0;
        virtual const char* GetTitle() = 0;
    };
}
