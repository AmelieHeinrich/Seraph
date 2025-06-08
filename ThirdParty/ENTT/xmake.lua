--
-- > Notice: Amélie Heinrich @ 2025
-- > Create Time: 2025-06-08 12:03:49
--

target("ENTT")
    set_kind("headeronly")
    set_group("Dependencies")

    add_headerfiles("Include/**.hpp")
    add_includedirs("Include", { public = true })
