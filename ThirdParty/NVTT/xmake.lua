--
-- > Notice: Amélie Heinrich @ 2025
-- > Create Time: 2025-06-07 19:36:12
--

target("NVTT")
    set_kind("headeronly")
    set_group("Dependencies")

    add_headerfiles("Include/**.h")
    add_includedirs("Include", { public = true })
    add_linkdirs("Lib/", { public = true })
    add_syslinks("nvtt30205.lib", { public = true })
