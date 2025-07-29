--
-- > Notice: Floating Trees Inc. @ 2025
-- > Create Time: 2025-07-18 15:19:51
--

target("KDAsset")
    set_kind("static")
    set_group("Kaleidoscope")

    add_files("*.cpp")
    add_headerfiles("*.h")
    add_deps("KernelCore", "stb", "cgltf", "mikktspace")
