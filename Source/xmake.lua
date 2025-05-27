--
-- > Notice: Amélie Heinrich @ 2025
-- > Create Time: 2025-05-26 19:25:02
--

target("Seraph")
    set_group("Executables")
    set_kind("binary")
    set_languages("c++17")

    add_files("Seraph/*.cpp",
              "Seraph/Core/*.cpp")
