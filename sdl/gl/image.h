#ifndef GL__IMAGE_H
#define GL__IMAGE_H

#include <xanadu/matrix>

#include "shaders.h"

namespace gl
{

void f_load_rgba(const std::wstring& a_path, size_t& a_width, size_t& a_height, std::vector<unsigned char>& a_data);

struct t_image
{
	float v_width;
	float v_height;
	t_program v_program;
	t_uniform_location v_projection;
	t_uniform_location v_vertex_matrix;
	GLuint v_vertex;
	GLuint v_texcoord;
	t_uniform_location v_color;
	t_texture v_texture;
	t_buffer v_vertices;

	void f_create(const std::wstring& a_path, GLint a_format = GL_RGBA);
	template<typename T_array>
	void operator()(const t_matrix4f& a_projection, const t_matrix4f& a_vertex, const T_array& a_array);
};

struct t_font
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
	void f_create(const std::wstring& a_path);
	void operator()(const t_matrix4f& a_projection, const t_matrix4f& a_vertex, const std::wstring& a_text);
};

template<typename T_array>
void t_image::operator()(const t_matrix4f& a_projection, const t_matrix4f& a_vertex, const T_array& a_array)
{
	f_use_program(v_program);
	v_projection.f_matrix4(a_projection.v_array);
	v_vertex_matrix.f_matrix4(a_vertex.v_array);
	f_bind_buffer(GL_ARRAY_BUFFER, v_vertices);
	f_buffer_data(GL_ARRAY_BUFFER, a_array.size() * sizeof(float), a_array.data(), GL_STREAM_DRAW);
	f_enable_vertex_attrib_array(v_vertex);
	f_vertex_attrib_pointer(v_vertex, 3, GL_FLOAT, false, 5 * sizeof(float), 0);
	f_enable_vertex_attrib_array(v_texcoord);
	f_vertex_attrib_pointer(v_texcoord, 2, GL_FLOAT, false, 5 * sizeof(float), 3 * sizeof(float));
	f_active_texture(GL_TEXTURE0);
	f_bind_texture(GL_TEXTURE_2D, v_texture);
	v_color.f_uniform(0);
	f_draw_arrays(GL_TRIANGLE_STRIP, 0, a_array.size() / 5);
	f_use_program(0);
	f_bind_buffer(GL_ARRAY_BUFFER, 0);
	f_bind_texture(GL_TEXTURE_2D, 0);
}

}

#endif
