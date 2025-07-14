//
// > Notice: Amélie Heinrich @ 2025
// > Create Time: 2025-06-12 22:06:51
//

#pragma once

#include "Test.h"

class RHIBaseTest
{
protected:
    TestStarters mStarters;
    IRHICommandList* mCommandList = nullptr;

public:
    RHIBaseTest(RHIBackend backend);
    ~RHIBaseTest();

    TestResult Run();

protected:
    virtual void Execute() = 0;
    virtual void Cleanup() = 0;

    void BeginCmd();
    void EndAndSubmitCmd();
    void CopyRenderToScreenshot();
};
