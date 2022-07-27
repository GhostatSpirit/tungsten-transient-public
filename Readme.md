# Tungsten Transient Renderer #

## About ##

Tungsten Transient extends the original physically based [Tungsten renderer](https://github.com/tunabrain/tungsten) to support volumetric transient rendering, which simulates light transport at specific time latencies.
It implements the "sliced photon primitives" algorithm described in [Temporally sliced photon primitives for time-of-flight rendering](https://cs.dartmouth.edu/wjarosz/publications/liu22temporally.html).

## Getting Started ##

To compile the code, please refer to the Compilation section in [Tungsten renderer repo](https://github.com/tunabrain/tungsten). The only difference is that now C++17 support is required. The code has been verified to be compilable on Windows (MSVC) and Linux (gcc).

After compiling the code, you can try rendering the example transient scenes under the directory `data/example-scenes/transient`. For example, to render the transient cornell-box scene, run

	tungsten <directory_to_cloned_repo>/data/example-scenes/transient/cornell-box/scene.json

## Usage ##

Please first refer to the original [Tungsten renderer repo](https://github.com/tunabrain/tungsten). This section will only document usage related to transient rendering.

All of the transient rendering parameters can be changed in the `"integrator"` section in the scene json file. For example, to use Marco et al.'s [progressive transient photon beam algorithm](https://cs.dartmouth.edu/wjarosz/publications/marco19progressive.html), change the `"volume_photon_type"` field's value to `"beams"`. Here is a detailed explanation of fields related to transient rendering:

- `"transient_time_center"`: the time gate center, i.e. the time latency that the camera takes a capture. Can also be specified with the `--time-center` option of tungsten.
- `"transient_time_width"`: the width of the box time gate function, i.e. the time window that photons are allowed to enter the camera.
- `"delta_time_gate"`: whether a Dirac delta time gate function is used. This option will override the value in `"transient_time_width"`.
- `"volume_photon_count"`: # of primitive to generate for each progressive step.
- `"volume_photon_type"`: the primitive used for transient volumetric rendering. Available options are `"beams"`, `"volumes_balls"`, `"mis_volumes_balls"`, `"volumes"`, `"hypervolumes"` and `"balls"`. Note that `"volumes"`, `"hypervolumes"` and `"balls"` options will not render the full light transport; only light path we can estimate with those individual primitives are rendered.

Finally, the `"spp"` option in `"renderer"` specifies the number of progressive steps to render. This can also be specified with the `--spp` option of tungsten.

## Where are the code changes ##
Most of the code changes are in `src/core/integrator/photon_map` and `src/core/integrator/progressive_photon_map`, where the original photon mapping integrator is modified to support transient rendering.

A `DirectionalPoint` primitive is also added to `src/core/primitives` to support collimated beam light sources.


## Installing OpenEXR ##

If you want to read and write in .exr format, you will need to install OpenEXR.
The process of installing OpenEXR is changed in this fork.
The easiest way is to use [vcpkg](https://vcpkg.io/en/getting-started.html).
Install vcpkg from the link above. Then, assuming you are building an x64 version of Tungsten on windows, run

    vcpkg install openexr:x64-windows

to install OpenEXR (v2.5.0 as time of writing). Then, either
- specify the vcpkg CMake toolchain file during configuration using `-DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake` or
- set the environment variable `VCPKG_ROOT` to `[path to vcpkg]`
to integrate vcpkg to CMake's build process.

Now you should be able to see

    [cmake] -- OpenEXR detected. Building with .exr support

during the CMake configuration process. This means that OpenEXR is successfully installed and detected by CMake.

## License ##

This fork uses the same license as the original Tungsten renderer. The original License notice is pasted below.

> To give developers as much freedom as is reasonable, Tungsten is distributed under the [libpng/zlib](http://opensource.org/licenses/Zlib) license. This allows you to modify, redistribute and sell all or parts of the code without attribution.

> Note that Tungsten includes several third-party libraries in the `src/thirdparty` folder that come with their own licenses. Please see the `LICENSE.txt` file for more information.