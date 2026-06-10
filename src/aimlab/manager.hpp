#ifndef AIMLAB_MANAGER_HPP
#define AIMLAB_MANAGER_HPP


#include <vector>
#include <cstdlib>
#include <cstdio>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <algorithm>
#include <config.hpp>

#include "component.hpp"
#include "mesh.hpp"
#include "score.hpp"


enum class EGameState { Waiting, Playing, Result, CountDown };

class GameStateManager {
public:
    float countdownTimer = 3.f;
    static GameStateManager& Get() {
        static GameStateManager state;
        return state;
    }
    GameStateManager(const GameStateManager&) = delete;
    GameStateManager& operator=(const GameStateManager&) = delete;

    EGameState state = EGameState::Waiting;

    void StartCountDown() {
        countdownTimer = 3.f;
        state = EGameState::CountDown;
    }
    void StartRound() { state = EGameState::Playing; }
    void EndRound() { state = EGameState::Result; }

private:
    GameStateManager() = default;
};

class ParticleSystem {
public:
    static ParticleSystem& Get() {
        static ParticleSystem instance;
        return instance;
    }
    ParticleSystem(const ParticleSystem&) = delete;
    ParticleSystem& operator=(const ParticleSystem&) = delete;

    std::vector<GameObject*> pendingParticles;

    void Emit(glm::vec3 pos, glm::vec3 dir, int count) {
        for (int i = 0; i < count; i++) {
            glm::vec3 up = glm::abs(dir.y) < 0.9f ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
            glm::vec3 tangent = glm::normalize(glm::cross(up, dir));
            glm::vec3 bitangent = glm::normalize(glm::cross(dir, tangent));

            float rx = ((float)rand() / RAND_MAX * 2.f - 1.f);
            float ry = ((float)rand() / RAND_MAX * 2.f - 1.f);
            float rz = ((float)rand() / RAND_MAX * 2.f - 1.f);
            glm::vec3 vel = glm::normalize(dir * (0.3f + rz * 0.7f) + // 메인 방향 성분
                                           tangent * rx * 0.8f +      // 옆 퍼짐
                                           bitangent * ry * 0.8f      // 위아래 퍼짐
                                           ) *
                            (4.f + (float)rand() / RAND_MAX * 4.f);
            auto* obj = new GameObject();
            obj->transform.position = pos;
            obj->transform.scale = glm::vec3(0.1f);

            Mesh* mesh = ResourceManager::Get().GetMesh("particle");
            Texture* tex = ResourceManager::Get().GetTexture("particle");
            obj->AddComponent(new MeshRenderer(mesh, new Material(tex)));
            obj->AddComponent(new ParticleComponent(-vel));

            pendingParticles.push_back(obj);
        }
    }

private:
    ParticleSystem() = default;
};
#endif // AIMLAB_MANAGER_HPP