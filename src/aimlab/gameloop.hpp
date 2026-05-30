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
        if (glfwGetKey(GraphicsContext::Get().GetWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
            isRunning = false;

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
        while (!glfwWindowShouldClose(GraphicsContext::Get().GetWindow()) && isRunning) {
            float currentTime = (float)glfwGetTime();
            float dt = currentTime - prevTime;
            prevTime = currentTime;

            Input();
            Update(dt);
            InputManager::Get().ResetMouseDelta();
            Render();
        }
    }
};

#endif // AIMLAB_GAMELOOP_HPP