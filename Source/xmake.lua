--
-- > Notice: Amélie Heinrich @ 2025
-- > Create Time: 2025-05-26 19:25:02
--

target("Seraph")
    set_group("Executables")
    set_kind("binary")
    set_languages("c++17")

    add_files("Seraph/**.cpp")
    add_headerfiles("Seraph/**.h")
    add_deps("SDL3")
    add_syslinks("user32")

    before_link(function (target)
        os.cp("Binaries/*", "$(buildir)/$(plat)/$(arch)/$(mode)/")
    end)
