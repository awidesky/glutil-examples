// #include <glad/gl.h>
// #include <GLFW/glfw3.h>
//
// #define GLM_ENABLE_EXPERIMENTAL
// #include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>
// #include <glm/gtc/quaternion.hpp>
// #include <glm/gtx/quaternion.hpp>
// #include <glm/gtx/euler_angles.hpp>
// #include <glm/gtx/norm.hpp>
//
// #include <glutil/glutil.hpp>
//
// #define STB_IMAGE_IMPLEMENTATION
// #include "stb_image.h"
// #include <glm/gtc/type_ptr.hpp>
//
// float keyVal = 0.2f;
// float camSpeed = 0.1f;
// float lastX = 400, lastY = 300;
// glm::vec3 camDir = glm::vec3(0.0f, 0.0f, -1.0f);
// glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);
// glm::vec3 camPos = glm::vec3(0.0f, 0.0f, 3.0f);
// glm::mat4 view = glm::lookAt(
//	glm::vec3(0.0f, 0.0f, 3.0f),
//	glm::vec3(0.0f, 0.0f, 0.0f),
//	glm::vec3(0.0f, 1.0f, 0.0f)
//);
// bool firstMouse = true;
// float pitch = 0.f;
// float yaw = 0.f;
//
//
// struct Material
//{
//	glm::vec3 ambient;
//	glm::vec3 diffuse;
//	glm::vec3 specular;
//	float shiness;
// };
// Material material;
//
// struct Light
//{
//	glm::vec3 pos;
//	glm::vec3 ambient;
//	glm::vec3 diffuse;
//	glm::vec3 specular;
// };
// Light light;
//
//
//
// void framebuffer_size_callback(GLFWwindow* window, int w, int h)
//{
//	glViewport(0, 0, w, h);
// }
//
// void processInput(GLFWwindow* window)
//{
//	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//	{
//		glfwSetWindowShouldClose(window, true);
//	}
//
//	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
//	{
//		keyVal += 0.01f;
//		keyVal = std::min(keyVal, 1.0f);
//	}
//
//	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
//	{
//		keyVal -= 0.01f;
//		keyVal = std::max(keyVal, 0.0f);
//	}
//
//	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
//		camPos += camSpeed * camDir;
//	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
//		camPos -= camSpeed * camDir;
//	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
//		camPos -= glm::normalize(glm::cross(camDir, camUp)) * camSpeed;
//	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
//		camPos += glm::normalize(glm::cross(camDir, camUp)) * camSpeed;
//
//	camPos = glm::vec3(camPos.x, 0.f, camPos.z);
// }
//
// glm::mat4 myLookAt(glm::vec3 pos, glm::vec3 target, glm::vec3 up)
//{
//	glm::vec3 zAxis = glm::normalize(pos - target);
//
//	glm::vec3 xAxis = glm::normalize(glm::cross(glm::normalize(up),zAxis));
//
//	glm::vec3 yAxis = glm::normalize(glm::cross(zAxis, xAxis));
//
//	glm::mat4 rotation = glm::mat4(1.0f);
//	rotation[0][0] = xAxis.x; rotation[1][0] = xAxis.y; rotation[2][0] = xAxis.z;
//	rotation[0][1] = yAxis.x; rotation[1][1] = yAxis.y; rotation[2][1] = yAxis.z;
//	rotation[0][2] = zAxis.x; rotation[1][2] = zAxis.y; rotation[2][2] = zAxis.z;
//
//	glm::mat4 tran = glm::mat4(1.0f);
//	tran[3][0] = -pos.x;
//	tran[3][1] = -pos.y;
//	tran[3][2] = -pos.z;
//
//	return rotation * tran;
// }
//
//
// void mouse_callback(GLFWwindow* window, double xpos, double ypos)
//{
//
//	if (firstMouse)
//	{
//		lastX = xpos;
//		lastY = ypos;
//		yaw = -90.f;
//		firstMouse = false;
//	}
//
//	float xoffset = xpos - lastX;
//	float yoffset = lastY - ypos;
//	lastX = xpos;
//	lastY = ypos;
//
//	float sensitivity = 0.03f;
//	xoffset *= sensitivity;
//	yoffset *= sensitivity;
//
//	yaw += xoffset;
//	pitch += yoffset;
//
//	if (pitch > 89.0f)
//		pitch = 89.0f;
//	if (pitch < -89.0f)
//		pitch = -89.0f;
//
//	glm::vec3 front;
//	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
//	front.y = sin(glm::radians(pitch));
//	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
//
//	camDir = glm::normalize(front);
// }
//
// int main()
//{
//	// glfw : Graphics Library FrameWork (창 생성, 입력 처리, 시간 측정, 버퍼 스왑, 이벤트 처리 등등)
//	glfwInit();
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//
//	// window 객체 생성
//	GLFWwindow* window = glfwCreateWindow(800, 600, "MAKEWINDOW", NULL, NULL);
//	if (window == NULL)
//	{
//		std::cout << "Failed Make Window\n";
//		glfwTerminate();
//		return -1;
//	}
//
//	// window를 현재 OpenGL 컨텍스트로 지정
//	glfwMakeContextCurrent(window);
//
//	// GLAD 로드 , glfwGetProcAddress : GLFW가 제공하는 함수 주소 조회기
//	// gladLoadGL : 이 조회기를 써서 모든 OpenGL 함수 주소를 한 번에 다 가져옴
//	if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
//	{
//		std::cout << "Failed Load GLAD\n";
//		glfwTerminate();
//		return -1;
//	}
//
//	// 그림 그릴 영역 설정
//	glViewport(0, 0, 800, 600);
//	glEnable(GL_DEPTH_TEST);
//	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
//	glfwSetCursorPosCallback(window, mouse_callback);
//
//	// 창의 크기가 바뀔 떄 마다 호출되는 함수
//	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
//
//
//	float vertices[] = {
//		// positions          // normals           // texture coords
//-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
//  0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
//  0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
//  0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
//-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
//-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
//
//-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
//  0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
//  0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
//  0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
//-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
//-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
//
//-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
//-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
//-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
//-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
//-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
//-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
//
//  0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
//  0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
//  0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
//  0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
//  0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
//  0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
//
//-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
//  0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
//  0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
//  0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
//-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
//-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
//
//-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
//  0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
//  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
//  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
//-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
//-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
//	};
//
//
//	unsigned int indices[] = {
//	0, 1, 2,  0, 2, 3,  // 뒷면
//	4, 5, 6,  4, 6, 7,  // 앞면
//	4, 0, 3,  4, 3, 7,  // 왼쪽
//	1, 5, 6,  1, 6, 2,  // 오른쪽
//	4, 5, 1,  4, 1, 0,  // 아랫면
//	3, 2, 6,  3, 6, 7,  // 윗면
//	};
//
//
//	// 셰이더 로딩
//	//const char* vs =
//	//	"#version 410 core\n"
//	//	"layout (location = 0) in vec3 aPos;"
//	//	"layout (location = 1) in vec2 aUV;"
//	//	"uniform mat4 model;"
//	//	"uniform mat4 view;"
//	//	"uniform mat4 projection;"
//	//	"out vec4 vertexColor;"
//	//	"out vec2 TexCoord;"
//	//	"void main() {"
//	//	"  gl_Position = projection*view*model * vec4( aPos, 1.0 );"
//	//	"	vertexColor = gl_Position;"
//	//	"	TexCoord = aUV;"
//	//	"}";
//
//	//const char* fs =
//	//	"#version 410 core\n"
//	//	"in vec4 vertexColor;"
//	//	"in vec2 TexCoord;"
//	//	"uniform sampler2D myTexture;"
//	//	"uniform sampler2D myTexture1;"
//	//	"uniform float alpha;"
//	//	"out vec4 frag_colour;"
//	//	"void main() {"
//	//	"  frag_colour = mix(texture(myTexture, TexCoord),texture(myTexture1, vec2(TexCoord.x, 1 - TexCoord.y)),
//alpha);"
//	//	"}";
//	const char* vs =
//		"#version 410 core\n"
//		"layout (location = 0) in vec3 aPos;"
//		"layout (location = 1) in vec3 aNormal;"
//		"layout(location = 2) in vec2 aTexCoords;"
//		"uniform mat4 model;"
//		"uniform mat4 view;"
//		"uniform mat4 projection;"
//		"out vec3 Normal;"
//		"out vec3 FragPos;"
//		"out vec2 TexCoords;"
//		"void main() {"
//		"  gl_Position = projection * view * model * vec4(aPos, 1.0);"
//		"  FragPos = vec3(model * vec4(aPos, 1.0));"
//		"  Normal = mat3(transpose(inverse(model))) * aNormal;"
//		"  TexCoords = aTexCoords;"
//		"}";
//	const char* fs =
//		"#version 410 core\n"
//		"in vec3 Normal;"
//		"in vec3 FragPos;"
//		"in vec2 TexCoords;"
//		"uniform vec3 viewPos;"
//
//		"struct Material {"
//		"    sampler2D diffuse;"
//		"    sampler2D specular;"
//		"    float shininess;"
//		"};"
//		"uniform Material material;"
//
//		"struct DirLight {"
//		"    vec3 direction;"
//		"    vec3 ambient;"
//		"    vec3 diffuse;"
//		"    vec3 specular;"
//		"};"
//		"uniform DirLight dirLight;"
//
//		"struct PointLight {"
//		"    vec3 position;"
//		"    float constant;"
//		"    float linear;"
//		"    float quadratic;"
//		"    vec3 ambient;"
//		"    vec3 diffuse;"
//		"    vec3 specular;"
//		"};\n"
//		"#define NR_POINT_LIGHTS 4\n"
//		"uniform PointLight pointLights[NR_POINT_LIGHTS];\n"
//
//		"struct SpotLight {"
//		"    vec3 position;"
//		"    vec3 direction;"
//		"    float cutOff;"
//		"    float outerCutOff;"
//		"    float constant;"
//		"    float linear;"
//		"    float quadratic;"
//		"    vec3 ambient;"
//		"    vec3 diffuse;"
//		"    vec3 specular;"
//		"};"
//		"uniform SpotLight spotLight;"
//
//		"out vec4 frag_colour;"
//
//		// 함수 프로토타입
//		"vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);"
//		"vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);"
//		"vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);"
//
//		"void main() {"
//		"    vec3 norm = normalize(Normal);"
//		"    vec3 viewDir = normalize(viewPos - FragPos);"
//		"    vec3 result = CalcDirLight(dirLight, norm, viewDir);"
//		"    for(int i = 0; i < NR_POINT_LIGHTS; i++)"
//		"        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);"
//		"    result += CalcSpotLight(spotLight, norm, FragPos, viewDir);"
//		"    frag_colour = vec4(result, 1.0);"
//		"}"
//
//		"vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {"
//		"    vec3 lightDir = normalize(-light.direction);"
//		"    float diff = max(dot(normal, lightDir), 0.0);"
//		"    vec3 reflectDir = reflect(-lightDir, normal);"
//		"    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);"
//		"    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, TexCoords));"
//		"    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.diffuse, TexCoords));"
//		"    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));"
//		"    return (ambient + diffuse + specular);"
//		"}"
//
//		"vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {"
//		"    vec3 lightDir = normalize(light.position - fragPos);"
//		"    float diff = max(dot(normal, lightDir), 0.0);"
//		"    vec3 reflectDir = reflect(-lightDir, normal);"
//		"    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);"
//		"    float distance = length(light.position - fragPos);"
//		"    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance *
//distance));" 		"    vec3 ambient  = light.ambient  * vec3(texture(material.diffuse, TexCoords));" 		"    vec3 diffuse  =
//light.diffuse  * diff * vec3(texture(material.diffuse, TexCoords));" 		"    vec3 specular = light.specular * spec *
//vec3(texture(material.specular, TexCoords));" 		"    ambient  *= attenuation;" 		"    diffuse  *= attenuation;" 		" specular
//*= attenuation;" 		"    return (ambient + diffuse + specular);"
//		"}"
//
//		"vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {"
//		"    vec3 lightDir = normalize(light.position - fragPos);"
//		"    float diff = max(dot(normal, lightDir), 0.0);"
//		"    vec3 reflectDir = reflect(-lightDir, normal);"
//		"    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);"
//		"    float distance = length(light.position - fragPos);"
//		"    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance *
//distance));" 		"    float theta = dot(lightDir, normalize(-light.direction));" 		"    float epsilon = light.cutOff -
//light.outerCutOff;" 		"    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);" 		"    vec3 ambient
//= light.ambient  * vec3(texture(material.diffuse, TexCoords));" 		"    vec3 diffuse  = light.diffuse  * diff *
//vec3(texture(material.diffuse, TexCoords));" 		"    vec3 specular = light.specular * spec *
//vec3(texture(material.specular, TexCoords));" 		"    ambient  *= attenuation * intensity;" 		"    diffuse  *= attenuation
//* intensity;" 		"    specular *= attenuation * intensity;" 		"    return (ambient + diffuse + specular);"
//		"}";
//
//	const char* lampVs =
//		"#version 410 core\n"
//		"layout (location = 0) in vec3 aPos;"
//		"uniform mat4 model;"
//		"uniform mat4 view;"
//		"uniform mat4 projection;"
//		"void main() {"
//		"  gl_Position = projection * view * model * vec4(aPos, 1.0);"
//		"}";
//
//	const char* lampFs =
//		"#version 410 core\n"
//		"out vec4 frag_colour;"
//		"void main() {"
//		"  frag_colour = vec4(1.0);"  // 항상 흰색
//		"}";
//
//	GLuint v = glCreateShader(GL_VERTEX_SHADER);
//	glShaderSource(v, 1, &vs, NULL);
//	glCompileShader(v);
//
//	GLuint f = glCreateShader(GL_FRAGMENT_SHADER);
//	glShaderSource(f, 1, &fs, NULL);
//	glCompileShader(f);
//
//	GLuint p = glCreateProgram();
//	glAttachShader(p, v);
//	glAttachShader(p, f);
//	glLinkProgram(p);
//	int success;
//	char infoLog[512];
//
//	glGetShaderiv(f, GL_COMPILE_STATUS, &success);
//	if (!success)
//	{
//		glGetShaderInfoLog(f, 512, NULL, infoLog);
//		std::cout << infoLog << '\n';
//		return 0;
//	}
//	glUseProgram(p);
//	glDeleteShader(v);
//	glDeleteShader(f);
//
//	GLuint lampV = glCreateShader(GL_VERTEX_SHADER);
//	glShaderSource(lampV, 1, &lampVs, NULL);
//	glCompileShader(lampV);
//
//	GLuint lampF = glCreateShader(GL_FRAGMENT_SHADER);
//	glShaderSource(lampF, 1, &lampFs, NULL);
//	glCompileShader(lampF);
//
//	GLuint lampP = glCreateProgram();
//	glAttachShader(lampP, lampV);
//	glAttachShader(lampP, lampF);
//	glLinkProgram(lampP);
//	glDeleteShader(lampV);
//	glDeleteShader(lampF);
//
//
//	// VBO VAO EBO
//	GLuint vao, vbo, ebo;
//	glGenBuffers(1, &vbo);
//	glGenVertexArrays(1, &vao);
//	//glGenBuffers(1, &ebo);
//
//	glBindVertexArray(vao);
//	glBindBuffer(GL_ARRAY_BUFFER, vbo);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
//
//	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
//	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
//
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)0);
//	glEnableVertexAttribArray(0);
//
//	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)12);
//	glEnableVertexAttribArray(1);
//
//	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)24);
//	glEnableVertexAttribArray(2);
//	glBindVertexArray(0);
//
//	GLuint lampVao;
//	glGenVertexArrays(1, &lampVao);
//	glBindVertexArray(lampVao);
//	glBindBuffer(GL_ARRAY_BUFFER, vbo);  // 같은 VBO 재사용
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
//	glEnableVertexAttribArray(0);
//	glBindVertexArray(0);
//
//	//// 텍스쳐
//	//int w, h, c;
//	//unsigned char* data = stbi_load("container.jpg", &w, &h, &c, 0);
//	//GLuint tex;
//	//glGenTextures(1, &tex);
//	//glBindTexture(GL_TEXTURE_2D, tex);
//
//	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
//	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
//	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
//	//glGenerateMipmap(GL_TEXTURE_2D);
//
//	//stbi_image_free(data);
//
//	//data = stbi_load("awesomeface.png", &w, &h, &c, 0);
//	//GLuint tex2;
//	//glGenTextures(1, &tex2);
//	//glBindTexture(GL_TEXTURE_2D, tex2);
//
//	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
//	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
//	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//
//	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
//	//glGenerateMipmap(GL_TEXTURE_2D);
//
//
//	//glUniform1i(glGetUniformLocation(p, "myTexture"), 0);
//	//glUniform1i(glGetUniformLocation(p, "myTexture1"), 1);
//
//	int w, h, c;
//	stbi_set_flip_vertically_on_load(true);
//
//	unsigned char* data = stbi_load("container2.png", &w, &h, &c, 0);
//	GLuint tex;
//	glGenTextures(1, &tex);
//	glBindTexture(GL_TEXTURE_2D, tex);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
//	glGenerateMipmap(GL_TEXTURE_2D);
//
//	stbi_image_free(data);
//	stbi_set_flip_vertically_on_load(true);
//
//
//	data = stbi_load("container2_specular.png", &w, &h, &c, 0);
//	GLuint tex2;
//	glGenTextures(1, &tex2);
//	glBindTexture(GL_TEXTURE_2D, tex2);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
//	glGenerateMipmap(GL_TEXTURE_2D);
//
//	stbi_image_free(data);
//
//	/*data = stbi_load("matrix.jpg", &w, &h, &c, 0);
//	GLuint tex3;
//	glGenTextures(1, &tex3);
//	glBindTexture(GL_TEXTURE_2D, tex3);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
//	glGenerateMipmap(GL_TEXTURE_2D);
//
//	stbi_image_free(data);*/
//
//	glm::mat4 projection = glm::mat4(1.0f);
//	projection = glm::perspective(glm::radians(75.f), 800.f / 600.f, 0.1f, 100.f);
//
//	glUniformMatrix4fv(glGetUniformLocation(p, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
//
//
//	// dir light
//	glUniform3f(glGetUniformLocation(p, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
//	glUniform3f(glGetUniformLocation(p, "dirLight.ambient"), 0.0f, 0.0f, 0.0f);
//	glUniform3f(glGetUniformLocation(p, "dirLight.diffuse"), 0.0f, 0.0f, 0.0f);
//	glUniform3f(glGetUniformLocation(p, "dirLight.specular"), 0.0f, 0.0f, 0.0f);
//
//
//	glm::vec3 pointLightPositions[] = {
//	glm::vec3(0.7f,  0.2f,  2.0f),
//	glm::vec3(2.3f, -3.3f, -4.0f),
//	glm::vec3(-4.0f,  2.0f, -12.0f),
//	glm::vec3(0.0f,  0.0f, -3.0f)
//	};
//
//
//	glm::vec3 cubePositions[] = {
//	glm::vec3(0.0f,  0.0f,  0.0f),
//	glm::vec3(2.0f,  5.0f, -15.0f),
//	glm::vec3(-1.5f, -2.2f, -2.5f),
//	glm::vec3(-3.8f, -2.0f, -12.3f),
//	glm::vec3(2.4f, -0.4f, -3.5f),
//	glm::vec3(-1.7f,  3.0f, -7.5f),
//	glm::vec3(1.3f, -2.0f, -2.5f),
//	glm::vec3(1.5f,  2.0f, -2.5f),
//	glm::vec3(1.5f,  0.2f, -1.5f),
//	glm::vec3(-1.3f,  1.0f, -1.5f)
//	};
//
//	float currentTime = 0, prevTime = 0;
//	float rotateOffset = 0;
//	// 렌더링 루프
//	while (!glfwWindowShouldClose(window))
//	{
//		processInput(window);
//
//		currentTime = glfwGetTime();
//		float deltaTime = currentTime - prevTime;
//		prevTime = currentTime;
//
//		rotateOffset += deltaTime * 3;
//		// 렌더링
//		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//
//		// 램프
//		float offX = cos(rotateOffset);
//		float offZ = sin(rotateOffset);
//
//		glm::vec3 lightPos(0.5f, 0.5f, 1.5f);
//
//		glUseProgram(lampP);
//		glBindVertexArray(lampVao);
//
//		for (int i = 0; i < 4; i++) {
//			glm::mat4 lampModel = glm::mat4(1.0f);
//			lampModel = glm::translate(lampModel, pointLightPositions[i]);
//			lampModel = glm::scale(lampModel, glm::vec3(0.2f));
//			glUniformMatrix4fv(glGetUniformLocation(lampP, "model"), 1, GL_FALSE, glm::value_ptr(lampModel));
//			glUniformMatrix4fv(glGetUniformLocation(lampP, "view"), 1, GL_FALSE, glm::value_ptr(view));
//			glUniformMatrix4fv(glGetUniformLocation(lampP, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
//			glDrawArrays(GL_TRIANGLES, 0, 36);
//		}
//
//		glBindVertexArray(0);
//
//
//		// 박스
//		glUseProgram(p);
//		glBindVertexArray(vao);
//
//		//glActiveTexture(GL_TEXTURE1);
//		//glBindTexture(GL_TEXTURE_2D, tex2);
//
//		glUniform1f(glGetUniformLocation(p, "alpha"), keyVal);
//
//		//float camX = cos(glfwGetTime()) * 10.f;
//		//float camZ = sin(glfwGetTime()) * 10.f;
//		view = glm::lookAt(camPos, camPos + camDir, camUp);
//		glUniformMatrix4fv(glGetUniformLocation(p, "view"), 1, GL_FALSE, glm::value_ptr(view));
//
//		// 박스
//		glUniform3f(glGetUniformLocation(p, "viewPos"), camPos.x, camPos.y, camPos.z);
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, tex);
//		glUniform1i(glGetUniformLocation(p, "material.diffuse"), 0);
//
//		glActiveTexture(GL_TEXTURE1);
//		glBindTexture(GL_TEXTURE_2D, tex2);
//		glUniform1i(glGetUniformLocation(p, "material.specular"), 1);
//
//		glUniform1f(glGetUniformLocation(p, "material.shininess"), 122.f);
//
//		for (int i = 0; i < 4; i++) {
//			std::string idx = "pointLights[" + std::to_string(i) + "]";
//			glUniform3fv(glGetUniformLocation(p, (idx + ".position").c_str()), 1,
//glm::value_ptr(pointLightPositions[i])); 			glUniform1f(glGetUniformLocation(p, (idx + ".constant").c_str()), 1.0f);
//			glUniform1f(glGetUniformLocation(p, (idx + ".linear").c_str()), 0.09f);
//			glUniform1f(glGetUniformLocation(p, (idx + ".quadratic").c_str()), 0.032f);
//			glUniform3f(glGetUniformLocation(p, (idx + ".ambient").c_str()), 0.0f, 0.0f, 0.0f);
//			glUniform3f(glGetUniformLocation(p, (idx + ".diffuse").c_str()), 0.1f, 0.0f, 0.0f);
//			glUniform3f(glGetUniformLocation(p, (idx + ".specular").c_str()), 0.1f, 0.0f, 0.0f);
//		}
//
//
//		// Spot Light (루프 안, 카메라 따라다님)
//		glUniform3fv(glGetUniformLocation(p, "spotLight.position"), 1, glm::value_ptr(camPos));
//		glUniform3fv(glGetUniformLocation(p, "spotLight.direction"), 1, glm::value_ptr(camDir));
//		glUniform1f(glGetUniformLocation(p, "spotLight.cutOff"), glm::cos(glm::radians(30.5f)));
//		glUniform1f(glGetUniformLocation(p, "spotLight.outerCutOff"), glm::cos(glm::radians(40.0f)));
//		glUniform1f(glGetUniformLocation(p, "spotLight.constant"), 1.0f);
//		glUniform1f(glGetUniformLocation(p, "spotLight.linear"), 0.09f);
//		glUniform1f(glGetUniformLocation(p, "spotLight.quadratic"), 0.032f);
//		glUniform3f(glGetUniformLocation(p, "spotLight.ambient"), 0.2f, 0.2f, 0.2f);
//		glUniform3f(glGetUniformLocation(p, "spotLight.diffuse"), 0.2f, 0.2f, 0.2f);
//		glUniform3f(glGetUniformLocation(p, "spotLight.specular"), 1.0f, 1.0f, 1.0f);
//
//		/*glActiveTexture(GL_TEXTURE2);
//		glBindTexture(GL_TEXTURE_2D, tex3);
//		glUniform1i(glGetUniformLocation(p, "material.emission"), 2);*/
//
//		glm::vec3 lightcolor = glm::vec3(1.0f);
//		//lightcolor.x = sin(rotateOffset);
//		//lightcolor.y = cos(rotateOffset);
//		//lightcolor.z = cos(rotateOffset * 2.f);
//
//		glm::vec3 diffuseColor = lightcolor * glm::vec3(1.0f);
//		glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f);
//
//		glUniform3f(glGetUniformLocation(p, "viewPos"), camPos.x, camPos.y, camPos.z);
//
//
//		for (int i = 0; i < 10; ++i)
//		{
//			glm::mat4 model = glm::mat4(1.0f);
//			model = glm::translate(model, cubePositions[i]);
//			int flag = 0;
//			if (i % 3 == 0) flag = 1;
//			float angle = 20 * i + rotateOffset * flag;
//			model = glm::rotate(model, glm::radians(angle), glm::vec3(0.1f, 0.4f, 0.2f));
//
//			glUniformMatrix4fv(glGetUniformLocation(p, "model"), 1, GL_FALSE, glm::value_ptr(model));
//			glDrawArrays(GL_TRIANGLES, 0, 36);
//		}
//
//		glm::mat4 model = glm::mat4(1.0f);
//		glUniform3f(glGetUniformLocation(p, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
//		glUniformMatrix4fv(glGetUniformLocation(p, "model"), 1, GL_FALSE, glm::value_ptr(model));
//		glDrawArrays(GL_TRIANGLES, 0, 36);
//		glBindVertexArray(0);
//
//
//		glfwSwapBuffers(window);
//		// OS 이벤트 처리
//		glfwPollEvents();
//	}
//
//	glfwTerminate();
//	return 0;
// }



#define AIMLAB_OPTION_GL_DEBUG


#include "config.hpp"
#include "gameloop.hpp"
#include "mesh.hpp"
#include "manager.hpp"

#include <glm/glm.hpp>
#include <glutil/glutil.hpp>
#include <iostream>

int main() {
    auto& gc = GraphicsContext::Get();
    bool ok = gc.Init(1920, 1080, "AIMLAB");
    if (!ok)
        return -1;
    gc.mouseSensitivity = 0.1f;
    gc.fov = 75.0f;
    gc.ambientStrength = 0.5f;

    InputManager::Get().Init();

    glutil::GLProgram program = glutil::ShaderLoader::loadProgramToGL(glutil::PROJECT_ROOT / "shader" / "aimlab.vert",
                                                                      glutil::PROJECT_ROOT / "shader" / "aimlab.frag");
    if (!program.ok) {
        std::cout << program.error;
        return -1;
    }
    gc.SetProgram(program.id);

    auto& rm = ResourceManager::Get();
    rm.SetDefaultTexture(glutil::PROJECT_ROOT / "texture" / "grid.bmp");
    //Texture* defaultTexture = ResourceManager::Get().GetDefaultTexture();
    //if (!defaultTexture || !defaultTexture->ok) { // TODO : better error checking and printing(inside add* function? and exit policy.
    //    std::cout << defaultTexture->error;
    //    return -1;
    //}

    rm.AddMesh("plane", glutil::PROJECT_ROOT / "model" / "plane.obj");
    Mesh* planeMesh = rm.GetMesh("plane");
    if (!planeMesh || !planeMesh->ok) {
        std::cout << planeMesh->error;
        return -1;
    }

    Camera& camera = Camera::Get();
    camera.position = glm::vec3(0.f, 1.f, 0.f);
    camera.up = glm::vec3(0.f, 1.f, 0.f);
    camera.yaw = -90.f;
    camera.pitch = 0.f;
    camera.speed = 5.f;

    GameLoop gEngine;

    GameObject* system = new GameObject();
    system->AddComponent(new SystemController());
    gEngine.system.push_back(system);

    GameObject* cameraObject = new GameObject();
    auto* cameraController = new CameraController();
    cameraObject->AddComponent(cameraController);
    auto* weaponsystem = new WeaponSystem();
    weaponsystem->targets = &gEngine.world3d;
    cameraObject->AddComponent(weaponsystem);
    gEngine.system.push_back(cameraObject);

    GameObject* plane = new GameObject();
    plane->transform.rotation.y = 180.f;
    plane->transform.scale = glm::vec3(10.f, 1.f, 10.f);

    auto* planeRenderer = new MeshRenderer(planeMesh, new Material());
    plane->AddComponent(planeRenderer);
    gEngine.world3d.push_back(plane);

    // Target ResourceRenderer
    rm.AddMesh("target", glutil::PROJECT_ROOT / "model" / "target.obj");
    rm.AddTexture("target", glutil::PROJECT_ROOT / "texture" / "target.png");
    
    GameObject* TargetSpawner = new GameObject();
    TargetSpawner->transform.position = glm::vec3(0.f, 0.f, 0.f);
    auto* spawner = new TargetSpawnerComponent();
    spawner->targetsToSpawn = &gEngine.targetsToSpawn;
    TargetSpawner->AddComponent(spawner);
    gEngine.system.push_back(TargetSpawner);

    GameObject* gun = new GameObject();
    gun->transform.scale = glm::vec3(0.01f);
    gun->transform.position = glm::vec3(-0.2f, 0.5f, 0.2f);
    gun->AddComponent(new GunController(weaponsystem));
    gun->AddComponent(new ViewModelRenderer(
      rm.AddMesh("gun", glutil::PROJECT_ROOT / "assets" / "ak47" / "ak47.obj"),
      new Material(rm.AddTexture("gun", glutil::PROJECT_ROOT / "assets" / "ak47" / "123456_wire_115115115_color.png"))));
    gEngine.world2d.push_back(gun);

    GameObject* crosshair = new GameObject();
    crosshair->AddComponent(new CrossHairComponent());
    crosshair->AddComponent(
     new OrthogonalRenderer(rm.AddMesh("crosshair", glutil::PROJECT_ROOT / "model" / "crosshair.obj"),
                      new Material(rm.AddTexture("crosshair", glutil::PROJECT_ROOT / "texture" / "crosshair.png"))));
    gEngine.world2d.push_back(crosshair);


#ifdef AIMLAB_OPTION_GL_DEBUG
    glutil::debug::snapshot(true).bufferVAOInfo(true, true, true).capture();
#endif

    gEngine.Run();

    return 0;
}