--
-- > Notice: Amélie Heinrich @ 2025
-- > Create Time: 2025-06-01 13:42:26
--

target("dxc")
    set_kind("headeronly")
    set_group("Third Party")

    add_headerfiles("Include/**.h")
    add_includedirs("Include/", { public = true })
