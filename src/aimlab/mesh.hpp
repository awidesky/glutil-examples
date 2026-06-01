#ifndef AIMLAB_MESH_HPP
#define AIMLAB_MESH_HPP

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/gl.h>

#include "component.hpp"
#include "engine.hpp"

class Material {
public:
    Material() = default;
    explicit Material(Texture* texture) : texture(texture) {}

    Texture* texture = nullptr;
    GLint myTextureSamplerLocation = -1;

    /** 프로그램은 이미 bind되어 있다고 간주하고, uniform 등 데이터만 바인드 */
    void Bind() const {
        glActiveTexture(GL_TEXTURE0);
        Texture* bindTexture = texture ? texture : ResourceManager::Get().GetDefaultTexture();
        if (bindTexture)
            bindTexture->bind();
        glUniform1i(myTextureSamplerLocation, 0);
    }
};

class MeshRenderer : public Component {
public:
    MeshRenderer(Mesh* mesh = nullptr, Material* material = nullptr) : pMeshData(mesh), pMaterial(material) {}
    ~MeshRenderer() override { delete pMaterial; }

    Mesh* pMeshData = nullptr;
    Material* pMaterial = nullptr;
    GLuint program = 0;
    GLint modelLocation = -1;

    void Start() override {
        program = GraphicsContext::Get().GetProgram();
        if (pMaterial)
            pMaterial->myTextureSamplerLocation = glGetUniformLocation(program, "myTextureSampler");
        modelLocation = glGetUniformLocation(program, "model");
    }
    void Render() override {
        if (!pMeshData || !pMaterial) return;

        pMaterial->Bind();

        const glm::mat4 model = pOwner ? pOwner->transform.GetWorldMatrix() : glm::mat4(1.0f);
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

        for (const auto& meshData : pMeshData->meshes) {
            if (!meshData.ok || meshData.vao == 0 || meshData.indexCount <= 0) continue;

            glBindVertexArray(meshData.vao);
            glDrawElements(GL_TRIANGLES, meshData.indexCount, GL_UNSIGNED_INT, nullptr);
        }
        glBindVertexArray(0);
    }
};
class OrthogonalRenderer : public Component {
public:
    OrthogonalRenderer(Mesh* mesh = nullptr, Material* material = nullptr) : pMeshData(mesh), pMaterial(material) {}

    ~OrthogonalRenderer() override { delete pMaterial; }

    Mesh* pMeshData = nullptr;
    Material* pMaterial = nullptr;

    GLuint program = 0;
    GLint modelLocation = -1;
    GLint viewLocation = -1;
    GLint projectionLocation = -1;

    void Start() override {
        program = GraphicsContext::Get().GetProgram();

        if (pMaterial)
            pMaterial->myTextureSamplerLocation = glGetUniformLocation(program, "myTextureSampler");

        modelLocation = glGetUniformLocation(program, "model");
        viewLocation = glGetUniformLocation(program, "view");
        projectionLocation = glGetUniformLocation(program, "projection");
    }

    void Render() override {
        if (!pMeshData || !pMaterial)
            return;

        auto& gc = GraphicsContext::Get();
        int w, h;
        gc.GetWindowSize(w, h);

        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        pMaterial->Bind();

        glm::mat4 projection = glm::ortho(0.0f, (float)w, (float)h, 0.0f, -1.0f, 1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 model = glm::mat4(1.0f);

        float size = 32.0f; //TODO some might need to move into new concept of material
        model = glm::translate(model, glm::vec3(w * 0.5f, h * 0.5f, 0.0f));
        model = glm::scale(model, glm::vec3(size, size, 1.0f));

        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

        for (const auto& meshData : pMeshData->meshes) {
            if (!meshData.ok || meshData.vao == 0 || meshData.indexCount <= 0)
                continue;

            glBindVertexArray(meshData.vao);
            glDrawElements(GL_TRIANGLES, meshData.indexCount, GL_UNSIGNED_INT, nullptr);
        }

        glBindVertexArray(0);

        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
    }
};
#endif // AIMLAB_MESH_HPP