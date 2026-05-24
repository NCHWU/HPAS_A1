#include "gpu_renderer.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/component_wise.hpp>
#include <iostream>


namespace render {

GPURenderer::GPURenderer(
    volume::GPUVolume* pGPUVolume,
    const volume::Volume* pVolume,
    const volume::GradientVolume* pGradientVolume,
    const ui::Trackball* pCamera,
    const RenderConfig& config,
    const GPUMeshConfig& meshConfig)
    : m_pGPUVolume(pGPUVolume)
    , m_pVolume(pVolume)
    , m_pGradientVolume(pGradientVolume)
    , m_pCamera(pCamera)
    , m_renderConfig(config)
    , m_meshConfig(meshConfig)
    , m_positions(std::vector<glm::vec3>())
    , m_blockActive(std::vector<int>())
    , m_minMaxValues(std::vector<glm::vec2>())

{
    // The general framebuffer with depth component
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // the following textures are all render buffer targets so we do not add data
    // Create a texture for the depth buffer
    glGenTextures(1, &m_depthTexture);
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //Create textures for ray parameters, cube positions and cube min max values
    glGenTextures(1, &m_frontfaces_texture);
    glBindTexture(GL_TEXTURE_2D, m_frontfaces_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &m_occupancy_texture);
    glBindTexture(GL_TEXTURE_2D, m_occupancy_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &m_backfaces_texture);
    glBindTexture(GL_TEXTURE_2D, m_backfaces_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Attach the depth buffer to the framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);

    // Attach the existing color texture
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_frontfaces_texture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Vertex Shader for rendering the cube geometry
    GLuint renderCubesVertexShader = loadShader("gpu_optimization_vert.glsl", GL_VERTEX_SHADER);
    // Setup shaders
    linkShaderProgram(m_facesShader, renderCubesVertexShader, loadShader("volvis_colorcube_frag.glsl", GL_FRAGMENT_SHADER));

    // Vertex Shader for rendering a screen filling quad
    GLuint screenFillingQuadVertexShader = loadShader("volvis_screen_filling_quad_vert.glsl", GL_VERTEX_SHADER);
    linkShaderProgram(m_screenFillingQuadShader, screenFillingQuadVertexShader, loadShader("volvis_screen_filling_quad_frag.glsl", GL_FRAGMENT_SHADER));

    GLuint occupancyVertexShader = loadShader("sparse_leap_occupancy_vert.glsl", GL_VERTEX_SHADER);
    linkShaderProgram(m_occupancyShader, occupancyVertexShader, loadShader("sparse_leap_occupancy_frag.glsl", GL_FRAGMENT_SHADER));

    // Render Mode shaders
    linkShaderProgram(m_mipShader, screenFillingQuadVertexShader, loadShader("volvis_rendermode_mip_frag.glsl", GL_FRAGMENT_SHADER));
    linkShaderProgram(m_isoShader, screenFillingQuadVertexShader, loadShader("volvis_rendermode_isosurface_frag.glsl", GL_FRAGMENT_SHADER));
    linkShaderProgram(m_compositeShader, screenFillingQuadVertexShader, loadShader("volvis_rendermode_compositing_frag.glsl", GL_FRAGMENT_SHADER));

    // == geometry
    // screen filling quad
    const float quadVertices[] = {
        // Positions     // Texture Coords
        -1.0f,  1.0f,    0.0f, 1.0f,
        -1.0f, -1.0f,    0.0f, 0.0f,
         1.0f, -1.0f,    1.0f, 0.0f,
    
        -1.0f,  1.0f,    0.0f, 1.0f,
         1.0f, -1.0f,    1.0f, 0.0f,
         1.0f,  1.0f,    1.0f, 1.0f
    };

    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);

    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // single cube, can be used for instanced rendering in case of blocking
    const std::array vertices {
		0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f,
    };

    const std::array<unsigned, 36> indices {
        0, 6, 4,
        0, 2, 6,
        0, 3, 2,
        0, 1, 3,
        2, 7, 6,
        2, 3, 7,
        4, 6, 7,
        4, 7, 5,
        0, 4, 5,
        0, 5, 1,
        1, 5, 7,
        1, 7, 3
    };

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

    glGenBuffers(1, &m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    unsigned int boundingBoxBufferSize = sizeof(BoundingBoxGeometry) * m_occupancyBoundingBoxes.size();
    glGenBuffers(1, &m_boundingBoxBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_boundingBoxBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, boundingBoxBufferSize, NULL, GL_DYNAMIC_DRAW); // Allocate memory
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_boundingBoxBuffer); // Bind to binding point 3
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // Unbind


    // Setup for the GPU linked list

    glGenBuffers(1, &m_headPointerBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_headPointerBuffer);
    std::vector<unsigned int> headData(m_renderConfig.renderResolution.x * m_renderConfig.renderResolution.y, 0);
    glBufferData(GL_SHADER_STORAGE_BUFFER, headData.size() * sizeof(unsigned int), headData.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_headPointerBuffer); // Bind to binding point 0
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // Unbind

    unsigned int bufferNodeSize = sizeof(float) + sizeof(int) + sizeof(int) + sizeof(unsigned int);
    glGenBuffers(1, &m_nodeBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_nodeBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 50 * bufferNodeSize * m_renderConfig.renderResolution.x * m_renderConfig.renderResolution.y, NULL, GL_DYNAMIC_DRAW); // Allocate memory
    float zero = 0.0f;
    glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32F, GL_RED, GL_FLOAT, &zero);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_nodeBuffer); // Bind to binding point 1
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // Unbind

    glGenBuffers(1, &m_atomicCounterBuffer);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomicCounterBuffer);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 2, m_atomicCounterBuffer); // Bind to binding point 2
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0); // Unbind

    glGenBuffers(1, &m_atomicSampleCounterBuffer);
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomicSampleCounterBuffer);
    glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 4, m_atomicSampleCounterBuffer); // Bind to binding point 4
    glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0); // Unbind



    // set clear color
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // initialize the geometry
    updateGPUMesh(true);

    updateOccupancy();
}

GPURenderer::~GPURenderer()
{
    // delete the occupancy histogram tree
    delete m_rootNode;

    // delete the buffers
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ibo);
    glDeleteBuffers(1, &m_quadVBO);
    glDeleteBuffers(1, &m_headPointerBuffer);
    glDeleteBuffers(1, &m_nodeBuffer);
    glDeleteBuffers(1, &m_atomicCounterBuffer);

    // delete the textures
    glDeleteTextures(1, &m_frontfaces_texture);
    glDeleteTextures(1, &m_backfaces_texture);
    glDeleteTextures(1, &m_occupancy_texture);
    glDeleteTextures(1, &m_depthTexture);

    // delete the framebuffers
    glDeleteFramebuffers(1, &m_fbo);

    // delete the shader programs
    glDeleteProgram(m_facesShader);
    glDeleteProgram(m_screenFillingQuadShader);
    glDeleteProgram(m_occupancyShader);
}

void GPURenderer::updateOccupancy()
{
    // we need to update the occupancy bounding boxes
    // this is done by traversing the occupancy histogram tree
    // and emitting the bounding boxes
    delete m_rootNode;
    m_rootNode = OHTNode::build(m_pVolume, m_renderConfig);
    m_occupancyBoundingBoxes.clear();
    m_rootNode->traverse(m_occupancyBoundingBoxes);
}

// ======= DO NOT MODIFY THIS FUNCTION ========
// updates the full blocking info
void GPURenderer::updateGPUMesh(bool updateMinMax)
{
    updateOccupancy();
}

// ======= DO NOT MODIFY THIS FUNCTION ========
// sets the render configuration
void GPURenderer::setRenderConfig(const RenderConfig& config)
{
    m_renderConfig = config;
    updateGPUMesh(false);
}

// ======= DO NOT MODIFY THIS FUNCTION ========
// sets the blocking configuration
void GPURenderer::setMeshConfig(const GPUMeshConfig& config)
{
    m_meshConfig = config;
    updateGPUMesh(true);
}

// ======= DO NOT MODIFY THIS FUNCTION ========
// sets the render resolution for the offscreen textures
void GPURenderer::setRenderSize(glm::ivec2 resolution)
{
    m_renderResolution = resolution;
}

// ======= DO NOT MODIFY THIS FUNCTION ========
// update the model view and projection matrices for the cube rendering
void GPURenderer::updateMatrices()
{
    m_modelMatrix = glm::scale(glm::identity<glm::mat4>(), glm::vec3(m_pVolume->dims()));
    const glm::mat4 viewMatrix = m_pCamera->viewMatrix();
    const glm::mat4 projectionMatrix = m_pCamera->projectionMatrix();
    m_viewProjectionMatrix = projectionMatrix * viewMatrix * m_modelMatrix;
}

void GPURenderer::renderOccupancy()
{
    // Manage the depth buffer
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_renderResolution.x, m_renderResolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glBindTexture(GL_TEXTURE_2D, m_occupancy_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_renderResolution.x, m_renderResolution.y, 0, GL_RGBA, GL_FLOAT, nullptr);

    // Bind the framebuffer and attach textures
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_occupancy_texture, 0);

    //sortBoundingBoxGeometry();
    m_rootNode->sort(m_pCamera->position());
    m_occupancyBoundingBoxes.clear();
    m_rootNode->traverse(m_occupancyBoundingBoxes);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_boundingBoxBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, m_occupancyBoundingBoxes.size() * sizeof(BoundingBoxGeometry), m_occupancyBoundingBoxes.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_boundingBoxBuffer);
    //m_rootNode->print(0);

    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Disable depth testing
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);



    // === render the front faces
    // Render frontfaces into a texture
    glUseProgram(m_occupancyShader);

    // the size of each cube in normalized volume coordinates
    glUniform1i(glGetUniformLocation(m_occupancyShader, "SCREEN_WIDTH"), m_renderConfig.renderResolution.x);
    glUniform3fv(glGetUniformLocation(m_occupancyShader, "cameraPos"), 1, glm::value_ptr(m_pCamera->position()));
    glUniformMatrix4fv(glGetUniformLocation(m_occupancyShader, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(glm::inverse(m_pCamera->viewMatrix())));
    glUniform2fv(glGetUniformLocation(m_occupancyShader, "screenSize"), 1, glm::value_ptr(glm::vec2(m_renderResolution.x, m_renderResolution.y)));
    glUniform1f(glGetUniformLocation(m_occupancyShader, "fov"), glm::degrees(m_pCamera->getFovy()));

    glClear(GL_COLOR_BUFFER_BIT);

    // we draw an instance of the cube for every item in m_positions
    // we can then handle cubes that are empty in the vertex shader by moving them out of the view so that they are automatically culled
    glBindVertexArray(m_quadVAO);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // Restore depth clear value
    glClearDepth(1.0f);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// ======= DO NOT MODIFY THIS FUNCTION ========
// function for the off screen front faces and directions render passes
void GPURenderer::renderDirections()
{
    // Manage the depth buffer
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_renderResolution.x, m_renderResolution.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glBindTexture(GL_TEXTURE_2D, m_frontfaces_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_renderResolution.x, m_renderResolution.y, 0, GL_RGBA, GL_FLOAT, nullptr);

    // Bind the framebuffer and attach textures
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_frontfaces_texture, 0);

    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    // === render the front faces
    // Render frontfaces into a texture
    glUseProgram(m_facesShader);
    drawGeometry(m_facesShader);

    // === render the back faces
    // Update texture for the directions
    glBindTexture(GL_TEXTURE_2D, m_backfaces_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_renderResolution.x, m_renderResolution.y, 0, GL_RGBA, GL_FLOAT, nullptr);

    // Attach direction texture to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_backfaces_texture, 0);

    // Clear the depth buffer for the next render pass
    glClearDepth(0.0f);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Render backfaces and extract direction and length of each ray
    glDisable(GL_CULL_FACE);
    glDepthFunc(GL_GREATER);
    
    drawGeometry(m_facesShader);

    // Restore depth clear value
    glClearDepth(1.0f);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDepthFunc(GL_LEQUAL);

    // Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// ======= DO NOT MODIFY THIS FUNCTION ========
// draws the actual bounding geometry
void GPURenderer::drawGeometry(GLuint shaderID)
{
    // the size of each cube in normalized volume coordinates
    glUniform1i(glGetUniformLocation(shaderID, "SCREEN_WIDTH"), m_renderConfig.renderResolution.x);

    // give the matrices to the shaders
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "u_model"), 1, GL_FALSE, glm::value_ptr(m_modelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shaderID, "u_modelViewProjection"), 1, GL_FALSE, glm::value_ptr(m_viewProjectionMatrix));

    glClear(GL_COLOR_BUFFER_BIT);

    // we draw an instance of the cube for every item in m_positions
    // we can then handle cubes that are empty in the vertex shader by moving them out of the view so that they are automatically culled
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

// ======= DO NOT MODIFY THIS FUNCTION ========
// Draw a screenfilling quad either just textured with the front/backfaces, or as the actual volume rendering pass
void GPURenderer::renderTextureToScreen(GLuint texture)
{
    glUseProgram(m_screenFillingQuadShader);

    glBindVertexArray(m_quadVAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(m_screenFillingQuadShader, "u_texture"), 0);
    
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

// ======= DO NOT MODIFY THIS FUNCTION ========
// Called to render a frame, passes the call along to the correct render method after some shared setup  
void GPURenderer::render()
{
    countSamples = m_renderConfig.doCountSamples;

    if (m_renderConfig.useEmptySpaceSkipping) {
        // Clear the head pointer buffer
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_headPointerBuffer);
        unsigned int zero = 0;
        glClearBufferData(GL_SHADER_STORAGE_BUFFER, GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, &zero);

        // bind the linked list buffers
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_headPointerBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_headPointerBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_nodeBuffer);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_nodeBuffer);
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomicCounterBuffer);

        // Initialize the counter to 1 (0 is considered the end of a list)
        GLuint initialCounterValue = 1;
        glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &initialCounterValue, GL_DYNAMIC_DRAW);
    }

    // update the model view projection
    updateMatrices();

    // we render front faces and directions into textures here
    if (!m_renderConfig.useEmptySpaceSkipping || m_renderConfig.renderMode == render::RenderMode::RenderMIP) {
        renderDirections();
    }

    // we render the occupancy texture here
    if (m_renderConfig.useEmptySpaceSkipping) {
        renderOccupancy();
    }

    glDisable(GL_BLEND);

    if (countSamples) {
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomicSampleCounterBuffer);
        GLuint initialCounterValue = 0;
        glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &initialCounterValue, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 4, m_atomicSampleCounterBuffer);
    }

    // when setting the gui to show front faces or directions we render the corresponding texture
    if (m_renderConfig.renderStep == 1) {
        renderTextureToScreen(m_frontfaces_texture);
    } else
    if (m_renderConfig.renderStep == 2) {
        renderTextureToScreen(m_backfaces_texture);
    } else if (m_renderConfig.renderStep == 3) {
        renderTextureToScreen(m_occupancy_texture);
    } else // otherwise render according to render mode
    if (m_renderConfig.renderStep == 4) {
        if (m_renderConfig.renderMode == render::RenderMode::RenderIso) {
            renderIso();
        } else 
        if (m_renderConfig.renderMode == render::RenderMode::RenderMIP) {
            renderMIP();
        } else
        if (m_renderConfig.renderMode == render::RenderMode::RenderComposite) {
            renderComposite();
        }
    }
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    if (countSamples) {
    	// Read the value of the atomic counter
        glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomicSampleCounterBuffer);
        GLuint* sampleCount = (GLuint*)glMapBuffer(GL_ATOMIC_COUNTER_BUFFER, GL_READ_ONLY);
        if (sampleCount) {
            m_numSamples = *sampleCount;
            glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
        }
    }
}

CompatibilityResult GPURenderer::checkCompatibility() const
{
    if (!GLEW_ARB_shader_storage_buffer_object) {
        std::string message = "ARB_shader_storage_buffer_object is not supported";
        return CompatibilityResult{
            false,
            message
        };
    }

    if (!GLEW_ARB_shader_atomic_counters) {
        std::string message = "ARB_shader_atomic_counters is not supported";
        return CompatibilityResult {
            false,
            message
        };
    }

    if (!GLEW_ARB_shader_image_load_store) {
        std::string message = "ARB_shader_image_load_store is not supported";
        return CompatibilityResult {
            false,
            message
        };
    }

    return CompatibilityResult {
        true,
        "All required systems are functioning as expected"
    };
}

int GPURenderer::getLatestSampleCount() const
{
    return m_numSamples;
}

// ======= DO NOT MODIFY THIS FUNCTION ========
// GPU implementation of a MIP raycaster
// This should be fully working.
// MIP always needs the whole volume so it does not work with blocking and bricking
// also have a look at the volvis_rendermode_mip_frag.glsl fragment shader file for the ray traversal on the GPU
void GPURenderer::renderMIP()
{
    glUseProgram(m_mipShader);

    // Pass textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_backfaces_texture);
    glUniform1i(glGetUniformLocation(m_mipShader, "backFaces"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_frontfaces_texture);
    glUniform1i(glGetUniformLocation(m_mipShader, "frontFaces"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, m_pGPUVolume->getTexId());
    glUniform1i(glGetUniformLocation(m_mipShader, "volumeData"), 2);

    // we bring the stepsize into normalized volume coordinates
    // first we need the max volume extent
    glm::vec3 volDims = m_pVolume->dims();
    float maxExtent = std::max(volDims.x, std::max(volDims.y, volDims.z));
    float stepSizeNorm = m_renderConfig.stepSize / maxExtent;
    glm::vec4 renderOptions = glm::vec4(stepSizeNorm, 1.0f / stepSizeNorm, 0.0f, 0.0f);
    glUniform4fv(glGetUniformLocation(m_mipShader, "renderOptions"), 1, glm::value_ptr(renderOptions));

    // the reciprocal of the volDims is the voxelSize in 0..1 space, the reciprocal of the maximum vol value eases GPU load
    glm::vec4 volumeInfo = glm::vec4(1.0f / volDims, 1.0f / m_pVolume->maximum());
    glUniform4fv(glGetUniformLocation(m_mipShader, "volumeInfo"), 1, glm::value_ptr(volumeInfo));

    glClear(GL_COLOR_BUFFER_BIT);

    // rendering happens drawing the screenfilling quad and using the front faces and direction as input
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

// ======= DO NOT MODIFY THIS FUNCTION ========
// GPU implementation of a iso-surface raycaster
void GPURenderer::renderIso()
{
    glUseProgram(m_isoShader);

    // Pass textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_backfaces_texture);
    glUniform1i(glGetUniformLocation(m_isoShader, "backFaces"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_frontfaces_texture);
    glUniform1i(glGetUniformLocation(m_isoShader, "frontFaces"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, m_pGPUVolume->getTexId());
    glUniform1i(glGetUniformLocation(m_isoShader, "volumeData"), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_3D, m_pGPUVolume->getIndexTexId());
    glUniform1i(glGetUniformLocation(m_isoShader, "volumeIndexData"), 3);

    glm::vec3 volDims = m_pVolume->dims();

    // the reciprocal of the volDims is the voxelSize in 0..1 space
    glm::vec4 volumeInfo = glm::vec4(1.0f / volDims, m_pGPUVolume->useBricking());
    glUniform4fv(glGetUniformLocation(m_isoShader, "volumeInfo"), 1, glm::value_ptr(volumeInfo));

    // we bring the stepsize into normalized volume coordinates
    float maxExtent = std::max(volDims.x, std::max(volDims.y, volDims.z));
    glUniform4fv(glGetUniformLocation(m_isoShader, "renderOptions"), 1, glm::value_ptr(glm::vec4( m_renderConfig.stepSize / maxExtent,
                                                                                                    maxExtent / m_renderConfig.stepSize,
                                                                                                    m_renderConfig.isoValue,
                                                                                                    m_renderConfig.volumeShading )));

    glUniform1f(glGetUniformLocation(m_isoShader, "stepSize"), m_renderConfig.stepSize);

    glUniform1i(glGetUniformLocation(m_isoShader, "SCREEN_WIDTH"), m_renderConfig.renderResolution.x);
    glUniform3fv(glGetUniformLocation(m_isoShader, "cameraPos"), 1, glm::value_ptr(m_pCamera->position()));
    glUniformMatrix4fv(glGetUniformLocation(m_isoShader, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(glm::inverse(m_pCamera->viewMatrix())));
    glUniform2fv(glGetUniformLocation(m_isoShader, "screenSize"), 1, glm::value_ptr(glm::vec2(m_renderResolution.x, m_renderResolution.y)));
    glUniform1f(glGetUniformLocation(m_isoShader, "fov"), glm::degrees(m_pCamera->getFovy()));
    glUniform1i(glGetUniformLocation(m_isoShader, "doEmptySpaceSkipping"), m_renderConfig.useEmptySpaceSkipping);
    glUniform1i(glGetUniformLocation(m_isoShader, "doCountSamples"), countSamples);

    glClear(GL_COLOR_BUFFER_BIT);

    // rendering happens drawing the screenfilling quad and using the front faces and direction as input
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

// ======= DO NOT MODIFY THIS FUNCTION ========
// GPU implementation of a Composite raycaster with a given 1D transferfunction 
void GPURenderer::renderComposite()
{
    glUseProgram(m_compositeShader);

    // Pass textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_backfaces_texture);
    glUniform1i(glGetUniformLocation(m_compositeShader, "backFaces"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_frontfaces_texture);
    glUniform1i(glGetUniformLocation(m_compositeShader, "frontFaces"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, m_pGPUVolume->getTexId());
    glUniform1i(glGetUniformLocation(m_compositeShader, "volumeData"), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_3D, m_pGPUVolume->getIndexTexId());
    glUniform1i(glGetUniformLocation(m_compositeShader, "volumeIndexData"), 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, m_renderConfig.tfTexId);
    glUniform1i(glGetUniformLocation(m_compositeShader, "transferFunction"), 4);

    // we bring the stepsize into normalized volume coordinates
    // first we need the max volume extent
    glm::vec3 volDims = m_pVolume->dims();
    float maxExtent = std::max(volDims.x, std::max(volDims.y, volDims.z));
    glUniform4fv(glGetUniformLocation(m_compositeShader, "renderOptions"), 1, glm::value_ptr(glm::vec4( m_renderConfig.stepSize / maxExtent,
                                                                                                        maxExtent / m_renderConfig.stepSize,
                                                                                                        m_renderConfig.stepSize,
                                                                                                        m_renderConfig.volumeShading )));

    glUniform1f(glGetUniformLocation(m_compositeShader, "stepSize"), m_renderConfig.stepSize);

    glUniform4fv(glGetUniformLocation(m_compositeShader, "gmParams"), 1, glm::value_ptr(glm::vec4( m_renderConfig.illustrativeParams.x,
                                                                                                    m_renderConfig.illustrativeParams.y,
                                                                                                    m_renderConfig.illustrativeParams.z,
                                                                                                    m_renderConfig.useOpacityModulation)));
    // the reciprocal of the volDims is the voxelSize in 0..1 space
    glm::vec4 volumeInfo = glm::vec4(1.0f / volDims, m_pGPUVolume->useBricking());
    glUniform4fv(glGetUniformLocation(m_compositeShader, "volumeInfo"), 1, glm::value_ptr(volumeInfo));

    // Here we provide maximum volume and maximum gradient magnitude for normalization in the shader
    // Note: we actually give the reciprocal, to avoid division in the shader
    glUniform2fv(glGetUniformLocation(m_compositeShader, "volumeMaxValues"), 1, glm::value_ptr(glm::vec2( 1.0f/m_pVolume->maximum(),
                                                                                                            1.0f/m_pGradientVolume->maxMagnitude())));

    glUniform1i(glGetUniformLocation(m_compositeShader, "SCREEN_WIDTH"), m_renderConfig.renderResolution.x);
    glUniform3fv(glGetUniformLocation(m_compositeShader, "cameraPos"), 1, glm::value_ptr(m_pCamera->position()));
    glUniformMatrix4fv(glGetUniformLocation(m_compositeShader, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(glm::inverse(m_pCamera->viewMatrix())));
    glUniform2fv(glGetUniformLocation(m_compositeShader, "screenSize"), 1, glm::value_ptr(glm::vec2(m_renderResolution.x, m_renderResolution.y)));
    glUniform1f(glGetUniformLocation(m_compositeShader, "fov"), glm::degrees(m_pCamera->getFovy()));
    glUniform1i(glGetUniformLocation(m_compositeShader, "doEmptySpaceSkipping"), m_renderConfig.useEmptySpaceSkipping);
    glUniform1i(glGetUniformLocation(m_compositeShader, "doCountSamples"), countSamples);

    glClear(GL_COLOR_BUFFER_BIT);

    // rendering happens drawing the screenfilling quad and using the front faces and direction as input
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

// ======= DO NOT MODIFY THIS FUNCTION ========
// Note: Only used to pass update events through to the GPU Volume
// Upate the volume bricks after a tf/iso change or setting the bricksize
// only updates the cache, should not be called when bricksize change. call setVolumeBricksSize in that case
void GPURenderer::updateVolumeBricks()
{
    m_pGPUVolume->updateBrickCache(m_renderConfig, m_opacitySumTable);
}

// ======= DO NOT MODIFY THIS FUNCTION ========
// Note: Only used to pass update events through to the GPU Volume
// sets a new bricksize and updates the bricking volume (full update)
void GPURenderer::setVolumeBricksSize()
{
    m_pGPUVolume->brickSizeChanged(m_renderConfig, m_opacitySumTable);
}

// ======= DO NOT MODIFY THIS FUNCTION ========
// Helper function to simplify shader creation
void GPURenderer::linkShaderProgram(GLuint& shader, GLuint vertexShader, GLuint fragmentShader)
{
    shader = glCreateProgram();
    glAttachShader(shader, vertexShader);
    glAttachShader(shader, fragmentShader);
    glLinkProgram(shader);

    glDetachShader(shader, vertexShader);
    glDetachShader(shader, fragmentShader);
}
}
