--
-- > Notice: Amélie Heinrich @ 2025
-- > Create Time: 2025-06-01 22:49:17
--

target("ImGui")
    set_kind("static")
    set_group("Dependencies")

    add_files("Include/ImGui/imgui_demo.cpp",
              "Include/ImGui/imgui_draw.cpp",
              "Include/ImGui/imgui_tables.cpp",
              "Include/ImGui/imgui_widgets.cpp",
              "Include/ImGui/imgui.cpp",
              "Include/ImGui/imgui_impl_sdl3.cpp")
    add_includedirs("Include/", { public = true })
    add_deps("SDL3")

    if is_plat("windows") then
        add_deps("DirectX", "Vulkan")
    else
        
    end
