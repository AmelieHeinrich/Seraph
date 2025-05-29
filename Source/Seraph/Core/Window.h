//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-28 07:13:24
//

#pragma once

#include <SDL3/SDL.h>

#include "Types.h"

class Window
{
public:
    Window(int width, int height, const StringView& title);
    ~Window();

    bool IsOpen() const;
    void PollEvents();

public:
    SDL_Window* GetWindow() { return mWindow; }

private:
    SDL_Window* mWindow;
    bool mOpen;
};
