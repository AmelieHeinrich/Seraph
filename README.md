# Seraph : A showcase renderer for all my graphics programming skills

## Requirements

Minimum requirements is a SM6.6 capable GPU with raytracing and mesh shading capabilities, running Vulkan 1.3 and the latest version of D3D12.
The Vulkan backend requires VK_EXT_mutable_descriptor.

## Disclaimer

If you have an NVIDIA card, both backends are tested and thus work. Vulkan backend might be a bit finnicky so I recommend you use the D3D12 one.
It is currently **untested** on AMD and Intel cards since I do not have the hardware. However it is likely that it will not work.

## Screenshots

### Tiled Light Culling
![](.github/june15.png)

### Raytraced Hard Shadows
![](.github/july7.png)

### Test Suite
![](.github/test2.png)

## Notable Features

- Complete RHI with D3D12/Vulkan support -- Bindless, raytracing, mesh shaders, GPU readback.
- Test Suite for RHI -- Generates JSON report used to render results in simple web page
- Asset compression and caching
- Techniques: Tiled light culling, deferred shading
- Lighting: Microfacet BRDF, CSM, raytraced hard shadows

## WIP

- Raytraced soft shadows
- SVGF denoising

## Planned

- Raytraced reflections
- Raytraced AO
- Dynamic Diffuse Global Illumination
- Cluster culling & LODing via mesh shaders
- Temporal Anti-Aliasing
- Bloom
- 6-pass Bokeh DOF

## Planned tests

- Indirect draw
- Indirect draw indexed
- Indirect dispatch
- Indirect dispatch mesh
