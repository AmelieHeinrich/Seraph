--
-- > Notice: Floating Trees Inc. @ 2025
-- > Create Time: 2025-07-29 18:58:57
--

target("json")
    set_kind("headeronly")
    set_group("Third Party")
    
    add_headerfiles("*.hpp")
    add_includedirs(".", { public = true })
