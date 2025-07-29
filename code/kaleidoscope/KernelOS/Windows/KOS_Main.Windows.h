//
// > Notice: Floating Trees Inc. @ 2025
// > Create Time: 2025-07-15 21:28:35
//

#pragma once

#include <Windows.h>

#if defined(KD_RETAIL)
    #define KD_MAIN int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#else
    #define KD_MAIN int main()
#endif
