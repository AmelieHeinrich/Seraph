--
-- > Notice: Floating Trees Inc. @ 2025
-- > Create Time: 2025-07-26 00:20:57
--

target("KernelGPUTests")
    set_kind("binary")
    set_group("Kaleidoscope")

    add_files("**.cpp")
    add_headerfiles("*.h")
    add_includedirs(".")
    add_deps("KernelCore", "KernelGPU", "KDAsset", "KDShader", "Graphics", "flip", "json")
