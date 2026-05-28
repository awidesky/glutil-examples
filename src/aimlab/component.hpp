#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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
    virtual void Update(float dt) {}
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

    void Render() {
        if (!active)
            return;
        for (auto c : m_components)
            c->Render();
    }

private:
    std::vector<Component*> m_components;
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
    Camera* camera = nullptr;
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
        if (currentAmmo <= 0 || isReloading || !targets || !camera)
            return;
        currentAmmo--;

        PhysicsSystem::Ray ray = {camera->GetPosition(), camera->GetForward()};
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
#endif // COMPONENT_HPP