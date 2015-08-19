#ifndef COLLADA_H
#define COLLADA_H

#include <iostream>
#include <memory>
#include <set>
#include <string>
#include <xanadu/transform>
#include <gl/shaders.h>

#include "xml_reader.h"

using namespace xanadu;

template<typename T>
inline T f_to(const std::wstring& a_x);

template<>
inline float f_to<float>(const std::wstring& a_x)
{
	return std::stof(a_x);
}

template<>
inline int f_to<int>(const std::wstring& a_x)
{
	return std::stoi(a_x);
}

template<>
inline size_t f_to<size_t>(const std::wstring& a_x)
{
	return std::stoul(a_x);
}

template<typename T_delimiter, typename T_callback>
void f_parse_texts(const std::wstring& a_text, T_delimiter a_delimiter, T_callback a_callback)
{
	auto i = a_text.begin();
	while (true) {
		i = std::find_if_not(i, a_text.end(), a_delimiter);
		if (i == a_text.end()) break;
		auto j = std::find_if(i, a_text.end(), a_delimiter);
		a_callback(i, j);
		i = j;
	}
}

inline bool f_is_whitespace(wchar_t a_x)
{
	return a_x <= L' ';
}

template<typename T>
void f_parse_vector(size_t a_count, const std::wstring& a_text, std::vector<T>& a_array)
{
	a_array.reserve(a_count);
	f_parse_texts(a_text, f_is_whitespace, [&](auto a_i, auto a_j)
	{
		a_array.push_back(f_to<T>(std::wstring(a_i, a_j)));
	});
	if (a_array.size() != a_count) throw std::runtime_error("wrong count");
}

struct t_unique
{
	std::wstring v_id;

	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
};

class t_resolve
{
protected:
	std::map<std::wstring, t_unique*> v_ids;

public:
	GLint v_texture_format = GL_RGBA;

	t_unique* operator[](const std::wstring& a_id) const
	{
		return v_ids.at(a_id.substr(1));
	}
};

struct t_with_sid
{
	std::wstring v_sid;

	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
};

struct t_array
{
	virtual ~t_array() = default;
	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const = 0;
};

template<typename T>
struct t_array_of : t_array, t_unique, std::vector<T>
{
	using std::vector<T>::vector;
	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const
	{
		a_out << a_indent << L"array of " << typeid(T).name() << std::endl;
		auto indent = a_indent + L'\t';
		t_unique::f_dump(a_out, indent);
		a_out << indent << L"values:";
		for (const auto& x : *this) a_out << L' ' << x;
		a_out << std::endl;
	}
};

struct t_accessor;

struct t_source : t_unique
{
	std::unique_ptr<t_array> v_array;
	size_t v_count = 0;
	size_t v_offset = 0;
	std::wstring v_source_id;
	size_t v_stride = 0;
	std::vector<size_t> v_params;
	std::wstring v_type;
	t_unique* v_source = nullptr;
	std::unique_ptr<t_accessor> v_accessor;

	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
	void f_build(const t_resolve& a_resolve);
};

struct t_accessor
{
	t_source* v_source;

	t_accessor(t_source* a_source) : v_source(a_source)
	{
	}
	virtual ~t_accessor() = default;
};

template<typename T>
struct t_accessor_of : public t_accessor
{
	const t_array_of<T>& v_array;

	t_accessor_of(t_source* a_source, const t_array_of<T>& a_array) : t_accessor(a_source), v_array(a_array)
	{
	}
	template<typename T_out>
	void f_copy(size_t a_i, T_out a_out) const
	{
		const T* p = &v_array[v_source->v_offset + a_i * v_source->v_stride];
		for (size_t i : v_source->v_params) *a_out++ = p[i];
	}
	T operator[](size_t a_i) const
	{
		const T* p = &v_array[v_source->v_offset + a_i * v_source->v_stride];
		return p[v_source->v_params.empty() ? 0 : v_source->v_params[0]];
	}
};

template<typename T>
struct t_accessor_of<t_matrix<4, T>> : public t_accessor
{
	const t_array_of<T>& v_array;

	t_accessor_of(t_source* a_source, const t_array_of<T>& a_array) : t_accessor(a_source), v_array(a_array)
	{
	}
	t_matrix<4, T> operator[](size_t a_i) const
	{
		const T* p = &v_array[v_source->v_offset + a_i * v_source->v_stride];
		t_matrix<4, T> m;
		std::copy(p, p + 16, m.v_array);
		return m;
	}
};

struct t_asset
{
	double v_unit_meter = 1.0;
	std::wstring v_unit_name = L"meter";
	std::wstring v_up_axis;

	void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
};

struct t_input
{
	size_t v_offset;
	std::wstring v_semantic;
	std::wstring v_source_id;
};

struct t_primitive
{
	GLenum v_mode;
	size_t v_unit;
	size_t v_count;
	std::wstring v_material;
	size_t v_stride;
	std::map<std::tuple<std::wstring, size_t>, t_input> v_inputs;
	std::vector<size_t> v_indices;
	gl::t_buffer v_vertices;
	gl::t_buffer v_normals;
	std::map<std::tuple<std::wstring, size_t>, gl::t_buffer> v_others;

	t_primitive(GLenum a_mode, size_t a_unit) : v_mode(a_mode), v_unit(a_unit)
	{
	}
	t_primitive(const t_primitive& a_other) : v_mode(a_other.v_mode), v_unit(a_other.v_unit), v_count(a_other.v_count), v_material(a_other.v_material), v_stride(a_other.v_stride), v_inputs(a_other.v_inputs), v_indices(a_other.v_indices)
	{
	}
	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
	void f_input_buffer(const t_resolve& a_resolve, const t_input& a_input, t_source& a_source, size_t a_dimension, gl::t_buffer& a_buffer);
	void f_normal_and_others(const t_resolve& a_resolve);
	const gl::t_buffer& f_input(const std::map<std::wstring, std::tuple<std::wstring, size_t>>& a_binds, const std::wstring& a_semantic);
};

struct t_mesh_primitive : t_primitive
{
	t_mesh_primitive(GLenum a_mode, size_t a_unit) : t_primitive(a_mode, a_unit)
	{
	}
	void f_build(const t_resolve& a_resolve);
	void f_create(size_t a_count, const std::wstring& a_material, const std::vector<float>& a_vertices, const std::vector<float>& a_normals, const std::map<std::tuple<std::wstring, size_t>, std::vector<float>>& a_others);
};

struct t_skin_primitive : t_primitive
{
	gl::t_buffer v_joints;
	gl::t_buffer v_weights;

	t_skin_primitive(const t_primitive& a_primitive) : t_primitive(a_primitive)
	{
	}
	void f_vertex_buffer(const t_input& a_input, const std::vector<std::tuple<t_vector3f, std::vector<std::tuple<size_t, float>>>>& a_vertices, size_t a_weights);
	void f_build(const t_resolve& a_resolve, const std::vector<std::tuple<t_vector3f, std::vector<std::tuple<size_t, float>>>>& a_vertices, size_t a_weights)
	{
		f_vertex_buffer(v_inputs.at(std::make_tuple(L"VERTEX", 0)), a_vertices, a_weights);
		f_normal_and_others(a_resolve);
	}
};

struct t_vertices : t_unique
{
	std::map<std::wstring, std::wstring> v_inputs;

	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
};

struct t_geometry : t_unique
{
	virtual ~t_geometry() = default;
};

struct t_mesh : t_geometry
{
	std::vector<std::unique_ptr<t_source>> v_sources;
	std::unique_ptr<t_vertices> v_vertices = std::make_unique<t_vertices>();
	std::vector<std::unique_ptr<t_mesh_primitive>> v_primitives;
	bool v_built = false;

	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
	void f_build(const t_resolve& a_resolve)
	{
		if (v_built) return;
		for (auto& x : v_primitives) x->f_build(a_resolve);
		v_built = true;
	}
};

struct t_common
{
	virtual ~t_common() = default;
	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const = 0;
	virtual void f_build(const t_resolve& a_resolve, const std::map<std::wstring, t_with_sid*>& a_sids);
};

struct t_common_color : t_common, t_vector4f
{
	using t_vector4f::t_vector4;
	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
};

struct t_common_param : t_common
{
	std::wstring v_ref_sid;
	t_with_sid* v_ref = nullptr;

	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
	virtual void f_build(const t_resolve& a_resolve, const std::map<std::wstring, t_with_sid*>& a_sids);
};

struct t_sampler2d;

struct t_common_texture : t_common
{
	std::wstring v_texture_sid;
	std::wstring v_texcoord;
	t_sampler2d* v_texture = nullptr;

	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
	virtual void f_build(const t_resolve& a_resolve, const std::map<std::wstring, t_with_sid*>& a_sids);
};

struct t_common_float : t_common
{
	float v_value;

	t_common_float(float a_value) : v_value(a_value)
	{
	}
	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
};

struct t_image;

struct t_surface : t_with_sid
{
	std::wstring v_type;
	std::wstring v_mip;
	std::wstring v_slice;
	std::wstring v_face;
	std::wstring v_from_id;
	std::wstring v_format;
	t_image* v_from;

	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
	void f_build(const t_resolve& a_resolve, const std::map<std::wstring, t_with_sid*>& a_sids);
};

struct t_sampler2d : t_with_sid
{
	std::wstring v_source_sid;
	std::wstring v_minfilter = L"NONE";
	std::wstring v_magfilter = L"NONE";
	t_surface* v_source = nullptr;
	gl::t_texture v_texture;

	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
	void f_build(const t_resolve& a_resolve, const std::map<std::wstring, t_with_sid*>& a_sids);
};

struct t_shading_model
{
	struct t_mesh_shader
	{
		virtual ~t_mesh_shader() = default;
		virtual void f_setup(t_shading_model* a_model, t_mesh_primitive& a_primitive, const std::map<std::wstring, std::tuple<std::wstring, size_t>>& a_binds) = 0;
		virtual void operator()(const t_matrix4f& a_projection, const t_matrix4f& a_viewing) = 0;
	};
	template<typename T>
	struct t_mesh_shader_of : t_mesh_shader
	{
		T& v_shader;
		typename T::t_uniforms v_uniforms;
		typename T::t_attributes v_attributes;
		GLenum v_mode;
		size_t v_count;

		t_mesh_shader_of(T& a_shader) : v_shader(a_shader)
		{
		}
		virtual void f_setup(t_shading_model* a_model, t_mesh_primitive& a_primitive, const std::map<std::wstring, std::tuple<std::wstring, size_t>>& a_binds);
		virtual void operator()(const t_matrix4f& a_projection, const t_matrix4f& a_viewing);
	};
	struct t_skin_shader
	{
		virtual ~t_skin_shader() = default;
		virtual void f_setup(t_shading_model* a_model, const std::vector<t_matrix4f>& a_vertices, t_skin_primitive& a_primitive, const std::map<std::wstring, std::tuple<std::wstring, size_t>>& a_binds) = 0;
		virtual void operator()(const t_matrix4f& a_projection) = 0;
	};
	template<typename T>
	struct t_skin_shader_of : t_skin_shader
	{
		T& v_shader;
		typename T::t_uniforms v_uniforms;
		typename T::t_attributes v_attributes;
		GLenum v_mode;
		size_t v_count;

		t_skin_shader_of(T& a_shader) : v_shader(a_shader)
		{
		}
		virtual void f_setup(t_shading_model* a_model, const std::vector<t_matrix4f>& a_vertices, t_skin_primitive& a_primitive, const std::map<std::wstring, std::tuple<std::wstring, size_t>>& a_binds);
		virtual void operator()(const t_matrix4f& a_projection);
	};

	std::unique_ptr<t_common> v_emission = std::make_unique<t_common_color>(0.0, 0.0, 0.0, 1.0);
	std::unique_ptr<t_common> v_ambient = std::make_unique<t_common_color>(0.0, 0.0, 0.0, 1.0);
	std::unique_ptr<t_common> v_diffuse = std::make_unique<t_common_color>(0.0, 0.0, 0.0, 1.0);
	std::unique_ptr<t_common> v_specular = std::make_unique<t_common_color>(0.0, 0.0, 0.0, 1.0);
	std::unique_ptr<t_common> v_shininess = std::make_unique<t_common_float>(0.0);
	std::unique_ptr<t_common> v_reflective = std::make_unique<t_common_color>(0.0, 0.0, 0.0, 1.0);
	std::unique_ptr<t_common> v_reflectivity = std::make_unique<t_common_float>(0.0);
	std::unique_ptr<t_common> v_transparent = std::make_unique<t_common_color>(0.0, 0.0, 0.0, 1.0);
	std::unique_ptr<t_common> v_transparency = std::make_unique<t_common_float>(0.0);
	std::unique_ptr<t_common> v_index_of_refraction = std::make_unique<t_common_float>(0.0);

	virtual ~t_shading_model() = default;
	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
	void f_build(const t_resolve& a_resolve, const std::map<std::wstring, t_with_sid*>& a_sids);
	virtual std::unique_ptr<t_mesh_shader> f_mesh_shader(gl::t_shaders& a_shaders) = 0;
	virtual std::unique_ptr<t_skin_shader> f_skin_shader(gl::t_shaders& a_shaders, size_t a_joints, size_t a_weights) = 0;
};

struct t_blinn : t_shading_model
{
	typedef t_mesh_shader_of<gl::t_diffuse_color_specular_refraction_shader> t_mesh_color_shader;
	typedef t_mesh_shader_of<gl::t_diffuse_texture_specular_refraction_shader> t_mesh_texture_shader;
	typedef t_skin_shader_of<gl::t_skin_diffuse_color_specular_refraction_shader> t_skin_color_shader;
	typedef t_skin_shader_of<gl::t_skin_diffuse_texture_specular_refraction_shader> t_skin_texture_shader;

	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
	virtual std::unique_ptr<t_mesh_shader> f_mesh_shader(gl::t_shaders& a_shaders);
	virtual std::unique_ptr<t_skin_shader> f_skin_shader(gl::t_shaders& a_shaders, size_t a_joints, size_t a_weights);
};

struct t_constant : t_shading_model
{
	typedef t_mesh_shader_of<gl::t_color_shader> t_mesh_color_shader;
	typedef t_skin_shader_of<gl::t_skin_color_shader> t_skin_color_shader;

	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
	virtual std::unique_ptr<t_mesh_shader> f_mesh_shader(gl::t_shaders& a_shaders);
	virtual std::unique_ptr<t_skin_shader> f_skin_shader(gl::t_shaders& a_shaders, size_t a_joints, size_t a_weights);
};

struct t_lambert : t_shading_model
{
	typedef t_mesh_shader_of<gl::t_diffuse_color_shader> t_mesh_color_shader;
	typedef t_mesh_shader_of<gl::t_diffuse_texture_shader> t_mesh_texture_shader;
	typedef t_skin_shader_of<gl::t_skin_diffuse_color_shader> t_skin_color_shader;
	typedef t_skin_shader_of<gl::t_skin_diffuse_texture_shader> t_skin_texture_shader;

	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
	virtual std::unique_ptr<t_mesh_shader> f_mesh_shader(gl::t_shaders& a_shaders);
	virtual std::unique_ptr<t_skin_shader> f_skin_shader(gl::t_shaders& a_shaders, size_t a_joints, size_t a_weights);
};

struct t_phong : t_shading_model
{
	typedef t_mesh_shader_of<gl::t_diffuse_color_specular_shader> t_mesh_color_shader;
	typedef t_mesh_shader_of<gl::t_diffuse_texture_specular_shader> t_mesh_texture_shader;
	typedef t_skin_shader_of<gl::t_skin_diffuse_color_specular_shader> t_skin_color_shader;
	typedef t_skin_shader_of<gl::t_skin_diffuse_texture_specular_shader> t_skin_texture_shader;

	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
	virtual std::unique_ptr<t_mesh_shader> f_mesh_shader(gl::t_shaders& a_shaders);
	virtual std::unique_ptr<t_skin_shader> f_skin_shader(gl::t_shaders& a_shaders, size_t a_joints, size_t a_weights);
};

struct t_technique_fx : t_unique, t_with_sid
{
	std::unique_ptr<t_shading_model> v_model;

	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
	void f_build(const t_resolve& a_resolve, const std::map<std::wstring, t_with_sid*>& a_sids)
	{
		v_model->f_build(a_resolve, a_sids);
	}
};

struct t_profile_common : t_unique
{
	std::map<std::wstring, t_with_sid*> v_sids;
	std::vector<std::unique_ptr<t_with_sid>> v_newparams;
	std::unique_ptr<t_technique_fx> v_technique;

	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
	void f_build(const t_resolve& a_resolve)
	{
		//for (const auto& x : v_newparams) x->f_build(a_resolve, v_sids);
		v_technique->f_build(a_resolve, v_sids);
	}
};

struct t_effect : t_unique
{
	std::vector<std::unique_ptr<t_profile_common>> v_profiles;
	bool v_built = false;

	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
	void f_build(const t_resolve& a_resolve)
	{
		if (v_built) return;
		for (const auto& x : v_profiles) x->f_build(a_resolve);
		v_built = true;
	}
};

struct t_image : t_unique
{
	std::wstring v_type;
	std::wstring v_value;
	bool v_built = false;
	size_t v_width;
	size_t v_height;
	std::vector<unsigned char> v_data;

	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
	void f_build(const t_resolve& a_resolve);
};

struct t_material : t_unique
{
	std::wstring v_instance_effect_id;
	t_effect* v_instance_effect = nullptr;

	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
	void f_build(const t_resolve& a_resolve)
	{
		if (v_instance_effect) return;
		v_instance_effect = &dynamic_cast<t_effect&>(*a_resolve[v_instance_effect_id]);
		v_instance_effect->f_build(a_resolve);
	}
	t_shading_model* f_model()
	{
		return v_instance_effect->v_profiles[0]->v_technique->v_model.get();
	}
};

struct t_transform : t_with_sid
{
	virtual ~t_transform() = default;
	virtual void operator()(t_matrix4f& a_x) = 0;
};

struct t_matrix_transform : t_transform, t_matrix4f
{
	t_matrix_transform(const t_matrix4f& a_x = t_matrix4f(1.0)) : t_matrix4f(a_x)
	{
	}
	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
	virtual void operator()(t_matrix4f& a_x);
};

struct t_translate : t_transform, t_translate3f
{
	t_translate(float a_x, float a_y, float a_z) : t_translate3f(a_x, a_y, a_z)
	{
	}
	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
	virtual void operator()(t_matrix4f& a_x);
};

struct t_rotate : t_transform, t_rotate3f
{
	t_rotate(float a_x, float a_y, float a_z, float a_angle) : t_rotate3f(a_x, a_y, a_z, a_angle)
	{
	}
	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
	virtual void operator()(t_matrix4f& a_x);
};

struct t_scale : t_transform, t_scale3f
{
	t_scale(float a_x, float a_y, float a_z) : t_scale3f(a_x, a_y, a_z)
	{
	}
	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
	virtual void operator()(t_matrix4f& a_x);
};

struct t_instance_material
{
	std::wstring v_target_id;
	std::wstring v_symbol;
	std::map<std::wstring, std::tuple<std::wstring, size_t>> v_bind_vertex_inputs;
	t_material* v_target = nullptr;

	virtual ~t_instance_material() = default;
	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
	virtual void f_build(const t_resolve& a_resolve);
	virtual t_shading_model* f_model();
};

struct t_instance_material_fallback : t_instance_material, t_shading_model
{
	typedef t_mesh_shader_of<gl::t_color_shader> t_mesh_color_shader;
	typedef t_skin_shader_of<gl::t_skin_color_shader> t_skin_color_shader;

	t_instance_material_fallback()
	{
		v_emission = std::make_unique<t_common_color>(1.0, 1.0, 1.0, 1.0);
	}
	virtual void f_build(const t_resolve& a_resolve);
	virtual t_shading_model* f_model();
	virtual std::unique_ptr<t_mesh_shader> f_mesh_shader(gl::t_shaders& a_shaders);
	virtual std::unique_ptr<t_skin_shader> f_skin_shader(gl::t_shaders& a_shaders, size_t a_joints, size_t a_weights);
};

struct t_instance_shape
{
	std::wstring v_url;
	std::map<std::wstring, t_instance_material*> v_materials;
	std::vector<std::unique_ptr<t_instance_material>> v_instance_materials;

	t_instance_shape(t_instance_material* a_fallback) : v_materials{{std::wstring(), a_fallback}}
	{
	}
	void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
};

struct t_instance_geometry : t_instance_shape
{
	t_mesh* v_geometry = nullptr;
	std::vector<std::unique_ptr<t_shading_model::t_mesh_shader>> v_shaders;

	using t_instance_shape::t_instance_shape;
	void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
	void f_build_render(const t_resolve& a_resolve, gl::t_shaders& a_shaders);
	void f_build(const t_resolve& a_resolve, gl::t_shaders& a_shaders);
	void f_create(const t_resolve& a_resolve, gl::t_shaders& a_shaders, t_mesh* a_geometry, const std::map<std::wstring, std::wstring>& a_materials);
	void f_render(const t_matrix4f& a_projection, const t_matrix4f& a_viewing)
	{
		for (const auto& x : v_shaders) (*x)(a_projection, a_viewing);
	}
};


struct t_controller : t_unique
{
};

struct t_instance_controller;

struct t_node : t_unique, t_with_sid
{
	bool v_joint = false;
	std::wstring v_layer;
	std::map<std::wstring, t_transform*> v_sids;
	std::vector<std::unique_ptr<t_transform>> v_transforms;
	std::vector<std::unique_ptr<t_instance_controller>> v_controllers;
	std::vector<std::unique_ptr<t_instance_geometry>> v_geometries;
	std::vector<std::wstring> v_instance_node_ids;
	std::vector<std::unique_ptr<t_node>> v_nodes;
	bool v_built = false;
	std::vector<t_node*> v_instance_nodes;
	std::vector<t_instance_controller*> v_referring_controllers;
	std::vector<t_instance_controller*> v_root_controllers;

	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
	void f_build(const t_resolve& a_resolve, gl::t_shaders& a_shaders);
	void f_postbuild(std::set<t_instance_controller*>& a_ancestors, std::vector<t_instance_controller*>& a_controllers);
	void f_render_transformed(const t_matrix4f& a_projection, const t_matrix4f& a_viewing, std::vector<std::map<std::wstring, t_matrix4f*>*>& a_joints);
	void f_render(const t_matrix4f& a_projection, const t_matrix4f& a_viewing, std::vector<std::map<std::wstring, t_matrix4f*>*>& a_joints);
};

struct t_skin : t_controller
{
	std::wstring v_source_id;
	t_matrix4f v_bind_shape_matrix;
	std::vector<std::unique_ptr<t_source>> v_sources;
	std::map<std::wstring, std::wstring> v_joints;
	std::map<std::wstring, std::tuple<size_t, std::wstring>> v_vertex_weights;
	std::vector<size_t> v_vcount;
	std::vector<size_t> v_v;
	t_mesh* v_source = nullptr;
	size_t v_weights_per_vertex = 0;
	std::vector<std::unique_ptr<t_skin_primitive>> v_primitives;

	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
	void f_build(const t_resolve& a_resolve);
};

struct t_instance_controller : t_instance_shape
{
	std::vector<std::wstring> v_skeletons;
	t_skin* v_controller = nullptr;
	std::vector<t_matrix4f> v_ibms;
	std::vector<t_matrix4f> v_joints;
	std::map<std::wstring, t_matrix4f*> v_joint2matrix;
	std::vector<std::unique_ptr<t_shading_model::t_skin_shader>> v_shaders;

	using t_instance_shape::t_instance_shape;
	void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
	void f_build(const t_resolve& a_resolve, gl::t_shaders& a_shaders);
	void f_render(const t_matrix4f& a_projection)
	{
		for (size_t i = 0; i < v_joints.size(); ++i) v_joints[i] *= v_ibms[i];
		for (const auto& x : v_shaders) (*x)(a_projection);
	}
};

struct t_sampler;

struct t_channel
{
	std::wstring v_source_id;
	std::wstring v_target_path;
	t_sampler* v_source = nullptr;
	t_node* v_node = nullptr;
	t_matrix_transform* v_target = nullptr;
	size_t v_ij;

	t_channel(const std::wstring& a_source_id, const std::wstring& a_target_path) : v_source_id(a_source_id), v_target_path(a_target_path)
	{
	}
	void f_build(const t_resolve& a_resolve);
	void operator()(float a_value)
	{
		if (v_target && v_ij < 15) v_target->v_array[v_ij] = a_value;
	}
	void operator()(const t_matrix4f& a_value)
	{
		if (v_target) *v_target = a_value;
	}
};

struct t_animation;

struct t_sampler : t_unique
{
	class t_iterator
	{
		friend struct t_animation;

	protected:
		t_accessor_of<float>& v_input;
		size_t v_count;
		size_t v_i = 0;
		std::vector<t_channel*> v_channels;

	public:
		t_iterator(const t_sampler* a_sampler) : v_input(dynamic_cast<t_accessor_of<float>&>(*a_sampler->v_input->v_accessor)), v_count(a_sampler->v_input->v_count)
		{
		}
		float f_duration() const
		{
			return v_count > 0 ? v_input[v_count - 1] : 0.0;
		}
		virtual void f_forward(float a_t) = 0;
		size_t f_index() const
		{
			return v_i;
		}
		void f_rewind(float a_t, size_t a_i = 0)
		{
			v_i = a_i;
			f_forward(a_t);
		}
	};
	template<typename T>
	class t_iterator_of : public t_iterator
	{
		t_accessor_of<T>& v_output;

		T f_get(float a_t)
		{
			if (v_count <= 0) return T(0.0);
			size_t i = v_i;
			while (i < v_count && v_input[i] <= a_t) ++i;
			if (i <= 0) return v_output[0];
			v_i = i - 1;
			T y0 = v_output[v_i];
			if (i >= v_count) return y0;
			float x0 = v_input[v_i];
			float x1 = v_input[i];
			T y1 = v_output[i];
			float x1x0 = x1 - x0;
			return y0 * ((x1 - a_t) / x1x0) + y1 * ((a_t - x0) / x1x0);
		}

	public:
		t_iterator_of(const t_sampler* a_sampler, t_accessor_of<T>& a_output) : t_iterator(a_sampler), v_output(a_output)
		{
		}
		virtual void f_forward(float a_t)
		{
			T value = f_get(a_t);
			for (auto x : v_channels) (*x)(value);
		}
	};

	std::map<std::wstring, std::wstring> v_inputs;
	t_source* v_input = nullptr;
	t_source* v_output = nullptr;
	t_source* v_interpolation = nullptr;

	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
	void f_build(const t_resolve& a_resolve);
	std::unique_ptr<t_iterator> f_iterator() const
	{
		auto accessor = v_output->v_accessor.get();
		auto output = dynamic_cast<t_accessor_of<t_matrix4f>*>(accessor);
		if (output) return std::make_unique<t_iterator_of<t_matrix4f>>(this, *output);
		return std::make_unique<t_iterator_of<float>>(this, dynamic_cast<t_accessor_of<float>&>(*accessor));
	}
};

struct t_animation : t_unique
{
	std::vector<std::unique_ptr<t_animation>> v_animations;
	std::vector<std::unique_ptr<t_source>> v_sources;
	std::vector<std::unique_ptr<t_sampler>> v_samplers;
	std::vector<t_channel> v_channels;

	t_animation() = default;
	t_animation(const t_animation&) = delete;
	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
	void f_iterators(const t_resolve& a_resolve, std::map<t_sampler*, std::unique_ptr<t_sampler::t_iterator>>& a_iterators, const std::function<bool (const t_channel&)>& a_use);
};

struct t_visual_scene : t_unique
{
	std::vector<std::unique_ptr<t_node>> v_nodes;
	bool v_built = false;
	std::vector<t_node*> v_instance_nodes;
	std::vector<t_instance_controller*> v_controllers;

	virtual void f_dump(std::wostream& a_out, const std::wstring& a_indent) const;
	void f_build(const t_resolve& a_resolve, gl::t_shaders& a_shaders);
	void f_render(const t_matrix4f& a_projection, const t_matrix4f& a_viewing)
	{
		std::vector<std::map<std::wstring, t_matrix4f*>*> joints;
		for (const auto& x : v_nodes) x->f_render(a_projection, a_viewing, joints);
		for (auto x : v_instance_nodes) x->f_render(a_projection, a_viewing, joints);
		for (auto x : v_controllers) x->f_render(a_projection);
	}
};

struct t_instance_visual_scene
{
	std::wstring v_url;
	t_visual_scene* v_scene;

	void f_build(const t_resolve& a_resolve, gl::t_shaders& a_shaders)
	{
		v_scene = &dynamic_cast<t_visual_scene&>(*a_resolve[v_url]);
		v_scene->f_build(a_resolve, a_shaders);
	}
	void f_render(const t_matrix4f& a_projection, const t_matrix4f& a_viewing)
	{
		v_scene->f_render(a_projection, a_viewing);
	}
};

struct t_scene
{
	t_instance_visual_scene v_instance_visual_scene;

	void f_build(const t_resolve& a_resolve, gl::t_shaders& a_shaders)
	{
		v_instance_visual_scene.f_build(a_resolve, a_shaders);
	}
	void f_render(const t_matrix4f& a_projection, const t_matrix4f& a_viewing)
	{
		v_instance_visual_scene.f_render(a_projection, a_viewing);
	}
};

struct t_document : t_resolve
{
	t_instance_material_fallback v_instance_material_fallback;
	std::wstring v_version;
	t_asset v_asset;
	std::vector<std::unique_ptr<t_animation>> v_library_animations;
	std::vector<std::unique_ptr<t_controller>> v_library_controllers;
	std::vector<std::unique_ptr<t_effect>> v_library_effects;
	std::vector<std::unique_ptr<t_geometry>> v_library_geometries;
	std::vector<std::unique_ptr<t_image>> v_library_images;
	std::vector<std::unique_ptr<t_material>> v_library_materials;
	std::vector<std::unique_ptr<t_node>> v_library_nodes;
	std::vector<std::unique_ptr<t_visual_scene>> v_library_visual_scenes;
	t_scene v_scene;

	void f_load(t_reader& a_reader, const std::wstring& a_base);
	void f_dump(std::wostream& a_out) const;
	void f_build(gl::t_shaders& a_shaders)
	{
		v_scene.f_build(*this, a_shaders);
	}
	void f_render(const t_matrix4f& a_projection, const t_matrix4f& a_viewing)
	{
		v_scene.f_render(a_projection, a_viewing);
	}
	std::map<t_sampler*, std::unique_ptr<t_sampler::t_iterator>> f_iterators(const std::function<bool (const t_channel&)>& a_use = [](auto a_x) { return true; })
	{
		std::map<t_sampler*, std::unique_ptr<t_sampler::t_iterator>> iterators;
		for (const auto& x : v_library_animations) x->f_iterators(*this, iterators, a_use);
		return iterators;
	}
};

#endif
