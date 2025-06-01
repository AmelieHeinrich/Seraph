--
-- > Notice: Amélie Heinrich @ 2025
-- > Create Time: 2025-06-01 13:39:38
--

target("D3D12")
    set_kind("headeronly")
    set_group("Dependencies")

    add_headerfiles("Include/Agility/**.h")
    add_includedirs("Include", { public = true })
