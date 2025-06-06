--
-- > Notice: Amélie Heinrich @ 2025
-- > Create Time: 2025-06-06 20:11:15
--

target("CGLTF")
    set_kind("static")
    set_group("Dependencies")

    add_files("Include/CGLTF/cgltf.cpp")
    add_headerfiles("Include/CGLTF/cgltf.h")
    add_includedirs("Include", { public = true })
