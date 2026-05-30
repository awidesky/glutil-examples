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
            glBindTexture(GL_TEXTURE_2D, bindTexture->id);
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
    void Input() override {}
    void Update(float dt) override { (void)dt; }

    void Render() override {
        if (!pMeshData || !pMaterial)
            return;

        pMaterial->Bind();

        const glm::mat4 model = pOwner ? pOwner->transform.GetWorldMatrix() : glm::mat4(1.0f);

        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

        for (const auto& meshData : pMeshData->meshes) {
            if (!meshData.ok || meshData.vao == 0 || meshData.indexCount <= 0)
                continue;

            glBindVertexArray(meshData.vao);
            glDrawElements(GL_TRIANGLES, meshData.indexCount, GL_UNSIGNED_INT, nullptr);
        }

        glBindVertexArray(0);
    }
};

#endif // AIMLAB_MESH_HPP