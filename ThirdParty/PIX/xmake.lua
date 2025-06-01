--
-- > Notice: Amélie Heinrich @ 2025
-- > Create Time: 2025-06-01 13:41:20
--

target("PIX")
    set_kind("headeronly")
    set_group("Dependencies")

    add_headerfiles("Include/**.h")
    add_includedirs("Include", { public = true })
    add_linkdirs("Lib/", { public = true })
    add_syslinks("WinPixEventRuntime.lib", { public = true })
