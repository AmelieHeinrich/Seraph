//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 07:31:18
//

#include "Window.h"
#include "Context.h"

Window::Window(int width, int height, const StringView& title)
    : mOpen(true)
{
    ASSERT_EQ(SDL_Init(SDL_INIT_VIDEO) == true, "Failed to initialize SDL3!");

    mWindow = SDL_CreateWindow(title.data(), width, height, 0);
    ASSERT_EQ(mWindow != nullptr, "Failed to create SDL3 window!");
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
        if (event.type == SDL_EVENT_QUIT)
            mOpen = false;
    }
}
