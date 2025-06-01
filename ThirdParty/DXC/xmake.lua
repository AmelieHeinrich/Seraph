--
-- > Notice: Amélie Heinrich @ 2025
-- > Create Time: 2025-06-01 13:42:26
--

target("DXC")
    set_kind("headeronly")
    set_group("Dependencies")

    add_headerfiles("Include/DXC/*.h")
    add_includedirs("Include", { public = true })
    add_linkdirs("Lib", { public = true })
    add_syslinks("dxcompiler.lib", { public = true })