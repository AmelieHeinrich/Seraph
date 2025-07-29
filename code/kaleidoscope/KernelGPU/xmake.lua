--
-- > Notice: Floating Trees Inc. @ 2025
-- > Create Time: 2025-07-05 18:55:40
--

target("KernelGPU")
    set_kind("static")
    set_group("Kaleidoscope")

    add_headerfiles("*.h")
    add_includedirs(".")
    add_files("*.cpp")

    add_deps("sdl3", "glm", "KernelOS", "KernelCore")

    add_deps("KernelGPU-Dummy")
    if is_plat("windows") then
        add_deps("KernelGPU-D3D12", "KernelGPU-Vulkan")
    end
