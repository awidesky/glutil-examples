#ifndef AIMLAB_GAMELOOP_HPP
#define AIMLAB_GAMELOOP_HPP

#include "engine.hpp"
#include "component.hpp"
#include "manager.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <vector>


class GameLoop {
public:
    std::vector<GameObject*> system;
    std::vector<GameObject*> world3d;
    std::vector<GameObject*> world2d;
    std::vector<TargetObject*> targetsToSpawn;
    bool isRunning = true;


    ~GameLoop() {
        for (auto* obj : system) delete obj;
        for (auto* obj : world3d) delete obj;
        for (auto* obj : world2d) delete obj;
        system.clear();
        world3d.clear();
        world2d.clear();
    }

    void Input() {
        GraphicsContext::Get().PollEvents();
        for (auto* obj : system)  obj->Input();
        for (auto* obj : world3d) obj->Input();
        for (auto* obj : world2d) obj->Input();
    }

    void Update(float dt) {
        for(auto* obj : system) obj->Update(dt);
        for (auto* obj : world3d) obj->Update(dt);
        for (auto* obj : world2d) obj->Update(dt);
    }

    void Render() {
        GraphicsContext::Get().Clear();
        for (auto* obj : system) obj->Render();
        for (auto* obj : world3d) obj->Render();
        for (auto* obj : world2d) obj->Render();
        GraphicsContext::Get().Present();
    }

    void Run() {
        float prevTime = (float)glfwGetTime();
        // 게임 내 로직으로 판단히 중지해야 하거나, 화면이 닫히면 끈다.
        while (!glfwWindowShouldClose(GraphicsContext::Get().GetWindow()) && isRunning) {
            float currentTime = (float)glfwGetTime();
            float dt = currentTime - prevTime;
            prevTime = currentTime;

            Input();
            Update(dt);
            InputManager::Get().ResetMouseDelta(); // TODO : 컴포넌트로 빼거나 3함수 중 하나에 넣기?
            Render();

            /** 추가됐던 월드 객체들을 다 넣어준다. */
            for (auto* t : targetsToSpawn) world3d.push_back(t);
            for (auto* t : ParticleSystem::Get().pendingParticles) world3d.push_back(t);
            ParticleSystem::Get().pendingParticles.clear();
            targetsToSpawn.clear();

            // 객체 삭제 Swap And Pop 방식
            for (size_t i = world3d.size() - 1; i > -1; --i)
            {
                if (world3d[i]->state == Died)
                {
                    delete world3d[i];
                    world3d[i] = world3d.back();
                    world3d.pop_back();
                }
            }
        }
    }
};

#endif // AIMLAB_GAMELOOP_HPP