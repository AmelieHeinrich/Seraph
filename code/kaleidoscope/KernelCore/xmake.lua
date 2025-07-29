--
-- > Notice: Floating Trees Inc. @ 2025
-- > Create Time: 2025-07-05 13:14:33
--

target("KernelCore")
    set_kind("static")
    set_group("Kaleidoscope")

    add_headerfiles("*.h")
    add_files("*.cpp")
    add_includedirs(".")
    add_deps("KernelOS")
