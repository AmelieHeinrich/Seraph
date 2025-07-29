--
-- > Notice: Floating Trees Inc. @ 2025
-- > Create Time: 2025-07-23 21:08:21
--

target("jolt")
    set_kind("static")
    set_group("Third Party")

    add_files("Jolt/**.cpp")
    add_headerfiles("Jolt/**.h")
    add_includedirs(".", { public = true })
