--
-- > Notice: Floating Trees Inc. @ 2025
-- > Create Time: 2025-07-18 17:26:11
--

target("mikktspace")
    set_kind("static")
    set_group("Third Party")

    add_headerfiles("include/*.h")
    add_files("src/*.c")
    add_includedirs("include/", { public = true })
