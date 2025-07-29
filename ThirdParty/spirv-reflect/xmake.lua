--
-- > Notice: Floating Trees Inc. @ 2025
-- > Create Time: 2025-07-25 20:30:15
--

target("spirv-reflect")
    set_kind("static")
    set_group("Third Party")

    add_files("*.cpp")
    add_headerfiles("*.h")
    add_includedirs(".", { public = true })
