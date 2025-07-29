--
-- > Notice: Floating Trees Inc. @ 2025
-- > Create Time: 2025-07-05 18:16:22
--

target("sdl3")
    set_kind("headeronly")
    set_group("Third Party")

    add_headerfiles("include/**.h")
    add_includedirs("include", { public = true })
    add_linkdirs("lib", { public = true })
    
    if is_plat("windows") then
        add_syslinks("SDL3.lib", { public = true })
    end
