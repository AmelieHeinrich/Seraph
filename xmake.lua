--
-- > Notice: Floating Trees Inc. @ 2025
-- > Create Time: 2025-07-05 13:10:14
--

set_rundir(".")
set_languages("c++20")
add_runenvs("PATH", "dlls")

includes("ext/kaleidoscope")
includes("code")

before_link(function (target)
    os.cp("dlls/*", "$(builddir)/$(plat)/$(arch)/$(mode)/dlls/")
end)
