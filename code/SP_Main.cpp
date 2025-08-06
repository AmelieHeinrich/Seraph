//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-28 19:00:55
//

#include <KernelCore/KC_Context.h>

#include "SP_Application.h"

KD_MAIN
{
    KC::ScopedContext ctx;
    {
        SP::Application app;
        app.Run();
    }
    return 0;
}
