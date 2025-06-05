--
-- > Notice: Amélie Heinrich @ 2025
-- > Create Time: 2025-06-05 22:27:27
--

target("GLM")
    set_kind("headeronly")
    set_group("Dependencies")

    add_headerfiles("Include/**.hpp")
    add_includedirs("Include/", { public = true })
