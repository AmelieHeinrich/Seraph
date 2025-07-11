--
-- > Notice: Amélie Heinrich @ 2025
-- > Create Time: 2025-05-26 19:24:06
--

add_rules("mode.debug", "mode.release", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

set_languages("c++20")

if is_plat("windows") then
    before_link(function (target)
        os.cp("Binaries/Windows/*", "$(buildir)/$(plat)/$(arch)/$(mode)/")
    end)
else
    before_link(function (target)
        os.cp("Binaries/Mac/*", "$(buildir)/$(plat)/$(arch)/$(mode)/")
    end)
end

includes("ThirdParty")
includes("Source")
