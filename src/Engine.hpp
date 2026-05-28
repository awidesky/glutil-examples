#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <glutil/glutil.hpp>

// Include GLM
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/quaternion.hpp>
using namespace glm;

class GraphicsContext {
public:
    static GraphicsContext& Get() {
        static GraphicsContext gc;
        return gc;
    }

    GraphicsContext(const GraphicsContext&) = delete;
    GraphicsContext& operator=(const GraphicsContext&) = delete;

    bool Init(int w, int h, std::string s) {
        m_width = w;
        m_height = h;
        m_title = s;

        if (!glfwInit())
            return false;

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_window = glfwCreateWindow(w, h, s.c_str(), NULL, NULL);
        if (!m_window) {
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(m_window);

        if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
            return false;

        glViewport(0, 0, w, h);
        glEnable(GL_DEPTH_TEST);
        return true;
    }

    void Clear(float r = 0.1f, float g = 0.1f, float b = 0.1f) {
        glClearColor(r, g, b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Present() {
        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }

    GLFWwindow* GetWindow() const { return m_window; }
    float GetWindowSize() const { return (float)m_width / (float)m_height; }

    ~GraphicsContext() {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

private:
    GraphicsContext() = default;

    GLFWwindow* m_window = nullptr;
    int m_width = 0, m_height = 0;
    std::string m_title;
};

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

    std::pair<float, float> GetMouseDelta() const { return {m_dX, m_dY}; }

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

class Camera {
public:
    Camera(glm::vec3 pos, glm::vec3 up) : m_pos(pos), m_up(up) {};
    glm::vec3 GetForward() {
        return glm::normalize(glm::vec3(cos(glm::radians(m_pitch)) * cos(glm::radians(m_yaw)),
                                        sin(glm::radians(m_pitch)),
                                        cos(glm::radians(m_pitch)) * sin(glm::radians(m_yaw))));
    }

    glm::mat4 GetViewMatrix() { return glm::lookAt(m_pos, m_pos + GetForward(), m_up); }

    glm::mat4 GetProjectionMatrix() {
        return glm::perspective(glm::radians(m_fov), GraphicsContext::Get().GetWindowSize(), 0.1f, 100.f);
    }

    void ProcessMouseMove(float dx, float dy) {
        m_yaw += dx * m_sensitivity;
        m_pitch += dy * m_sensitivity;
        if (m_pitch > 89.0f)
            m_pitch = 89.0f;
        if (m_pitch < -89.0f)
            m_pitch = -89.0f;
    }

    void ProcessKeyboard(float dt) {
        glm::vec3 forward = GetForward();
        glm::vec3 flatDir = glm::normalize(glm::vec3(forward.x, 0, forward.z));
        glm::vec3 right = glm::normalize(glm::cross(flatDir, m_up));

        if (InputManager::Get().IsKeyDown(GLFW_KEY_W))
            m_pos += flatDir * m_speed * dt;
        if (InputManager::Get().IsKeyDown(GLFW_KEY_S))
            m_pos -= flatDir * m_speed * dt;
        if (InputManager::Get().IsKeyDown(GLFW_KEY_A))
            m_pos -= right * m_speed * dt;
        if (InputManager::Get().IsKeyDown(GLFW_KEY_D))
            m_pos += right * m_speed * dt;

        m_pos.y = 0.0f;
    }

    glm::vec3 GetPosition() const { return m_pos; }

private:
    glm::vec3 m_pos = glm::vec3(0.f);
    glm::vec3 m_up = glm::vec3(0.f);
    float m_yaw = -90.0f;
    float m_pitch = 0.0f;
    float m_fov = 45.0f;
    float m_speed = 5.0f;
    float m_sensitivity = 0.05f;
};
#endif // ENGINE_HPP