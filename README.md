# ExperimEngine

ExperimEngine is a personal side project made to _experiment_ (hence the name) with various tech pieces in order to make a tiny _C++ game engine_. 

Along the way, I aim to learn many things such as 3D graphics with [Vulkan](https://www.khronos.org/vulkan/), some shader fun, and to embed a scripting/modding language.

### Current work :

+ Investigation for a [scripting language](#modding-and-scripting)
+ Integration of an old-school CPU renderer made by a friend, as an application to test texture rendering

### Done :

+ Integration of [Dear ImGui](https://github.com/ocornut/imgui) docking/viewports branch in the native Vulkan Renderer.
+ [Web-support](#web-support) : WebGPU renderer implementation and integration of Dear ImGui in the WebGPU renderer.

## Modding and Scripting

ExperimEngine will include an API for a second language both for scripting and modding. A common way of doing this is to embed a language into the host language (here C++), see [embeddable_languages](https://en.wikipedia.org/wiki/Scripting_language#Extension/embeddable_languages). In adddition to modding, this allow for quick iteration while working a on a game by reloading scripts at run-time instead of a slow C++ compilation.
Both _modding_ and _scripting_ have similar needs : there needs to be a _fast, easy to use and powerful C++ API_.

Lua is the language considered for now but this may still change.

### Languages being evaluated

#### Lua :

Classic embedded language in game engines.
Pros :
+ Blazing fast [perfomances](http://luajit.org/performance.html), thanks to LuaJIT
+ Awesome C++ interop (with [sol](https://github.com/ThePhD/sol2), although [others](https://sol2.readthedocs.io/en/latest/features.html) exist)
+ Open-source and light-weight
+ Easy to embed

Cons :
+ LuaJIT is *not* maintained (and quite complex) and is 32-bit only
+ Lua's best performances come from the JIT but JIT compiling seems to be forbidden on apple devices
+ Weird syntax, 1-indexed arrays, feels quite different from usual languages nowadays.

#### Wren

[Wren](https://wren.io/) is described as "Smalltalk in a Lua-sized package with a dash of Erlang and wrapped up in a familiar, modern syntax"
Pros :
+ [Fast](https://wren.io/performance.html), without JIT
+ Open-source and light-weight
+ Easy to embed
+ Modern syntax

Cons :
+ C++ interop seems seems less advanced than sol in lua
+ Quite a niche language (ecosystem, tooling, user-base)

#### C#/.NET

I planned to :
+ [Host the .NET Core](https://docs.microsoft.com/en-us/dotnet/core/tutorials/netcore-hosting) with the new 3.0 hosting API (see [this document](https://github.com/dotnet/core-setup/blob/master/Documentation/design-docs/native-hosting.md))
+ Use [Roslyn](https://github.com/dotnet/roslyn) as a way to dynamically compile script files.

Scripting in C# could great but the main problem is to obtain a clean C#/C++(classes/objects) interop without resorting to Windows-only solutions.
The C#/.NET ecosystem seems to be rapidly evolving with interop being taken into account.

Pros:
+ I enjoy coding with C#
+ Good tooling & ecosystem, huge user-base
+ The C#/.NET ecosystem went open-source and seems to be rapidly evolving with interop being taken into account:
  + [Improvements in native code interop in .NET 5.0](https://devblogs.microsoft.com/dotnet/improvements-in-native-code-interop-in-net-5-0/)

Cons :
+ Not light-weight
+ Hard to embed, hard to provide an egronomic & performant C++ interop

[Mono](https://www.mono-project.com/) is an open source implementation of Microsoft's .NET Framework, and provides a good [embedding API](https://www.mono-project.com/docs/advanced/embedding/) but seems to be behind in terms of features and performances.

Elements to investigate :
+ [UnrealCLR](https://github.com/nxrighthere/UnrealCLR) for C++/C# in Unreal Engine.
+ Compatibility with emscripten/webassembly ?

#### Javascript :

Embedding google's [V8 and its C++ API](https://v8.dev/docs/embed) or another JS engine (Mozilla's [SpiderMonkey](https://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey)).

Pros :
+ Gigantic eco-system, widespread language with good community support/tooling
+ Good C++ interop (V8)

Cons :
+ V8/SpiderMonkey are not light-weight

There are some lighter JS engine out there : [Duktape](https://github.com/svaarala/duktape), [JerryScript](https://github.com/jerryscript-project/jerryscript) and more, but I have yet to investigate the performance impact & C++ interop possibilities.


## Cross-platform support

### Native support

Libraries and languages are chosen with _cross-platform_ as a focus point. As of today, there should be no major concern in targeting platforms such as Windows, Linux, MacOs, Android and more.

Right now, the build system is focused on Windows 64 bits only for practical reasons (read laziness, the current development environment is Wx64).

### Web support

Idealy, an ExperimEngine app should be able to run on a web-browser.

**Current state** : Implementation of a separate WebGPU rendering backend

Technologies :
+ [Emscripten](https://github.com/emscripten-core/emscripten) to compile C++ to [WebAssembly](https://webassembly.org/).
  + emscripten also provides support for the SLD2 API (already used as an OS abstraction in ExperimEngine).
  + If OpenGL was a rendering target, emscripten could also be used to automatically convert it to WebGL.
+ [WebGPU](https://github.com/gpuweb/gpuweb), a modern API for GPU rendering on the web. It is pretty similar to (and relies on) APIs such as Metal, Vulkan and DirectX12, although a bit higher level, and with web safety in mind.
  + As of today (06/12/2020) WebGPU is still in a really early stage and only available in experimentals features of some web-browsers, which does not seem like an issue :  by the time I would be able to develop a back-end for it, it could be stabilized/mature enough.
  + Supporting WebGPU and Vulkan would mean implementing/supporting **2** rendering backends in ExperimEngine (*)
 
(*) About supporting 2 backends :

As far as I understand, since WebGPU has Metal/Vulkan/DX12 implementations, it's in theory more portable than Vulkan. So I could get away with just a WebGPU backend and target pretty much the same platforms as Vulkan+WebGPU.

See projects like [wgpu-rs](https://github.com/gfx-rs/wgpu-rs) (see also : [wgpu](https://github.com/gfx-rs/wgpu) & [gfx-rs](https://github.com/gfx-rs/wgpu)) for Rust or [Dawn](https://dawn.googlesource.com/dawn) in C++.

I still intend to keep working on a separate Native Vulkan renderer for now. This may prove too heavy but I hope that WebGPU relying on Vulkan means that both backends would share a lot in term of workflow with WebGPU being the simplest.

Issues encountered : 
+ I was hoping to reuse the same glsl/spirv shaders used for Vulkan renderer in WebGPU. So far it is not the case since some Vulkan shaders use features not (yet?) available in WebGPU :
  + Push constants are not included in WebGPU yet, see [issue 75](https://github.com/gpuweb/gpuweb/issues/75)
  + Combined image samplers are not included in WebGPU yet, see [issue 770](https://github.com/gpuweb/gpuweb/issues/770)

## Dependencies/Libraries

For now dependencies (windows 64bits) are _packaged within the project_ in order to facilitate the environment setup on multiples machines.

Some of the dependencies may become git submodule.

Libraries are picked trying to respect as much as possible the following criterias :
1. C/C++
2. Cross-platform
3. Light-weight & Performant

### Used :

Vulkan:
+ [Vulkan](https://www.khronos.org/vulkan/) - [version](includes/vulkan/version.txt)
+ [Vulkan-Hpp](https://github.com/KhronosGroup/Vulkan-Hpp) (now included in Vulkan SDK). C++ API to Vulkan, and also used for [dynamic dispatching](https://gpuopen.com/learn/reducing-vulkan-api-call-overhead/).
+ [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) - [version](includes/vma/version.txt)

Scripting:
+ [LuaJIT](http://luajit.org/luajit.html) - [version](libs/lua/version.txt)
+ [sol](https://github.com/ThePhD/sol2) - [version](includes/sol/version.txt)

Others:
+ [OpenGL Mathematics (GLM)](https://glm.g-truc.net/0.9.9/index.html) - [version](includes/glm/version.txt)
+ [Simple DirectMedia Layer (SDL2)](https://www.libsdl.org/index.php) - [version](includes/SDL2/version.txt)
+ [Dear ImGui](https://github.com/ocornut/imgui) - Docking branch, [version](src/engine/render/imgui/lib/version.txt)
+ [spdlog](https://github.com/gabime/spdlog) - [version](includes/spdlog/version.h)
+ [stb_image](https://github.com/nothings/stb) - [version](includes/stb/stb_image.h)

### May be used in the future :
+ [Bullet Physics](https://github.com/bulletphysics/bullet3)
+ [Doxygen](http://www.doxygen.nl/)
+ ENet, RakNet, [GameNetworkingSockets (Valve)](https://github.com/ValveSoftware/GameNetworkingSockets)

## Tooling

+ [CMake](https://cmake.org/)
+ [ClangFormat](https://clang.llvm.org/docs/ClangFormat.html)

## Credits and useful resources

### Vulkan
+ [Sascha Willems Vulkan examples](https://github.com/SaschaWillems/Vulkan)
+ [Vulkan tutorials](https://vulkan-tutorial.com/Introduction)
+ [Khronos Vulkan-Samples](https://github.com/KhronosGroup/Vulkan-Samples)
+ [Awesome Vulkan](https://github.com/vinjn/awesome-vulkan)
+ [Writing an efficient Vulkan renderer](https://zeux.io/2020/02/27/writing-an-efficient-vulkan-renderer/) by Arseny Kapoulkine

### WebGPU
+ [WebGPU cross platform demo](https://github.com/kainino0x/webgpu-cross-platform-demo) by kainino0x
+ [ImGui WebGPU backend sample](https://github.com/ocornut/imgui/pull/3632) by bfierz

### OpenGl and 3D
+ [GPU Gems](https://developer.nvidia.com/gpugems/GPUGems/gpugems_pref01.html), by NVIDIA
+ [LearnOpenGL](https://learnopengl.com/Introduction)
+ [Learning Modern 3D Graphics Programming](http://opengl.datenwolf.net/gltut/html/index.html), by Jason L.McKesson
+ [EDX course, Computer Graphics](https://www.edx.org/course/computer-graphics-2)

## License:

MIT. See [LICENSE](LICENSE)
