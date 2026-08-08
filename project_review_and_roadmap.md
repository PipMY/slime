# Software Renderer Project Review & Roadmap

This document provides a comprehensive overview of your software renderer project, explaining its current architecture, identifying critical issues, and outlining clear next steps to get your renderer to the next level.

## 1. Current State of the Project

You are building a CPU-based software renderer from scratch in C++, likely inspired by the *tinyrenderer* tutorial. The current structure is as follows:

*   **`main.cpp`**: The entry point. It sets up a 1000x1000 framebuffer, loads the `african_head.obj` mesh, initializes a Z-buffer, invokes the drawing logic, and outputs the result to `framebuffer.tga`.
*   **`geometry.h`**: A custom math library providing `vec2`, `vec3`, `vec4` and matrices (`mat2`, `mat3`, `mat4`), along with essential operations like dot product, cross product, and normalization.
*   **`obj_decoder.cpp/h`**: Responsible for parsing the `.obj` file. It extracts vertices, normals, and texture coordinates. It also computes the transformation matrices (`ModelView`, `Perspective`, `Viewport`) and transforms the object into screen coordinates.
*   **`drawing.cpp/h`**: Contains the core rasterization loop. It iterates over the bounding box of a triangle, computes barycentric coordinates, applies depth testing via the Z-buffer, and calculates ambient, diffuse, and specular lighting.
*   **`lighting.cpp/h`**: Provides utilities for blending colors. (Note: The `compute_lighting` function defined here is currently unused, as `drawing.cpp` computes lighting manually inline).
*   **`tgaimage.cpp/h`**: A robust library for reading and writing `.tga` image files.

---

## 2. Critical Issues to Fix

While the foundation is solid, there are a few mathematical and structural bugs that will cause visual artifacts if left unaddressed:

### A. Coordinate Space Mismatch in Lighting
In `drawing.cpp`, you interpolate the exact 3D position of the pixel:
```cpp
geo::vec3 mid = (v0 * w0) + (v1 * w1) + (v2 * w2);
```
However, `v0`, `v1`, and `v2` are passed into `rasterise` as **screen coordinates** (transformed all the way through the Viewport matrix in `obj_decoder.cpp`). Later, you do this for specular lighting:
```cpp
geo::vec3 eye{500, 500, 127.5};
geo::vec3 viewDir = geo::normalize(eye - mid);
```
You are subtracting a *screen-space coordinate* from a *world-space eye vector*. This math is invalid and will cause weird specular highlights. 
**The Fix:** You need to pass the *world/view-space* vertices to `rasterise` alongside the screen-space vertices, and interpolate the world-space vertices to compute `mid`.

### B. Normal Mapping Implementation
As discussed previously, your normal mapping has a few issues:
1.  **Per-vertex instead of per-pixel:** You are calling `getTextureNormal` in `drawMesh` and discarding the result. You need to pass `t0`, `t1`, `t2` into `rasterise` and sample the normal map per-pixel using interpolated UVs.
2.  **Color to Vector mapping:** `getTextureNormal` must convert RGB `[0, 255]` to Normal `[-1, 1]`.
3.  **Tangent Space:** The `african_head_nm.tga` normal map is in Tangent Space. To use it, you must construct a TBN (Tangent, Bitangent, Normal) matrix to transform the normal from the texture to world/view space.

### C. Redundant Depth Sorting
In `obj_decoder.cpp` (around line 165), you are sorting all faces by depth (the Painter's Algorithm). However, you are *also* using a Z-buffer in `drawing.cpp` (which is the modern, robust way to handle depth).
**The Fix:** You can completely remove the `std::sort` in `obj_decoder.cpp`. It's computationally expensive and completely redundant since your Z-buffer handles overlapping geometry correctly.

### D. Duplicate Lighting Logic
You have a `compute_lighting` function in `lighting.cpp`, but the actual lighting math (diffuse dot products, specular power) is hardcoded inside the `rasterise` loop in `drawing.cpp`.
**The Fix:** Refactor `rasterise` to call `lighting::compute_lighting()`, passing in the interpolated position, normal, etc. This will make `rasterise` much cleaner.

---

## 3. Roadmap: What to Do From Here

Here is a step-by-step guide on how to evolve your renderer:

### Step 1: Fix the Core Math and Spaces (Priority)
Before adding new features, fix the bugs mentioned above.
*   Remove the face sorting (Painter's algorithm) in `obj_decoder.cpp`.
*   Update your rasterizer so that it interpolates *world space* positions, not screen space positions, for the lighting calculation.
*   *Bonus:* Implement perspective-correct interpolation. Right now, your barycentric interpolation is linear in screen space, which causes textures and normals to warp slightly when viewed at sharp angles. You need to divide by `w` and interpolate `1/w`.

### Step 2: Get Normal Mapping Working
Follow the steps to implement per-pixel normal mapping:
1.  Update `getTextureNormal` to map `0..255` to `-1.0..1.0`.
2.  Pass UV coordinates (`t0, t1, t2`) into `rasterise`.
3.  Interpolate UVs per-pixel.
4.  Implement the TBN matrix calculation inside `rasterise` to transform the sampled normal map vector into the correct space.

### Step 3: Implement Diffuse and Specular Texturing
Right now, your object is a flat color. 
*   Load the diffuse texture map (e.g., `african_head_diffuse.tga`).
*   Load the specular map (e.g., `african_head_spec.tga`).
*   Inside `rasterise`, use the interpolated UV coordinates to sample the diffuse color (instead of grey) and the specular intensity (to determine how shiny a specific pixel is).

### Step 4: Refactor and Clean Up
*   Move global variables like `TGAImage normalMap` into a proper configuration or `Material` struct that gets passed around.
*   Unify your lighting code into `lighting.cpp` so that `drawing.cpp` is strictly about rasterization math.

### Step 5: Advanced Features (Optional)
Once the above is working, you can tackle advanced concepts:
*   **Shadow Mapping:** Render the scene from the perspective of the light source into a depth buffer first, then use that depth buffer to calculate shadows during the main render pass.
*   **Shaders Architecture:** Instead of hardcoding lighting in `rasterise`, you can create abstract `IShader` classes with `vertex()` and `fragment()` virtual methods, exactly like OpenGL works!

Let me know which step you'd like to tackle first, and I can provide the exact code changes to get it done!
