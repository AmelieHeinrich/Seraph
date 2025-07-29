--
-- > Notice: Floating Trees Inc. @ 2025
-- > Create Time: 2025-07-25 19:58:48
--

target("KernelGPU-Dummy")
    set_kind("static")
    set_group("Kaleidoscope")

    add_files("*.cpp")
    add_headerfiles("*.h")
    add_includedirs(".", "../KernelGPU", { public = true })
    add_deps("sdl3", "glm", "KernelOS", "KernelCore")
