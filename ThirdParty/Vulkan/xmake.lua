--
-- > Notice: Amélie Heinrich @ 2025
-- > Create Time: 2025-05-28 20:27:42
--

target("Vulkan")
    set_kind("headeronly")
    set_group("Dependencies")

    add_includedirs(os.getenv("VULKAN_SDK") .. "/Include", { public = true })
    add_linkdirs(os.getenv("VULKAN_SDK") .. "/Lib", { public = true })
    add_syslinks("volk", { public = true })
