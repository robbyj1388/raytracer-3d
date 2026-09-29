# 3D Ray Tracer
A lightweight, minimal **3D Ray Tracer** written from scratch in Pure C. 
The project currently generates mathematical primitives by calculating ray-sphere intersections and rendering the results directly to an image file.

<p align="center">
  <img width="434" height="472" alt="image" src="https://github.com/user-attachments/assets/2865e7ff-009e-421b-90d0-79d2d1eb62ee" />
</p>

## Features

* **Pure C Implementation**: Built with zero heavy external dependencies.
* **Ray-Sphere Intersection**: Uses discriminant-based math to find precise ray collisions with spheres.
* **STB Image Integration**: Outputs rendered frames directly to a clean `.png` file using `stb_image_write.h`.
* **Normalized Viewport Camera**: Maps pixel space coordinates to custom 3D world spaces.

## Upcoming Features (In Progress)

The engine is actively being developed. The following features are currently being implemented:

* **Shadows**: Adding light source vectors and shadow rays to handle object occlusion.
* **Triangle Reflections**: Implementing Möller–Trumbore intersection math to support complex 3D triangle meshes and reflective ray bounces.
* **Diffuse & Specular Lighting**: Transitioning from flat colors to dynamic Blinn-Phong shading.

<p align="center">
  <img width="506" height="504" alt="image" src="https://github.com/user-attachments/assets/4c57d8a2-917c-440b-a3c6-89d60854fafd" />
</p>

## Requirements

* A standard **C compiler** (e.g., `gcc`, `clang`).
* The `stb_image_write.h` header file placed inside your project directory.

## Compilation & Setup

To compile and run the engine locally, open your terminal and run:

```bash
gcc main.c -o raytracer -lm
./raytracer
```

*Note: The `-lm` flag is required on Linux/macOS systems to correctly link the math library for functions like `sqrt()` and `pow()`.*

## Architecture & Math Overview

The project relies on casting primary camera rays from an origin into a viewport grid:

1. **Ray Normalization**: Vectors are converted to unit length vectors using the standard Euclidean norm:
   \[\text{norm} = \sqrt{x^2 + y^2 + z^2}\]
2. **Discriminant Check**: Sphere intersections use the quadratic discriminant:
   \[b^2 - 4ac\]
   * If the result is **\(\ge 0\)**, a hit is registered, painting a white sphere pixel.
   * If the result is **\(< 0\)**, it maps to the empty background canvas, painting a dark red pixel.
