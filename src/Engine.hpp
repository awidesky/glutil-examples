#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glutil/glutil.hpp>

// Include GLM
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>
using namespace glm;


class GraphicsContext
{
public:
	static GraphicsContext& Get()
	{
		static GraphicsContext gc;
		return gc;
	}

	GraphicsContext(const GraphicsContext&) = delete;
	GraphicsContext& operator=(const GraphicsContext&) = delete;

	bool Init(float w, float h, std::string s)
	{
		m_width = w; m_height = h; m_title = s;

		if (!glfwInit()) return false;

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		m_window = glfwCreateWindow(w, h, s.c_str(), NULL, NULL);
		if (!m_window)
		{
			glfwTerminate();
			return false;
		}

		glfwMakeContextCurrent(m_window);

		if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) return false;

		glViewport(0, 0, w, h);
		glEnable(GL_DEPTH_TEST);
		return true;
	}

	void Clear(float r = 0.1f, float g = 0.1f, float b = 0.1f)
	{
		glClearColor(r, g, b,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Present()
	{
		glfwSwapBuffers(m_window);
		glfwPollEvents();
	}

	GLFWwindow* GetWindow() const { return m_window; }

	float GetWindowSize() const {
		return m_width / m_height;
	}

	~GraphicsContext()
	{
		glfwDestroyWindow(m_window);
		glfwTerminate();
	}
private:
	GraphicsContext() = default;
	
	GLFWwindow* m_window = nullptr;
	float m_width = 0, m_height = 0;
	std::string m_title;
};

class InputManager
{
public:
	static InputManager& Get()
	{
		static InputManager input;
		return input;
	}

	InputManager(const InputManager&) = delete;
	InputManager& operator=(const InputManager&) = delete;

	void Init()
	{
		GLFWwindow* window = GraphicsContext::Get().GetWindow();
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetCursorPosCallback(window, MouseCallback);
	}

	static void MouseCallback(GLFWwindow* window, double x, double y)
	{
		InputManager::Get().OnMouseMove(x, y);
	}
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

	std::pair<float, float> GetMouseDelta() const {
		return { m_dX, m_dY };
	}

	void Reset()
	{
		m_dX = m_dY = 0;
	}

	bool IsKeyDown(int key) const {
		return glfwGetKey(GraphicsContext::Get().GetWindow(), key) == GLFW_PRESS;
	}

	bool IsMouseDown(int button) const {
		return glfwGetMouseButton(GraphicsContext::Get().GetWindow(), button) == GLFW_PRESS;
	}
private:
	InputManager() = default;
	double m_lastX = 0, m_lastY = 0;
	double m_dX = 0, m_dY = 0;
	bool m_firstMouse = true;
};