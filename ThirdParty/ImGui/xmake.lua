--
-- > Notice: Amélie Heinrich @ 2025
-- > Create Time: 2025-06-01 22:49:17
--

target("ImGui")
    set_kind("static")
    set_group("Dependencies")

    add_files("Include/ImGui/*.cpp")
    add_includedirs("Include/", { public = true })
    add_deps("DirectX", "Vulkan", "SDL3")
