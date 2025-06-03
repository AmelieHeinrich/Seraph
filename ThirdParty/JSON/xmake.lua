--
-- > Notice: Amélie Heinrich @ 2025
-- > Create Time: 2025-06-03 13:49:13
--

target("JSON")
    set_kind("headeronly")
    set_group("Dependencies")

    add_headerfiles("Include/**.hpp")
    add_includedirs("Include", { public = true })
