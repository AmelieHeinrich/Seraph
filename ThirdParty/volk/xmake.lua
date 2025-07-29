--
-- > Notice: Floating Trees Inc. @ 2025
-- > Create Time: 2025-07-25 20:20:19
--

target("volk")
    set_group("Third Party")

    if is_plat("windows") or is_plat("linux") then
        set_kind("static")

        add_headerfiles("*.h")
        add_files("*.cpp")
        
        add_includedirs(os.getenv("VULKAN_SDK") .. "/Include", ".", { public = true })
        add_linkdirs(os.getenv("VULKAN_SDK") .. "/Lib", { public = true })
        add_defines("VK_NO_PROTOTYPES", { public = true })
    else
        set_kind("headeronly")
    end
