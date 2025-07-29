//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-18 01:14:05
//

#include "ToolImGui_Input.h"

#include <sdl3/imgui_impl_sdl3.h>

namespace ToolImGui
{
    void Input::Initialize(KOS::IWindow* window)
    {
        ImGui_ImplSDL3_InitForD3D((SDL_Window*)window->GetHandle());
    }

    void Input::Shutdown()
    {
        ImGui_ImplSDL3_Shutdown();
    }

    void Input::NewFrame()
    {
        ImGui_ImplSDL3_NewFrame();
    }

    void Input::ReadEvent(void* event)
    {
        ImGui_ImplSDL3_ProcessEvent((SDL_Event*)event);
    }
}
