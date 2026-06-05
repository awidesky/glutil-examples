#ifndef AIMLAB_COMPONENT_HPP
#define AIMLAB_COMPONENT_HPP
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
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

    m = glm::rotate(m, glm::radians(rotation.y), {0, 1, 0});
    m = glm::rotate(m, glm::radians(rotation.x), {1, 0, 0});
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

enum ETargetState {
    Spawned,
    Dying,
    Died,
};


class GameObject {
public:
    Transform transform;
    ETargetState state = ETargetState::Spawned;

    virtual ~GameObject() {
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
        if (state == ETargetState::Died)
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
        if (state == ETargetState::Died)
            return;
        for (auto c : m_components)
            c->Input();
    }

    void Render() {
        if (state == ETargetState::Died)
            return;
        for (auto c : m_components)
            c->Render();
    }

private:
    std::vector<Component*> m_components;
};


class TargetObject : public GameObject {
public:
    float radius = 1.0f;
    float spawnTime = 0.f;

    TargetObject() { spawnTime = (float)glfwGetTime(); }

    void OnHit() { state = ETargetState::Dying;}

    float GetReactionTime() const { return (float)glfwGetTime() - spawnTime; }
};


class CameraController : public Component {
public:
    void Start() override {
        const GLuint program = GraphicsContext::Get().GetProgram();
        viewLocation = glGetUniformLocation(program, "view");
        viewPosLocation = glGetUniformLocation(program, "viewPos");
    }

    void Input() override {
        crouching = InputManager::Get().IsKeyDown(GLFW_KEY_LEFT_SHIFT) && canJump;
        if (canJump && InputManager::Get().IsKeyDown(GLFW_KEY_SPACE))
            playerYV = jumpVelocity;
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

        // 이동 처리
        if (InputManager::Get().IsKeyDown(GLFW_KEY_W))
            camera.position += flatDir * camera.speed * dt;
        if (InputManager::Get().IsKeyDown(GLFW_KEY_S))
            camera.position -= flatDir * camera.speed * dt;
        if (InputManager::Get().IsKeyDown(GLFW_KEY_A))
            camera.position -= right * camera.speed * dt;
        if (InputManager::Get().IsKeyDown(GLFW_KEY_D))
            camera.position += right * camera.speed * dt;

        if (canJump) { // 웅크리기 처리
            float targetHeight = Camera::playerHeight;
            if (crouching) targetHeight *= crouchRatio;

            if (camera.position.y < targetHeight) {
                camera.position.y += crouchSpeed * dt;
                camera.position.y = std::min(camera.position.y, targetHeight);
            } else if (camera.position.y > targetHeight) {
                camera.position.y -= crouchSpeed * dt;
                camera.position.y = std::max(camera.position.y, targetHeight);
            }
        } else { // 점프 처리
            camera.position.y += playerYV * dt + 0.5f * gravity * dt * dt;
            playerYV += gravity * dt;
            if (camera.position.y <= Camera::playerHeight) {
                camera.position.y = Camera::playerHeight;
                playerYV = 0.0f;
            }
        }

        canJump = (camera.position.y <= Camera::playerHeight + 0.001f) && (playerYV == 0.0f);
    }

    void Render() override {
        const Camera& camera = Camera::Get();
        glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
        glUniform3fv(viewPosLocation, 1, glm::value_ptr(camera.position));
    }

private:
    GLint viewLocation = -1;
    GLint viewPosLocation = -1;

    bool crouching = false;
    static constexpr float crouchRatio = 0.7f;
    static constexpr float crouchSpeed = 5.5f;

    bool canJump = true;
    float playerYV = 0.0f;
    static constexpr float gravity = -9.8f * 2;
    static constexpr float jumpVelocity = 6.0f;
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

        const float sensitivityRate = 0.1f;
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
struct FireListener {
    virtual void Fire() = 0;
    virtual void Reload(float reloadTime) = 0;
};
class WeaponSystem : public Component {
public:
    int maxAmmo = 30;
    int currentAmmo = 30;
    bool isReloading = false;
    float reloadTime = 1.5f;
    float reloadTimer = 0.f;
    float fireInterval = 0.1f; // 600 RPM
    float fireCooldown = 0.f;
    std::vector<GameObject*>* targets = nullptr;
    std::vector<FireListener*> fireListeners;

    void Update(float dt) override {
        if (fireCooldown > 0.f)
            fireCooldown -= dt;
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

    void addFireListener(FireListener* listener) { fireListeners.push_back(listener); }

    void Fire() {
        if (currentAmmo <= 0 || isReloading || !targets || fireCooldown > 0.f)
            return;
        currentAmmo--;
        fireCooldown = fireInterval;

       
        const Camera& camera = Camera::Get();
        PhysicsSystem::Ray ray = {camera.position, camera.GetForward()};
        TargetObject* nearestHitTarget = NULL;
        float minHitDist = FLT_MAX;
        // TODO : 로드는 한번만
        CpuMesh* cpuMesh = ResourceManager::Get().GetCpuMesh("target");

        for (auto* obj : *targets) {
            if (obj->state != ETargetState::Spawned)
                continue;
            auto* target = dynamic_cast<TargetObject*>(obj);
            if (!target || target->state != ETargetState::Spawned)
                continue;
  
            if (PhysicsSystem::Get().RaySphereIntersect(ray, target->transform.position, target->radius)) {
                if (PhysicsSystem::Get().RayMeshIntersect(ray, *cpuMesh, target->transform.GetWorldMatrix())) {
                    float dist = glm::distance(camera.position, target->transform.position);
                    if (dist < minHitDist) {
                        nearestHitTarget = target;
                        minHitDist = dist;     
                    }

                }
            }
        }
        if (nearestHitTarget != nullptr)
            nearestHitTarget->OnHit();
        for (auto* fireListener : fireListeners)
            fireListener->Fire();
    }

    void Reload() {
        if (!isReloading && currentAmmo < maxAmmo) {
            isReloading = true;
            reloadTimer = reloadTime;
            for (auto* fireListener : fireListeners)
                fireListener->Reload(reloadTime / 2.f);
        }
    }
};
class CrossHairComponent : public Component {
public:
    void Start() override { pOwner->transform.scale = glm::vec3(32.0f, 32.0f, 1.0f); }
    void Update(float dt) override {
        (void)dt;
        auto& gc = GraphicsContext::Get();

        int w, h;
        gc.GetWindowSize(w, h);

        pOwner->transform.position.x = w * 0.5f;
        pOwner->transform.position.y = h * 0.5f;
        pOwner->transform.position.z = 0.0f;
    }
};
class GunController : public Component, FireListener {
public:
    glm::vec3 offset;

    float recoilBack = 0.0f;
    float recoilUp = 0.0f;
    float recoilPitch = 0.0f;

    float reloadOffset = 0.f;
    float reloadRotZ = 0.f;
    float reloadTimer = 0.f;

    GunController(WeaponSystem* ws, glm::vec3 offset) : offset(offset) {
        ws->addFireListener(this);
    }

    void Fire() override {
        recoilBack = std::min(recoilBack + 0.15f, 0.25f);
        recoilUp = std::min(recoilUp + 0.05f, 0.11f);
        recoilPitch = std::min(recoilPitch + 3.0f, 6.0f);
    }

    // TODO :  장전시 총 위치 아래로 + 살짝 회전
    void Reload(float reloadTime) override { 
        reloadTimer = reloadTime;
    }

    void Update(float dt) override {
        recoilBack = std::max(0.0f, recoilBack - 0.6f * dt);
        recoilUp = std::max(0.0f, recoilUp - 1.9f * dt);
        recoilPitch = std::max(0.0f, recoilPitch - 90.0f * dt);

         if (reloadTimer > 0.f) {
            reloadTimer -= dt;
            reloadOffset = std::min(reloadOffset + 1.f * dt, 0.2f);
            reloadRotZ = std::min(reloadRotZ + 30.f * dt, 45.f);
        } else {
            reloadOffset = std::max(0.0f, reloadOffset - 1.f * dt);
            reloadRotZ = std::max(0.0f, reloadRotZ - 90.f * dt);
        }

        const Camera& camera = Camera::Get();
        glm::vec3 forward = camera.GetForward();
        glm::vec3 right = glm::normalize(glm::cross(forward, camera.up));
        glm::vec3 up = glm::normalize(glm::cross(right, forward));
        glm::vec3 finalOffset = offset + glm::vec3(0.0f, recoilUp - reloadOffset, -recoilBack);
        pOwner->transform.position =
          camera.position + right * finalOffset.x + up * finalOffset.y + forward * finalOffset.z;

        pOwner->transform.rotation.x = -camera.pitch - recoilPitch;
        pOwner->transform.rotation.y = 90.0f - camera.yaw;
        pOwner->transform.rotation.z = reloadRotZ;
    }
};

class TargetController : public Component
{
public:
    float angle;
    float radius = 1.0f;
    bool canMove = false;
    glm::vec3 cachedOwnerLocation;
    float moveTimer = 0.f;
    float moveRange = 9.f;
    float moveSpeed = 1.f;
    void Start()
    { 
        cachedOwnerLocation = pOwner->transform.position;
        angle = pOwner->transform.rotation.x;
    }

    void Update(float dt) override
    { 
        if (pOwner->state == ETargetState::Spawned) {
            angle = std::min(0.0f, angle + 180.f * dt);
            canMove = angle == 0.0f;
        } 
        if (pOwner->state == ETargetState::Dying) {
            angle = std::max(-90.0f, angle - 180.f * dt);
            canMove = false;
            if (angle == -90.f)
            {
                pOwner->state = ETargetState::Died;
            }
        }

        if (canMove)
        {
            moveTimer += dt;
            pOwner->transform.position.x = cachedOwnerLocation.x + sin(moveTimer * moveSpeed)  * moveRange;
        }

        float rad = angle * (3.14159f / 180.f);

        pOwner->transform.position.y = cachedOwnerLocation.y + radius * std::cos(rad);
        pOwner->transform.position.z = cachedOwnerLocation.z + radius * std::sin(rad); 
        pOwner->transform.rotation.x = angle;
    }
};

#endif // AIMLAB_COMPONENT_HPP