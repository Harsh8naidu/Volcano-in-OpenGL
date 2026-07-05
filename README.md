# OpenGL 3D Graphics Engine with Custom Model Loader & Skeletal Animation

A real-time 3D renderer built in C++ and OpenGL featuring a custom OBJ/MTL model loader, skeletal animation system, advanced post-processing effects, and FMOD audio integration. The primary focus of this project is building a complete 3D model loading pipeline that parses geometry data, manages materials and textures, and integrates seamlessly with an animation and rendering system. Built as a portfolio piece to demonstrate comprehensive graphics programming, asset pipeline management, and engine architecture skills.

## Features

### Custom 3D Model Loader (Core System)
The foundation of this project is a complete custom model loader built from scratch without using existing asset libraries. The system handles:

**OBJ File Parsing** — Reads and parses OBJ geometry files including vertices, texture coordinates, normals, and face definitions. Handles face triangulation for quads and polygons, manages vertex attribute pointers, and constructs vertex arrays from raw geometry data.

**MTL File Parsing** — Parses material definition files extracting material properties like ambient, specular, emission, shininess, and most importantly, diffuse texture paths. Material ranges track which triangle indices belong to which material for per-submesh texture binding.

**Texture Management** — Loads textures from paths specified in MTL files using SOIL, creates fallback default textures when materials lack texture paths, and manages texture IDs for per-submesh binding during rendering.

**ObjModel Class** — Wraps ModelLoader and MaterialLoader together, manages VAO/VBO/EBO setup, stores material ranges, and provides getter methods for material lookups by name. This unified interface makes it easy to load and render OBJ models throughout the renderer.

The volcano and arena models in the scene are both loaded using this custom system, demonstrating that the loader works with real, complex geometry.

### Skeletal Animation System
Full skeletal animation pipeline for animating rigged characters:

**Mesh Animation Structure** — Stores animation data loaded from custom `.anm` files including frame count, joint count, and per-frame joint transformation matrices (16 values per joint per frame in column-major order).

**AnimatedMesh Struct** — Unified structure storing mesh, animation, material, and animation state (current frame, frame time tracking). Extended to support both orbiting objects and static positioned objects with rotation.

**Skinning Shader** — Vertex shader that blends vertex positions across up to 4 bone influences using precomputed joint matrices and stored bone weights. Combines inverse bind pose matrices with current frame transformations to deform vertices in real-time.

**RegisterAnimatedMesh Function** — Registers animated meshes with orbit parameters (center, radius, angle, speed, height, scale) or static position parameters. Loads submesh textures from materials or uses provided color textures.

Currently animating:
- **Flying birds** — Four instances of the same animated bird mesh orbiting the volcano at different speeds and heights, each facing the direction of travel
- **Dancing skeletons** — 15 skeleton meshes with colored textures spread across the scene, all playing the same dancing animation while slowly rotating
- **Rainbow skeleton** — Special skeleton with animated RGB gradient effect

### Dynamic Post-Processing Pipeline
Advanced multi-pass rendering using framebuffer objects:

**HDR Rendering Pass** — Renders entire scene (skybox, terrain, static models, animated meshes) to floating-point color texture (GL_RGB16F) allowing values above 1.0 for realistic bright light representation.

**Bright Extraction Pass** — Shader isolates pixels above brightness threshold, creating mask of glowing areas for bloom effect.

**Gaussian Blur Passes** — 10-pass blur (5 horizontal, 5 vertical) using ping-pong framebuffers. Each pass reads from one buffer and writes to the other, applying separable Gaussian blur for efficient bloom spreading.

**Tone Mapping & Composite Pass** — Combines original HDR scene with blurred bloom texture and applies Reinhard tone mapping: `color / (color + 1.0)` to compress HDR values back to displayable range while preserving contrast and creating cinematic look.

### DuDv Distortion Effect
Lava terrain uses DuDv (Dudv map) based distortion:
- DuDv map texture stores X and Y offset values (red and green channels)
- Shader samples DuDv map and uses result to distort texture coordinates
- Movement factor (based on time) animates the distortion creating flowing lava effect
- Only applied to first terrain, second terrain renders static

### Lighting System
**Point Lights** — Four point lights positioned around the scene at different locations. Each light contributes diffuse lighting with distance-based attenuation calculated in shaders. Lights are passed to shaders via uniform arrays and affect all lit surfaces.

**Light Structure** — Position (vec3), color and intensity (vec4), and radius (float) determine light influence range.

### Terrain & Environment
**Heightmap Terrain** — Two separate heightmap-based terrains with:
- Different texture mappings (UV scale adjustable per terrain)
- Different height scales and positions
- First terrain has DuDv distortion applied, second is static
- Loaded from heightmap image files and rendered with terrain shader

**Skybox** — Cubemap rendering using six separate textures (right, left, top, bottom, front, back) creating infinite environment. Rendered first with depth writes disabled to avoid obscuring scene geometry.

**Static OBJ Models** — Volcano and arena loaded via custom OBJ loader, rendered with material-per-submesh approach allowing different textures on different parts of same model.

### Audio System (FMOD Integration)
Complete audio system using FMOD SoundSystem API:

**FMOD System Initialization** — Creates FMOD system on startup, initializes with 32 audio channels, and properly cleans up resources in destructor.

**Background Music** — Loads background music file (bg_music.mp3) with looping enabled, plays on startup at 50% volume, continuously throughout scene execution.

**PlaySoundEffect Function** — Loads sound files from disk using FMOD's createSound, plays via fmodSystem->playSound, stores loaded sounds in vector to prevent garbage collection.

**SetMusicVolume Function** — Runtime volume control clamped between 0.0 and 1.0, allows dynamic music volume adjustment during execution.

**Per-Frame Update** — fmodSystem->update() called every frame in UpdateScene() to maintain FMOD's internal state and handle audio streaming.

Audio demonstrates integration of professional middleware into graphics engine and provides immersive sound for the scene.

### Scene Management
**AnimatedMesh Vector** — All animated objects (birds, skeletons) managed in single vector allowing unified update and render loops.

**Orbit Movement** — Birds calculate position on circular path using center point, radius, and current angle updated each frame. Face direction of travel using tangent calculation (angle + 90 degrees).

**Rotation** — Skeletons slowly rotate while dancing. Rotation speed added to AnimatedMesh struct, updated in UpdateScene(), applied via Y-axis rotation matrix.

**Multiple Instances** — Same mesh/animation/material can be registered multiple times with different transforms, demonstrating efficient object reuse.

## Technical Implementation

### Custom File Loaders

**ModelLoader Class** — Reads OBJ files line by line, parses vertex positions (v), texture coordinates (vt), normals (vn), and faces (f). Handles multiple materials via `usemtl` directives. Remaps indices to avoid duplicate vertices, flips V texture coordinate (OBJ vs OpenGL convention), triangulates quads/polygons, tracks material ranges (start index, count) for submesh organization.

**MaterialLoader Class** — Parses MTL files extracting material names and properties (Ka, Ks, Ke, Ns, Ni, d, illum). Loads textures from diffuse/roughness/metallic map paths using SOIL with mipmaps and repeat wrapping. Returns materials by name for lookup during rendering.

**ObjModel Wrapper** — Combines both loaders, manages VAO/VBO/EBO creation, stores material ranges and material objects, provides const getter methods for safe external access.

### Custom Animation File Format

**.anm Format** — Binary animation file storing:
- Header: "MeshAnim", version number
- Frame count, joint count, frame rate
- Per-frame joint data: 16 floats per joint (4x4 matrix in column-major order)

Loaded by MeshAnimation class which provides GetJointData(frame) returning pointer to frame's joint matrices.

### Shader Architecture

**Skinning Vertex Shader** — Takes joint indices and weights per vertex, accumulates transformations from up to 4 bones, applies model/view/projection matrices.

**Fragment Shaders** — Handle:
- Textured rendering with diffuse texture sampling
- Rainbow gradient with time-based animation and DuDv distortion
- Tone mapping for HDR composite pass

**Multiple Shader Programs** — Separate shaders for different rendering tasks (model shader, skinning shader, terrain shader, skybox shader, post-processing shaders).

### Rendering Pipeline Flow

1. **UpdateScene()** — Updates camera, animates meshes (frame advancement, orbit angles, rotations), updates FMOD system
2. **RenderScene()** — Four-pass post-processing:
   - Pass 1: Render all scene geometry to HDR framebuffer
   - Pass 2: Extract bright pixels to separate framebuffer
   - Pass 3: 10-pass Gaussian blur on bright texture
   - Pass 4: Composite original + bloom with tone mapping to screen

### Build System

**CMake Integration** — Links FMOD via environment variable (FMOD_ROOT), includes vcpkg-managed dependencies (OpenEXR), manages third-party libraries (GLAD, SOIL), compiles custom nclgl library.

**FMOD Setup** — CMakeLists.txt reads FMOD_ROOT environment variable set to FMOD SoundSystem api/core directory, manually specifies include and lib paths, links fmod library to executable.

## What This Project Demonstrates

**Graphics Programming** — Understanding of rendering pipeline, matrix transformations, lighting calculations, texture mapping, framebuffer objects, post-processing techniques, and HDR rendering.

**Asset Pipeline** — Building custom loaders for industry-standard formats (OBJ, MTL), managing vertex data, implementing material systems, organizing submeshes and texture binding.

**Animation Systems** — Skeletal animation with bone hierarchies, vertex skinning, frame interpolation, managing animation state across multiple instances.

**Engine Architecture** — Object-oriented design with clear separation of concerns, resource lifecycle management (initialization, updates, cleanup), unified data structures for managing scene objects.

**Integration** — Combining graphics, animation, audio, and physics-related systems into cohesive engine, demonstrating ability to work with multiple APIs and middleware (OpenGL, CMake, FMOD).

**Problem Solving** — Debugging bone hierarchy mismatches, resolving texture binding conflicts, optimizing per-frame updates, implementing complex post-processing pipelines.

## Getting Started

### Prerequisites
- C++17 compiler (MSVC, GCC, or Clang)
- OpenGL 3.3 or higher
- CMake 3.20+
- FMOD Studio (free version)
- vcpkg for package management

### Setup

1. **Install FMOD SoundSystem API** (not FMOD Studio)
   - Download from https://www.fmod.com/download
   - Set environment variable: `FMOD_ROOT = C:\Program Files\FMOD SoundSystem\api\core` (adjust path as needed)

2. **Clone and build**
   ```bash
   git clone <repository>
   cd Advanced-Graphics-For-Games-master
   mkdir build
   cd build
   cmake ..
   cmake --build . --config Debug
   ```

3. **Add assets**
   Create Assets folder structure:
   ```
   Assets/
   ├── models/          (OBJ files: volcano.obj, arena.obj, etc.)
   ├── textures/        (PNG, JPG textures)
   ├── meshes/          (Custom .msh files: fly.msh, DancingSkeleton.msh)
   ├── animations/      (Custom .anm files: FlyAnim.anm, DancingSkeletonAnim.anm)
   ├── materials/       (MTL files and material definitions)
   └── music/           (bg_music.mp3 or OGG file)
   ```

### Running
Execute the compiled binary. The scene will load all assets and begin rendering with background music playing.

## Project Statistics

- **Custom loaders**: OBJ geometry, MTL materials, binary animation files
- **Animated meshes**: 4 birds + 15 skeletons + 1 rainbow skeleton = 20 total
- **Post-processing passes**: 4 (scene, bright extraction, blur, composite)
- **Blur passes**: 10 (5 horizontal + 5 vertical)
- **Shader programs**: 10+ (model, skinning, terrain, skybox, post-processing variants)
- **Joints per skeleton**: 52
- **Animation frames per clip**: 100+ for bird, 456 for skeleton dance

## Notes

This is a comprehensive graphics portfolio project focused on demonstrating core rendering and asset pipeline concepts. The custom model loader is the foundation that enabled all other systems to function with real game assets. Scene design prioritizes showcasing various rendering techniques rather than gameplay, with orbiting birds and dancing skeletons serving as visual demonstrations of animation systems, post-processing, and dynamic scene management.
