--
-- > Notice: Amélie Heinrich @ 2025
-- > Create Time: 2025-06-13 19:34:08
--

target("MikkTSpace")
    set_kind("static")
    set_group("Dependencies")

    add_files("Include/MikkTSpace/mikktspace.c")
    add_headerfiles("Include/MikkTSpace/mikktspace.h")
    add_includedirs("Include", { public = true })
