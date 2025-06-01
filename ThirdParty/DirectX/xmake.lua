--
-- > Notice: Amélie Heinrich @ 2025
-- > Create Time: 2025-06-01 13:39:38
--

target("DirectX")
    set_kind("static")
    set_group("Dependencies")

    add_headerfiles("Include/Agility/**.h")
    add_files("Source/D3D12MemAlloc.cpp")
    add_includedirs("Include", { public = true })
    add_syslinks("d3d12", "dxgi", { public = true })
