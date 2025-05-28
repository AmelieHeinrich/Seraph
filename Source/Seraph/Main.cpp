//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-26 19:21:37
//

#include "Core/Context.h"
#include "Application.h"

int main(void)
{
    Context::Initialize();
    {
        Application app;
        app.Run();
    }
    Context::Shutdown();
    return 0;
}
