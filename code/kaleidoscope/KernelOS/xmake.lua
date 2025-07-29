--
-- > Notice: Floating Trees Inc. @ 2025
-- > Create Time: 2025-07-05 15:38:55
--

target("KernelOS")
    set_kind("static")
    set_group("Kaleidoscope")

    add_headerfiles("*.h")
    add_includedirs(".")

    add_deps("sdl3")

    if is_plat("windows") then
        add_files("Windows/*.cpp")
        add_files("PC/*.cpp")

        add_headerfiles("Windows/*.h")
        add_headerfiles("PC/*.h")

        add_syslinks("kernel32", "user32")
        add_deps("KernelInput")
    end
