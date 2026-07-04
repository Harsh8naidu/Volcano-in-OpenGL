#include "Renderer.h"
#include "../nclgl/Camera.h"
#include "../nclgl/Light.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/MeshAnimation.h"
#include "../nclgl/MeshMaterial.h"
#include "Volcano.h"
#include "BonyWall.h"
#include "Monster.h"
#include "../nclgl/Math_Utility.h"
#include "VolcanicRock.h"

#include <filesystem>
#include <iostream>

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	// Load .obj files
	volcanoModel = new ObjModel(MODELDIR "volcano_model.obj", MATERIALDIR "volcano_model.mtl");
    mountainModel   = new ObjModel(MODELDIR "mountain.obj", MATERIALDIR "mountain.mtl");

    LoadMeshes();
    LoadMaterials();
    LoadAnimations();
    LoadTextures();
    LoadShaders();

    AnimateBird();
    AnimateSkeleton();

	// Set up the camera and light
	camera = new Camera(-10.0f, 190.0f, Vector3(2500.0f, 2000.0f, 0.0f)); // -0.88f
	camera->SetPitch(0.0f);
	camera->SetYaw(180.0f);

    AddLightsToScene();

	// Set up the matrices
	projMatrix = Matrix4::Perspective(1.0f, 50000.0f, (float)width / (float)height, 45.0f);

	// Set up the root node and add the models
	rootNode = new SceneNode();
	//rootNode->AddChild(new Volcano(volcanoMesh, volcanoTexture));

	// Set up OpenGL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	init = true;
}

Renderer::~Renderer(void) {
	// Cleanups
	delete camera;
	delete heightMap;
	delete quad;

	// shaders cleanup
    for (Shader* shader : shaders) {
        delete shader;
    }

	// light cleanup
    for (Light* light : sceneLights) {
        delete light;
    }

	// models cleanup
    for (Mesh* mesh : meshes) {
        delete mesh;
    }

	// textures cleanup
    for (GLuint tex : textures) {
        glDeleteTextures(1, &tex);
    }

	// objects cleanup
    delete volcanoModel;
}

void Renderer::UpdateScene(float dt) {
    frameTime += dt;
	
    //Update the camera
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();

    for (auto& entry : animatedMeshes) {
        // Animation Update
        entry.frameTime -= dt;
        while (entry.frameTime < 0.0f) {
            entry.currentFrame = (entry.currentFrame + 1) % entry.animation->GetFrameCount();
            entry.frameTime += 1.0f / entry.animation->GetFrameRate(); // 
        }

        // Orbit movement
        if (entry.orbitSpeed > 0.0f) {
            entry.orbitAngle += entry.orbitSpeed * dt;
        }

        // Rotation for skeletons
        if (entry.rotationSpeed > 0.0f) {
            entry.currentYaw += entry.rotationSpeed * dt * (PI / 180.0f);  // Convert to radians per second
        }
    }
	//rootNode->Update(dt);
}

void Renderer::RenderScene() {
    // =========================================================
    // PASS 1 : Render HDR scene into hdrFBO
    // =========================================================

    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    DrawSkybox();
	DrawHeightmap();
	DrawVolcano();
    DrawArena();
    DrawAnimatedMesh();
	
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // =========================================================
    // PASS 2 : Extract bright areas into brightFBO
    // =========================================================
    glBindFramebuffer(GL_FRAMEBUFFER, brightFBO);
    glViewport(0, 0, width, height);
    
    glClear(GL_COLOR_BUFFER_BIT);

    BindShader(brightShader);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);
    
    glUniform1i(glGetUniformLocation(brightShader->GetProgram(), "sceneTexture"), 0);

    DrawPostProcessQuad();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // =========================================================
    // PASS 3 : Blur bright areas using ping-pong FBOs
    // =========================================================

    bool horizontal = true, firstIteration = true;
    
    int blurPasses = 10; // Number of blur passes

    BindShader(blurShader);

    for (int i = 0; i < blurPasses; i++) {
        // Write to the opposite ping-pong FBO each pass
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        // Tell the shader which direction to blur (horizontal or vertical)
        glUniform1i(glGetUniformLocation(blurShader->GetProgram(), "horizontal"), horizontal);

        // First blur pass uses the bright texture, subsequent passes use the previous ping-pong result
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, firstIteration ? brightTexture : pingpongColorbuffers[!horizontal]);

        glUniform1i(glGetUniformLocation(blurShader->GetProgram(), "image"), 0);
        DrawPostProcessQuad();

        horizontal = !horizontal; // Toggle direction
        
        if (firstIteration) {
            firstIteration = false;
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);

    // =========================================================
    // PASS 4 : Combine original scene with blurred bloom texture
    // =========================================================

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    BindShader(combineShader);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrColorBuffer);
    glUniform1i(glGetUniformLocation(combineShader->GetProgram(), "sceneTexture"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]); // Final blurred texture
    glUniform1i(glGetUniformLocation(combineShader->GetProgram(), "bloomTexture"), 1);

    DrawPostProcessQuad();
}

void Renderer::DrawVolcano() {
	BindShader(objModelShader);

	modelMatrix = Matrix4::Translation(Vector3(4500.0f, 400.0f, 4800.0f)) *
		Matrix4::Scale(Vector3(1.5f, 1.5f, 1.5f)) *
		Matrix4::Rotation(90, Vector3(0, 1, 0)) * 
        Matrix4::Rotation(3, Vector3(1, 0, 0));

    UpdateShaderMatrices();

    Vector3 camPos = camera->GetPosition();
    glUniform3fv(glGetUniformLocation(objModelShader->GetProgram(), "cameraPos"), 1, (float*)&camPos);

    SetShaderLights(sceneLights);

    glBindVertexArray(volcanoModel->modelVAO); // Bind the VAO
    for (const MaterialRange& range : volcanoModel->GetMaterialRanges()) {
        BindMaterial(volcanoModel->GetMaterialByName(range.materialName));
        glDrawElements(GL_TRIANGLES, range.indexCount, GL_UNSIGNED_INT, (void*)(range.startIndex * sizeof(unsigned int)));
    }
    glBindVertexArray(0);
}

void Renderer::DrawArena() {
    BindShader(objModelShader);

    modelMatrix =
        Matrix4::Translation(Vector3(7000.0f, 80.0f, 8000.0f)) *   // Change position
        Matrix4::Scale(Vector3(1600.0f,700.0f, 1600.0f)) *          // Change scale
        Matrix4::Rotation(0.0f, Vector3(0, 1, 0));           // Change rotation if needed

    UpdateShaderMatrices();

    Vector3 camPos = camera->GetPosition();
    glUniform3fv(
        glGetUniformLocation(objModelShader->GetProgram(), "cameraPos"),
        1,
        (float*)&camPos
    );

    SetShaderLights(sceneLights);

    glBindVertexArray(mountainModel->modelVAO);

    for (const MaterialRange& range : mountainModel->GetMaterialRanges()) {
        BindMaterial(mountainModel->GetMaterialByName(range.materialName));

        glDrawElements(
            GL_TRIANGLES,
            range.indexCount,
            GL_UNSIGNED_INT,
            (void*)(range.startIndex * sizeof(unsigned int))
        );
    }

    glBindVertexArray(0);
}

void Renderer::DrawSkybox() {
	// Draw the skybox
	glDepthMask(GL_FALSE);

	BindShader(skyboxShader);
	UpdateShaderMatrices();

    BindTexture(cubeMap, GL_TEXTURE0, "cubeTex", GL_TEXTURE_CUBE_MAP);

	quad->Draw();

	glDepthMask(GL_TRUE);
}

void Renderer::DrawHeightmap() {
	// Draw the heightmap
	BindShader(terrainShader);

    Vector3 camPos = camera->GetPosition();
    glUniform3fv(glGetUniformLocation(terrainShader->GetProgram(), "cameraPos"), 1, (float*)&camPos);

    SetShaderLights(sceneLights);

    float scaleHMapX = 1.0f; 
    float scaleHMapY = 1.0f; 
    float scaleHMapZ = 1.0f;

    Vector3 hMapSize = heightMap->GetHeightmapSize();
    Vector3 nMapSize = heightMap2->GetHeightmapSize();

    // Scale noiseHeightMap to match heightMap's world size
    float noiseScaleX = scaleHMapX * (hMapSize.x / nMapSize.x);
    float noiseScaleZ = scaleHMapZ * (hMapSize.z / nMapSize.z);

    // ---------- First Terrain ----------
    glUniform1i(glGetUniformLocation(terrainShader->GetProgram(), "applyDuDv"), 1);
    glUniform2f(glGetUniformLocation(terrainShader->GetProgram(), "uvScale"), 0.01f, 0.01f);
    glUniform1i(glGetUniformLocation(terrainShader->GetProgram(), "diffuseTex"), 0);
    glUniform1i(glGetUniformLocation(terrainShader->GetProgram(), "heightMap"), 1);
    BindTexture(volcanoTexture, GL_TEXTURE0, "diffuseTex", GL_TEXTURE_2D);
    BindTexture(heightMapTex2, GL_TEXTURE1, "heightMap", GL_TEXTURE_2D);
    BindTexture(lavaNoiseTex, GL_TEXTURE2, "dudvMap", GL_TEXTURE_2D);
    float moveFactor = fmod(frameTime, 50.0f);
    glUniform1f(glGetUniformLocation(terrainShader->GetProgram(), "moveFactor"), moveFactor);
    
    modelMatrix = Matrix4::Translation(Vector3(0, 200.0f, 0)) *
        Matrix4::Scale(Vector3(scaleHMapX, 5.0f, scaleHMapZ)) *
        Matrix4::Rotation(-6.2f, Vector3(0, 1, 0));
	UpdateShaderMatrices();
	heightMap->Draw();

    // ---------- Second Terrain ----------
    // UV scale for diffuse texturing
    glUniform1i(glGetUniformLocation(terrainShader->GetProgram(), "applyDuDv"), 0);
    glUniform2f(glGetUniformLocation(terrainShader->GetProgram(), "uvScale"), 0.8f, 0.8f);
    BindTexture(earthTex, GL_TEXTURE3, "diffuseTex", GL_TEXTURE_2D);
    BindTexture(heightMapTex, GL_TEXTURE4, "heightMap", GL_TEXTURE_2D);

    modelMatrix = Matrix4::Translation(Vector3(0.0f, 360.0f, 0.0f)) *
        Matrix4::Scale(Vector3(noiseScaleX, 4.0f, noiseScaleZ)) *
        Matrix4::Rotation(-6.2f, Vector3(0, 1, 0));
    UpdateShaderMatrices();
    heightMap2->Draw();
}

void Renderer::DrawAnimatedMesh() {
    BindShader(skinningShader);

    BindTexture(rainbowGradientTex, GL_TEXTURE0, "rainbowGradientTex", GL_TEXTURE_1D);
    BindTexture(lavaNoiseTex, GL_TEXTURE1, "dudvMap", GL_TEXTURE_2D);
    
    GLenum useRainbowLoc = glGetUniformLocation(skinningShader->GetProgram(), "useRainbowGradient");

    float timeOffset = fmod(frameTime, 10.0f) / 10.0f; // Loop every 10 seconds
    glUniform1f(glGetUniformLocation(skinningShader->GetProgram(), "timeOffset"), timeOffset);

    Vector3 pos = Vector3(0.0f, 0.0f, 0.0f);
    float yaw = 0.0f;
    for (AnimatedMesh& entry : animatedMeshes) {
        
        if (entry.orbitSpeed > 0.0f) {
            // Orbit movement (birds)
            // Calculate position on circular path
            float x = entry.position.x + entry.orbitRadius * cos(entry.orbitAngle);
            float z = entry.position.z + entry.orbitRadius * sin(entry.orbitAngle);
            pos = Vector3(x, entry.height, z);

            // Face the direction of travel (tangent to the circle)
            yaw = entry.orbitAngle + PI * 1.5; // +90 degrees to face forward
            yaw = -yaw; // Negate to correct for coordinate system
        }
        else {
            // Static position (skeleton or any other non-orbiting model)
            pos = entry.position;
            yaw = entry.currentYaw;
        }

        modelMatrix = Matrix4::Translation(Vector3(pos))
            * Matrix4::Rotation(yaw * (180.0f / PI), Vector3(0, 1, 0))
            * Matrix4::Scale(Vector3(entry.scale, entry.scale, entry.scale));
        UpdateShaderMatrices();

        // Upload animation joint matrices
        vector<Matrix4> frameMatrices;
        const Matrix4* invBindPose = entry.mesh->GetInverseBindPose();
        const Matrix4* frameData = entry.animation->GetJointData(entry.currentFrame);
        
        for (int i = 0; i < entry.mesh->GetJointCount(); i++) {
           frameMatrices.emplace_back(frameData[i] * invBindPose[i]);
        }

        int j = glGetUniformLocation(skinningShader->GetProgram(), "joints");
        glUniformMatrix4fv(j, frameMatrices.size(), false, (float*)frameMatrices.data());

        
        // Draw submeshes with their textures
        for (int i = 0; i < entry.mesh->GetSubMeshCount(); i++) {
            if (!entry.isSkeleton) {
                // Birds: bind their color texture to GL_TEXTURE2 and set the uniform
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, entry.textures[i]);
                glUniform1i(glGetUniformLocation(skinningShader->GetProgram(), "diffuseTex"), 2);
                glUniform1i(glGetUniformLocation(skinningShader->GetProgram(), "useRainbowGradient"), 0);
            }
            else {
                // Skeletons: use rainbowGradient (already bound to unit 0)
                glUniform1i(glGetUniformLocation(skinningShader->GetProgram(), "useRainbowGradient"), 1);
            }
            entry.mesh->DrawSubMesh(i);
        }
    }
}

void Renderer::RegisterAnimatedMesh(Mesh* mesh, MeshAnimation* anim, MeshMaterial* material, 
    Vector3 center, float radius, float startAngle, float speed, float height, float scale) {
    
    AnimatedMesh entry;
    entry.mesh        = mesh;
    entry.animation   = anim;
    entry.material    = material;
    entry.position      = center;
    entry.orbitRadius = radius;
    entry.orbitAngle  = startAngle;
    entry.orbitSpeed  = speed;
    entry.height      = height;
    entry.scale       = scale;

    for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
        const MeshMaterialEntry* matEntry = material->GetMaterialForLayer(i);
        const string* filename = nullptr;
        if (matEntry && matEntry->GetEntry("Diffuse", &filename) && filename) {
            // Load the actual diffuse texture
            GLuint texID = SOIL_load_OGL_texture(
                (TEXTUREDIR + *filename).c_str(),
                SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
                SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS
            );
            heightMapTex = SOIL_load_OGL_texture(TEXTUREDIR "noise.png", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS);
            entry.textures.push_back(texID);
        }
        else {
            // Fall back to default white texture
            entry.textures.push_back(defaultDiffuse);
        }
    }

    animatedMeshes.push_back(entry);
}

void Renderer::RegisterAnimatedMesh(Mesh* mesh, MeshAnimation* anim, MeshMaterial* material, Vector3 positon, float yaw, float scale, GLuint colorTexture = 0)
{
    AnimatedMesh entry;
    entry.mesh = mesh;
    entry.animation = anim;
    entry.material = material;
    entry.position = positon;
    entry.currentYaw = yaw;
    entry.scale = scale;
    entry.orbitSpeed = 0.0f;
    entry.orbitRadius = 0.0f;
    entry.isSkeleton = true;
    entry.rotationSpeed = entry.isSkeleton ? 45.0f : 0.0f; // 45 degrees per second

    for (int i = 0; i < mesh->GetSubMeshCount(); ++i) {
        if (colorTexture != 0) {
            entry.textures.push_back(defaultDiffuse);
        }
        else {
            const MeshMaterialEntry* matEntry = material->GetMaterialForLayer(i);
            const string* filename = nullptr;
            if (matEntry && matEntry->GetEntry("Diffuse", &filename) && filename) {
                // Load the actual diffuse texture from material
                GLuint texID = SOIL_load_OGL_texture(
                    (TEXTUREDIR + *filename).c_str(),
                    SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
                    SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS
                );
                heightMapTex = SOIL_load_OGL_texture(TEXTUREDIR "noise.png", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS);
                entry.textures.push_back(texID);
            }
            else {
                // Fall back to default white texture
                entry.textures.push_back(defaultDiffuse);
            }
        }
    }

    animatedMeshes.push_back(entry);
}

void Renderer::DrawNode(SceneNode* n) {
	// Draw all the children of the node
	BindShader(modelShader);
	UpdateShaderMatrices();

	if (n->GetMesh()) {
		Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
		glUniformMatrix4fv(glGetUniformLocation(modelShader->GetProgram(), "modelMatrix"), 1, false, model.values);
		glUniform4fv(glGetUniformLocation(modelShader->GetProgram(), "nodeColour"), 1, (float*)&n->GetColour());
		modelTexture = n->GetTexture();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, modelTexture);
		glUniform1i(glGetUniformLocation(modelShader->GetProgram(), "useTexture"), modelTexture);
	}

	n->Draw(*this);

	for (vector<SceneNode*>::const_iterator i = n->GetChildIteratorStart();
		i != n->GetChildIteratorEnd(); ++i) {
		DrawNode(*i);
	}
}

void Renderer::DrawPostProcessQuad()
{
    glDisable(GL_DEPTH_TEST);

    postProcessQuad->Draw();

    glEnable(GL_DEPTH_TEST);
}

void Renderer::AddLightsToScene() {
    // south lights
    sceneLights.push_back(new Light(
        Vector3(7000.0f, 500.0f, 1000.0f),
        Vector4(30.0f, 2.0f, 0.0f, 1.0f),   // Blood red
        5000.0f));

    sceneLights.push_back(new Light(
        Vector3(500.0f, 3000.0f, 700.0f),
        Vector4(35.0f, 12.0f, 0.0f, 1.0f),  // Molten orange
        4000.0f));

    sceneLights.push_back(new Light(
        Vector3(8000.0f, 500.0f, 1000.0f),
        Vector4(18.0f, 0.0f, 12.0f, 1.0f),  // Dark purple
        5000.0f));

    sceneLights.push_back(new Light(
        Vector3(13000.0f, 3000.0f, 2000.0f),
        Vector4(25.0f, 5.0f, 3.0f, 1.0f),   // Deep crimson
        4000.0f));

    // north lights
    sceneLights.push_back(new Light(
        Vector3(3000.0f, 3000.0f, 18000.0f),
        Vector4(30.0f, 2.0f, 0.0f, 1.0f),   // Blood red
        3000.0f));

    sceneLights.push_back(new Light(
        Vector3(8000.0f, 3000.0f, 15000.0f),
        Vector4(18.0f, 0.0f, 12.0f, 1.0f),  // Dark purple
        3000.0f));

    sceneLights.push_back(new Light(
        Vector3(13000.0f, 2000.0f, 13000.0f),
        Vector4(30.0f, 2.0f, 0.0f, 1.0f),   // Blood Red
        5000.0f));

    sceneLights.push_back(new Light(
        Vector3(1000.0f, 2000.0f, 13000.0f),
        Vector4(18.0f, 0.0f, 12.0f, 1.0f),   // Dark purple
        5000.0f));

    // West lights
    sceneLights.push_back(new Light(
        Vector3(-1000.0f, 3000.0f, 8200.0f),
        Vector4(18.0f, 0.0f, 12.0f, 1.0f),  // Dark purple
        3000.0f));

    sceneLights.push_back(new Light(
        Vector3(-500.0f, 3000.0f, 5000.0f),
        Vector4(35.0f, 12.0f, 0.0f, 1.0f),  // Molten orange
        2500.0f));

    // East lights
    sceneLights.push_back(new Light(
        Vector3(16000.0f, 3000.0f, 7000.0f),
        Vector4(30.0f, 2.0f, 0.0f, 1.0f),  // Blood red
        3000.0f));

    // Lava lights
    sceneLights.push_back(new Light(
        Vector3(2300.0f, 3000.0f, 10000.0f),
        Vector4(35.0f, 12.0f, 0.0f, 1.0f),  // Molten orange
        3200.0f));

    sceneLights.push_back(new Light(
        Vector3(4900.0f, 3000.0f, 7800.0f),
        Vector4(35.0f, 12.0f, 0.0f, 1.0f),  // Molten orange
        2800.0f));

    sceneLights.push_back(new Light(
        Vector3(6000.0f, 3000.0f, 5000.0f),
        Vector4(35.0f, 12.0f, 0.0f, 1.0f),  // Molten orange
        3200.0f));
}

void Renderer::LoadShaders()
{
    // Load the shaders
    objModelShader = new Shader("modelVertexShader.glsl", "modelFragmentShader.glsl");
    modelShader = new Shader("SceneVertex.glsl", "SceneFragment.glsl");
    reflectShader = new Shader("reflectVertex.glsl", "reflectFragment.glsl");
    skyboxShader = new Shader("skyboxVertex.glsl", "skyboxFragment.glsl");
    lightShader = new Shader("PerPixelVertex.glsl", "PerPixelFragment.glsl");
    terrainShader = new Shader("terrainVertexShader.glsl", "terrainFragmentShader.glsl");
    skinningShader = new Shader("SkinningVertex.glsl", "TexturedFragment.glsl");
    brightShader = new Shader("post.glsl", "bright.glsl");
    blurShader = new Shader("post.glsl", "blur.glsl");
    combineShader = new Shader("post.glsl", "combine.glsl");

    shaders.push_back(objModelShader);
    shaders.push_back(modelShader);
    shaders.push_back(reflectShader);
    shaders.push_back(skyboxShader);
    shaders.push_back(lightShader);
    shaders.push_back(terrainShader);
    shaders.push_back(skinningShader);
    shaders.push_back(brightShader);
    shaders.push_back(blurShader);
    shaders.push_back(combineShader);

    // Check if any shader failed to load
    for (Shader* shader : shaders) {
        if (!shader->LoadSuccess()) {
            std::cerr << "Failed to load a shader. Check the file paths and ensure the files exist.\n";
            return;
        }
    }
}

void Renderer::LoadTextures()
{
    // Load the heightmaps
    heightMap = new HeightMap(TEXTUREDIR "Heightmap_01_Mountain.jpg");
    heightMap2 = new HeightMap(TEXTUREDIR "Heightmap_Dunes.jpg");

    // Load the textures
    heightMapTex = SOIL_load_OGL_texture(TEXTUREDIR "noise.png", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS);
    heightMapTex2 = SOIL_load_OGL_texture(TEXTUREDIR "Heightmap_Dunes.png", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS);
    lavaTex = SOIL_load_OGL_texture(TEXTUREDIR "lava_texture.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS);
    earthTex = SOIL_load_OGL_texture(TEXTUREDIR "volcanic_rock.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS);
    volcanoTexture = SOIL_load_OGL_texture(TEXTUREDIR "lava_texture.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS);
    lavaNoiseTex = SOIL_load_OGL_texture(TEXTUREDIR "lava-noise.png", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS);

    // Load the cubemap textures
    cubeMap = SOIL_load_OGL_cubemap(
        TEXTUREDIR "right.jpg", TEXTUREDIR "left.jpg",
        TEXTUREDIR "top.jpg", TEXTUREDIR "bottom.jpg",
        TEXTUREDIR "front.jpg", TEXTUREDIR "back.jpg",
        SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS
    );

    // Push textures into a vector array
    textures.push_back(heightMapTex);
    textures.push_back(heightMapTex2);
    textures.push_back(lavaTex);
    textures.push_back(earthTex);
    textures.push_back(volcanoTexture);
    textures.push_back(cubeMap);

    // Check if any texture failed to load
    for (GLuint tex : textures) {
        if (!tex) {
            std::cerr << "Failed to load a texture. Check the file paths and ensure the files exist.\n";
            return;
        }
    }

    // Set the texture to repeat
    SetTextureRepeating(earthTex, true);
    SetTextureRepeating(lavaTex, true);

    rainbowGradientTex = CreateRainbowGradient();
}

void Renderer::LoadMeshes()
{
    // Load the meshes
    quad = Mesh::GenerateQuad();
    postProcessQuad = Mesh::GenerateQuad();
    bird = Mesh::LoadFromMeshFile("fly.msh");
    skeleton = Mesh::LoadFromMeshFile("DancingSkeleton.msh");

    // Push meshes into a vector array
    meshes.push_back(quad);
    meshes.push_back(bird);
    meshes.push_back(skeleton);

    for (Mesh* mesh : meshes) {
        if (!mesh) {
            std::cerr << "Failed to load a mesh. Check the file paths and ensure the files exist.\n";
            return;
        }
    }
}

void Renderer::LoadAnimations()
{
    // Load the animation files
    birdAnim = new MeshAnimation("FlyAnim.anm");
    skeletonAnim = new MeshAnimation("DancingSkeletonAnim.anm");

    animations.push_back(birdAnim);
    animations.push_back(skeletonAnim);

    for (MeshAnimation* anim : animations) {
        if (!anim) {
            std::cerr << "Failed to load an animation. Check the file paths and ensure the files exist.\n";
            return;
        }
    }
}

void Renderer::LoadMaterials()
{
    // Load the material
    birdMaterial = new MeshMaterial("fly.mat");
    skeletonMaterial = new MeshMaterial("DancingSkeleton.mat");

    meshMaterials.push_back(birdMaterial);
    meshMaterials.push_back(skeletonMaterial);

    for (MeshMaterial* mtl : meshMaterials) {
        if (!mtl) {
            std::cerr << "Failed to load a mesh's material. Check the file paths and ensure the files exist.\n";
            return;
        }
    }
}

void Renderer::AnimateBird()
{
    // Mountain center - adjust to match your heightmap
    Vector3 mountainCenter = Vector3(7000.0f, 400.0f, 7500.0f);
    float orbitRadius = 2000.0f;
    float birdScale = 800.0f;

    // Spread birds evenly around the circle using PI
    RegisterAnimatedMesh(bird, birdAnim, birdMaterial, mountainCenter, orbitRadius, 0.0f, 0.5f, 2500.0f, birdScale);
    RegisterAnimatedMesh(bird, birdAnim, birdMaterial, mountainCenter, orbitRadius, PI * 0.5f, 0.6f, 2700.0f, birdScale);
    RegisterAnimatedMesh(bird, birdAnim, birdMaterial, mountainCenter, orbitRadius, PI, 0.4f, 2600.0f, birdScale);
    RegisterAnimatedMesh(bird, birdAnim, birdMaterial, mountainCenter, orbitRadius, PI * 1.5f, 0.5f, 2800.0f, birdScale);
}

void Renderer::AnimateSkeleton()
{
    Vector3 positions[] = {
        Vector3(3000.0f, 2000.0f, 2000.0f),
        Vector3(12000.0f, 2000.0f, 3000.0f),
        Vector3(5000.0f, 2000.0f, 12000.0f),
        Vector3(14000.0f, 2000.0f, 10000.0f),
        Vector3(2000.0f, 2000.0f, 8000.0f),
        Vector3(11000.0f, 2000.0f, 1500.0f),
        Vector3(7000.0f, 2000.0f, 15000.0f),
        Vector3(15000.0f, 2000.0f, 6000.0f),
        Vector3(4500.0f, 2000.0f, 4500.0f),
        Vector3(13000.0f, 2000.0f, 13000.0f),
        Vector3(1500.0f, 2000.0f, 14000.0f),
        Vector3(10500.0f, 2000.0f, 7500.0f),
        Vector3(6000.0f, 2000.0f, 2500.0f),
        Vector3(12500.0f, 2000.0f, 14500.0f),
        Vector3(3500.0f, 2000.0f, 11000.0f),
    };

    float yaws[] = {
        0.0f, 45.0f, 90.0f, 135.0f, 180.0f,
        225.0f, 270.0f, 315.0f, 22.5f, 67.5f,
        112.5f, 157.5f, 202.5f, 247.5f, 292.5f
    };

    float skeletonScale = 100.0f * 500.0f;

    for (int i = 0; i < 15; ++i) {
        RegisterAnimatedMesh(skeleton, skeletonAnim, skeletonMaterial,
            positions[i], yaws[i], skeletonScale, rainbowGradientTex);
    }
}