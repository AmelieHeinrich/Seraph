--
-- > Notice: Floating Trees Inc. @ 2025
-- > Create Time: 2025-07-17 21:09:46
--

target("imgui")
    set_kind("static")
    set_group("Third Party")

    add_files("include/*.cpp")
    add_headerfiles("include/*.h")
    add_includedirs("include", { public = true })

    if is_plat("windows") then
        add_files("include/sdl3/*.cpp")
        add_headerfiles("include/sdl3/*.h")
        add_deps("sdl3")
    end
