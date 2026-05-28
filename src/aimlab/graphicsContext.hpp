#ifndef GRAPHICSCONTEXT_HPP
#define GRAPHICSCONTEXT_HPP

#include <glutil/glutil.hpp>

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

#endif // GRAPHICSCONTEXT_HPP