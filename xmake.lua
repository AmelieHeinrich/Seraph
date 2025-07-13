--
-- > Notice: Amélie Heinrich @ 2025
-- > Create Time: 2025-05-26 19:24:06
--

add_rules("mode.debug", "mode.release", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

set_policy("build.sanitizer.address", true)
add_defines("SERAPH_ENABLE_LOGGING")
set_languages("c++20")

if is_plat("windows") then
    before_link(function (target)
        os.cp("Binaries/Windows/*", "$(buildir)/$(plat)/$(arch)/$(mode)/")
    end)
else
    add_rpathdirs("Binaries/Mac/")
    before_link(function (target)
        os.cp("Binaries/Mac/*", "$(buildir)/$(plat)/$(arch)/$(mode)/")
    end)
    add_cxxflags("-fobjc-arc")
end

includes("ThirdParty")
includes("Source")
