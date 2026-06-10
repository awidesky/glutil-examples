#ifndef AIMLAB_MESH_HPP
#define AIMLAB_MESH_HPP

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glutil/gl.hpp>

#include "component.hpp"
#include "engine.hpp"

class Material {
public:
    Material() = default;
    explicit Material(Texture* texture) : texture(texture) {}
    Material(std::initializer_list<Texture*> texList) : textures(texList) {}

    void useTexture(size_t index) {
        if (index < textures.size())
            texIndex = index;
    }
    std::vector<Texture*> textures;
    size_t texIndex = 0;
    Texture* texture = nullptr;
    GLint myTextureSamplerLocation = -1;

    /** 프로그램은 이미 bind되어 있다고 간주하고, uniform 등 데이터만 바인드 */
    void Bind() const {
        glActiveTexture(GL_TEXTURE0);
        Texture* bindTexture = texture ? texture : ResourceManager::Get().GetDefaultTexture();

        if (!textures.empty() && texIndex < textures.size())
            bindTexture = textures[texIndex];
        if (bindTexture)
            bindTexture->bind();
        glUniform1i(myTextureSamplerLocation, 0);
    }
};

class MeshRenderer : public Component {
public:
    MeshRenderer(Mesh* mesh = nullptr, Material* material = nullptr) : pMeshData(mesh), pMaterial(material) {}
    virtual ~MeshRenderer() override { delete pMaterial; }

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

        const glm::mat4 model = GetModelMatrix();
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

        for (const auto& meshData : pMeshData->meshes) {
            if (!meshData.ok || meshData.vao == 0 || meshData.indexCount <= 0) continue;

            glBindVertexArray(meshData.vao);
            glDrawElements(GL_TRIANGLES, meshData.indexCount, GL_UNSIGNED_INT, nullptr);
        }
        glBindVertexArray(0);
    }

protected:
    virtual glm::mat4 GetModelMatrix() const {
        return pOwner ? pOwner->transform.GetWorldMatrix() : glm::mat4(1.0f);
    }
};
class ViewModelRenderer : public MeshRenderer {
public:
    ViewModelRenderer(Mesh* mesh = nullptr, Material* material = nullptr) : MeshRenderer(mesh, material) {}

    void Render() override {
        glClear(GL_DEPTH_BUFFER_BIT); // 이거 이후로 다른 3D 렌더링이 안됨.
        MeshRenderer::Render();
    }

protected:
    glm::mat4 GetModelMatrix() const override {
        glm::mat4 model(1.0f);

        if (pOwner) {
            const auto& t = pOwner->transform;
            model = glm::translate(model, t.position);
            model = glm::rotate(model, glm::radians(t.rotation.y), glm::vec3(0, 1, 0));
            model = glm::rotate(model, glm::radians(t.rotation.x), glm::vec3(1, 0, 0));
            model = glm::rotate(model, glm::radians(t.rotation.z), glm::vec3(0, 0, 1));
            model = glm::scale(model, t.scale);
        }

        return model;
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
    GLint isUILocation = -1;


    void Start() override {
        program = GraphicsContext::Get().GetProgram();

        if (pMaterial)
            pMaterial->myTextureSamplerLocation = glGetUniformLocation(program, "myTextureSampler");

        modelLocation = glGetUniformLocation(program, "model");
        viewLocation = glGetUniformLocation(program, "view");
        projectionLocation = glGetUniformLocation(program, "projection");
        isUILocation = glGetUniformLocation(program, "isUI");
    }

    void Render() override {
        if (!pMeshData || !pMaterial)
            return;

        auto& gc = GraphicsContext::Get();
        int w, h;
        gc.GetWindowSize(w, h);

        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND); //TODO :init으로 옮기기
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glUniform1i(isUILocation, GL_TRUE);

        pMaterial->Bind();

        glm::mat4 projection = glm::ortho(0.0f, (float)w, 0.0f, (float)h, -1.0f, 1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 model = glm::mat4(1.0f);

        model = glm::translate(model,
            glm::vec3(pOwner->transform.position.x, pOwner->transform.position.y, pOwner->transform.position.z));
        model = glm::scale(model, glm::vec3(pOwner->transform.scale.x, pOwner->transform.scale.y, 1.0f));

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

        
        glUniform1i(isUILocation, GL_FALSE);
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
    }
};
#endif // AIMLAB_MESH_HPP