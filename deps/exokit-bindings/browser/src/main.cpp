// Ŭnicode please
#include <string>
#include <map>
#include <algorithm>
#include <cassert>

// GL
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "gl_core.h"

#include "render_handler.h"
#include "web_core.h"

std::weak_ptr<WebCore> web_core;
WebCoreManager g_web_core_manager;

static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	bool pressed = (action == GLFW_PRESS);
	web_core.lock()->keyPress(key, pressed);
}

void mouse_callback(GLFWwindow* window, int btn, int state, int mods)
{
	int mouse_up = (GLFW_RELEASE == state);

	std::map<int, CefBrowserHost::MouseButtonType> btn_type_map;
	btn_type_map[GLFW_MOUSE_BUTTON_LEFT] = MBT_LEFT;
	btn_type_map[GLFW_MOUSE_BUTTON_MIDDLE] = MBT_MIDDLE;
	btn_type_map[GLFW_MOUSE_BUTTON_RIGHT] = MBT_RIGHT;
	CefBrowserHost::MouseButtonType btn_type = btn_type_map[btn];

	web_core.lock()->mouseClick(btn_type, mouse_up);
}

void motion_callback(GLFWwindow* window, double x, double y)
{
	web_core.lock()->mouseMove(x, y);
}

GLFWwindow *initialize_glfw_window(int w, int h)
{
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
	{
		return nullptr;
	}

	GLFWwindow* window = glfwCreateWindow(w, h, "CEF + OpenGL", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	return window;
}

void shutdown_glfw(GLFWwindow *window)
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

bool initialize_glew_context()
{
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		// Problem: glewInit failed, something is seriously wrong
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		return false;
	}
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
	return true;
}

void reshape_callback(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
}


GLint pos_loc = -1;
GLint texcoord_loc = -1;
GLint tex_loc = -1;
GLint mvp_loc = -1;


int main(int argc, char *argv[])
{
	// CEF / GL 초기화 순서
	// CEF -> GL 순서로 초기화할 경우 다음과 같은 문제가 생길수 있다
	// * CEF 초기화 성공 -> GL 초기화 실패 -> 현재 프로세스 종료 -> 근데 CEF는 별도 프로세스로 돌아가니까 제대로 안꺼짐
	// GL -> CEF 순서로 초기화 할 경우 다음의 문제가 생길수 있다
	// * CEF는 내부에서 sub-process를 생성한다.
	// * CefExecuteProcess 를 호출하지 않으면 현재 프로세스가 자식인지 부모인지 확실히 알수없다.

	// 두가지 문제중에서 CEF 프로세스의 정체를 확인하는게 더 골치아플거라고 생각해서 CEF->GL 순서로 초기화함
	int exit_code = 0;
	bool success = g_web_core_manager.setUp( &exit_code );
	if ( !success ) { 
		return exit_code; 
	}

	int width = 640;
	int height = 480;

	// create GL context
	GLFWwindow *window = initialize_glfw_window(width, height);
	if (!window) {
		g_web_core_manager.shutDown();
		return -1;
	}

	// initialize glew context
	bool glew_init_success = initialize_glew_context();	
	if (!glew_init_success) {
		g_web_core_manager.shutDown();
		return -1;
	}

	std::string url = "http://google.com";
	web_core = g_web_core_manager.createBrowser(url);
	web_core.lock()->reshape(width, height);

	std::string other_url = "https://github.com/if1live/";
	auto web_core_other = g_web_core_manager.createBrowser(other_url);
	web_core_other.lock()->reshape(width, height);

	// setup glfw
	glfwSwapInterval(1);
	glfwSetKeyCallback(window, key_callback);  
	glfwSetCursorPosCallback(window, motion_callback);
	glfwSetMouseButtonCallback(window, mouse_callback);
	glfwSetFramebufferSizeCallback(window, reshape_callback);

	// shader
	GLuint prog = GLCore::createShaderProgram("shaders/tex.vert", "shaders/tex.frag");
	assert(prog != 0 && "shader compile failed");

	pos_loc = glGetAttribLocation(prog, "a_position");
	texcoord_loc = glGetAttribLocation(prog, "a_texcoord");
	tex_loc = glGetUniformLocation(prog, "s_tex");
	mvp_loc = glGetUniformLocation(prog, "u_mvp");

	// initial GL state
	glViewport(0, 0, width, height);
	glClearColor(0.0, 0.0, 0.0, 0.0);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);

	// 2 3
	// 0 1
	float vertices[] = {
		-1, -1, 0,
		1, -1, 0,
		-1, 1, 0,
		1, 1, 0,
	};

	float texcoords[] = {
		0, 1,
		1, 1,
		0, 0,
		1, 0,
	};

	unsigned short indices[] = {
		0, 1, 3,
		0, 3, 2,
	};

	while (!glfwWindowShouldClose( window ) )
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(prog);

		glm::mat4 mvp = glm::ortho(-1, 1, -1, 1);
		//mvp *= glm::rotate((float)glfwGetTime() * 10.f, glm::vec3(0, 0, 1));
		glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, glm::value_ptr(mvp));

		glEnableVertexAttribArray(pos_loc);
		glEnableVertexAttribArray(texcoord_loc);

		// bind texture
		glBindTexture(GL_TEXTURE_2D, web_core.lock()->render_handler()->tex());

		// draw
		glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE, 0, vertices);
		glVertexAttribPointer(texcoord_loc, 2, GL_FLOAT, GL_FALSE, 0, texcoords);
		glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_SHORT, indices);


		// draw other web core
		mvp *= glm::rotate((float)glfwGetTime() * 0.1f, glm::vec3(0, 0, 1));
		mvp *= glm::scale(glm::vec3(0.5, 0.5, 0.5));
		glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, glm::value_ptr(mvp));
		glBindTexture(GL_TEXTURE_2D, web_core_other.lock()->render_handler()->tex());
		glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_SHORT, indices);

		// render end
		glfwSwapBuffers(window);

		// update
		glfwPollEvents();

		g_web_core_manager.update();
	}

	GLCore::deleteProgram(prog);
	shutdown_glfw(window);

	// close cef
	g_web_core_manager.removeBrowser( web_core );
	g_web_core_manager.shutDown();

	return 0;  
}