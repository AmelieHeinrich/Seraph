--
-- > Notice: Floating Trees Inc. @ 2025
-- > Create Time: 2025-07-16 19:18:17
--

target("KernelCoreTests")
    set_kind("binary")
    set_group("Kaleidoscope")

    add_files("*.cpp")
    add_deps("KernelCore", "catch2")
    add_defines("KD_DISABLE_LOGGING")
