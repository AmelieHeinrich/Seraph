--
-- > Notice: Amélie Heinrich @ 2025
-- > Create Time: 2025-07-11 17:16:00
--

target("Metal")
    set_group("Dependencies")

    if is_plat("macosx") then
        set_kind("static")
        add_files("Source/Metal.cpp")
        add_includedirs("Include/", { public = true })
        add_frameworks("Foundation", "QuartzCore", "Metal", { public = true })
    else
        set_kind("headeronlh")
    end
