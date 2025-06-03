//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-26 19:21:37
//

#include "Core/Context.h"
#include "Core/FileSystem.h"

#include "Application.h"

int main(void)
{
    Context::Initialize();
    FileSystem::Initialize();
    
    {
        Application app;
        app.Run();
    }
    
    FileSystem::Shutdown();
    Context::Shutdown();
    return 0;
}
