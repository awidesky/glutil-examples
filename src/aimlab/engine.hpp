#ifndef AIMLAB_ENGINE_HPP
#define AIMLAB_ENGINE_HPP

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
        glfwSwapInterval(1);

        if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
            return false;

        glViewport(0, 0, w, h);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // glEnable(GL_CULL_FACE);
        // glCullFace(GL_BACK);

#ifdef AIMLAB_OPTION_GL_DEBUG
        glutil::debug::printRuntimeInfo();
        glutil::debug::debugCallbackSeverityThreshold = GL_DEBUG_SEVERITY_LOW;
        glutil::debug::init();
        lastPrint = frameBegin = std::chrono::steady_clock::now();
#endif

        return true;
    }

    void Clear(float r = 0.1f, float g = 0.1f, float b = 0.1f) {
        glClearColor(r, g, b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Present() {
#ifdef AIMLAB_OPTION_GL_DEBUG
        const auto renderEnd = std::chrono::steady_clock::now();
#endif
        glFinish();
        glfwSwapBuffers(m_window);
#ifdef AIMLAB_OPTION_GL_DEBUG
        const std::chrono::duration<double> renderElapsed = renderEnd - frameBegin;
        renderTime += renderElapsed.count();
        frameCount++;

        const auto printNow = std::chrono::steady_clock::now();
        const std::chrono::duration<double> printElapsed = printNow - lastPrint;
        if (printElapsed.count() >= 5.0) {
            const double frameTimeAvgMs = (printElapsed.count() / static_cast<double>(frameCount)) * 1000.0;
            const double renderTimeAvgMs = (renderTime / static_cast<double>(frameCount)) * 1000.0;
            const double renderFramePercentage = 100.0 * renderTimeAvgMs / frameTimeAvgMs;
            if (renderFramePercentage < 110.0) // 100 이상은 말이 되지 않음(lastPrint의 초기화 때문에 맨 처음은 그렇게 나옴, 무시해야 함)
                std::cout << "[Render Time / Frame Time] " << std::fixed << std::setprecision(4) << renderTimeAvgMs
                          << " / " << frameTimeAvgMs << " ms (" << std::setprecision(2) << renderFramePercentage << "%)"
                          << std::endl;

            lastPrint = printNow;
            frameCount = 0;
            renderTime = 0.0;
        }
        frameBegin = std::chrono::steady_clock::now();
#endif
    }
    void PollEvents() { glfwPollEvents(); }

    GLFWwindow* GetWindow() const { return m_window; }
    float GetWindowAspect() const {
        int w = 1;
        int h = 1;
        GetWindowSize(w, h);
        return h > 0 ? (float)w / (float)h : 1.0f;
    }
    void GetWindowSize(int& width, int& height) const {
        glfwGetFramebufferSize(m_window, &width, &height);
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

#ifdef AIMLAB_OPTION_GL_DEBUG
    std::chrono::steady_clock::time_point lastPrint, frameBegin;
    unsigned int frameCount = 0;
    double renderTime = 0.0;
#endif
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
using CpuMesh = glutil::ModelData;
struct Texture {
    glutil::GLTexture2D tex;
    Texture(glutil::GLTexture2D&& loadedTex) : tex(std::move(loadedTex)) {
        glBindTexture(GL_TEXTURE_2D, tex.id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        GLfloat maxAniso = 1.0f;
        //glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
        //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, std::min(8.0f, maxAniso));
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

    CpuMesh* GetCpuMesh(const std::string& name) {
        auto it = _cpuMeshes.find(name);

        if (it == _cpuMeshes.end())
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

    Mesh* AddMesh(const std::string& name, const std::filesystem::path& path) {
        return _meshes[name] = new Mesh(glutil::ModelLoader::loadOBJtoGL(path));
    }
    CpuMesh* AddCpuMesh(const std::string& name, const std::filesystem::path& path) {
        return _cpuMeshes[name] = new CpuMesh(glutil::ModelLoader::loadOBJ(path));
    }
    Texture* AddTexture(const std::string& name, const std::filesystem::path& path) {
        return _textures[name] = new Texture(glutil::ImageLoader::loadImageToGL(path));
    }
    Program* AddProgram(const std::string& name, const std::filesystem::path& vs, const std::filesystem::path& fs) {
        return _programs[name] = new Program(glutil::ShaderLoader::loadProgramToGL(vs, fs));
    }

    void Clear() {
        delete _defaultTexture;
        _defaultTexture = nullptr;

        for (auto& pair : _meshes)
            delete pair.second;

        for (auto& pair : _cpuMeshes)
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
    std::unordered_map<std::string, CpuMesh*> _cpuMeshes;
    std::unordered_map<std::string, Texture*> _textures;
    std::unordered_map<std::string, Program*> _programs;
};
#endif // AIMLAB_ENGINE_HPP