#ifndef CAMERA_HPP
#define CAMERA_HPP

// Include GLM
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "engine.hpp"

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
#endif // CAMERA_HPP