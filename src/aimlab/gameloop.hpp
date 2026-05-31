#ifndef AIMLAB_GAMELOOP_HPP
#define AIMLAB_GAMELOOP_HPP

#include "engine.hpp"
#include "component.hpp"

#include <GLFW/glfw3.h>
#include <vector>

class GameLoop {
public:
    std::vector<GameObject*> world;
    bool isRunning = true;

    ~GameLoop() {
        for (auto* obj : world)
            delete obj;
        world.clear();
    }

    void Input() {
        for (auto* obj : world)
            obj->Input();
    }

    void Update(float dt) {
        for (auto* obj : world)
            obj->Update(dt);
    }

    void Render() {
        GraphicsContext::Get().Clear();
        for (auto* obj : world)
            obj->Render();
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
        }
    }
};

#endif // AIMLAB_GAMELOOP_HPP