--
-- > Notice: Amélie Heinrich @ 2025
-- > Create Time: 2025-05-26 19:25:02
--

target("Seraph")
    set_group("Seraph")
    set_kind("static")
    set_languages("c++20")
    set_rundir("$(projectdir)")

    add_files("Seraph/**.cpp")
    add_headerfiles("Seraph/**.h")
    add_includedirs("Seraph")
    add_syslinks("user32", { public = true })
    add_defines("ENABLE_PIX", "GLM_ENABLE_EXPERIMENTAL", { public = true })
    add_deps("SDL3", "Vulkan", "Slang", "DirectX", "PIX", "DXC", "ImGui", "JSON", "STB", "GLM", "CGLTF", "NVTT", "MikkTSpace", { public = true })

    before_link(function (target)
        os.cp("Binaries/*", "$(buildir)/$(plat)/$(arch)/$(mode)/")
    end)

    if is_mode("debug") then
        set_symbols("debug")
        set_optimize("none")
        add_defines("NDEBUG")
    else
        set_symbols("hidden")
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
    add_deps("Seraph", "PIX")
    add_defines("ENABLE_PIX")

    if is_mode("debug") then
        set_symbols("debug")
        set_optimize("none")
        add_defines("NDEBUG")
    else
        set_symbols("hidden")
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
        add_defines("NDEBUG")
    else
        set_symbols("hidden")
        set_optimize("fastest")
        set_strip("all")
    end
