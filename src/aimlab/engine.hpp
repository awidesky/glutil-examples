#ifndef AIMLAB_ENGINE_HPP
#define AIMLAB_ENGINE_HPP

//#defing AIMLAB_OPTION_GL_DEBUG

#include <glutil/glutil.hpp>

#include <glm/glm.hpp>

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

        glEnable(GL_MULTISAMPLE);

#ifdef AIMLAB_OPTION_GL_DEBUG
        glutil::debug::init();
#endif

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
    float GetWindowSize() const {
        int w = 1;
        int h = 1;
        glfwGetFramebufferSize(m_window, &w, &h);
        return h > 0 ? (float)w / (float)h : 1.0f;
    }

    void ToggleFullscreen() {
        if (m_isFullscreen)
            ExitFullscreen();
        else
            EnterFullscreen();
    }

    void EnterFullscreen() {
        if (m_isFullscreen)
            return;

        glfwGetWindowPos(m_window, &m_windowedX, &m_windowedY);
        glfwGetWindowSize(m_window, &m_windowedW, &m_windowedH);

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(m_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        glViewport(0, 0, mode->width, mode->height);
        m_isFullscreen = true;
    }

    void ExitFullscreen() {
        if (!m_isFullscreen)
            return;

        glfwSetWindowMonitor(m_window, nullptr, m_windowedX, m_windowedY, m_windowedW, m_windowedH, 0);
        glViewport(0, 0, m_windowedW, m_windowedH);
        m_isFullscreen = false;
    }

    bool IsFullscreen() const { return m_isFullscreen; }

    void SetProgram(GLuint id) {
        m_program = id;
        glUseProgram(id);
    }
    GLuint GetProgram() const { return m_program; }

    ~GraphicsContext() {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    float mouseSensitivity = 0.1f;
    float fov = 75.0f;
    float ambientStrength = 0.5f;
private:
    GraphicsContext() = default;

    GLFWwindow* m_window = nullptr;
    GLuint m_program = 0;
    bool m_isFullscreen = false;
    int m_windowedX = 100;
    int m_windowedY = 100;
    int m_windowedW = 1280;
    int m_windowedH = 720;
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

    static void MouseCallback(GLFWwindow* window, double x, double y) { (void)window; InputManager::Get().OnMouseMove(x, y); }
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
struct Texture {
    glutil::GLTexture2D tex;
    Texture(glutil::GLTexture2D&& loadedTex) : tex(std::move(loadedTex)) {
        glBindTexture(GL_TEXTURE_2D, tex.id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        GLfloat maxAniso = 1.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, std::min(8.0f, maxAniso));
    }
    void bind() const { glBindTexture(GL_TEXTURE_2D, tex.id);}
};
using Program = glutil::GLProgram;

class ResourceManager {
public:
    static ResourceManager& Get() {
        static ResourceManager rm;
        return rm;
    }
    ~ResourceManager() { Clear(); }

    Texture* GetDefaultTexture() const { return _defaultTexture; }

    void SetDefaultTexture(const std::filesystem::path& path) {
        delete _defaultTexture;
        _defaultTexture = new Texture(glutil::ImageLoader::loadImageToGL(path));
    }

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
        delete _defaultTexture;
        _defaultTexture = nullptr;

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
    ResourceManager() = default;
    Texture* _defaultTexture = nullptr;
    std::unordered_map<std::string, Mesh*> _meshes;
    std::unordered_map<std::string, Texture*> _textures;
    std::unordered_map<std::string, Program*> _programs;
};
#endif // AIMLAB_ENGINE_HPP