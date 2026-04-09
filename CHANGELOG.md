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

