--[[
Most of this project is licensed under the standard BSD 3-clause license listed below. An exception is anything involving Mandible Games, pavlovian.net, or mandible.net. You'll have to go through and remove those references before releasing anything with this platform (especially the URL of the error reporting code, please.) This isn't as easy as it could be. Sorry!

Obviously, libraries and programs that are included in their entirety are not licensed under my particular license.



Copyright (c) 2010, Ben Wilhelm
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holders' companies nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

]]

------------------------------------------------------------------
-- BASIC SETUP, UTILITIES

local params = select(1, ...)
if type(params) ~= "table" then
  print("Stop running it from this directory, moron!")
  os.exit(1)
end

require "luarocks.loader"
require "ursa"

function token_literal(name, data)
  ursa.token.rule{name, "!" .. data, function () return data end}
end
ursa.token.rule{"PWD", nil, function () return ursa.system{"pwd"}:gsub("/cygdrive/c", "c:") end}

------------------------------------------------------------------
-- CONFIGURATION, DERIVING INFO

local optflags = "-O2"
token_literal("optflags", optflags)

params.targets = ursa.util.cli_parameter_parse{unpack(params.targets)}

assert(params.font, "choose a font, you dumbass")

local noluajit = params.noluajit
do
  local newtargets = {}
  -- NOTE: This is the intended location for adding build parameters
  for _, v in pairs(params.targets) do
    if v == "noluajit" then
      noluajit = true
    else
      table.insert(newtargets, v)
    end
  end
  params.targets = newtargets
end

-- midname is the longname with whitespace removed - useful for package filenames
params.midname = params.longname:gsub("[^%w]", "")

------------------------------------------------------------------
-- PLATFORM-SPECIFIC

ursa.token.rule{"os_name", nil, "uname -o || uname"}

local platform
local os_files = "os_universal"
if ursa.token{"os_name"} == "Cygwin" then
  platform = "cygwin"
  os_files = os_files .. " os_win32"
elseif ursa.token{"os_name"} == "Darwin" then
  platform = "osx"
  os_files = os_files .. " os_posix os_osx"
elseif ursa.token{"os_name"} == "GNU/Linux" then
  platform = "linux"
  os_files = os_files .. " os_posix os_linux"
else
  print("unknown OS", ursa.token{"os_name"})
  assert(false)
end

local builddir_pre = "build/" .. platform
token_literal("BUILDDIR", builddir_pre)
local builddir = builddir_pre .. "/"

-- This loads in all our OS-dependent stuff
local osd = assert(loadfile("glorp/Den_os_" .. platform .. ".lua")){name = params.name, longname = params.longname, midname = params.midname, builddir = builddir, icon = params.icon}


------------------------------------------------------------------
-- LIBRARIES

-- this will contain all the libs that get built
local libs = {}
local headers = {}

local function addHeadersFor(lib)
  if headers[lib] then return end
  
  assert(loadfile("glorp/Den_lib_" .. lib .. ".lua")){libs = libs, headers = headers, platform = platform, builddir = builddir, addHeadersFor = addHeadersFor}
  
  assert(headers[lib])
end

-- Test code to build a single library easily
if params.targets[1] == "libtest" then
  local testy = params.targets[2]
  addHeadersFor(testy)
  ursa.command{"test", {libs[testy], headers[testy]}}
  ursa.build{"test"}
  do return end
end

local libcflags = string.format("%s -I%s/build/%s/lib_release/include", ursa.token{"CCFLAGS"}, ursa.token{"PWD"}, platform)
local libldflags = string.format("%s -L%s/build/%s/lib_release/lib", ursa.token{"LDFLAGS"}, ursa.token{"PWD"}, platform)

ursa.token.rule{"gitrefs", nil, "git show-ref", always_rebuild = true}
ursa.token.rule{"version", "#gitrefs", ("git describe --always --match %s-* | sed s/%s-//"):format(params.name, params.name)}

local genfiles = {}

ursa.rule{builddir .. "glorp/version.cpp", {"!" .. params.longname, "!" .. params.font, "!" .. params.resolution[1], "!" .. params.resolution[2], "#version", "!" .. platform}, function ()
  print("Writing file version.cpp")
  local fil = io.open(builddir .. "glorp/version.cpp", "w")
  fil:write([[#include "version.h"]] .. "\n")
  fil:write([[namespace Glorp { namespace Version {]] .. "\n")
  fil:write(([[const char gameVersion[] = "%s";]]):format(ursa.token{"version"}) .. "\n")
  fil:write(([[const char gameFullname[] = "%s";]]):format(params.longname) .. "\n")
  fil:write(([[const char gameMidname[] = "%s";]]):format(params.midname) .. "\n")
  fil:write(([[const char gameSlug[] = "%s";]]):format(params.name) .. "\n")
  fil:write(([[const char gamePlatform[] = "%s";]]):format(platform) .. "\n")
  fil:write(([[const char gameFontDefault[] = "%s";]]):format(params.font) .. "\n")
  fil:write(([[const int gameXres = %d;]]):format(params.resolution[1]) .. "\n")
  fil:write(([[const int gameYres = %d;]]):format(params.resolution[2]) .. "\n")
  fil:write(([[const bool resizable = true;]]) .. "\n")
  fil:write(([[const int stencil = 0;]]) .. "\n")
  fil:write([[}}]] .. "\n")
  fil:close()
end}
table.insert(genfiles, ursa.rule{builddir .. "glorp/constants.lua", {"!" .. params.gl_version_expected, "!" .. params.resolution[1], "!" .. params.resolution[2], "#version"}, function ()
  print("Writing file constants.lua")
  local fil = io.open(builddir .. "glorp/constants.lua", "w")
  fil:write("return {\n")
  fil:write(([[  gl_version_expected = %f,]]):format(params.gl_version_expected) .. "\n")
  fil:write(([[  resolution = {%d, %d},]]):format(params.resolution[1], params.resolution[2]) .. "\n")
  fil:write(([[  version = %q,]]):format(ursa.token{"version"}) .. "\n")
  fil:write("}\n")
  fil:close()
end})

-- logal build
local logal_deps
do
  local logal = ursa.embed{"glorp/libs/logal", "Den", {{}}}
  
  logal.lgl_generate(ursa.absolute_from{builddir .. "glorp/lgl.cpp"}, "")
  
  logal_deps = {ursa.rule{builddir .. "glorp/lgl.h", logal.lgl_header, ursa.util.copy{}}}
end

------------------------------------------------------------------
-- BUILD

local buildables = {
  [params.name] = {dynfiles = "version lgl", glorpfiles = "core core_lua debug_911 debug util parse args init perfbar pak reporter " .. os_files, mmfiles = (osd.gles and "main_iphone") or ((platform == "osx") and "os_ui_osx") or "", libs = "boost lua zlib libpng libjpeg libvorbis libogg libflac portaudio frame glew libfreetype libcurl minizip" .. (params.libs.box2d and " box2d" or "") .. (params.libs.lfs and " lfs" or "")}
}

-- TEST CODE: build all libraries
--[[
local tb = {}
for _, v in pairs(buildables) do
  for k in string.gmatch(v.libs, "([^%s]+)") do
    addHeadersFor(k)
    table.insert(tb, headers[k])
    table.insert(tb, libs[k])
  end
end
ursa.build{tb}
do return end]]

if platform == "cygwin" then
  buildables[params.name].libs = buildables[params.name].libs .. " dx"
end

local depsed = {}

local buildflags = ""
if params.libs.box2d then
  buildflags = buildflags .. " -DGLORP_BOX2D"
end
if params.libs.lfs then
  buildflags = buildflags .. " -DGLORP_LFS"
end

local function build_object(src, prefix, params)
  local compiler = ursa.token{"CXX"}
  if params.compiler then compiler = compiler:gsub("g%+%+", params.compiler) end
  local cli = ("%s %s %s -Wall -Wno-sign-compare -Wno-uninitialized -g -DDPRINTF_MARKUP %s -Iglorp -Iglorp/.. -I%sglorp -I%slib_release/include %s %s %s "):format(compiler, params.flags or "", optflags, buildflags, builddir, builddir, ursa.token{"CXXFLAGS"}, ursa.token{"FINALCXXFLAGS"}, src)
  
  local sig = src .. "." .. prefix
  
  local extraheaders = {}
  for v in params.builddata.libs:gmatch("([^%s]+)") do
    addHeadersFor(v)
    assert(headers[v], "Missing headers for package " .. v)
    table.insert(extraheaders, headers[v])
  end
  
  -- this little block is probably no longer necessary
  if not depsed[sig] then
    local function make_dependencies(srcfile)
      local deps = ursa.system{cli .. "-MM"}
      deps = deps:match("^.*: (.*)$")
      
      local dependencies = {}
      for file in deps:gmatch("([^%s]+)") do
        if file ~= "\\" then
          table.insert(dependencies, file)
        end
      end
      
      return dependencies
    end
    
    local depend = sig .. " dependencies"
    
    ursa.token.rule{depend, {ursa.util.token_deferred{depend, default = src}, "!" .. cli, extraheaders, logal_deps}, function () return make_dependencies(cpp) end}
    depsed[sig] = ursa.util.token_deferred{depend}
  end
  
  local dst = prefix .. "/" .. src:gsub("%.cpp", ".o"):gsub("%.c", ".o"):gsub("%.mm", ".o")
  ursa.rule{dst, {src, depsed[sig], extraheaders, logal_deps}, ursa.util.system_template{"nice glorp/gcc_wrapper $TARGET " .. cli .. "-o $TARGET -c"}}
  return dst
end

local function build_program(name)
  local item = buildables[name]
  assert(item)
  
  local prefix = builddir .. name .. "_objs"
  
  local objs = {}
  
  for k in (item.dynfiles or ""):gmatch("([^%s]+)") do
    table.insert(objs, build_object(builddir .. "glorp/" .. k .. ".cpp", prefix, {builddata = item, flags = item.flags}))
  end
  for k in (item.glorpfiles or ""):gmatch("([^%s]+)") do
    table.insert(objs, build_object("glorp/" .. k .. ".cpp", prefix, {builddata = item, flags = item.flags}))
  end
  --[[for k in (item.cfiles or ""):gmatch("([^%s]+)") do
    table.insert(objs, build_object(k .. ".c", prefix, {builddata = item, compiler = "gcc", flags = item.flags}))
  end]] -- currently not used
  for k in (item.mmfiles or ""):gmatch("([^%s]+)") do
    table.insert(objs, build_object("glorp/" .. k .. ".mm", prefix, {builddata = item, flags = (item.flags or "") .. " -x objective-c++"}))
  end
  
  -- feed in all the lib files we need
  local extradeps = {}
  for v in item.libs:gmatch("([^%s]+)") do
    assert(libs[v])
    table.insert(extradeps, libs[v])
  end
  
  -- extra platform-dependent sources
  if osd.extraLinkSources then
    for _, v in ipairs(osd.extraLinkSources) do
      table.insert(objs, v)
    end
  end
  
  local sg, eg = "-Wl,--start-group", "-Wl,--end-group"
  if platform == "osx" then
    sg, eg = "", ""
  end
  
  if platform == "linux" and name == params.name then
    eg = eg .. " '-Wl,-rpath,$ORIGIN/data'"
  end
  
  local dst = builddir .. name .. osd.extension
  ursa.rule{dst, {objs, extradeps}, ursa.util.system_template{("nice glorp/gcc_wrapper $TARGET #CXX #optflags -o $TARGET -g %s $SOURCES -L#BUILDDIR/lib_release/lib -lm #LDFLAGS #FINALLDFLAGS %s"):format(sg, eg)}}
  return dst
end

local mainprog = build_program(params.name)

------------------------------------------------------------------
-- PACKAGING

ursa.token.rule{"oggquality", "!6", function () return "6" end}  -- heh.

-- here is where we're generating the file list for packaging
do
  local data_deps = {}
  
  local converts = {}
  function converts.png(dst)
    return "pngcrush -brute -rem alla -cc $SOURCE $TARGET"
  end
  function converts.wav(dst)
    return "oggenc -q #oggquality -o $TARGET $SOURCE", dst:gsub("%.wav", ".ogg")
  end
  function converts.flac(dst)
    return [[(#FLAC -d $SOURCE -c | oggenc -q #oggquality -o $TARGET -)]], dst:gsub("%.flac", ".ogg")
  end
  if osd.build_overrides then
    for k, v in pairs(osd.build_overrides) do
      converts[k] = v
    end
  end
  
  local function copy_a_lot(token, destprefix, sourceprefix)
    ursa.token.rule{token .. "_copy", {"#" .. token .. "_files", "!" .. destprefix, "!" .. sourceprefix}, function ()
        local data_items = {}
        for k in ursa.token{token .. "_files"}:gmatch("[^\n]+") do
          -- narsty hack for level_.lev.lua
          if params.libs.box2d and k:match("^level_.*%.lev%.lua$") then
            local dst = destprefix .. k
            local src = sourceprefix .. k
            
            table.insert(data_items, {src = src, deps = {"glorp/tool/level_bake.lua", "glorp/util.lua", "glorp/stage_persistence.lua", "scaffold.lua", "terra.lua", "worldparams.lua", lua_exec}, dst = dst, cli = lua_exec .. " glorp/tool/level_bake.lua $SOURCE $TARGET 8"})
          else
            local ext = k:match("^.*%.([^%.]+)$")
            
            local dst = destprefix .. k
            local src = sourceprefix .. k
            
            -- copyright infringement hack
            if src:match("copyright_infringement") then
              src = "glorp/resources/common/nil." .. ext
            end
            
            local odst = dst
            
            local cli
            
            if converts[ext] then
              cli, dst = converts[ext](dst)
            end
            
            dst = dst or odst
            
            if not cli then
              cli = "cp $SOURCE $TARGET"
            end
            
            table.insert(data_items, {src = src, dst = dst, cli = cli})
          end
        end
        return data_items
      end}
    return "#" .. token .. "_copy"
  end

  -- replicate the data structure in
  ursa.token.rule{"data_files", nil, "if cd data; then find . -type f | sed s*\\\\./**; fi", always_rebuild = true}
  table.insert(data_deps, copy_a_lot("data", "data/", "data/"))

  -- resources
  ursa.token.rule{"stock_files", nil, "cd glorp/resources/common && ls mandible_games.png licenses.txt", always_rebuild = true}
  table.insert(data_deps, copy_a_lot("stock", "data/", "glorp/resources/common/"))

  -- Lua files in Glorp
  ursa.token.rule{"luaglorp_files", nil, "cd glorp && ls *.lua | grep -v Den | grep -v box2d", always_rebuild = true}
  table.insert(data_deps, copy_a_lot("luaglorp", "data/glorp/", "glorp/"))
  
  -- Box2d lua files
  if params.libs.box2d then
    ursa.token.rule{"luabox2d_files", nil, "cd glorp && ls *.lua | grep box2d", always_rebuild = true}
    table.insert(data_deps, copy_a_lot("luabox2d", "data/glorp/", "glorp/"))
  end

  -- Lua files in our core
  ursa.token.rule{"core_files", nil, "ls *.lua | grep -v crashmelt | grep -v playtest.lev.lua", always_rebuild = true}
  table.insert(data_deps, copy_a_lot("core", "data/", ""))
  
  -- generated files
  ursa.token.rule{"generated_files", genfiles, ("cd build/%s/glorp && (ls *.lua *.png; true)"):format(platform), always_rebuild = true}
  table.insert(data_deps, copy_a_lot("generated", "data/", ("build/%s/glorp/"):format(platform)))

  ursa.token.rule{"datafiles", data_deps, function ()
    local chunks = {}
    for _, v in pairs(data_deps) do
      for _, tv in pairs(ursa.token{v:sub(2)}) do
        table.insert(chunks, tv)
      end
    end
    return chunks
  end}
  
  -- generate our actual data copies
  ursa.token.rule{"built_data", "#datafiles", function ()
    local items = {}
    for _, v in pairs(ursa.token{"datafiles"}) do
      local dst = osd.dataprefix .. v.dst
      
      local cli = v.cli
      if not cli:find("$SOURCES") then cli = cli:gsub("$SOURCE", '"' .. v.src .. '"') end
      if not cli:find("$TARGETS") then cli = cli:gsub("$TARGET", '"' .. dst .. '"') end
      
      table.insert(items, ursa.rule{dst, {v.src, v.deps or {}}, ursa.util.system_template{cli}})
    end
    return items
  end, always_rebuild = true}
  
  -- NOTE: this function should probably be renamed to register_cull_exceptions or something of that like, and then we actually do the culling later. right now it's sort of strange.
  function cull_data(dat)
    local path = osd.appprefix
    ursa.token.rule{"culled_data", {"#built_data"}, function ()
      local valids = {}
      for _, v in pairs(ursa.relative_from{{dat, ursa.token{"built_data"}}}) do
        local val = v:match(path:gsub("%-", "%%-") .. "(.*)")
        if not val then
          print(val, v, path:gsub("%-", "%%-") .. "(.*)")
        end
        assert(val)
        valids[val] = true
      end
      
      for f in ursa.system{("cd \"%s\" && find . -type f || true"):format(path)}:gmatch("([^\n]+)") do
        local fi = f:match("%./(.*)")
        if not valids[fi] then
          print("======== REMOVING", fi)
          ursa.system{"rm \"" .. path .. fi .. "\""}
        end
      end
      
      return "" -- alright we return nothing this is just a command basically. I need a better way to handle this.
    end, always_rebuild = true}
  end

  ursa.token.rule{"outputprefix", "#version", function ()
    return ("%s-%s"):format(params.midname, ursa.token{"version"})
  end}
end

------------------------------------------------------------------
-- RUN THINGS

local runnable = osd.create_runnable{mainprog = mainprog}

ursa.command{ursa.command.default, {runnable.deps, genfiles}}

if osd.no_cli_params then
  ursa.command{"run", {runnable.deps, genfiles}, runnable.cli}
else
  ursa.command{"run", {runnable.deps, genfiles}, runnable.cli .. " --debug --nosound"}
  ursa.command{"runsound", {runnable.deps, genfiles}, runnable.cli .. " --debug"}
  ursa.command{"runclean", {runnable.deps, genfiles}, runnable.cli}
  ursa.command{"editor", {runnable.deps, genfiles}, runnable.cli .. " editor"}
end

if not noluajit and optflags == "-O2" then
  local installers = osd.installers("datafiles")
  ursa.command{"package", installers}
  ursa.command{"deploy", installers, "deploy.sh " .. params.midname .. "-" .. ursa.token{"version"} .. ".exe"}
end

clean_dirs = {}
table.insert(clean_dirs, "build")
ursa.command{"clean", function ()
  ursa.util.clean()
  for _, v in ipairs(clean_dirs) do
    ursa.system{"rm -rf " .. v}
  end
end}

ursa.build{unpack(params.targets)}
