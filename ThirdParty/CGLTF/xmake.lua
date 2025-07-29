--
-- > Notice: Floating Trees Inc. @ 2025
-- > Create Time: 2025-07-18 16:34:25
--

target("cgltf")
    set_kind("static")
    set_group("Third Party")

    add_headerfiles("include/*.h")
    add_files("src/*.cpp")
    add_includedirs("include", { public = true })
