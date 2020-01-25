#pragma once

#include <string>

namespace dang::gl
{

class GLFW {
public:
	static GLFW Instance;

	void setContext(GLFWwindow* window);

private:
	GLFW();
	~GLFW();

	static std::string formatError(int error_code, const char* description);
	static void errorCallback(int error_code, const char* description);

	bool gladInitialized = false;
	GLFWwindow* window_ = nullptr;
};

}
