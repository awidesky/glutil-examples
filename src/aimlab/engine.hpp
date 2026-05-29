#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <glutil/glutil.hpp>

#include <utility>
#include <filesystem>
#include <string>
#include <unordered_map>

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

        const auto version = glutil::debug::availableGLversion();
        glfwWindowHint(GLFW_SAMPLES, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, version.major);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, version.minor);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

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

    std::pair<float, float> GetMouseDelta() const { return {float(m_dX), float(m_dY)}; }

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


using Mesh = glutil::GLModelData;
using Texture = glutil::GLTexture2D;
using Program = glutil::GLProgram;

class ResourceManager {
public:
    ResourceManager() = default;
    ~ResourceManager() { Clear(); }

public:
    Mesh* GetMesh(const std::string& name) {
        auto it = _meshes.find(name);

        if (it == _meshes.end())
            return nullptr;

        return it->second;
    }

    Texture* GetTexture(const std::string& name) {
        auto it = _textures.find(name);

        if (it == _textures.end())
            return nullptr;

        return it->second;
    }

    Program* GetProgram(const std::string& name) {
        auto it = _programs.find(name);

        if (it == _programs.end())
            return nullptr;

        return it->second;
    }

    void AddMesh(const std::string& name, const std::filesystem::path& path) {
        _meshes[name] = new Mesh(glutil::ModelLoader::loadOBJtoGL(path));
    }
    void AddTexture(const std::string& name, const std::filesystem::path& path) {
        _textures[name] = new Texture(glutil::ImageLoader::loadImageToGL(path));
    }
    void AddProgram(const std::string& name, const std::filesystem::path& vs, const std::filesystem::path& fs) {
        _programs[name] = new Program(glutil::ShaderLoader::loadProgramToGL(vs, fs));
    }

    void Clear() {
        for (auto& pair : _meshes)
            delete pair.second;

        for (auto& pair : _textures)
            delete pair.second;

        for (auto& pair : _programs)
            delete pair.second;

        _meshes.clear();
        _textures.clear();
        _programs.clear();
    }

private:
    std::unordered_map<std::string, Mesh*> _meshes;
    std::unordered_map<std::string, Texture*> _textures;
    std::unordered_map<std::string, Program*> _programs;
};

struct Material {
    Material(Program* program) { _program = program; }

    virtual void Bind(GraphicsContext* context) = 0;

private:
    Program* _program;
};
#endif // ENGINE_HPP