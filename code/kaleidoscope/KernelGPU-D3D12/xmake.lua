--
-- > Notice: Floating Trees Inc. @ 2025
-- > Create Time: 2025-07-21 18:28:17
--

target("KernelGPU-D3D12")
    set_kind("static")
    set_group("Kaleidoscope")

    add_files("*.cpp")
    add_headerfiles("*.h")
    add_includedirs(".", "../KernelGPU", { public = true })
    add_deps("sdl3", "glm", "KernelOS", "KernelCore", "directx", "pix")
