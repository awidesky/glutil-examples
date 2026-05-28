#ifndef INPUTMANAGER_HPP
#define INPUTMANAGER_HPP

#include "graphicsContext.hpp"

#include <utility>

class InputManager {
public:
    static InputManager& Get() {
        static InputManager input;
        return input;
    }

    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    void Init() {
        GLFWwindow* window = GraphicsContext::Get().GetWindow();
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(window, MouseCallback);
    }

    static void MouseCallback(GLFWwindow* window, double x, double y) { InputManager::Get().OnMouseMove(x, y); }
    void OnMouseMove(double x, double y) {
        if (m_firstMouse) {
            m_lastX = x;
            m_lastY = y;
            m_firstMouse = false;
        }
        m_dX = x - m_lastX;
        m_dY = m_lastY - y;
        m_lastX = x;
        m_lastY = y;
    }

    std::pair<double, double> GetMouseDelta() const { return {m_dX, m_dY}; }

    void ResetMouseDelta() { m_dX = m_dY = 0; }

    bool IsKeyDown(int key) const { return glfwGetKey(GraphicsContext::Get().GetWindow(), key) == GLFW_PRESS; }
    bool IsMouseDown(int button) const {
        return glfwGetMouseButton(GraphicsContext::Get().GetWindow(), button) == GLFW_PRESS;
    }

private:
    InputManager() = default;
    double m_lastX = 0, m_lastY = 0;
    double m_dX = 0, m_dY = 0;
    bool m_firstMouse = true;
};

#endif // INPUTMANAGER_HPP