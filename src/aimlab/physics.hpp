#ifndef PHYSCIS_HPP
#define PHYSCIS_HPP
#include <glm/glm.hpp>

class PhysicsSystem {
public:
    static PhysicsSystem& Get() {
        static PhysicsSystem instance;
        return instance;
    }
    PhysicsSystem(const PhysicsSystem&) = delete;
    PhysicsSystem& operator=(const PhysicsSystem&) = delete;

    struct Ray {
        glm::vec3 origin;
        glm::vec3 direction;
    };

    bool RaySphereIntersect(const Ray& ray, const glm::vec3& center, float radius, float& t) {
        glm::vec3 oc = ray.origin - center;
        float a = glm::dot(ray.direction, ray.direction);
        float b = 2.f * glm::dot(oc, ray.direction);
        float c = glm::dot(oc, oc) - radius * radius;
        float disc = b*b - 4*a*c;
        if (disc < 0) return false;
        t = (-b - sqrt(disc)) / (2.f * a);
        return t > 0;
    }

private:
    PhysicsSystem() = default;
};

#endif // PHYSCIS_HPP