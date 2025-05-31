--
-- > Notice: Amélie Heinrich @ 2025
-- > Create Time: 2025-05-28 20:27:42
--

target("Slang")
    set_kind("static")
    set_group("Dependencies")

    add_includedirs("Include", { public = true })
    add_linkdirs("Lib", { public = true })
    add_syslinks("slang", { public = true })
