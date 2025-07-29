//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-05 14:03:27
//

#include "KC_Context.h"
#include "KC_BasicAllocator.h"
#include "KC_TrackingAllocator.h"

#include <KernelOS/KOS_DirectAllocation.h>

namespace KC {
    Context Context::sContext;

    void Context::Initialize()
    {
#ifdef KD_RETAIL
        sContext.allocator = KOS::New<BasicAllocator>();
#else
        sContext.allocator = KOS::New<TrackingAllocator>();
#endif

        MultiLogger* multiLogger = KC_NEW(MultiLogger);
        multiLogger->AddLogger(KC_NEW(FileLogger, "kaleidoscope.txt"));
        multiLogger->AddLogger(KC_NEW(ConsoleLogger));

        sContext.logger = multiLogger;
    }

    void Context::Shutdown()
    {
        KC_DELETE(sContext.logger);

        sContext.allocator->Report();
        KOS::Delete(sContext.allocator);
    }
}
