#ifndef GL__SHADERS_H
#define GL__SHADERS_H

#include <map>
#include <xanadu/vector>

#include "portable.h"
#include "core.h"

namespace gl
{

using namespace xanadu;

inline void f_use_program(GLuint a_program)
{
	glUseProgram(a_program);
	f_check();
}

inline void f_bind_buffer(GLenum a_target, GLuint a_buffer)
{
	glBindBuffer(a_target, a_buffer);
	f_check();
}

inline void f_buffer_data(GLenum a_target, GLsizeiptr a_size, const GLvoid* a_data, GLenum a_usage)
{
	glBufferData(a_target, a_size, a_data, a_usage);
	f_check();
}

inline void f_enable_vertex_attrib_array(GLuint a_index)
{
	glEnableVertexAttribArray(a_index);
	f_check();
}

inline void f_disable_vertex_attrib_array(GLuint a_index)
{
	glDisableVertexAttribArray(a_index);
	f_check();
}

inline void f_vertex_attrib_pointer(GLuint a_index, GLint a_size, GLenum a_type, bool a_normalized, GLsizei a_stride, GLintptr a_offset)
{
	glVertexAttribPointer(a_index, a_size, a_type, a_normalized ? GL_TRUE : GL_FALSE, a_stride, reinterpret_cast<const GLvoid*>(a_offset));
	f_check();
}

inline void f_draw_arrays(GLenum a_mode, GLint a_first, GLsizei a_count)
{
	glDrawArrays(a_mode, a_first, a_count);
	f_check();
}

inline void f_active_texture(GLenum a_texture)
{
	glActiveTexture(a_texture);
	f_check();
}

inline void f_bind_texture(GLenum a_target, GLuint a_texture)
{
	glBindTexture(a_target, a_texture);
	f_check();
}

inline void f_tex_image2d(GLenum a_target, GLint a_level, GLint a_internal_format, GLsizei a_width, GLsizei a_height, GLint a_border, GLenum a_format, GLenum a_type, const GLvoid* a_data)
{
	glTexImage2D(a_target, a_level, a_internal_format, a_width, a_height, a_border, a_format, a_type, a_data);
	f_check();
}

inline void f_tex_parameteri(GLenum a_target, GLenum a_name, GLint a_value)
{
	glTexParameteri(a_target, a_name, a_value);
	f_check();
}

inline void f_generate_mipmap(GLenum a_target)
{
	glGenerateMipmap(a_target);
	f_check();
}

template<typename T>
struct t_shader_of : T
{
	typedef typename T::t_uniforms t_uniforms;

	using T::T;
	void operator()(const t_uniforms& a_uniforms, GLuint a_attributes, GLenum a_mode, GLint a_offset, GLsizei a_count)
	{
		f_use_program(T::v_program);
		f_bind_buffer(GL_ARRAY_BUFFER, a_attributes);
		T::f_call(a_uniforms, a_mode, a_offset, a_count);
		f_use_program(0);
		f_bind_buffer(GL_ARRAY_BUFFER, 0);
	};
};

class t_shader_base
{
protected:
	t_program v_program;

	void f_build(GLuint a_vshader, GLuint a_fshader)
	{
		v_program.f_create();
		v_program.f_attach(a_vshader);
		v_program.f_attach(a_fshader);
		v_program.f_link();
	}

public:
	struct t_uniforms
	{
		GLsizei v_stride;
		GLsizei v_texcoords;
	};

	operator GLuint() const
	{
		return v_program;
	}
};

struct t_mesh_shader : t_shader_base
{
	struct t_uniforms : t_shader_base::t_uniforms
	{
		const GLfloat* v_projection;
		const GLfloat* v_vertex;
	};

private:
	t_uniform_location v_projection;
	t_uniform_location v_vertex_matrix;
	GLint v_vertex;

protected:
	void f_call(const t_uniforms& a_uniforms, GLenum a_mode, GLint a_offset, GLsizei a_count)
	{
		v_projection.f_matrix4(a_uniforms.v_projection);
		v_vertex_matrix.f_matrix4(a_uniforms.v_vertex);
		f_enable_vertex_attrib_array(v_vertex);
		f_vertex_attrib_pointer(v_vertex, 3, GL_FLOAT, false, a_uniforms.v_stride, 0);
		f_draw_arrays(a_mode, a_offset, a_count);
		f_disable_vertex_attrib_array(v_vertex);
	}

public:
	void f_build(GLuint a_vshader, GLuint a_fshader)
	{
		t_shader_base::f_build(a_vshader, a_fshader);
		v_projection = v_program.f_get_uniform_location("projection");
		v_vertex_matrix = v_program.f_get_uniform_location("vertexMatrix");
		v_vertex = v_program.f_get_attrib_location("vertex");
	}
};

template<typename T>
struct t_with_normal : T
{
	struct t_uniforms : T::t_uniforms
	{
		const GLfloat* v_normal;
	};

private:
	GLint v_normal;
	t_uniform_location v_normal_matrix;

protected:
	void f_call(const t_uniforms& a_uniforms, GLenum a_mode, GLint a_offset, GLsizei a_count)
	{
		f_enable_vertex_attrib_array(v_normal);
		f_vertex_attrib_pointer(v_normal, 3, GL_FLOAT, false, a_uniforms.v_stride, 3 * sizeof(float));
		v_normal_matrix.f_matrix3(a_uniforms.v_normal);
		T::f_call(a_uniforms, a_mode, a_offset, a_count);
		f_disable_vertex_attrib_array(v_normal);
	}

public:
	void f_build(GLuint a_vshader, GLuint a_fshader)
	{
		T::f_build(a_vshader, a_fshader);
		v_normal = T::v_program.f_get_attrib_location("normal");
		v_normal_matrix = T::v_program.f_get_uniform_location("normalMatrix");
	}
};

template<typename T>
struct t_with_color : T
{
	struct t_uniforms : T::t_uniforms
	{
		t_vector4f v_color;
	};

private:
	t_uniform_location v_color;

protected:
	void f_call(const t_uniforms& a_uniforms, GLenum a_mode, GLint a_offset, GLsizei a_count)
	{
		const auto& color = a_uniforms.v_color;
		v_color.f_uniform(color.v_x, color.v_y, color.v_z, color.v_w);
		T::f_call(a_uniforms, a_mode, a_offset, a_count);
	}

public:
	using T::T;
	void f_build(GLuint a_vshader, GLuint a_fshader)
	{
		T::f_build(a_vshader, a_fshader);
		v_color = T::v_program.f_get_uniform_location("color");
	}
};

template<typename T>
struct t_with_texture : T
{
	struct t_uniforms : T::t_uniforms
	{
		GLuint v_color;
	};

private:
	GLint v_texcoord;
	t_uniform_location v_color;

protected:
	void f_call(const t_uniforms& a_uniforms, GLenum a_mode, GLint a_offset, GLsizei a_count)
	{
		f_enable_vertex_attrib_array(v_texcoord);
		f_vertex_attrib_pointer(v_texcoord, 2, GL_FLOAT, false, a_uniforms.v_stride, a_uniforms.v_texcoords);
		f_active_texture(GL_TEXTURE0);
		f_bind_texture(GL_TEXTURE_2D, a_uniforms.v_color);
		v_color.f_uniform(0);
		T::f_call(a_uniforms, a_mode, a_offset, a_count);
		f_disable_vertex_attrib_array(v_texcoord);
		f_bind_texture(GL_TEXTURE_2D, 0);
	}

public:
	using T::T;
	void f_build(GLuint a_vshader, GLuint a_fshader)
	{
		T::f_build(a_vshader, a_fshader);
		v_texcoord = T::v_program.f_get_attrib_location("texcoord");
		v_color = T::v_program.f_get_uniform_location("color");
	}
};

template<typename T>
struct t_with_diffuse_color : T
{
	struct t_uniforms : T::t_uniforms
	{
		t_vector4f v_diffuse;
	};

private:
	t_uniform_location v_diffuse;

protected:
	void f_call(const t_uniforms& a_uniforms, GLenum a_mode, GLint a_offset, GLsizei a_count)
	{
		const auto& diffuse = a_uniforms.v_diffuse;
		v_diffuse.f_uniform(diffuse.v_x, diffuse.v_y, diffuse.v_z, diffuse.v_w);
		T::f_call(a_uniforms, a_mode, a_offset, a_count);
	}

public:
	using T::T;
	void f_build(GLuint a_vshader, GLuint a_fshader)
	{
		T::f_build(a_vshader, a_fshader);
		v_diffuse = T::v_program.f_get_uniform_location("diffuse");
	}
};

template<typename T>
struct t_with_diffuse_texture : T
{
	struct t_uniforms : T::t_uniforms
	{
		GLuint v_diffuse;
	};

private:
	GLint v_texcoord;
	t_uniform_location v_diffuse;

protected:
	void f_call(const t_uniforms& a_uniforms, GLenum a_mode, GLint a_offset, GLsizei a_count)
	{
		f_enable_vertex_attrib_array(v_texcoord);
		f_vertex_attrib_pointer(v_texcoord, 2, GL_FLOAT, false, a_uniforms.v_stride, a_uniforms.v_texcoords);
		f_active_texture(GL_TEXTURE0);
		f_bind_texture(GL_TEXTURE_2D, a_uniforms.v_diffuse);
		v_diffuse.f_uniform(0);
		T::f_call(a_uniforms, a_mode, a_offset, a_count);
		f_disable_vertex_attrib_array(v_texcoord);
		f_bind_texture(GL_TEXTURE_2D, 0);
	}

public:
	using T::T;
	void f_build(GLuint a_vshader, GLuint a_fshader)
	{
		T::f_build(a_vshader, a_fshader);
		v_texcoord = T::v_program.f_get_attrib_location("texcoord");
		v_diffuse = T::v_program.f_get_uniform_location("diffuse");
	}
};

template<typename T>
struct t_with_specular : T
{
	struct t_uniforms : T::t_uniforms
	{
		t_vector4f v_specular;
		float v_shininess;
	};

private:
	t_uniform_location v_specular;
	t_uniform_location v_shininess;

protected:
	void f_call(const t_uniforms& a_uniforms, GLenum a_mode, GLint a_offset, GLsizei a_count)
	{
		const auto& specular = a_uniforms.v_specular;
		v_specular.f_uniform(specular.v_x, specular.v_y, specular.v_z, specular.v_w);
		v_shininess.f_uniform(a_uniforms.v_shininess);
		T::f_call(a_uniforms, a_mode, a_offset, a_count);
	};

public:
	using T::T;
	void f_build(GLuint a_vshader, GLuint a_fshader)
	{
		T::f_build(a_vshader, a_fshader);
		v_specular = T::v_program.f_get_uniform_location("specular");
		v_shininess = T::v_program.f_get_uniform_location("shininess");
	}
};

template<typename T>
struct t_with_refraction : T
{
	struct t_uniforms : T::t_uniforms
	{
		float v_refraction;
	};

private:
	t_uniform_location v_refraction;

protected:
	void f_call(const t_uniforms& a_uniforms, GLenum a_mode, GLint a_offset, GLsizei a_count)
	{
		v_refraction.f_uniform(a_uniforms.v_refraction);
		T::f_call(a_uniforms, a_mode, a_offset, a_count);
	}

public:
	using T::T;
	void f_build(GLuint a_vshader, GLuint a_fshader)
	{
		T::f_build(a_vshader, a_fshader);
		v_refraction = T::v_program.f_get_uniform_location("refraction");
	}
};

struct t_skin_shader : t_shader_base
{
	struct t_uniforms : t_shader_base::t_uniforms
	{
		const GLfloat* v_projection;
		const GLfloat* v_vertices;
		size_t v_count;
	};

protected:
	t_uniform_location v_projection;
	t_uniform_location v_vertex_matrices;
	GLint v_vertex;
	std::vector<GLint> v_joints;
	std::vector<GLint> v_weights;

	void f_call(const t_uniforms& a_uniforms, GLenum a_mode, GLint a_offset, GLsizei a_count)
	{
		v_projection.f_matrix4(a_uniforms.v_projection);
		v_vertex_matrices.f_matrix4(a_uniforms.v_vertices, a_uniforms.v_count);
		f_enable_vertex_attrib_array(v_vertex);
		f_vertex_attrib_pointer(v_vertex, 3, GL_FLOAT, false, a_uniforms.v_stride, 0);
		size_t n = v_joints.size();
		for (size_t i = 0; i < n; ++i) {
			f_enable_vertex_attrib_array(v_joints[i]);
			f_vertex_attrib_pointer(v_joints[i], 1, GL_UNSIGNED_INT, false, a_uniforms.v_stride, 3 * sizeof(float) + i * sizeof(float));
			f_enable_vertex_attrib_array(v_weights[i]);
			f_vertex_attrib_pointer(v_weights[i], 1, GL_FLOAT, false, a_uniforms.v_stride, 3 * sizeof(float) + n * sizeof(uint32_t) + i * sizeof(float));
		}
		f_draw_arrays(a_mode, a_offset, a_count);
		f_disable_vertex_attrib_array(v_vertex);
		for (size_t i = 0; i < n; ++i) {
			f_disable_vertex_attrib_array(v_joints[i]);
			f_disable_vertex_attrib_array(v_weights[i]);
		}
	}

public:
	t_skin_shader(size_t a_n) : v_joints(a_n), v_weights(a_n)
	{
	}
	void f_build(GLuint a_vshader, GLuint a_fshader)
	{
		t_shader_base::f_build(a_vshader, a_fshader);
		v_projection = v_program.f_get_uniform_location("projection");
		v_vertex_matrices = v_program.f_get_uniform_location("vertexMatrices");
		v_vertex = v_program.f_get_attrib_location("vertex");
		size_t n = v_joints.size();
		for (size_t i = 0; i < n; ++i) {
			v_joints[i] = v_program.f_get_attrib_location(("joint" + std::to_string(i)).c_str());
			v_weights[i] = v_program.f_get_attrib_location(("weight" + std::to_string(i)).c_str());
		}
	}
};

template<typename T>
struct t_with_skin_normal : T
{
	typedef typename T::t_uniforms t_uniforms;

private:
	GLint v_normal;

protected:
	void f_call(const t_uniforms& a_uniforms, GLenum a_mode, GLint a_offset, GLsizei a_count)
	{
		f_enable_vertex_attrib_array(v_normal);
		f_vertex_attrib_pointer(v_normal, 3, GL_FLOAT, false, a_uniforms.v_stride, 3 * sizeof(float) + T::v_joints.size() * (sizeof(uint32_t) + sizeof(float)));
		T::f_call(a_uniforms, a_mode, a_offset, a_count);
		f_disable_vertex_attrib_array(v_normal);
	}

public:
	using T::T;
	void f_build(GLuint a_vshader, GLuint a_fshader)
	{
		T::f_build(a_vshader, a_fshader);
		v_normal = T::v_program.f_get_attrib_location("normal");
	}
};

typedef t_shader_of<t_with_color<t_mesh_shader>> t_color_shader;
typedef t_shader_of<t_with_texture<t_mesh_shader>> t_texture_shader;
typedef t_shader_of<t_with_normal<t_with_color<t_with_diffuse_color<t_mesh_shader>>>> t_diffuse_color_shader;
typedef t_shader_of<t_with_diffuse_texture<t_with_color<t_with_normal<t_mesh_shader>>>> t_diffuse_texture_shader;
typedef t_shader_of<t_with_specular<t_with_diffuse_color<t_with_color<t_with_normal<t_mesh_shader>>>>> t_diffuse_color_specular_shader;
typedef t_shader_of<t_with_specular<t_with_diffuse_texture<t_with_color<t_with_normal<t_mesh_shader>>>>> t_diffuse_texture_specular_shader;
typedef t_shader_of<t_with_refraction<t_with_specular<t_with_diffuse_color<t_with_color<t_with_normal<t_mesh_shader>>>>>> t_diffuse_color_specular_refraction_shader;
typedef t_shader_of<t_with_refraction<t_with_specular<t_with_diffuse_texture<t_with_color<t_with_normal<t_mesh_shader>>>>>> t_diffuse_texture_specular_refraction_shader;

typedef t_shader_of<t_with_color<t_skin_shader>> t_skin_color_shader;
typedef t_shader_of<t_with_diffuse_color<t_with_color<t_with_skin_normal<t_skin_shader>>>> t_skin_diffuse_color_shader;
typedef t_shader_of<t_with_specular<t_with_diffuse_color<t_with_color<t_with_skin_normal<t_skin_shader>>>>> t_skin_diffuse_color_specular_shader;
typedef t_shader_of<t_with_refraction<t_with_specular<t_with_diffuse_color<t_with_color<t_with_skin_normal<t_skin_shader>>>>>> t_skin_diffuse_color_specular_refraction_shader;
typedef t_shader_of<t_with_texture<t_skin_shader>> t_skin_texture_shader;
typedef t_shader_of<t_with_diffuse_texture<t_with_color<t_with_skin_normal<t_skin_shader>>>> t_skin_diffuse_texture_shader;
typedef t_shader_of<t_with_specular<t_with_diffuse_texture<t_with_color<t_with_skin_normal<t_skin_shader>>>>> t_skin_diffuse_texture_specular_shader;
typedef t_shader_of<t_with_refraction<t_with_specular<t_with_diffuse_texture<t_with_color<t_with_skin_normal<t_skin_shader>>>>>> t_skin_diffuse_texture_specular_refraction_shader;

class t_shaders
{
	t_shader v_vertex_shader;
	t_shader v_vertex_shader_texture;
	t_shader v_vertex_shader_normal;
	t_shader v_vertex_shader_normal_texture;
	t_shader v_constant_shader_color;
	t_shader v_constant_shader_texture;
	t_color_shader v_constant_color;
	t_texture_shader v_constant_texture;
	t_shader v_blinn_shader_color;
	t_shader v_blinn_shader_texture;
	t_diffuse_color_specular_refraction_shader v_blinn_color;
	t_diffuse_texture_specular_refraction_shader v_blinn_texture;
	t_shader v_lambert_shader_color;
	t_shader v_lambert_shader_texture;
	t_diffuse_color_shader v_lambert_color;
	t_diffuse_texture_shader v_lambert_texture;
	t_shader v_phong_shader_color;
	t_shader v_phong_shader_texture;
	t_diffuse_color_specular_shader v_phong_color;
	t_diffuse_texture_specular_shader v_phong_texture;
	std::map<std::tuple<size_t, size_t>, t_shader> v_skins_shader_normal;
	std::map<std::tuple<size_t, size_t>, t_shader> v_skins_shader_normal_texture;
	std::map<std::tuple<size_t, size_t>, t_skin_color_shader> v_skin_color_constant_shaders;
	std::map<std::tuple<size_t, size_t>, t_skin_diffuse_color_specular_refraction_shader> v_skin_color_blinn_shaders;
	std::map<std::tuple<size_t, size_t>, t_skin_diffuse_color_shader> v_skin_color_lambert_shaders;
	std::map<std::tuple<size_t, size_t>, t_skin_diffuse_color_specular_shader> v_skin_color_phong_shaders;
	std::map<std::tuple<size_t, size_t>, t_skin_texture_shader> v_skin_texture_constant_shaders;
	std::map<std::tuple<size_t, size_t>, t_skin_diffuse_texture_specular_refraction_shader> v_skin_texture_blinn_shaders;
	std::map<std::tuple<size_t, size_t>, t_skin_diffuse_texture_shader> v_skin_texture_lambert_shaders;
	std::map<std::tuple<size_t, size_t>, t_skin_diffuse_texture_specular_shader> v_skin_texture_phong_shaders;

public:
	void f_vertex_shader(t_shader& a_shader, const std::string& a_defines);
	GLuint f_vertex_shader()
	{
		if (v_vertex_shader == 0) f_vertex_shader(v_vertex_shader, "");
		return v_vertex_shader;
	}
	GLuint f_vertex_shader_texture()
	{
		if (v_vertex_shader_texture == 0) f_vertex_shader(v_vertex_shader_texture, "#define USE_TEXTURE");
		return v_vertex_shader_texture;
	}
	void f_vertex_shader_normal(t_shader& a_shader, const std::string& a_defines);
	GLuint f_vertex_shader_normal()
	{
		if (v_vertex_shader_normal == 0) f_vertex_shader_normal(v_vertex_shader_normal, "");
		return v_vertex_shader_normal;
	}
	GLuint f_vertex_shader_normal_texture()
	{
		if (v_vertex_shader_normal_texture == 0) f_vertex_shader_normal(v_vertex_shader_normal_texture, "#define USE_TEXTURE");
		return v_vertex_shader_normal_texture;
	}
	void f_constant_shader(t_shader& a_shader, const std::string& a_defines);
	GLuint f_constant_shader_color()
	{
		if (v_constant_shader_color == 0) f_constant_shader(v_constant_shader_color, "");
		return v_constant_shader_color;
	}
	GLuint f_constant_shader_texture()
	{
		if (v_constant_shader_texture == 0) f_constant_shader(v_constant_shader_texture, "#define USE_TEXTURE");
		return v_constant_shader_texture;
	}
	auto& f_constant_color()
	{
		if (v_constant_color == 0) v_constant_color.f_build(f_vertex_shader(), f_constant_shader_color());
		return v_constant_color;
	}
	auto& f_constant_texture()
	{
		if (v_constant_texture == 0) v_constant_texture.f_build(f_vertex_shader_texture(), f_constant_shader_texture());
		return v_constant_texture;
	}
	void f_blinn_shader(t_shader& a_shader, const std::string& a_defines);
	GLuint f_blinn_shader_color()
	{
		if (v_blinn_shader_color == 0) f_blinn_shader(v_blinn_shader_color, "");
		return v_blinn_shader_color;
	}
	GLuint f_blinn_shader_texture()
	{
		if (v_blinn_shader_texture == 0) f_blinn_shader(v_blinn_shader_texture, "#define USE_TEXTURE");
		return v_blinn_shader_texture;
	}
	auto& f_blinn_color()
	{
		if (v_blinn_color == 0) v_blinn_color.f_build(f_vertex_shader_normal(), f_blinn_shader_color());
		return v_blinn_color;
	}
	auto& f_blinn_texture()
	{
		if (v_blinn_texture == 0) v_blinn_texture.f_build(f_vertex_shader_normal_texture(), f_blinn_shader_texture());
		return v_blinn_texture;
	}
	void f_lambert_shader(t_shader& a_shader, const std::string& a_defines);
	GLuint f_lambert_shader_color()
	{
		if (v_lambert_shader_color == 0) f_lambert_shader(v_lambert_shader_color, "");
		return v_lambert_shader_color;
	}
	GLuint f_lambert_shader_texture()
	{
		if (v_lambert_shader_texture == 0) f_lambert_shader(v_lambert_shader_texture, "#define USE_TEXTURE");
		return v_lambert_shader_texture;
	}
	auto& f_lambert_color()
	{
		if (v_lambert_color == 0) v_lambert_color.f_build(f_vertex_shader_normal(), f_lambert_shader_color());
		return v_lambert_color;
	}
	auto& f_lambert_texture()
	{
		if (v_lambert_texture == 0) v_lambert_texture.f_build(f_vertex_shader_normal_texture(), f_lambert_shader_texture());
		return v_lambert_texture;
	}
	void f_phong_shader(t_shader& a_shader, const std::string& a_defines);
	GLuint f_phong_shader_color()
	{
		if (v_phong_shader_color == 0) f_phong_shader(v_phong_shader_color, "");
		return v_phong_shader_color;
	}
	GLuint f_phong_shader_texture()
	{
		if (v_phong_shader_texture == 0) f_phong_shader(v_phong_shader_texture, "#define USE_TEXTURE");
		return v_phong_shader_texture;
	}
	auto& f_phong_color()
	{
		if (v_phong_color == 0) v_phong_color.f_build(f_vertex_shader_normal(), f_phong_shader_color());
		return v_phong_color;
	}
	auto& f_phong_texture()
	{
		if (v_phong_texture == 0) v_phong_texture.f_build(f_vertex_shader_normal_texture(), f_phong_shader_texture());
		return v_phong_texture;
	}
	void f_skin_shader(t_shader& a_shader, size_t a_joints, size_t a_weights, const std::string& a_defines);
	GLuint f_skin_shader_of(std::map<std::tuple<size_t, size_t>, t_shader>& a_skins, size_t a_joints, size_t a_weights, const std::string& a_defines)
	{
		auto key = std::make_tuple(a_joints, a_weights);
		auto i = a_skins.lower_bound(key);
		if (i == a_skins.end() || i->first != key) {
			i = a_skins.emplace_hint(i, key, t_shader());
			f_skin_shader(i->second, a_joints, a_weights, a_defines);
		}
		return i->second;
	}
	GLuint f_skin_shader_normal(size_t a_joints, size_t a_weights)
	{
		return f_skin_shader_of(v_skins_shader_normal, a_joints, a_weights, "");
	}
	GLuint f_skin_shader_normal_texture(size_t a_joints, size_t a_weights)
	{
		return f_skin_shader_of(v_skins_shader_normal_texture, a_joints, a_weights, "#define USE_TEXTURE");
	}
	template<GLuint (t_shaders::*A_skin)(size_t, size_t), GLuint (t_shaders::*A_fragment)(), typename T_shaders>
	typename T_shaders::mapped_type& f_skin(T_shaders& a_shaders, size_t a_joints, size_t a_weights)
	{
		auto key = std::make_tuple(a_joints, a_weights);
		auto i = a_shaders.lower_bound(key);
		if (i == a_shaders.end() || i->first != key) {
			i = a_shaders.emplace_hint(i, key, typename T_shaders::mapped_type(a_weights));
			i->second.f_build((this->*A_skin)(a_joints, a_weights), (this->*A_fragment)());
		}
		return i->second;
	};
	auto& f_skin_color_constant(size_t a_joints, size_t a_weights)
	{
		return f_skin<&t_shaders::f_skin_shader_normal, &t_shaders::f_constant_shader_color>(v_skin_color_constant_shaders, a_joints, a_weights);
	};
	auto& f_skin_color_blinn(size_t a_joints, size_t a_weights)
	{
		return f_skin<&t_shaders::f_skin_shader_normal, &t_shaders::f_blinn_shader_color>(v_skin_color_blinn_shaders, a_joints, a_weights);
	};
	auto& f_skin_color_lambert(size_t a_joints, size_t a_weights)
	{
		return f_skin<&t_shaders::f_skin_shader_normal, &t_shaders::f_lambert_shader_color>(v_skin_color_lambert_shaders, a_joints, a_weights);
	};
	auto& f_skin_color_phong(size_t a_joints, size_t a_weights)
	{
		return f_skin<&t_shaders::f_skin_shader_normal, &t_shaders::f_phong_shader_color>(v_skin_color_phong_shaders, a_joints, a_weights);
	};
	auto& f_skin_texture_constant(size_t a_joints, size_t a_weights)
	{
		return f_skin<&t_shaders::f_skin_shader_normal_texture, &t_shaders::f_constant_shader_texture>(v_skin_texture_constant_shaders, a_joints, a_weights);
	};
	auto& f_skin_texture_blinn(size_t a_joints, size_t a_weights)
	{
		return f_skin<&t_shaders::f_skin_shader_normal_texture, &t_shaders::f_blinn_shader_texture>(v_skin_texture_blinn_shaders, a_joints, a_weights);
	};
	auto& f_skin_texture_lambert(size_t a_joints, size_t a_weights)
	{
		return f_skin<&t_shaders::f_skin_shader_normal_texture, &t_shaders::f_lambert_shader_texture>(v_skin_texture_lambert_shaders, a_joints, a_weights);
	};
	auto& f_skin_texture_phong(size_t a_joints, size_t a_weights)
	{
		return f_skin<&t_shaders::f_skin_shader_normal_texture, &t_shaders::f_phong_shader_texture>(v_skin_texture_phong_shaders, a_joints, a_weights);
	};
};

}

#endif
