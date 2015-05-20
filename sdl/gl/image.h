#ifndef GL__IMAGE_H
#define GL__IMAGE_H

#include <xanadu/matrix>

#include "shaders.h"

namespace gl
{

void f_load_rgba(const std::wstring& a_path, size_t& a_width, size_t& a_height, std::vector<unsigned char>& a_data);

struct t_text_renderer
{
	float v_unit;
	t_program v_program;
	t_uniform_location v_projection;
	t_uniform_location v_vertex_matrix;
	t_uniform_location v_offset;
	GLuint v_vertex;
	t_uniform_location v_color;
	t_texture v_texture;
	t_buffer v_vertices;

	void f_create(GLsizei a_width, GLsizei a_height, const GLvoid* a_data, size_t a_count, float a_unit);
	void f_create_from_font(const std::wstring& a_path);
	void f_create_from_file(const std::wstring& a_path, size_t a_count);
	void operator()(const t_matrix4f& a_projection, const t_matrix4f& a_vertex, const std::wstring& a_text);
};

}

#endif
