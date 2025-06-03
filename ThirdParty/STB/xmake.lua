--
-- > Notice: Amélie Heinrich @ 2025
-- > Create Time: 2025-06-03 18:56:56
--

target("STB")
    set_kind("static")
    set_group("dependencies")

    add_files("Source/stb.c")
    add_headerfiles("Include/*.h")
    add_includedirs("Include", { public = true })
