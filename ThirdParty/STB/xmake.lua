--
-- > Notice: Floating Trees Inc. @ 2025
-- > Create Time: 2025-07-18 14:04:12
--

target("stb")
    set_kind("static")
    set_group("Third Party")

    add_includedirs("include", { public = true })
    add_files("src/*.cpp")
    add_headerfiles("include/*.h")
