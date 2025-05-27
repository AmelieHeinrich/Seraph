//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-05-27 07:40:44
//

#include "Context.h"

Context Context::sContext;

void Context::Initialize()
{
    MultiLogger* multiLogger = new MultiLogger;
    multiLogger->AddLogger(new FileLogger("seraph.log"));
    multiLogger->AddLogger(new ConsoleLogger());

    sContext.logger = multiLogger;
}

void Context::Shutdown()
{
    delete sContext.logger;
}
