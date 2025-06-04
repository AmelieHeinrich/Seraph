--
-- > Notice: Amélie Heinrich @ 2025
-- > Create Time: 2025-06-04 17:52:30
--

target("FLIP")
    set_kind("headeronly")
    set_group("Dependencies")

    add_headerfiles("Include/FLIP.h")
    add_includedirs("Include", { public = true })
