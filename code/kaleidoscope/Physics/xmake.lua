--
-- > Notice: Floating Trees Inc. @ 2025
-- > Create Time: 2025-07-23 21:13:33
--

target("Physics")
    set_kind("static")
    set_group("Kaleidoscope")

    add_files("*.cpp")
    add_headerfiles("*.h")
    add_deps("jolt")
