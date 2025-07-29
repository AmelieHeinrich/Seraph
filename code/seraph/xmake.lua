--
-- > Notice: Floating Trees Inc. @ 2025
-- > Create Time: 2025-07-28 19:00:08
--

target("seraph")
    set_kind("binary")
    set_group("Games")

    add_files("**.cpp")
    add_headerfiles("**.h")
    add_includedirs(".")
    add_deps("Kaleidoscope")
