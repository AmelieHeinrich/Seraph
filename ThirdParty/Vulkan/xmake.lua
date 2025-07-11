--
-- > Notice: Amélie Heinrich @ 2025
-- > Create Time: 2025-05-28 20:27:42
--

target("Vulkan")
    set_group("Dependencies")

    if is_plat("windows") then
        set_kind("static")
        add_files("Source/*.cpp")
        add_includedirs(os.getenv("VULKAN_SDK") .. "/Include", "Include", { public = true })
        add_linkdirs(os.getenv("VULKAN_SDK") .. "/Lib", { public = true })
        add_defines("VK_NO_PROTOTYPES", { public = true })
    else
        set_kind("headeronly")
    end
