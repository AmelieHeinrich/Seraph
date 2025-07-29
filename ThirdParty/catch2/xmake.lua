--
-- > Notice: Floating Trees Inc. @ 2025
-- > Create Time: 2025-07-16 19:15:39
--

target("catch2")
    set_kind("static")
    set_group("Third Party")

    add_files("code/**.cpp")
    add_headerfiles("code/**.hpp")
    add_includedirs("code/", { public = true })

