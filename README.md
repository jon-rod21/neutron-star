# Neutron Star Render in OpenGL

## How to Run: 

Clone this repo then run the make_build.sh script and you should be good to go.
Download any dependencies as needed (aka all the opengl and cpp stuff).

## Goals to complete

- [ ] Render star
  - [ ] Star properties (mass, color, rotational speed, heat)
  - [ ] Visible spin
  - [ ] Periodic pulsing at poles
  - [ ] Emit sound depending on rotational frequency (woosh woosh)
  - [ ] Togglable magnetic field

- [ ] Space-time grid
  - [ ] Toggle on and off
  - [ ] Visible distortion that depends on star properties (mass)
  
- [ ] Stars in the background (little spheres or just dots, dont know yet)

- [ ] Solar bodies interacting with star
  - [ ] Asteroids, comets
  - [ ] Planets

- [ ] Interface
  - [ ] Data entry for custom stars (aka put in information somewhere and have that information render in real time)

# Neutron Star Project Timeline
**Target Completion:** End of Semester (mid-May 2026)
**Time Commitment:** 5-10 hours/week

## ✅ Foundation (Already Complete)
- [x] Basic OpenGL window and rendering
- [x] Sphere generation
- [x] Camera controls
- [x] Phong lighting

---

## Week 1-2: Star Visual Improvements (Now - Feb 2)
**Goal:** Sphere that looks and feels like a glowing star

- [ ] Add emissive material (star glow)
- [ ] Implement bloom/glow post-processing effect
- [ ] Add basic rotation animation
- [ ] Color temperature mapping (hot = blue/white, cool = red/orange)

**Deliverable:** Glowing, rotating star with realistic appearance

---

## Week 3-4: Pulsing & Properties (Feb 3 - Feb 16)
**Goal:** Interactive star with visible pulsar beams

- [ ] Pulsing animation at poles (time-based shader)
- [ ] Set up ImGui for UI
- [ ] Add star property controls:
  - Mass slider
  - Rotation speed
  - Temperature
  - Emissive strength
- [ ] Connect UI values to shader uniforms

**Deliverable:** Interactive star with customizable properties and pulsing

---

## Week 5-7: Star Refinement (Feb 17 - Mar 9)
**Goal:** Polished star visuals

- [ ] Fine-tune bloom effect
- [ ] Add procedural surface details (optional)
- [ ] Improve color gradients based on temperature
- [ ] Performance optimization
- [ ] Polish UI layout

**Deliverable:** Production-quality star rendering

---

## Week 8-9: Magnetic Field (Mar 10 - Mar 23)
*Spring break buffer week*

**Goal:** Toggleable magnetic field visualization

- [ ] Research field line visualization techniques
- [ ] Implement magnetic field rendering (particle system or line rendering)
- [ ] Make field strength depend on star properties
- [ ] Add UI toggle

**Deliverable:** Working magnetic field effect

---

## Week 10-11: Space-time Grid (Mar 24 - Apr 6)
**Goal:** Working gravitational distortion

- [ ] Create background grid plane
- [ ] Implement gravitational lensing/distortion shader
- [ ] Make distortion strength depend on mass
- [ ] Add toggle functionality
- [ ] Tune visual effect for realism

**Deliverable:** Toggleable space-time distortion

---

## Week 12-13: Background Stars (Apr 7 - Apr 20)
**Goal:** Populated space environment

- [ ] Add star field (points or small spheres)
- [ ] Implement skybox or procedural background
- [ ] Optional: Parallax effect when camera moves
- [ ] Ensure stars visible through grid

**Deliverable:** Complete space environment

---

## Week 14-15: Audio & Polish (Apr 21 - May 4)
**Goal:** Complete audio-visual experience

- [ ] Integrate audio library (OpenAL or miniaudio)
- [ ] Generate/implement rotation-based sound
- [ ] Bug fixes and visual polish
- [ ] Optimize performance
- [ ] Clean up UI

**Deliverable:** Polished, complete experience

---

## Week 16-17: Solar Bodies (May 5 - Semester End)
**STRETCH GOAL** - Skip if behind schedule

- [ ] Add orbiting objects (asteroids/planets)
- [ ] Basic gravitational interaction visualization
- [ ] Final touches
- [ ] Documentation and README update
- [ ] Demo video/screenshots

**Deliverable:** Final project with all features

---

## Key Milestones
- **Week 7 (Early March):** Assess progress, cut magnetic field if needed
- **Week 11 (Early April):** Final feature cut-off decision
- **Week 14 (Late April):** No new features, polish only

## Success Tips
1. Commit often - even small progress
2. Don't get stuck (2 sessions max per problem)
3. Test on both machines regularly
4. Document as you go
5. Prioritize: Star → Grid → Everything else

## Core MVP Features (Cannot cut)
- Glowing star with rotation
- Property controls (mass, rotation, temperature)
- Space-time grid distortion

## Optional Features (Can cut if time-pressed)
- Magnetic field
- Audio
- Solar bodies
