#ifndef GL__CORE_H
#define GL__CORE_H

#include <vector>
#include <cassert>
#include <SDL_opengles2.h>

namespace gl
{

inline const char* gluErrorString(GLenum a_error)
{
	switch (a_error) {
	case GL_NO_ERROR:
		return "GL_NO_ERROR";
	case GL_INVALID_ENUM:
		return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE:
		return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:
		return "GL_INVALID_OPERATION";
#ifdef GL_STACK_OVERFLOW
	case GL_STACK_OVERFLOW:
		return "GL_STACK_OVERFLOW";
#endif
#ifdef GL_STACK_UNDERFLOW
	case GL_STACK_UNDERFLOW:
		return "GL_STACK_UNDERFLOW";
#endif
	case GL_OUT_OF_MEMORY:
		return "GL_OUT_OF_MEMORY";
#ifdef GL_TABLE_TOO_LARGE
	case GL_TABLE_TOO_LARGE:
		return "GL_TABLE_TOO_LARGE";
#endif
	default:
		return "Unknown error";
	}
}

inline void f_check()
{
	GLenum error = glGetError();
	if (error != GL_NO_ERROR) throw std::runtime_error(reinterpret_cast<const char*>(gluErrorString(error)));
}

class t_shader
{
	GLuint v_id = 0;

public:
	~t_shader()
	{
		if (v_id == 0) return;
		glDeleteShader(v_id);
		f_check();
	}
	void f_create(GLenum a_type)
	{
		assert(v_id == 0);
		v_id = glCreateShader(a_type);
		f_check();
	}
	operator GLuint() const
	{
		return v_id;
	}
	GLint f_iv(GLenum a_name) const
	{
		GLint value;
		glGetShaderiv(v_id, a_name, &value);
		return value;
	}
	void f_compile(const GLchar* a_source)
	{
		glShaderSource(v_id, 1, &a_source, NULL);
		glCompileShader(v_id);
		if (f_iv(GL_COMPILE_STATUS) == GL_TRUE) return;
		GLint n = f_iv(GL_INFO_LOG_LENGTH);
		std::vector<GLchar> log(n);
		glGetShaderInfoLog(v_id, n, NULL, &log[0]);
		throw std::runtime_error(&log[0]);
	}
};

class t_uniform_location
{
	GLint v_id;

public:
	t_uniform_location(GLint a_id = 0) : v_id(a_id)
	{
	}
	operator GLint() const
	{
		return v_id;
	}
	void f_uniform(GLfloat a_x)
	{
		glUniform1f(v_id, a_x);
		f_check();
	}
	void f_uniform1(GLsizei a_count, const GLfloat* a_values)
	{
		glUniform1fv(v_id, a_count, a_values);
		f_check();
	}
	void f_uniform(GLint a_x)
	{
		glUniform1i(v_id, a_x);
		f_check();
	}
	void f_uniform1(GLsizei a_count, const GLint* a_values)
	{
		glUniform1iv(v_id, a_count, a_values);
		f_check();
	}
	void f_uniform(GLfloat a_x, GLfloat a_y)
	{
		glUniform2f(v_id, a_x, a_y);
		f_check();
	}
	void f_uniform2(GLsizei a_count, const GLfloat* a_values)
	{
		glUniform2fv(v_id, a_count, a_values);
		f_check();
	}
	void f_uniform(GLint a_x, GLint a_y)
	{
		glUniform2i(v_id, a_x, a_y);
		f_check();
	}
	void f_uniform2(GLsizei a_count, const GLint* a_values)
	{
		glUniform2iv(v_id, a_count, a_values);
		f_check();
	}
	void f_uniform(GLfloat a_x, GLfloat a_y, GLfloat a_z)
	{
		glUniform3f(v_id, a_x, a_y, a_z);
		f_check();
	}
	void f_uniform3(GLsizei a_count, const GLfloat* a_values)
	{
		glUniform3fv(v_id, a_count, a_values);
		f_check();
	}
	void f_uniform(GLint a_x, GLint a_y, GLint a_z)
	{
		glUniform3i(v_id, a_x, a_y, a_z);
		f_check();
	}
	void f_uniform3(GLsizei a_count, const GLint* a_values)
	{
		glUniform3iv(v_id, a_count, a_values);
		f_check();
	}
	void f_uniform(GLfloat a_x, GLfloat a_y, GLfloat a_z, GLfloat a_w)
	{
		glUniform4f(v_id, a_x, a_y, a_z, a_w);
		f_check();
	}
	void f_uniform4(GLsizei a_count, const GLfloat* a_values)
	{
		glUniform4fv(v_id, a_count, a_values);
		f_check();
	}
	void f_uniform(GLint a_x, GLint a_y, GLint a_z, GLint a_w)
	{
		glUniform4i(v_id, a_x, a_y, a_z, a_w);
		f_check();
	}
	void f_uniform4(GLsizei a_count, const GLint* a_values)
	{
		glUniform4iv(v_id, a_count, a_values);
		f_check();
	}
	void f_matrix2(const GLfloat* a_values, GLsizei a_count = 1)
	{
		glUniformMatrix2fv(v_id, a_count, GL_FALSE, a_values);
		f_check();
	}
	void f_matrix3(const GLfloat* a_values, GLsizei a_count = 1)
	{
		glUniformMatrix3fv(v_id, a_count, GL_FALSE, a_values);
		f_check();
	}
	void f_matrix4(const GLfloat* a_values, GLsizei a_count = 1)
	{
		glUniformMatrix4fv(v_id, a_count, GL_FALSE, a_values);
		f_check();
	}
};

class t_program
{
	GLuint v_id = 0;

public:
	~t_program()
	{
		if (v_id == 0) return;
		glDeleteProgram(v_id);
		f_check();
	}
	void f_create()
	{
		assert(v_id == 0);
		v_id = glCreateProgram();
		f_check();
	}
	operator GLuint() const
	{
		return v_id;
	}
	GLint f_iv(GLenum a_name) const
	{
		GLint value;
		glGetProgramiv(v_id, a_name, &value);
		return value;
	}
	void f_attach(GLuint a_shader)
	{
		glAttachShader(v_id, a_shader);
		f_check();
	}
	void f_link()
	{
		glLinkProgram(v_id);
		if (f_iv(GL_LINK_STATUS) == GL_TRUE) return;
		GLint n = f_iv(GL_INFO_LOG_LENGTH);
		std::vector<GLchar> log(n);
		glGetProgramInfoLog(v_id, n, NULL, &log[0]);
		throw std::runtime_error(&log[0]);
	}
	GLint f_get_attrib_location(const GLchar* a_name) const
	{
		GLint index = glGetAttribLocation(v_id, a_name);
		if (index < 0) throw std::runtime_error(reinterpret_cast<const char*>(gluErrorString(glGetError())));
		return index;
	}
	t_uniform_location f_get_uniform_location(const GLchar* a_name) const
	{
		GLint index = glGetUniformLocation(v_id, a_name);
		if (index < 0) throw std::runtime_error(reinterpret_cast<const char*>(gluErrorString(glGetError())));
		return index;
	}
};

class t_buffer
{
	GLuint v_id = 0;

public:
	~t_buffer()
	{
		if (v_id == 0) return;
		glDeleteBuffers(1, &v_id);
		f_check();
	}
	void f_create()
	{
		assert(v_id == 0);
		glGenBuffers(1, &v_id);
		f_check();
	}
	operator GLuint() const
	{
		return v_id;
	}
};

class t_texture
{
	GLuint v_id = 0;

public:
	~t_texture()
	{
		if (v_id == 0) return;
		glDeleteTextures(1, &v_id);
		f_check();
	}
	void f_create()
	{
		assert(v_id == 0);
		glGenTextures(1, &v_id);
		f_check();
	}
	operator GLuint() const
	{
		return v_id;
	}
};

}

#endif
