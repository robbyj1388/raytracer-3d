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
make raytracing 
./raytracer
```

*Note: The makefile `-lm` flag is required on Linux/macOS systems to correctly link the math library for functions like `sqrt()` and `pow()`.*

## Architecture & Math Overview

The project relies on casting primary camera rays from an origin into a viewport grid and using vector and intersection mathematics to determine what each pixel should display.

### 1. Ray Normalization

Camera rays and other direction vectors are normalized into unit-length vectors using the standard Euclidean norm:

$$
||v|| = \sqrt{x^2 + y^2 + z^2}
$$

The normalized vector is calculated by dividing each component by the vector's magnitude:

$$
\hat{v} = \frac{v}{||v||}
$$

This allows direction vectors to be used consistently when calculating intersections, lighting, and reflections.

### 2. Ray-Sphere Intersection

Sphere intersections are calculated by substituting the ray equation into the sphere equation and solving the resulting quadratic equation.

For a ray:

$$
P(t) = e + td
$$

where \(e\) is the ray origin and \(d\) is the normalized ray direction.

For a sphere with center \(c\) and radius \(R\):

$$
||P(t)-c||^2 = R^2
$$

The resulting quadratic uses:

$$
a = d \cdot d
$$

$$
b = 2d \cdot (e-c)
$$

$$
c = (e-c)\cdot(e-c)-R^2
$$

The discriminant determines whether the ray intersects the sphere:

$$
\Delta = b^2 - 4ac
$$

* If `Δ < 0`, the ray misses the sphere.
* If `Δ >= 0`, the ray intersects the sphere.

The intersection distances are:

$$
t = \frac{-b \pm \sqrt{\Delta}}{2a}
$$

The smallest positive \(t\) is selected as the closest visible intersection.

### 3. Ray-Triangle Intersection

Triangle intersections use the Möller-Trumbore algorithm.

The triangle is defined by vertices \(a\), \(b\), and \(c\), while the ray is defined by its origin and direction.

The edge vectors are:

$$
E_1 = a-b
$$

$$
E_2 = a-c
$$

The algorithm solves for the ray distance \(t\) and the barycentric coordinates \(\beta\) and \(\gamma\).

A valid intersection must satisfy:

$$
t \geq 0
$$

$$
\gamma \geq 0
$$

$$
\gamma \leq 1
$$

$$
\beta \geq 0
$$

$$
\beta \leq 1-\gamma
$$

These conditions ensure that the intersection occurs in front of the camera and inside the triangle rather than outside its edges.

### 4. Surface Normals

Surface normals describe the direction perpendicular to a surface.

For a sphere, the normal is calculated from the sphere's center to the intersection point:

$$
N = P-C
$$

where \(P\) is the intersection point and \(C\) is the sphere center.

The resulting vector is normalized before being used for lighting and reflection calculations.

For triangles, two edges are calculated:

$$
E_1 = b-a
$$

$$
E_2 = c-a
$$

The normal is then calculated using the cross product:

$$
N = E_1 \times E_2
$$

which produces:

$$
N =
\begin{bmatrix}
E_{1y}E_{2z}-E_{1z}E_{2y}\\
E_{1z}E_{2x}-E_{1x}E_{2z}\\
E_{1x}E_{2y}-E_{1y}E_{2x}
\end{bmatrix}
$$

### 5. Closest Intersection

Each camera ray is tested against the scene's spheres and triangles. When multiple objects are intersected, the intersection with the smallest positive \(t\) is selected.

This ensures that objects closer to the camera correctly appear in front of objects behind them.

### 6. Diffuse Lighting

Diffuse lighting is calculated using the dot product between the surface normal and the normalized direction from the surface toward the light.

The light direction is:

$$
L = LightPosition - P
$$

where \(P\) is the intersection point.

After normalizing \(L\), the diffuse value is calculated as:

$$
D = N \cdot L
$$

The dot product determines how directly the surface faces the light:

* \(D \approx 1\): Surface faces the light directly.
* \(D \approx 0\): Surface receives little direct light.
* \(D < 0\): Surface faces away from the light.

The renderer applies a minimum diffusion threshold:

$$
D = \max(D, 0.2)
$$

This prevents surfaces from becoming completely black.

The final pixel color is then calculated by multiplying the material color by the diffuse value:

$$
Color = MaterialColor \times D
$$

### 7. Reflections

Reflective objects calculate a new ray direction using the incoming direction \(d\) and the surface normal \(n\).

The reflection equation used by the renderer is:

$$
r = d - 2(d\cdot n)n
$$

where:

* \(d\) is the incoming normalized ray direction.
* \(n\) is the normalized surface normal.
* \(r\) is the resulting reflection direction.

The reflected ray originates from the previous intersection point and is traced through the scene again. This process can continue for multiple bounces, with the current implementation allowing up to 10 reflection bounces.

### 8. Camera and Viewport

The camera is positioned at the origin:

$$
(0,0,0)
$$

The renderer creates a ray for each pixel by converting the pixel's coordinates into a normalized viewport ranging approximately from \(-1\) to \(1\).

The ray direction is constructed from the pixel's \(x\) and \(y\) coordinates and a fixed \(z\) value:

$$
d = (x,y,-2)
$$

The resulting direction is normalized before being used for intersection calculations.

Each pixel is therefore represented by a primary ray traveling from the camera through the viewport into the 3D scene.
