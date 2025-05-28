//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-26 19:21:37
//

#include "Core/Context.h"
#include "Core/Window.h"

int main(void)
{
    Context::Initialize();

    {
        Window window(1280, 720, "What's up y'all");
        while (window.IsOpen()) {
            window.PollEvents();
        }
    }

    Context::Shutdown();
    return 0;
}
