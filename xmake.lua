--
-- > Notice: Floating Trees Inc. @ 2025
-- > Create Time: 2025-07-05 13:10:14
--

add_rules("mode.debug", "mode.release", "mode.releasedbg")
set_rundir(".")
set_languages("c++20")

-- Platform
if is_plat("windows") then
    add_defines("KD_WINDOWS", "USE_PIX")
elseif is_plat("linux") then
    add_defines("KD_LINUX")
else
    add_defines("KD_MAC")
end

-- Config
add_defines("GLM_FORCE_DEPTH_ZERO_TO_ONE")
if is_mode("debug") then
    add_defines("KD_DEBUG")
    set_symbols("debug")
    set_optimize("none")
elseif is_mode("releasedbg") then
    add_defines("KD_RELEASE")
    set_symbols("debug")
    set_optimize("fastest")
    set_strip("all")
else
    add_defines("KD_RETAIL")
    set_symbols("hidden")
    set_optimize("fastest")
    set_strip("all")
end

-- Build steps
before_link(function (target)
    os.cp("dlls/*", "$(builddir)/$(plat)/$(arch)/$(mode)/")
end)

-- Includes
includes("thirdparty")
includes("code/kaleidoscope")
includes("code/seraph")
