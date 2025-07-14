--
-- > Notice: Amélie Heinrich @ 2025
-- > Create Time: 2025-05-26 19:25:02
--

target("Seraph")
    set_group("Seraph")
    set_kind("static")
    set_languages("c++20")
    set_rundir("$(projectdir)")

    add_files("Seraph/Asset/*.cpp",
              "Seraph/Core/*.cpp",
              "Seraph/Renderer/*.cpp",
              "Seraph/RHI/*.cpp",
              "Seraph/Util/*.cpp",
              "Seraph/World/*.cpp")
    add_headerfiles("Seraph/*.h",
                    "Seraph/Asset/*.h",
                    "Seraph/Core/*.h",
                    "Seraph/Renderer/*.h",
                    "Seraph/RHI/*.h",
                    "Seraph/Util/*.h",
                    "Seraph/World/*.h")

    add_includedirs("Seraph")
    add_defines("GLM_ENABLE_EXPERIMENTAL", { public = true })
    add_deps("SDL3", "DXC", "ImGui", "JSON", "STB", "GLM", "CGLTF", "MikkTSpace", { public = true })

    if is_plat("windows") then
        add_deps("DirectX", "Vulkan", "PIX", "NVTT", { public = true })
        add_syslinks("user32", { public = true })
        add_defines("SERAPH_VULKAN", "SERAPH_D3D12", "SERAPH_DUMMY", { public = true })
        add_files("Seraph/RHI/Vulkan/*.cpp",
                  "Seraph/RHI/D3D12/*.cpp",
                  "Seraph/RHI/Dummy/*.cpp",
                  "Seraph/Core/Windows/*.cpp",
                  "Seraph/Asset/Windows/*.cpp")
    else
        add_deps("Metal", { public = true })
        add_defines("SERAPH_DUMMY", "SERAPH_METAL", { public = true })
        add_files("Seraph/Core/Mac/*.cpp",
                  "Seraph/Asset/Mac/*.cpp",
                  "Seraph/RHI/Dummy/*.cpp",
                  "Seraph/RHI/Metal/*.cpp",
                  "Seraph/RHI/Metal/*.mm")
    end

    if is_mode("debug") then
        set_symbols("debug")
        set_optimize("none")
    elseif is_mode("release") then
        set_symbols("hidden")
        set_optimize("fastest")
        set_strip("all")
    elseif is_mode("releasedbg") then
        set_symbols("debug")
        set_optimize("fastest")
        set_strip("all")
    end

target("DemoApp")
    set_group("Executables")
    set_kind("binary")
    set_languages("c++20")
    set_rundir("$(projectdir)")

    add_files("DemoApp/**.cpp")
    add_headerfiles("DemoApp/**.h")
    add_includedirs(".", "Seraph")
    add_deps("Seraph")
    add_defines("GLM_FORCE_DEPTH_ZERO_TO_ONE")

    if is_plat("windows") then
        add_deps("PIX")
    end

    if is_mode("debug") then
        set_symbols("debug")
        set_optimize("none")
    elseif is_mode("release") then
        set_symbols("hidden")
        set_optimize("fastest")
        set_strip("all")
    elseif is_mode("releasedbg") then
        set_symbols("debug")
        set_optimize("fastest")
        set_strip("all")
    end

target("Tests")
    set_group("Executables")
    set_kind("binary")
    set_languages("c++20")
    set_rundir("$(projectdir)")

    add_files("Tests/**.cpp")
    add_headerfiles("Tests/**.h")
    add_includedirs(".", "Seraph", "Tests")
    add_deps("Seraph", "FLIP")

    if is_mode("debug") then
        set_symbols("debug")
        set_optimize("none")
    elseif is_mode("release") then
        set_symbols("hidden")
        set_optimize("fastest")
        set_strip("all")
    elseif is_mode("releasedbg") then
        set_symbols("debug")
        set_optimize("fastest")
        set_strip("all")
    end
