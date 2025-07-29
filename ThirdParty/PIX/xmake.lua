--
-- > Notice: Amélie Heinrich @ 2025
-- > Create Time: 2025-06-01 13:41:20
--

target("pix")
    set_kind("headeronly")
    set_group("Third Party")

    add_headerfiles("Include/**.h")
    add_includedirs("Include", { public = true })
    if is_plat("windows") then
        add_linkdirs("Lib/", { public = true })
        add_syslinks("WinPixEventRuntime.lib", { public = true })
    end
