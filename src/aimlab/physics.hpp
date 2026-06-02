#ifndef AIMLAB_PHYSICS_HPP
#define AIMLAB_PHYSICS_HPP
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

    bool RayMeshIntersect(const Ray& ray, const glutil::ModelData& cpuMesh, const glm::mat4& modelMatrix, float& tMin) {
        tMin = FLT_MAX;
        bool hit = false;

        for (const auto& meshData : cpuMesh.meshes) {
            const glutil::VertexPNT* verts = static_cast<const glutil::VertexPNT*>(meshData.vertexData());
            const unsigned int* indices = meshData.indexData();

            for (size_t i = 0; i < meshData.indexCount(); i += 3) {
                glm::vec3 v0 = glm::vec3(
                  modelMatrix * glm::vec4(verts[indices[i]].x, verts[indices[i]].y, verts[indices[i]].z, 1.0f));
                glm::vec3 v1 = glm::vec3(modelMatrix * glm::vec4(verts[indices[i + 1]].x, verts[indices[i + 1]].y,
                                                                 verts[indices[i + 1]].z, 1.0f));
                glm::vec3 v2 = glm::vec3(modelMatrix * glm::vec4(verts[indices[i + 2]].x, verts[indices[i + 2]].y,
                                                                 verts[indices[i + 2]].z, 1.0f));

                float t;
                if (RayTriangleIntersect(ray, v0, v1, v2, t))
                    if (t < tMin) {
                        tMin = t;
                        hit = true;
                        return hit;
                    }
            }
        }
        return hit;
    }

private:
    PhysicsSystem() = default;

    bool RayTriangleIntersect(const Ray& ray, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, float& t) {
        glm::vec3 e1 = v1 - v0;
        glm::vec3 e2 = v2 - v0;
        glm::vec3 h = glm::cross(ray.direction, e2);
        float a = glm::dot(e1, h);

        if (a > -1e-6f && a < 1e-6f)
            return false; // 레이가 삼각형과 평행

        float f = 1.0f / a;
        glm::vec3 s = ray.origin - v0;
        float u = f * glm::dot(s, h);

        if (u < 0.0f || u > 1.0f)
            return false;

        glm::vec3 q = glm::cross(s, e1);
        float v = f * glm::dot(ray.direction, q);

        if (v < 0.0f || u + v > 1.0f)
            return false;

        t = f * glm::dot(e2, q);
        return t > 0.0f;
    }
};

#endif // AIMLAB_PHYSICS_HPP