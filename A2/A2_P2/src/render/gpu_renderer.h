#pragma once
#include "ui/opengl.h"
#include "ui/trackball.h"
#include "render/render_config.h"
#include "render/gpu_mesh_config.h"
#include "volume/gpu_volume.h"
#include "volume/volume.h"
#include "volume/gradient_volume.h"
#include "sparseleap/OccupancyHistogramTree.h"


namespace render {

struct CompatibilityResult
{
    bool compatible = false;
    std::string message;
};


class GPURenderer {
public:
    GPURenderer(
        volume::GPUVolume* pGPUVolume,
        const volume::Volume* m_pVolume,
        const volume::GradientVolume* pGradientVolume,
        const ui::Trackball* pCamera,
        const RenderConfig& config,
        const GPUMeshConfig& meshConfig);

    ~GPURenderer();

    void updateGPUMesh(bool updateMinMax);

    void setRenderConfig(const RenderConfig& config);
    void setMeshConfig(const GPUMeshConfig& config);

    // blocking / empty space skipping
    void updateOccupancy();

    // bricking
    void updateVolumeBricks();
    void setVolumeBricksSize();

    void setRenderSize(glm::ivec2 resolution);

    void render();

    CompatibilityResult checkCompatibility() const;

    int getLatestSampleCount() const;

private:
    void updateMatrices();

    void drawGeometry(GLuint shaderID);
    void renderTextureToScreen(GLuint texture);

    void renderOccupancy();
    void renderDirections();

    void renderIso();
    void renderMIP();
    void renderComposite();

    void linkShaderProgram(GLuint& shader, GLuint vertexShader, GLuint fragmentShader);

private:
    glm::ivec2 m_renderResolution;

    glm::mat4 m_modelMatrix;
    glm::mat4 m_viewProjectionMatrix;

    volume::GPUVolume* m_pGPUVolume;
    const volume::Volume* m_pVolume;
    const volume::GradientVolume* m_pGradientVolume;
    const ui::Trackball* m_pCamera;
    RenderConfig  m_renderConfig {};
    GPUMeshConfig m_meshConfig {};

    GLuint m_ibo, m_vbo, m_vao, m_fbo, m_instance_vbo;
    GLuint m_quadVAO, m_quadVBO;
    GLuint m_facesShader, m_isoShader, m_mipShader, m_compositeShader, m_screenFillingQuadShader, m_occupancyShader;
    GLuint m_backfaces_texture;
    GLuint m_frontfaces_texture;
    GLuint m_occupancy_texture;
    GLuint m_depthTexture;
    GLuint m_boundingBoxBuffer;

    OHTNode* m_rootNode { nullptr };
    std::vector<BoundingBoxGeometry> m_occupancyBoundingBoxes;

    unsigned int m_headPointerBuffer { 0 };
    unsigned int m_nodeBuffer { 0 };
    unsigned int m_atomicCounterBuffer { 0 };
    unsigned int m_atomicSampleCounterBuffer { 0 };
    bool countSamples { true };
    unsigned int m_numSamples { 0 };

    glm::vec3 m_numBlocks3D;
    std::vector<glm::vec3> m_positions;
    std::vector<int> m_blockActive;
    std::vector<glm::vec2> m_minMaxValues; // min = x, max = y in the vector
    std::array<float, 256> m_opacitySumTable;
};
}
