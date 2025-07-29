--
-- > Notice: Amélie Heinrich @ 2025
-- > Create Time: 2025-06-01 13:39:38
--

target("directx")
    set_kind("static")
    set_group("Third Party")

    add_headerfiles("Include/Agility/**.h")
    add_includedirs("Include", { public = true })
    
    if is_plat("windows") then
        add_syslinks("d3d12", "dxgi", { public = true })
    end
