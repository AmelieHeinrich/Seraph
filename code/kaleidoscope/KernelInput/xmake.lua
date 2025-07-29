--
-- > Notice: Floating Trees Inc. @ 2025
-- > Create Time: 2025-07-18 00:49:39
--

target("KernelInput")
    set_kind("static")
    set_group("Kaleidoscope")

    add_files("*.cpp")
    add_headerfiles("*.h")
    add_deps("glm")
