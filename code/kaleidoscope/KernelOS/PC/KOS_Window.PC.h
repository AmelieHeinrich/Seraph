//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 18:15:14
//

#pragma once

#include "KOS_Window.h"

#include <SDL3/SDL.h>

namespace KOS
{
    class PCWindow : public IWindow
    {
    public:
        PCWindow(uint width, uint height, const char* title);
        ~PCWindow();

        bool IsOpen() override;
        bool PollEvents(void** data) override;
        void* GetGPUSurface() override;
        void* GetHandle() override { return (void*)mWindow; }

        void GetSize(int& width, int& height) override;
        void SetTitle(const char* title) override;
        const char* GetTitle() override;
    
    private:
        void UpdateInputSystem(SDL_Event& event);

    private:
        SDL_Window* mWindow;
        SDL_Event mEvent;
        bool mRunning = false;
    };
}
