// Ŭnicode please
#include <GL/glew.h>

namespace GLCore
{
	GLuint compileShaderFromCode(GLenum shader_type, const char *src);
	GLuint compileShaderFromFile(GLenum shader_type, const char *filepath);

	GLuint createShaderProgram(const char *vert, const char *frag);
	GLuint createShaderProgram(GLuint vert, GLuint frag);

	bool deleteShader(GLuint shader);
	bool deleteProgram(GLuint program);
};