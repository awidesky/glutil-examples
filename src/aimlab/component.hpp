#ifndef AIMLAB_COMPONENT_HPP
#define AIMLAB_COMPONENT_HPP
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <GLFW/glfw3.h>
#include "engine.hpp"
#include "physics.hpp"
#include "camera.hpp"

struct Transform {
    glm::vec3 position{0, 0, 0};
    glm::vec3 rotation{0, 0, 0};
    glm::vec3 scale{1, 1, 1};

    glm::mat4 GetWorldMatrix() const {
        glm::mat4 m(1.f);
        m = glm::translate(m, position);
        m = glm::rotate(m, glm::radians(rotation.x), {1, 0, 0});
        m = glm::rotate(m, glm::radians(rotation.y), {0, 1, 0});
        m = glm::rotate(m, glm::radians(rotation.z), {0, 0, 1});
        return glm::scale(m, scale);
    }
};

class GameObject;
class Component {
public:
    GameObject* pOwner = nullptr;
    bool isStarted = false;
    virtual void Start() {}
    virtual void Input() {}
    virtual void Update(float dt) { (void)dt; }
    virtual void Render() {}
    virtual ~Component() {}
};


class GameObject {
public:
    Transform transform;
    bool active = true;

    ~GameObject() {
        for (auto c : m_components)
            delete c;
    }

    void AddComponent(Component* c) {
        c->pOwner = this;
        m_components.push_back(c);
    }

    template <typename T> T* GetComponent() {
        for (auto c : m_components)
            if (auto t = dynamic_cast<T*>(c))
                return t;
        return nullptr;
    }

    void Update(float dt) {
        if (!active)
            return;
        for (auto c : m_components) {
            if (!c->isStarted) {
                c->Start();
                c->isStarted = true;
            }
            c->Update(dt);
        }
    }

    void Input() {
        if (!active)
            return;
        for (auto c : m_components)
            c->Input();
    }

    void Render() {
        if (!active)
            return;
        for (auto c : m_components)
            c->Render();
    }

private:
    std::vector<Component*> m_components;
};

class CameraController : public Component {
public:
    void Start() override {
        const GLuint program = GraphicsContext::Get().GetProgram();
        viewLocation = glGetUniformLocation(program, "view");
        viewPosLocation = glGetUniformLocation(program, "viewPos");
    }

    void Update(float dt) override {
        auto& gc = GraphicsContext::Get();
        auto& camera = Camera::Get();
        auto [dx, dy] = InputManager::Get().GetMouseDelta();

        camera.yaw += dx * gc.mouseSensitivity;
        camera.pitch += dy * gc.mouseSensitivity;
        camera.pitch = std::clamp(camera.pitch, -89.0f, 89.0f);

        glm::vec3 forward = camera.GetForward();
        glm::vec3 flatDir = glm::normalize(glm::vec3(forward.x, 0.0f, forward.z));
        glm::vec3 right = glm::normalize(glm::cross(flatDir, camera.up));

        if (InputManager::Get().IsKeyDown(GLFW_KEY_W))
            camera.position += flatDir * camera.speed * dt;
        if (InputManager::Get().IsKeyDown(GLFW_KEY_S))
            camera.position -= flatDir * camera.speed * dt;
        if (InputManager::Get().IsKeyDown(GLFW_KEY_A))
            camera.position -= right * camera.speed * dt;
        if (InputManager::Get().IsKeyDown(GLFW_KEY_D))
            camera.position += right * camera.speed * dt;
    }

    void Render() override {
        const Camera& camera = Camera::Get();
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
        glUniform3fv(viewPosLocation, 1, glm::value_ptr(camera.position));
    }

private:
    GLint viewLocation = -1;
    GLint viewPosLocation = -1;
};

class SystemController : public Component {
public:
    void Start() override {
        const GLuint program = GraphicsContext::Get().GetProgram();
        projectionLocation = glGetUniformLocation(program, "projection");
        lightPosLocation = glGetUniformLocation(program, "lightPos");
        ambientStrengthLocation = glGetUniformLocation(program, "ambientStrength");
    }

    void Update(float dt) override {
        auto& gc = GraphicsContext::Get();
        auto& im = InputManager::Get();

        const float sensitivityRate = 1.5f;
        const float fovRate = 30.0f;
        const float ambientRate = 0.05f;

        if (im.IsKeyDown(GLFW_KEY_LEFT_BRACKET)) {
            float oldValue = gc.mouseSensitivity;
            gc.mouseSensitivity = std::max(0.001f, gc.mouseSensitivity - sensitivityRate * dt);
            if (gc.mouseSensitivity != oldValue)
                std::cout << "mouseSensitivity: " << gc.mouseSensitivity << '\n';
        }
        if (im.IsKeyDown(GLFW_KEY_RIGHT_BRACKET)) {
            float oldValue = gc.mouseSensitivity;
            gc.mouseSensitivity = std::min(2.0f, gc.mouseSensitivity + sensitivityRate * dt);
            if (gc.mouseSensitivity != oldValue)
                std::cout << "mouseSensitivity: " << gc.mouseSensitivity << '\n';
        }

        if (im.IsKeyDown(GLFW_KEY_O)) {
            float oldValue = gc.fov;
            gc.fov = std::max(20.0f, gc.fov - fovRate * dt);
            if (gc.fov != oldValue)
                std::cout << "fov: " << gc.fov << '\n';
        }
        if (im.IsKeyDown(GLFW_KEY_P)) {
            float oldValue = gc.fov;
            gc.fov = std::min(120.0f, gc.fov + fovRate * dt);
            if (gc.fov != oldValue)
                std::cout << "fov: " << gc.fov << '\n';
        }

        if (im.IsKeyDown(GLFW_KEY_SEMICOLON)) {
            float oldValue = gc.ambientStrength;
            gc.ambientStrength = std::max(0.0f, gc.ambientStrength - ambientRate * dt);
            if (gc.ambientStrength != oldValue)
                std::cout << "ambientStrength: " << gc.ambientStrength << '\n';
        }
        if (im.IsKeyDown(GLFW_KEY_APOSTROPHE)) {
            float oldValue = gc.ambientStrength;
            gc.ambientStrength = std::min(3.0f, gc.ambientStrength + ambientRate * dt);
            if (gc.ambientStrength != oldValue)
                std::cout << "ambientStrength: " << gc.ambientStrength << '\n';
        }

        const bool keyC = im.IsKeyDown(GLFW_KEY_C);
        const bool keyEsc = im.IsKeyDown(GLFW_KEY_ESCAPE);

        if (keyC && !m_prevC)
            gc.ToggleFullscreen();

        if (keyEsc && !m_prevEsc) {
            if (gc.IsFullscreen())
                gc.ExitFullscreen();
            else
                glfwSetWindowShouldClose(gc.GetWindow(), GLFW_TRUE);
        }

        m_prevC = keyC;
        m_prevEsc = keyEsc;
    }

    void Render() override {
        const auto& gc = GraphicsContext::Get();
        const Camera& camera = Camera::Get();
        static const glm::vec3 lightPos(1.0f, 3.0f, 2.0f);
        glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(camera.GetProjectionMatrix(gc.fov)));
        glUniform3fv(lightPosLocation, 1, glm::value_ptr(lightPos));
        glUniform1f(ambientStrengthLocation, gc.ambientStrength);
    }

private:
    GLint projectionLocation = -1;
    GLint lightPosLocation = -1;
    GLint ambientStrengthLocation = -1;
    bool m_prevC = false;
    bool m_prevEsc = false;
};

class TargetLogic : public Component {
public:
    float radius = 0.5f;
    bool isAlive = true;
    float spawnTime = 0.f;

    void Start() override { spawnTime = (float)glfwGetTime(); }

    void OnHit() {
        isAlive = false;
        pOwner->active = false;
    }

    float GetReactionTime() const { return (float)glfwGetTime() - spawnTime; }
};

class WeaponSystem : public Component {
public:
    int maxAmmo = 30;
    int currentAmmo = 30;
    bool isReloading = false;
    float reloadTime = 1.5f;
    float reloadTimer = 0.f;
    std::vector<GameObject*>* targets = nullptr;

    void Update(float dt) override {
        if (isReloading) {
            reloadTimer -= dt;
            if (reloadTimer <= 0.f) {
                currentAmmo = maxAmmo;
                isReloading = false;
            }
        }
        if (InputManager::Get().IsMouseDown(GLFW_MOUSE_BUTTON_LEFT))
            Fire();
        if (InputManager::Get().IsKeyDown(GLFW_KEY_R))
            Reload();
    }

    void Fire() {
        if (currentAmmo <= 0 || isReloading || !targets)
            return;
        currentAmmo--;

        const Camera& camera = Camera::Get();
        PhysicsSystem::Ray ray = {camera.position, camera.GetForward()};
        float t;
        for (auto* obj : *targets) {
            if (!obj->active)
                continue;
            auto* logic = obj->GetComponent<TargetLogic>();
            if (!logic || !logic->isAlive)
                continue;
            if (PhysicsSystem::Get().RaySphereIntersect(ray, obj->transform.position, logic->radius, t)) {
                logic->OnHit();
                break;
            }
        }
    }

    void Reload() {
        if (!isReloading && currentAmmo < maxAmmo) {
            isReloading = true;
            reloadTimer = reloadTime;
        }
    }
};
#endif // AIMLAB_COMPONENT_HPP