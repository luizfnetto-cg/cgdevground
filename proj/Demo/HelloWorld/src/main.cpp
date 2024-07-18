#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_internal.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

typedef struct Vertex
{
	glm::vec2 pos;
	glm::vec3 col;
} Vertex;

static const Vertex vertices[3] = {
	{{-0.6f, -0.4f}, {1.f, 0.f, 0.f}}, {{0.6f, -0.4f}, {0.f, 1.f, 0.f}}, {{0.f, 0.6f}, {0.f, 0.f, 1.f}}};

static const char* vertex_shader_text =
	"#version 330\n"
	"uniform mat4 MVP;\n"
	"in vec3 vCol;\n"
	"in vec2 vPos;\n"
	"out vec3 color;\n"
	"void main()\n"
	"{\n"
	"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
	"    color = vCol;\n"
	"}\n";

static const char* fragment_shader_text =
	"#version 330\n"
	"in vec3 color;\n"
	"out vec4 fragment;\n"
	"void main()\n"
	"{\n"
	"    fragment = vec4(color, 1.0);\n"
	"}\n";

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void UpdateImGUIDpiScale()
{
	float MonitorScale = 1.f;
	for (const ImGuiPlatformMonitor& Monitor : ImGui::GetPlatformIO().Monitors)
	{
		static const float BaseDPI = 96.f;
		ImVec2 WinSize = Monitor.MainSize;

		if (MonitorScale < Monitor.DpiScale)
			MonitorScale = Monitor.DpiScale;

		printf("List of monitors:\n");
		printf("WxH: %dx%d DPI: %.2f (x%.2f)\n", (int) WinSize.x, (int) WinSize.y, BaseDPI * Monitor.DpiScale, Monitor.DpiScale);
	}
	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(MonitorScale);
	ImGuiIO& io = ImGui::GetIO();
	float FontSize = 13.f*MonitorScale;
	io.Fonts->AddFontFromFileTTF("../resources/fonts/DejaVuSansMono.ttf", FontSize);
}

int main(void)
{
	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(1920, 1080, "OpenGL Triangle", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window);
	gladLoadGL(glfwGetProcAddress);
	glfwSwapInterval(1); // Enable vsync

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;	 // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;	 // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;		 // IF using Docking Branch

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(
		window, true);	  // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init();
	
	// NOTE: OpenGL error checks have been omitted for brevity

	GLuint vertex_buffer;
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
	glCompileShader(vertex_shader);

	const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
	glCompileShader(fragment_shader);

	const GLuint program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	const GLint mvp_location = glGetUniformLocation(program, "MVP");
	const GLint vpos_location = glGetAttribLocation(program, "vPos");
	const GLint vcol_location = glGetAttribLocation(program, "vCol");

	GLuint vertex_array;
	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);
	glEnableVertexAttribArray(vpos_location);
	glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, pos));
	glEnableVertexAttribArray(vcol_location);
	glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, col));
	
	UpdateImGUIDpiScale();

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT);
		
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::ShowDemoWindow();	// Show demo window! :)

		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		const float ratio = width / (float) height;

		glViewport(0, 0, width, height);

		glm::mat4 m(1);
		glm::mat4 p = glm::ortho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
		m = glm::rotate(m, (float) glfwGetTime(), glm::vec3(0, 0, 1));
		glm::mat4 mvp = p * m;

		glUseProgram(program);
		glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) &mvp);
		glBindVertexArray(vertex_array);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);

	glfwTerminate();
	exit(EXIT_SUCCESS);
}