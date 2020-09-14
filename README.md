# ExperimEngine

ExperimEngine is a personal side project made to _experiment_ (hence the name) with various tech pieces in order to make a tiny _C++ game engine_. 

Along the way, I aim to learn many things such as 3D graphics with [Vulkan](https://www.khronos.org/vulkan/), some shader fun, and to embed [Lua](https://www.lua.org/) as a scripting/modding language.

## Modding/Scripting

The answer to the "why" for a second scripting language is already quite old and widespread see some information [here](https://en.wikipedia.org/wiki/Scripting_language#Extension/embeddable_languages), but to be short, C++ compilation takes time.
Both _modding_ and _scripting_ have similar needs : there needs to be a _clear, fast and powerful C++ API_

I chose Lua as a scripting language for the following reasons :

+ Blazing fast [perfomances](http://luajit.org/performance.html), thanks to LuaJIT
+ Awesome C++ interop (with [sol](https://github.com/ThePhD/sol2), although [others](https://sol2.readthedocs.io/en/latest/features.html) exist)
+ Large community
+ Open-source
+ Learning experience

C#/.NET was also considered as a candidate. I planned to :

+ [Host the .NET Core](https://docs.microsoft.com/en-us/dotnet/core/tutorials/netcore-hosting) with the new 3.0 hosting API (see [this document](https://github.com/dotnet/core-setup/blob/master/Documentation/design-docs/native-hosting.md))
+ Use [Roslyn](https://github.com/dotnet/roslyn) as a way to dynamically compile script files.

Scripting in C# would have been great but the main problem was to obtain a clean C#/C++(classes/objects) interop without resorting to Windows-only solutions.

## Cross-platform support

Libraries and languages are chosen with _cross-platform_ as a focus point. As of today, there should be no problem in targeting platforms such as Windows, Linux, MacOs, Android and more.

Right now, the build system is focused on Windows 64 bits only for practical reasons (read laziness, the current development environment is Wx64).

## Dependencies/Libraries

For now dependencies (windows 64bits) are _packaged within the project_ in order to facilitate the environment setup on multiples machines.

Some of the dependencies may become git submodule.

### Used :
+ [OpenGL Mathematics (GLM)](https://glm.g-truc.net/0.9.9/index.html) V0.9.9.6
+ [Simple DirectMedia Layer (SDL2)](https://www.libsdl.org/index.php) V2.0.10
+ [LuaJIT](http://luajit.org/luajit.html) V2.0.5
+ [sol](https://github.com/ThePhD/sol2) v3.2.1
+ [Vulkan](https://www.khronos.org/vulkan/) 1.2.148.1
+ [Vulkan-Hpp](https://github.com/KhronosGroup/Vulkan-Hpp) (now included in Vulkan SDK)
+ [spdlog](https://github.com/gabime/spdlog) (V1.8.0)
+ [Dear ImGui](https://github.com/ocornut/imgui) (Docking branch, 770c99536533923d2cf07fc5004c752ef2c63f5c)

### May be used in the future :
+ [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
+ [Bullet Physics](https://github.com/bulletphysics/bullet3)
+ [Doxygen](http://www.doxygen.nl/)
+ ENet, RakNet

## Tooling

+ [CMake](https://cmake.org/) V3.17.2
+ [ClangFormat](https://clang.llvm.org/docs/ClangFormat.html)

## Credits and useful resources

### Vulkan
+ [Sascha Willems vulkan examples](https://github.com/SaschaWillems/Vulkan)
+ [Vulkan tutorials](https://vulkan-tutorial.com/Introduction)
+ [Vulkan-Samples](https://github.com/KhronosGroup/Vulkan-Samples)
+ [Awesome Vulkan](https://github.com/vinjn/awesome-vulkan)

### OpenGl and 3D
+ [GPU Gems](https://developer.nvidia.com/gpugems/GPUGems/gpugems_pref01.html), by NVIDIA
+ [LearnOpenGL](https://learnopengl.com/Introduction)
+ [Learning Modern 3D Graphics Programming](http://opengl.datenwolf.net/gltut/html/index.html), by Jason L.McKesson
+ [EDX course, Computer Graphics](https://www.edx.org/course/computer-graphics-2)

## License:

MIT. See [LICENSE](LICENSE)
