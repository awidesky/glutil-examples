#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "engine.hpp"

class Camera {
public:
    static Camera& Get() {
        static Camera camera;
        return camera;
    }

    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;

    glm::vec3 position{0.f, 1.f, 0.f};
    glm::vec3 up{0.f, 1.f, 0.f};
    float yaw = -90.0f;
    float pitch = 0.0f;
    float speed = 5.0f;

    glm::vec3 GetForward() const {
        return glm::normalize(glm::vec3(cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
                                        sin(glm::radians(pitch)),
                                        sin(glm::radians(yaw)) * cos(glm::radians(pitch))));
    }

    glm::mat4 GetViewMatrix() const {
        return glm::lookAt(position, position + GetForward(), up);
    }

    glm::mat4 GetProjectionMatrix(float fov) const {
        return glm::perspective(glm::radians(fov), GraphicsContext::Get().GetWindowSize(), 0.1f, 100.f);
    }

private:
    Camera() = default;
};

#endif // CAMERA_HPP