4/8/2026 11:50pm

Feature: HDR Bloom & Dynamic Surface Rendering

== Added ==

HDR Framebuffer Class (Framebuffer.h): Implemented a custom FBO supporting GL_RGBA16F floating-point color attachments to preserve high dynamic range lighting values without clamping.

3-Pass Render Pipeline (main.cpp): Restructured the main application render loop to support advanced post-processing:

Pass 1: Scene rendering and bright-pixel extraction via Multiple Render Targets (MRT).

Pass 2: Two-pass "ping-pong" Gaussian blur to generate the soft bloom effect.

Pass 3: Final composite blending the base scene with the blurred glow, utilizing exposure-based tone mapping and gamma correction.

Dynamic Surface Animation: Introduced a time uniform to the star's fragment shader, wiring it to glfwGetTime() to continuously animate the Fractal Brownian Motion (FBM) surface noise.

Resource Cleanup: Added proper deallocation logic for custom framebuffers and ping-pong textures prior to application termination.

== Changed ==

Fragment Shader Outputs: Upgraded fragment_shader.glsl from a single FragColor output to utilize Multiple Render Targets (location = 0 for base color, location = 1 for brightness threshold).

Surface Emission Math: Reworked the emission pattern calculation in the fragment shader. Applied exponential darkening (pow(surfaceNoise, 2.5)) to the FBM noise to create deep, dark surface crevices, drastically increasing the contrast and intensity of the glowing hot spots.

=========================================================
4/9/2026 12:50am

Feature: Procedural Environment & Post-Processing Optimization

Added

Procedural Skybox Implementation: Developed a dynamic starfield using a 3D coordinate-based hash function within a cube-map shader.

Integrated a grid-snapping algorithm (floor() based) to eliminate temporal aliasing and "shimmering" effects during camera movement.

Implemented density and brightness controls to generate a sparse, realistic deep-space background.

BloomRenderer Class: Abstracted the complex multi-pass post-processing logic into a modular C++ class to improve main.cpp maintainability.

Encapsulated the 2D screen quad, Gaussian blur ping-pong buffers, and final HDR composition logic.

Rim Lighting (Fresnel Effect): Added an edge-glow effect in the star's fragment shader using the dot product of the surface normal and view direction to simulate a corona/eclipse aesthetic.

== Changed ==

Main Render Loop Refinement: Streamlined the application's core loop by replacing manual OpenGL state management with the BloomRenderer interface.

Initial Camera Configuration: Adjusted the default camera position to Z=6.5f to ensure the full visual extent of the star and its HDR glow is centered and visible upon startup.

Star Surface Contrast: Enhanced the Fractal Brownian Motion (FBM) math to increase the depth of dark surface segments, creating a more dramatic contrast with glowing poles and emission patterns.
+1

== Fixed ==

Background Recalculation Issues: Resolved an issue where the starfield appeared to jitter or recalculate rapidly when the mouse moved by locking the procedural noise to a fixed spatial resolution.

Memory Management: Added comprehensive cleanup for skybox vertex arrays and post-processing buffers to prevent memory leaks during application exit.