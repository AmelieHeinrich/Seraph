//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 07:31:18
//

#include "Window.h"
#include "Context.h"

#include <ImGui/imgui_impl_sdl3.h>

Window::Window(RHIBackend backend, int width, int height, const String& title)
    : mOpen(true)
{
    ASSERT_EQ(SDL_Init(SDL_INIT_VIDEO) == true, "Failed to initialize SDL3!");

    int windowFlags = 0;
    if (backend == RHIBackend::kVulkan) windowFlags = SDL_WINDOW_VULKAN;
    if (backend == RHIBackend::kMetal) windowFlags = SDL_WINDOW_METAL;

    mWindow = SDL_CreateWindow(title.data(), width, height, windowFlags);
    if (!mWindow) {
        SERAPH_ERROR("SDL ERROR: %s", SDL_GetError());
        ASSERT_EQ(false, "Failed to create SDL3 window!");
    }

    // SDL_SetWindowFullscreen(mWindow, true);
}

Window::~Window()
{
    SDL_DestroyWindow(mWindow);
    SDL_Quit();
}

bool Window::IsOpen() const
{
    return mOpen;
}

void Window::PollEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);
        if (event.type == SDL_EVENT_QUIT)
            mOpen = false;
    }
}
