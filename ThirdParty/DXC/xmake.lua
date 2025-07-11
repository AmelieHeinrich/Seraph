--
-- > Notice: Amélie Heinrich @ 2025
-- > Create Time: 2025-06-01 13:42:26
--

local vulkan_sdk = os.getenv("VULKAN_SDK")
if vulkan_sdk == nil then
    -- Default path for Vulkan SDK on macOS (change version as needed)
    local home = os.getenv("HOME")
    local default_version = "1.4.313.1"
    vulkan_sdk = home .. "/VulkanSDK/" .. default_version .. "/macos"
end

target("DXC")
    set_kind("headeronly")
    set_group("Dependencies")

    if is_plat("windows") then
        add_headerfiles("Windows/Include/*.h")
        add_includedirs("Windows/Include/", { public = true })
        add_linkdirs("Windows/Lib", { public = true })
        add_syslinks("dxcompiler.lib", { public = true })
    else
        add_includedirs(vulkan_sdk .. "/Include", { public = true })
        add_linkdirs(vulkan_sdk .. "/Lib", { public = true })
    end