--
-- > Notice: Amélie Heinrich @ 2025
-- > Create Time: 2025-05-28 07:07:34
--

target("SDL3")
    set_kind("headeronly")
    set_group("Dependencies")

    add_headerfiles("include/**.h")
    add_includedirs("include", { public = true })
    add_linkdirs("lib", { public = true })
    add_syslinks("SDL3.lib", { public = true })
