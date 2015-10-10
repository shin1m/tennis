#include "collada.h"

#include <gl/image.h>

#include "portable.h"
#include "xml_reader.h"

void t_unique::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"id: " << v_id << std::endl;
}

void t_with_sid::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"sid: " << v_sid << std::endl;
}

void t_source::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"source" << std::endl;
	auto indent = a_indent + L'\t';
	t_unique::f_dump(a_out, indent);
	if (v_array) v_array->f_dump(a_out, indent);
	a_out << a_indent << L"\tcount: " << v_count << std::endl;
	a_out << a_indent << L"\toffset: " << v_offset << std::endl;
	a_out << a_indent << L"\tsource: " << v_source_id << std::endl;
	a_out << a_indent << L"\tstride: " << v_stride << std::endl;
	a_out << a_indent << L"\tparams:";
	for (auto x : v_params) a_out << L' ' << x;
	a_out << std::endl << a_indent << L"\ttype: " << v_type << std::endl;
}

void t_source::f_build(const t_resolve& a_resolve)
{
	if (v_source) return;
	v_source = a_resolve[v_source_id];
	auto type = v_type;
	std::transform(type.begin(), type.end(), type.begin(), std::towlower);
	if (type == L"float")
		v_accessor.reset(new t_accessor_of<float>(this, dynamic_cast<t_array_of<float>&>(*v_source)));
	else if (type == L"int")
		v_accessor.reset(new t_accessor_of<int>(this, dynamic_cast<t_array_of<int>&>(*v_source)));
	else if (type == L"float4x4")
		v_accessor.reset(new t_accessor_of<t_matrix4f>(this, dynamic_cast<t_array_of<float>&>(*v_source)));
	else if (type == L"name")
		v_accessor.reset(new t_accessor_of<std::wstring>(this, dynamic_cast<t_array_of<std::wstring>&>(*v_source)));
	else
		throw std::runtime_error("unknown type: " + f_convert(v_type));
}

void t_asset::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"asset" << std::endl;
	a_out << a_indent << L"\tunit meter: " << v_unit_meter << std::endl;
	a_out << a_indent << L"\tunit name: " << v_unit_name << std::endl;
	a_out << a_indent << L"\tup axis: " << v_up_axis << std::endl;
}

void t_primitive::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"primitive" << std::endl;
	a_out << a_indent << L"\tmode: " << v_mode << std::endl;
	a_out << a_indent << L"\tunit: " << v_unit << std::endl;
	a_out << a_indent << L"\tcount: " << v_count << std::endl;
	a_out << a_indent << L"\tmaterial: " << v_material << std::endl;
	a_out << a_indent << L"\tstride: " << v_stride << std::endl;
	a_out << a_indent << L"\tinputs" << std::endl;
	for (const auto& x : v_inputs) a_out << a_indent << L"\t\t(" << std::get<0>(x.first) << L", " << std::get<1>(x.first) << L"): " << x.second.v_offset << L", " << x.second.v_semantic << L", " << x.second.v_source_id << std::endl;
	a_out << a_indent << L"\tindices: " << std::endl;
	for (auto x : v_indices) a_out << L' ' << x;
	a_out << std::endl;
}

size_t t_primitive::f_estimate_normal_and_others(const t_resolve& a_resolve, size_t a_offset)
{
	auto i = v_inputs.find(std::make_tuple(L"NORMAL", 0));
	if (i != v_inputs.end()) {
		auto& source = dynamic_cast<t_source&>(*a_resolve[i->second.v_source_id]);
		source.f_build(a_resolve);
		a_offset += 3 * sizeof(float);
	}
	for (const auto& x : v_inputs) {
		if (std::get<0>(x.first) == L"VERTEX" || std::get<0>(x.first) == L"NORMAL") continue;
		v_others.emplace(x.first, a_offset);
		auto& source = dynamic_cast<t_source&>(*a_resolve[x.second.v_source_id]);
		source.f_build(a_resolve);
		a_offset += (x.second.v_semantic == L"TEXCOORD" ? 2 : source.v_params.size()) * sizeof(float);
	}
	return a_offset;
}

void t_primitive::f_input_buffer(const t_input& a_input, t_source& a_source, size_t a_dimension, char* a_bytes, size_t a_stride, size_t a_offset)
{
	auto& accessor = dynamic_cast<t_accessor_of<float>&>(*a_source.v_accessor);
	size_t n = v_count * v_unit;
	assert(a_source.v_params.size() >= a_dimension);
	std::vector<float> params(a_source.v_params.size());
	size_t i = a_input.v_offset;
	a_bytes += a_offset;
	for (size_t k = 0; k < n; ++k) {
		accessor.f_copy(v_indices[i], params.begin());
		std::copy(params.begin(), params.begin() + a_dimension, reinterpret_cast<float*>(a_bytes));
		i += v_stride;
		a_bytes += a_stride;
	}
}

void t_primitive::f_normal_and_others(const t_resolve& a_resolve, char* a_bytes, size_t a_stride, size_t a_offset)
{
	auto i = v_inputs.find(std::make_tuple(L"NORMAL", 0));
	if (i != v_inputs.end()) {
		auto& source = dynamic_cast<t_source&>(*a_resolve[i->second.v_source_id]);
		f_input_buffer(i->second, source, 3, a_bytes, a_stride, a_offset);
	}
	for (const auto& x : v_inputs) {
		if (std::get<0>(x.first) == L"VERTEX" || std::get<0>(x.first) == L"NORMAL") continue;
		auto& source = dynamic_cast<t_source&>(*a_resolve[x.second.v_source_id]);
		f_input_buffer(x.second, source, x.second.v_semantic == L"TEXCOORD" ? 2 : source.v_params.size(), a_bytes, a_stride, v_others.at(x.first));
	}
}

size_t t_primitive::f_input(const std::map<std::wstring, std::tuple<std::wstring, size_t>>& a_binds, const std::wstring& a_semantic) const
{
	const auto& key = a_binds.at(a_semantic);
	auto i = v_others.find(key);
	if (i == v_others.end()) {
		i = v_others.find(std::make_tuple(std::get<0>(key), 0));
		if (i == v_others.end()) throw std::runtime_error("cannot find input");
	}
	return i->second;
}

void t_mesh_primitive::f_build(const t_resolve& a_resolve)
{
	const auto& vertices = v_inputs.at(std::make_tuple(L"VERTEX", 0));
	auto& source = dynamic_cast<t_source&>(*a_resolve[dynamic_cast<t_vertices&>(*a_resolve[vertices.v_source_id]).v_inputs.at(L"POSITION")]);
	source.f_build(a_resolve);
	v_vertices_stride = f_estimate_normal_and_others(a_resolve, 3 * sizeof(float));
	std::vector<char> bytes(v_count * v_unit * v_vertices_stride);
	f_input_buffer(vertices, source, 3, bytes.data(), v_vertices_stride, 0);
	f_normal_and_others(a_resolve, bytes.data(), v_vertices_stride, 3 * sizeof(float));
	v_vertices.f_create();
	gl::f_bind_buffer(GL_ARRAY_BUFFER, v_vertices);
	gl::f_buffer_data(GL_ARRAY_BUFFER, bytes.size(), bytes.data(), GL_STATIC_DRAW);
	gl::f_bind_buffer(GL_ARRAY_BUFFER, 0);
}

void t_mesh_primitive::f_create(size_t a_count, const std::wstring& a_material, const std::vector<char>& a_vertices, size_t a_stride, const std::map<std::tuple<std::wstring, size_t>, size_t>& a_others)
{
	v_count = a_count;
	v_material = a_material;
	v_vertices.f_create();
	gl::f_bind_buffer(GL_ARRAY_BUFFER, v_vertices);
	gl::f_buffer_data(GL_ARRAY_BUFFER, a_vertices.size(), a_vertices.data(), GL_STATIC_DRAW);
	gl::f_bind_buffer(GL_ARRAY_BUFFER, 0);
	v_vertices_stride = a_stride;
	v_others = a_others;
}

void t_skin_primitive::f_build(const t_resolve& a_resolve, const std::vector<std::tuple<t_vector3f, std::vector<std::tuple<size_t, float>>>>& a_vertices, size_t a_weights)
{
	size_t normals = sizeof(t_vector3f) + a_weights * sizeof(uint32_t) + a_weights * sizeof(float);
	v_vertices_stride = f_estimate_normal_and_others(a_resolve, normals);
	size_t n = v_count * v_unit;
	std::vector<char> bytes(n * v_vertices_stride);
	auto p = bytes.data();
	size_t i = v_inputs.at(std::make_tuple(L"VERTEX", 0)).v_offset;
	for (size_t j = 0; j < n; ++j) {
		const auto& x = a_vertices[v_indices[i]];
		*reinterpret_cast<t_vector3f*>(p) = std::get<0>(x);
		const auto& bones = std::get<1>(x);
		auto jarray = reinterpret_cast<uint32_t*>(p + sizeof(t_vector3f));
		auto warray = reinterpret_cast<float*>(p + sizeof(t_vector3f) + a_weights * sizeof(uint32_t));
		size_t k = 0;
		for (; k < bones.size(); ++k) {
			jarray[k] = std::get<0>(bones[k]) + 1;
			warray[k] = std::get<1>(bones[k]);
		}
		if (k < 1) {
			jarray[k] = 0;
			warray[k] = 1.0f;
			k = 1;
		}
		for (; k < a_weights; ++k) {
			jarray[k] = 0;
			warray[k] = 0.0f;
		}
		i += v_stride;
		p += v_vertices_stride;
	}
	f_normal_and_others(a_resolve, bytes.data(), v_vertices_stride, normals);
	v_vertices.f_create();
	gl::f_bind_buffer(GL_ARRAY_BUFFER, v_vertices);
	gl::f_buffer_data(GL_ARRAY_BUFFER, bytes.size(), bytes.data(), GL_STATIC_DRAW);
	gl::f_bind_buffer(GL_ARRAY_BUFFER, 0);
}

void t_vertices::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"vertices" << std::endl;
	auto indent = a_indent + L'\t';
	for (const auto& x : v_inputs) a_out << indent << x.first << L": " << x.second << std::endl;
}

void t_mesh::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"mesh" << std::endl;
	a_out << a_indent << L"\tsources" << std::endl;
	auto indent = a_indent + L"\t\t";
	for (const auto& x : v_sources) x->f_dump(a_out, indent);
	v_vertices->f_dump(a_out, a_indent + L'\t');
	a_out << a_indent << L"\tprimitives" << std::endl;
	for (const auto& x : v_primitives) x->f_dump(a_out, indent);
}

void t_common::f_build(const t_resolve& a_resolve, const std::map<std::wstring, t_with_sid*>& a_sids)
{
}

void t_common_color::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"common color: " << v_x << L' ' << v_y << L' ' << v_z << L' ' << v_w << std::endl;
}

void t_common_param::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"common param: " << v_ref_sid << std::endl;
}

void t_common_param::f_build(const t_resolve& a_resolve, const std::map<std::wstring, t_with_sid*>& a_sids)
{
	v_ref = a_sids.at(v_ref_sid);
}

void t_common_texture::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"common texture" << std::endl;
	a_out << a_indent << L"\ttexture: " << v_texture_sid << std::endl;
	a_out << a_indent << L"\ttexcoord: " << v_texcoord << std::endl;
}

void t_common_texture::f_build(const t_resolve& a_resolve, const std::map<std::wstring, t_with_sid*>& a_sids)
{
	v_texture = &dynamic_cast<t_sampler2d&>(*a_sids.at(v_texture_sid));
	v_texture->f_build(a_resolve, a_sids);
}

void t_common_float::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"common float: " << v_value << std::endl;
}

void t_surface::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"surface" << std::endl;
	t_with_sid::f_dump(a_out, a_indent + L'\t');
	a_out << a_indent << L"\ttype: " << v_type << std::endl;
	a_out << a_indent << L"\tmip: " << v_mip << std::endl;
	a_out << a_indent << L"\tslice: " << v_slice << std::endl;
	a_out << a_indent << L"\tface: " << v_face << std::endl;
	a_out << a_indent << L"\tfrom: " << v_from_id << std::endl;
	a_out << a_indent << L"\tformat: " << v_format << std::endl;
}

void t_surface::f_build(const t_resolve& a_resolve, const std::map<std::wstring, t_with_sid*>& a_sids)
{
	v_from = &dynamic_cast<t_image&>(*a_resolve[L'#' + v_from_id]);
	v_from->f_build(a_resolve);
}

void t_sampler2d::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"sampler 2D" << std::endl;
	t_with_sid::f_dump(a_out, a_indent + L'\t');
	a_out << a_indent << L"\tsource: " << v_source_sid << std::endl;
	a_out << a_indent << L"\tminfilter: " << v_minfilter << std::endl;
	a_out << a_indent << L"\tmagfilter: " << v_magfilter << std::endl;
}

GLint f_filter(const std::wstring& a_name)
{
	if (a_name == L"NEAREST") return GL_NEAREST;
	if (a_name == L"LINEAR") return GL_LINEAR;
	if (a_name == L"NEAREST_MIPMAP_NEAREST") return GL_NEAREST_MIPMAP_NEAREST;
	if (a_name == L"LINEAR_MIPMAP_NEAREST") return GL_LINEAR_MIPMAP_NEAREST;
	if (a_name == L"NEAREST_MIPMAP_LINEAR") return GL_NEAREST_MIPMAP_LINEAR;
	if (a_name == L"LINEAR_MIPMAP_LINEAR") return GL_LINEAR_MIPMAP_LINEAR;
	return GL_NONE;
}

void t_sampler2d::f_build(const t_resolve& a_resolve, const std::map<std::wstring, t_with_sid*>& a_sids)
{
	if (v_source) return;
	v_source = &dynamic_cast<t_surface&>(*a_sids.at(v_source_sid));
	v_source->f_build(a_resolve, a_sids);
	t_image* from = v_source->v_from;
	v_texture.f_create();
	gl::f_active_texture(GL_TEXTURE0);
	gl::f_bind_texture(GL_TEXTURE_2D, v_texture);
	gl::f_tex_image2d(GL_TEXTURE_2D, 0, a_resolve.v_texture_format, from->v_width, from->v_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, from->v_data.data());
	GLint filter = f_filter(v_minfilter);
	if (filter == GL_NONE) filter = GL_NEAREST_MIPMAP_LINEAR;
	switch (filter) {
		case GL_NEAREST_MIPMAP_NEAREST:
		case GL_NEAREST_MIPMAP_LINEAR:
		case GL_LINEAR_MIPMAP_NEAREST:
		case GL_LINEAR_MIPMAP_LINEAR:
			gl::f_generate_mipmap(GL_TEXTURE_2D);
			break;
	}
	gl::f_tex_parameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	filter = f_filter(v_magfilter);
	if (filter == GL_NONE) filter = GL_LINEAR;
	gl::f_tex_parameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	gl::f_bind_texture(GL_TEXTURE_2D, 0);
}

void t_shading_model::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	v_emission->f_dump(a_out, a_indent);
	v_ambient->f_dump(a_out, a_indent);
	v_diffuse->f_dump(a_out, a_indent);
	v_specular->f_dump(a_out, a_indent);
	v_shininess->f_dump(a_out, a_indent);
	v_reflective->f_dump(a_out, a_indent);
	v_reflectivity->f_dump(a_out, a_indent);
	v_transparent->f_dump(a_out, a_indent);
	v_transparency->f_dump(a_out, a_indent);
	v_index_of_refraction->f_dump(a_out, a_indent);
}

void t_shading_model::f_build(const t_resolve& a_resolve, const std::map<std::wstring, t_with_sid*>& a_sids)
{
	v_emission->f_build(a_resolve, a_sids);
	v_ambient->f_build(a_resolve, a_sids);
	v_diffuse->f_build(a_resolve, a_sids);
	v_specular->f_build(a_resolve, a_sids);
	v_shininess->f_build(a_resolve, a_sids);
	v_reflective->f_build(a_resolve, a_sids);
	v_reflectivity->f_build(a_resolve, a_sids);
	v_transparent->f_build(a_resolve, a_sids);
	v_transparency->f_build(a_resolve, a_sids);
	v_index_of_refraction->f_build(a_resolve, a_sids);
}

template<>
void t_shading_model::t_mesh_shader_of<gl::t_diffuse_color_specular_refraction_shader>::f_setup(t_shading_model* a_model, t_mesh_primitive& a_primitive, const std::map<std::wstring, std::tuple<std::wstring, size_t>>& a_binds)
{
	v_uniforms.v_stride = a_primitive.v_vertices_stride;
	v_uniforms.v_color = dynamic_cast<t_common_color&>(*a_model->v_emission) + dynamic_cast<t_common_color&>(*a_model->v_ambient) * 0.125f;
	v_uniforms.v_diffuse = dynamic_cast<t_common_color&>(*a_model->v_diffuse);
	v_uniforms.v_specular = dynamic_cast<t_common_color&>(*a_model->v_specular);
	v_uniforms.v_shininess = dynamic_cast<t_common_float&>(*a_model->v_shininess).v_value;
	v_uniforms.v_refraction = dynamic_cast<t_common_float&>(*a_model->v_index_of_refraction).v_value;
	v_attributes = a_primitive.v_vertices;
	v_mode = a_primitive.v_mode;
	v_count = a_primitive.v_count * a_primitive.v_unit;
}

template<>
void t_shading_model::t_mesh_shader_of<gl::t_diffuse_color_specular_refraction_shader>::operator()(const t_matrix4f& a_projection, const t_matrix4f& a_viewing)
{
	v_uniforms.v_projection = a_projection.v_array;
	v_uniforms.v_vertex = a_viewing.v_array;
	auto normal = ~t_matrix3f(a_viewing);
	v_uniforms.v_normal = normal.v_array;
	v_shader(v_uniforms, v_attributes, v_mode, 0, v_count);
}

template<>
void t_shading_model::t_mesh_shader_of<gl::t_diffuse_texture_specular_refraction_shader>::f_setup(t_shading_model* a_model, t_mesh_primitive& a_primitive, const std::map<std::wstring, std::tuple<std::wstring, size_t>>& a_binds)
{
	v_uniforms.v_stride = a_primitive.v_vertices_stride;
	v_uniforms.v_texcoords = a_primitive.f_input(a_binds, static_cast<t_common_texture&>(*a_model->v_diffuse).v_texcoord);
	v_uniforms.v_color = dynamic_cast<t_common_color&>(*a_model->v_emission) + dynamic_cast<t_common_color&>(*a_model->v_ambient) * 0.125f;
	v_uniforms.v_diffuse = dynamic_cast<t_common_texture&>(*a_model->v_diffuse).v_texture->v_texture;
	v_uniforms.v_specular = dynamic_cast<t_common_color&>(*a_model->v_specular);
	v_uniforms.v_shininess = dynamic_cast<t_common_float&>(*a_model->v_shininess).v_value;
	v_uniforms.v_refraction = dynamic_cast<t_common_float&>(*a_model->v_index_of_refraction).v_value;
	v_attributes = a_primitive.v_vertices;
	v_mode = a_primitive.v_mode;
	v_count = a_primitive.v_count * a_primitive.v_unit;
}

template<>
void t_shading_model::t_mesh_shader_of<gl::t_diffuse_texture_specular_refraction_shader>::operator()(const t_matrix4f& a_projection, const t_matrix4f& a_viewing)
{
	v_uniforms.v_projection = a_projection.v_array;
	v_uniforms.v_vertex = a_viewing.v_array;
	auto normal = ~t_matrix3f(a_viewing);
	v_uniforms.v_normal = normal.v_array;
	v_shader(v_uniforms, v_attributes, v_mode, 0, v_count);
}

template<>
void t_shading_model::t_mesh_shader_of<gl::t_color_shader>::f_setup(t_shading_model* a_model, t_mesh_primitive& a_primitive, const std::map<std::wstring, std::tuple<std::wstring, size_t>>& a_binds)
{
	v_uniforms.v_stride = a_primitive.v_vertices_stride;
	v_uniforms.v_color = dynamic_cast<t_common_color&>(*a_model->v_emission) + dynamic_cast<t_common_color&>(*a_model->v_ambient) * 0.125f;
	v_attributes = a_primitive.v_vertices;
	v_mode = a_primitive.v_mode;
	v_count = a_primitive.v_count * a_primitive.v_unit;
}

template<>
void t_shading_model::t_mesh_shader_of<gl::t_color_shader>::operator()(const t_matrix4f& a_projection, const t_matrix4f& a_viewing)
{
	v_uniforms.v_projection = a_projection.v_array;
	v_uniforms.v_vertex = a_viewing.v_array;
	v_shader(v_uniforms, v_attributes, v_mode, 0, v_count);
}

template<>
void t_shading_model::t_mesh_shader_of<gl::t_texture_shader>::f_setup(t_shading_model* a_model, t_mesh_primitive& a_primitive, const std::map<std::wstring, std::tuple<std::wstring, size_t>>& a_binds)
{
	v_uniforms.v_stride = a_primitive.v_vertices_stride;
	v_uniforms.v_texcoords = a_primitive.f_input(a_binds, static_cast<t_common_texture&>(*a_model->v_emission).v_texcoord);
	v_uniforms.v_color = dynamic_cast<t_common_texture&>(*a_model->v_emission).v_texture->v_texture;
	v_attributes = a_primitive.v_vertices;
	v_mode = a_primitive.v_mode;
	v_count = a_primitive.v_count * a_primitive.v_unit;
}

template<>
void t_shading_model::t_mesh_shader_of<gl::t_texture_shader>::operator()(const t_matrix4f& a_projection, const t_matrix4f& a_viewing)
{
	v_uniforms.v_projection = a_projection.v_array;
	v_uniforms.v_vertex = a_viewing.v_array;
	v_shader(v_uniforms, v_attributes, v_mode, 0, v_count);
}

template<>
void t_shading_model::t_mesh_shader_of<gl::t_diffuse_color_shader>::f_setup(t_shading_model* a_model, t_mesh_primitive& a_primitive, const std::map<std::wstring, std::tuple<std::wstring, size_t>>& a_binds)
{
	v_uniforms.v_stride = a_primitive.v_vertices_stride;
	v_uniforms.v_color = dynamic_cast<t_common_color&>(*a_model->v_emission) + dynamic_cast<t_common_color&>(*a_model->v_ambient) * 0.125f;
	v_uniforms.v_diffuse = dynamic_cast<t_common_color&>(*a_model->v_diffuse);
	v_attributes = a_primitive.v_vertices;
	v_mode = a_primitive.v_mode;
	v_count = a_primitive.v_count * a_primitive.v_unit;
}

template<>
void t_shading_model::t_mesh_shader_of<gl::t_diffuse_color_shader>::operator()(const t_matrix4f& a_projection, const t_matrix4f& a_viewing)
{
	v_uniforms.v_projection = a_projection.v_array;
	v_uniforms.v_vertex = a_viewing.v_array;
	auto normal = ~t_matrix3f(a_viewing);
	v_uniforms.v_normal = normal.v_array;
	v_shader(v_uniforms, v_attributes, v_mode, 0, v_count);
}

template<>
void t_shading_model::t_mesh_shader_of<gl::t_diffuse_texture_shader>::f_setup(t_shading_model* a_model, t_mesh_primitive& a_primitive, const std::map<std::wstring, std::tuple<std::wstring, size_t>>& a_binds)
{
	v_uniforms.v_stride = a_primitive.v_vertices_stride;
	v_uniforms.v_texcoords = a_primitive.f_input(a_binds, static_cast<t_common_texture&>(*a_model->v_diffuse).v_texcoord);
	v_uniforms.v_color = dynamic_cast<t_common_color&>(*a_model->v_emission) + dynamic_cast<t_common_color&>(*a_model->v_ambient) * 0.125f;
	v_uniforms.v_diffuse = dynamic_cast<t_common_texture&>(*a_model->v_diffuse).v_texture->v_texture;
	v_attributes = a_primitive.v_vertices;
	v_mode = a_primitive.v_mode;
	v_count = a_primitive.v_count * a_primitive.v_unit;
}

template<>
void t_shading_model::t_mesh_shader_of<gl::t_diffuse_texture_shader>::operator()(const t_matrix4f& a_projection, const t_matrix4f& a_viewing)
{
	v_uniforms.v_projection = a_projection.v_array;
	v_uniforms.v_vertex = a_viewing.v_array;
	auto normal = ~t_matrix3f(a_viewing);
	v_uniforms.v_normal = normal.v_array;
	v_shader(v_uniforms, v_attributes, v_mode, 0, v_count);
}

template<>
void t_shading_model::t_mesh_shader_of<gl::t_diffuse_color_specular_shader>::f_setup(t_shading_model* a_model, t_mesh_primitive& a_primitive, const std::map<std::wstring, std::tuple<std::wstring, size_t>>& a_binds)
{
	v_uniforms.v_stride = a_primitive.v_vertices_stride;
	v_uniforms.v_color = dynamic_cast<t_common_color&>(*a_model->v_emission) + dynamic_cast<t_common_color&>(*a_model->v_ambient) * 0.125f;
	v_uniforms.v_diffuse = dynamic_cast<t_common_color&>(*a_model->v_diffuse);
	v_uniforms.v_specular = dynamic_cast<t_common_color&>(*a_model->v_specular);
	v_uniforms.v_shininess = dynamic_cast<t_common_float&>(*a_model->v_shininess).v_value;
	v_attributes = a_primitive.v_vertices;
	v_mode = a_primitive.v_mode;
	v_count = a_primitive.v_count * a_primitive.v_unit;
}

template<>
void t_shading_model::t_mesh_shader_of<gl::t_diffuse_color_specular_shader>::operator()(const t_matrix4f& a_projection, const t_matrix4f& a_viewing)
{
	v_uniforms.v_projection = a_projection.v_array;
	v_uniforms.v_vertex = a_viewing.v_array;
	auto normal = ~t_matrix3f(a_viewing);
	v_uniforms.v_normal = normal.v_array;
	v_shader(v_uniforms, v_attributes, v_mode, 0, v_count);
}

template<>
void t_shading_model::t_mesh_shader_of<gl::t_diffuse_texture_specular_shader>::f_setup(t_shading_model* a_model, t_mesh_primitive& a_primitive, const std::map<std::wstring, std::tuple<std::wstring, size_t>>& a_binds)
{
	v_uniforms.v_stride = a_primitive.v_vertices_stride;
	v_uniforms.v_texcoords = a_primitive.f_input(a_binds, static_cast<t_common_texture&>(*a_model->v_diffuse).v_texcoord);
	v_uniforms.v_color = dynamic_cast<t_common_color&>(*a_model->v_emission) + dynamic_cast<t_common_color&>(*a_model->v_ambient) * 0.125f;
	v_uniforms.v_diffuse = dynamic_cast<t_common_texture&>(*a_model->v_diffuse).v_texture->v_texture;
	v_uniforms.v_specular = dynamic_cast<t_common_color&>(*a_model->v_specular);
	v_uniforms.v_shininess = dynamic_cast<t_common_float&>(*a_model->v_shininess).v_value;
	v_attributes = a_primitive.v_vertices;
	v_mode = a_primitive.v_mode;
	v_count = a_primitive.v_count * a_primitive.v_unit;
}

template<>
void t_shading_model::t_mesh_shader_of<gl::t_diffuse_texture_specular_shader>::operator()(const t_matrix4f& a_projection, const t_matrix4f& a_viewing)
{
	v_uniforms.v_projection = a_projection.v_array;
	v_uniforms.v_vertex = a_viewing.v_array;
	auto normal = ~t_matrix3f(a_viewing);
	v_uniforms.v_normal = normal.v_array;
	v_shader(v_uniforms, v_attributes, v_mode, 0, v_count);
}

template<>
void t_shading_model::t_skin_shader_of<gl::t_skin_diffuse_color_specular_refraction_shader>::f_setup(t_shading_model* a_model, const std::vector<t_matrix4f>& a_vertices, t_skin_primitive& a_primitive, const std::map<std::wstring, std::tuple<std::wstring, size_t>>& a_binds)
{
	v_uniforms.v_stride = a_primitive.v_vertices_stride;
	v_uniforms.v_color = dynamic_cast<t_common_color&>(*a_model->v_emission) + dynamic_cast<t_common_color&>(*a_model->v_ambient) * 0.125f;
	v_uniforms.v_diffuse = dynamic_cast<t_common_color&>(*a_model->v_diffuse);
	v_uniforms.v_specular = dynamic_cast<t_common_color&>(*a_model->v_specular);
	v_uniforms.v_shininess = dynamic_cast<t_common_float&>(*a_model->v_shininess).v_value;
	v_uniforms.v_refraction = dynamic_cast<t_common_float&>(*a_model->v_index_of_refraction).v_value;
	v_uniforms.v_vertices = a_vertices.data()->v_array;
	v_uniforms.v_count = a_vertices.size();
	v_attributes = a_primitive.v_vertices;
	v_mode = a_primitive.v_mode;
	v_count = a_primitive.v_count * a_primitive.v_unit;
}

template<>
void t_shading_model::t_skin_shader_of<gl::t_skin_diffuse_color_specular_refraction_shader>::operator()(const t_matrix4f& a_projection)
{
	v_uniforms.v_projection = a_projection.v_array;
	v_shader(v_uniforms, v_attributes, v_mode, 0, v_count);
}

template<>
void t_shading_model::t_skin_shader_of<gl::t_skin_diffuse_texture_specular_refraction_shader>::f_setup(t_shading_model* a_model, const std::vector<t_matrix4f>& a_vertices, t_skin_primitive& a_primitive, const std::map<std::wstring, std::tuple<std::wstring, size_t>>& a_binds)
{
	v_uniforms.v_stride = a_primitive.v_vertices_stride;
	v_uniforms.v_texcoords = a_primitive.f_input(a_binds, static_cast<t_common_texture&>(*a_model->v_diffuse).v_texcoord);
	v_uniforms.v_color = dynamic_cast<t_common_color&>(*a_model->v_emission) + dynamic_cast<t_common_color&>(*a_model->v_ambient) * 0.125f;
	v_uniforms.v_diffuse = dynamic_cast<t_common_texture&>(*a_model->v_diffuse).v_texture->v_texture;
	v_uniforms.v_specular = dynamic_cast<t_common_color&>(*a_model->v_specular);
	v_uniforms.v_shininess = dynamic_cast<t_common_float&>(*a_model->v_shininess).v_value;
	v_uniforms.v_refraction = dynamic_cast<t_common_float&>(*a_model->v_index_of_refraction).v_value;
	v_uniforms.v_vertices = a_vertices.data()->v_array;
	v_uniforms.v_count = a_vertices.size();
	v_attributes = a_primitive.v_vertices;
	v_mode = a_primitive.v_mode;
	v_count = a_primitive.v_count * a_primitive.v_unit;
}

template<>
void t_shading_model::t_skin_shader_of<gl::t_skin_diffuse_texture_specular_refraction_shader>::operator()(const t_matrix4f& a_projection)
{
	v_uniforms.v_projection = a_projection.v_array;
	v_shader(v_uniforms, v_attributes, v_mode, 0, v_count);
}

template<>
void t_shading_model::t_skin_shader_of<gl::t_skin_color_shader>::f_setup(t_shading_model* a_model, const std::vector<t_matrix4f>& a_vertices, t_skin_primitive& a_primitive, const std::map<std::wstring, std::tuple<std::wstring, size_t>>& a_binds)
{
	v_uniforms.v_stride = a_primitive.v_vertices_stride;
	v_uniforms.v_color = dynamic_cast<t_common_color&>(*a_model->v_emission) + dynamic_cast<t_common_color&>(*a_model->v_ambient) * 0.125f;
	v_uniforms.v_vertices = a_vertices.data()->v_array;
	v_uniforms.v_count = a_vertices.size();
	v_attributes = a_primitive.v_vertices;
	v_mode = a_primitive.v_mode;
	v_count = a_primitive.v_count * a_primitive.v_unit;
}

template<>
void t_shading_model::t_skin_shader_of<gl::t_skin_color_shader>::operator()(const t_matrix4f& a_projection)
{
	v_uniforms.v_projection = a_projection.v_array;
	v_shader(v_uniforms, v_attributes, v_mode, 0, v_count);
}

template<>
void t_shading_model::t_skin_shader_of<gl::t_skin_texture_shader>::f_setup(t_shading_model* a_model, const std::vector<t_matrix4f>& a_vertices, t_skin_primitive& a_primitive, const std::map<std::wstring, std::tuple<std::wstring, size_t>>& a_binds)
{
	v_uniforms.v_stride = a_primitive.v_vertices_stride;
	v_uniforms.v_texcoords = a_primitive.f_input(a_binds, static_cast<t_common_texture&>(*a_model->v_emission).v_texcoord);
	v_uniforms.v_color = dynamic_cast<t_common_texture&>(*a_model->v_emission).v_texture->v_texture;
	v_uniforms.v_vertices = a_vertices.data()->v_array;
	v_uniforms.v_count = a_vertices.size();
	v_attributes = a_primitive.v_vertices;
	v_mode = a_primitive.v_mode;
	v_count = a_primitive.v_count * a_primitive.v_unit;
}

template<>
void t_shading_model::t_skin_shader_of<gl::t_skin_texture_shader>::operator()(const t_matrix4f& a_projection)
{
	v_uniforms.v_projection = a_projection.v_array;
	v_shader(v_uniforms, v_attributes, v_mode, 0, v_count);
}

template<>
void t_shading_model::t_skin_shader_of<gl::t_skin_diffuse_color_shader>::f_setup(t_shading_model* a_model, const std::vector<t_matrix4f>& a_vertices, t_skin_primitive& a_primitive, const std::map<std::wstring, std::tuple<std::wstring, size_t>>& a_binds)
{
	v_uniforms.v_stride = a_primitive.v_vertices_stride;
	v_uniforms.v_color = dynamic_cast<t_common_color&>(*a_model->v_emission) + dynamic_cast<t_common_color&>(*a_model->v_ambient) * 0.125f;
	v_uniforms.v_diffuse = dynamic_cast<t_common_color&>(*a_model->v_diffuse);
	v_uniforms.v_vertices = a_vertices.data()->v_array;
	v_uniforms.v_count = a_vertices.size();
	v_attributes = a_primitive.v_vertices;
	v_mode = a_primitive.v_mode;
	v_count = a_primitive.v_count * a_primitive.v_unit;
}

template<>
void t_shading_model::t_skin_shader_of<gl::t_skin_diffuse_color_shader>::operator()(const t_matrix4f& a_projection)
{
	v_uniforms.v_projection = a_projection.v_array;
	v_shader(v_uniforms, v_attributes, v_mode, 0, v_count);
}

template<>
void t_shading_model::t_skin_shader_of<gl::t_skin_diffuse_texture_shader>::f_setup(t_shading_model* a_model, const std::vector<t_matrix4f>& a_vertices, t_skin_primitive& a_primitive, const std::map<std::wstring, std::tuple<std::wstring, size_t>>& a_binds)
{
	v_uniforms.v_stride = a_primitive.v_vertices_stride;
	v_uniforms.v_texcoords = a_primitive.f_input(a_binds, static_cast<t_common_texture&>(*a_model->v_diffuse).v_texcoord);
	v_uniforms.v_color = dynamic_cast<t_common_color&>(*a_model->v_emission) + dynamic_cast<t_common_color&>(*a_model->v_ambient) * 0.125f;
	v_uniforms.v_diffuse = dynamic_cast<t_common_texture&>(*a_model->v_diffuse).v_texture->v_texture;
	v_uniforms.v_vertices = a_vertices.data()->v_array;
	v_uniforms.v_count = a_vertices.size();
	v_attributes = a_primitive.v_vertices;
	v_mode = a_primitive.v_mode;
	v_count = a_primitive.v_count * a_primitive.v_unit;
}

template<>
void t_shading_model::t_skin_shader_of<gl::t_skin_diffuse_texture_shader>::operator()(const t_matrix4f& a_projection)
{
	v_uniforms.v_projection = a_projection.v_array;
	v_shader(v_uniforms, v_attributes, v_mode, 0, v_count);
}

template<>
void t_shading_model::t_skin_shader_of<gl::t_skin_diffuse_color_specular_shader>::f_setup(t_shading_model* a_model, const std::vector<t_matrix4f>& a_vertices, t_skin_primitive& a_primitive, const std::map<std::wstring, std::tuple<std::wstring, size_t>>& a_binds)
{
	v_uniforms.v_stride = a_primitive.v_vertices_stride;
	v_uniforms.v_color = dynamic_cast<t_common_color&>(*a_model->v_emission) + dynamic_cast<t_common_color&>(*a_model->v_ambient) * 0.125f;
	v_uniforms.v_diffuse = dynamic_cast<t_common_color&>(*a_model->v_diffuse);
	v_uniforms.v_specular = dynamic_cast<t_common_color&>(*a_model->v_specular);
	v_uniforms.v_shininess = dynamic_cast<t_common_float&>(*a_model->v_shininess).v_value;
	v_uniforms.v_vertices = a_vertices.data()->v_array;
	v_uniforms.v_count = a_vertices.size();
	v_attributes = a_primitive.v_vertices;
	v_mode = a_primitive.v_mode;
	v_count = a_primitive.v_count * a_primitive.v_unit;
}

template<>
void t_shading_model::t_skin_shader_of<gl::t_skin_diffuse_color_specular_shader>::operator()(const t_matrix4f& a_projection)
{
	v_uniforms.v_projection = a_projection.v_array;
	v_shader(v_uniforms, v_attributes, v_mode, 0, v_count);
}

template<>
void t_shading_model::t_skin_shader_of<gl::t_skin_diffuse_texture_specular_shader>::f_setup(t_shading_model* a_model, const std::vector<t_matrix4f>& a_vertices, t_skin_primitive& a_primitive, const std::map<std::wstring, std::tuple<std::wstring, size_t>>& a_binds)
{
	v_uniforms.v_stride = a_primitive.v_vertices_stride;
	v_uniforms.v_texcoords = a_primitive.f_input(a_binds, static_cast<t_common_texture&>(*a_model->v_diffuse).v_texcoord);
	v_uniforms.v_color = dynamic_cast<t_common_color&>(*a_model->v_emission) + dynamic_cast<t_common_color&>(*a_model->v_ambient) * 0.125f;
	v_uniforms.v_diffuse = dynamic_cast<t_common_texture&>(*a_model->v_diffuse).v_texture->v_texture;
	v_uniforms.v_specular = dynamic_cast<t_common_color&>(*a_model->v_specular);
	v_uniforms.v_shininess = dynamic_cast<t_common_float&>(*a_model->v_shininess).v_value;
	v_uniforms.v_vertices = a_vertices.data()->v_array;
	v_uniforms.v_count = a_vertices.size();
	v_attributes = a_primitive.v_vertices;
	v_mode = a_primitive.v_mode;
	v_count = a_primitive.v_count * a_primitive.v_unit;
}

template<>
void t_shading_model::t_skin_shader_of<gl::t_skin_diffuse_texture_specular_shader>::operator()(const t_matrix4f& a_projection)
{
	v_uniforms.v_projection = a_projection.v_array;
	v_shader(v_uniforms, v_attributes, v_mode, 0, v_count);
}

void t_blinn::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"blinn" << std::endl;
	t_shading_model::f_dump(a_out, a_indent + L'\t');
}

std::unique_ptr<t_shading_model::t_mesh_shader> t_blinn::f_mesh_shader(gl::t_shaders& a_shaders)
{
	if (dynamic_cast<t_common_texture*>(v_diffuse.get())) return std::make_unique<t_mesh_texture_shader>(a_shaders.f_blinn_texture());
	return std::make_unique<t_mesh_color_shader>(a_shaders.f_blinn_color());
}

std::unique_ptr<t_shading_model::t_skin_shader> t_blinn::f_skin_shader(gl::t_shaders& a_shaders, size_t a_joints, size_t a_weights)
{
	if (dynamic_cast<t_common_texture*>(v_diffuse.get())) return std::make_unique<t_skin_texture_shader>(a_shaders.f_skin_texture_blinn(a_joints, a_weights));
	return std::make_unique<t_skin_color_shader>(a_shaders.f_skin_color_blinn(a_joints, a_weights));
}

void t_constant::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"constant" << std::endl;
	t_shading_model::f_dump(a_out, a_indent + L'\t');
}

std::unique_ptr<t_shading_model::t_mesh_shader> t_constant::f_mesh_shader(gl::t_shaders& a_shaders)
{
	if (dynamic_cast<t_common_texture*>(v_emission.get())) return std::make_unique<t_mesh_texture_shader>(a_shaders.f_constant_texture());
	return std::make_unique<t_mesh_color_shader>(a_shaders.f_constant_color());
}

std::unique_ptr<t_shading_model::t_skin_shader> t_constant::f_skin_shader(gl::t_shaders& a_shaders, size_t a_joints, size_t a_weights)
{
	if (dynamic_cast<t_common_texture*>(v_emission.get())) return std::make_unique<t_skin_texture_shader>(a_shaders.f_skin_texture_constant(a_joints, a_weights));
	return std::make_unique<t_skin_color_shader>(a_shaders.f_skin_color_constant(a_joints, a_weights));
}

void t_lambert::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"lambert" << std::endl;
	t_shading_model::f_dump(a_out, a_indent + L'\t');
}

std::unique_ptr<t_shading_model::t_mesh_shader> t_lambert::f_mesh_shader(gl::t_shaders& a_shaders)
{
	if (dynamic_cast<t_common_texture*>(v_diffuse.get())) return std::make_unique<t_mesh_texture_shader>(a_shaders.f_lambert_texture());
	return std::make_unique<t_mesh_color_shader>(a_shaders.f_lambert_color());
}

std::unique_ptr<t_shading_model::t_skin_shader> t_lambert::f_skin_shader(gl::t_shaders& a_shaders, size_t a_joints, size_t a_weights)
{
	if (dynamic_cast<t_common_texture*>(v_diffuse.get())) return std::make_unique<t_skin_texture_shader>(a_shaders.f_skin_texture_lambert(a_joints, a_weights));
	return std::make_unique<t_skin_color_shader>(a_shaders.f_skin_color_lambert(a_joints, a_weights));
}

void t_phong::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"phong" << std::endl;
	t_shading_model::f_dump(a_out, a_indent + L'\t');
}

std::unique_ptr<t_shading_model::t_mesh_shader> t_phong::f_mesh_shader(gl::t_shaders& a_shaders)
{
	if (dynamic_cast<t_common_texture*>(v_diffuse.get())) return std::make_unique<t_mesh_texture_shader>(a_shaders.f_phong_texture());
	return std::make_unique<t_mesh_color_shader>(a_shaders.f_phong_color());
}

std::unique_ptr<t_shading_model::t_skin_shader> t_phong::f_skin_shader(gl::t_shaders& a_shaders, size_t a_joints, size_t a_weights)
{
	if (dynamic_cast<t_common_texture*>(v_diffuse.get())) return std::make_unique<t_skin_texture_shader>(a_shaders.f_skin_texture_phong(a_joints, a_weights));
	return std::make_unique<t_skin_color_shader>(a_shaders.f_skin_color_phong(a_joints, a_weights));
}

void t_technique_fx::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"technique FX" << std::endl;
	auto indent = a_indent + L'\t';
	t_unique::f_dump(a_out, indent);
	t_with_sid::f_dump(a_out, indent);
	v_model->f_dump(a_out, indent);
}

void t_profile_common::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"profile COMMON" << std::endl;
	auto indent = a_indent + L'\t';
	t_unique::f_dump(a_out, indent);
	a_out << a_indent << L"\tnewparams" << std::endl;
	auto indent0 = indent + L'\t';
	for (const auto& x : v_newparams) x->f_dump(a_out, indent0);
	if (v_technique) v_technique->f_dump(a_out, indent);
}

void t_effect::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"effect" << std::endl;
	t_unique::f_dump(a_out, a_indent + L'\t');
	a_out << a_indent << L"\tprofiles" << std::endl;
	auto indent = a_indent + L"\t\t";
	for (const auto& x : v_profiles) x->f_dump(a_out, indent);
}

void t_image::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"image" << std::endl;
	t_unique::f_dump(a_out, a_indent + L'\t');
	a_out << a_indent << L"\ttype: " << v_type << std::endl;
	a_out << a_indent << L"\tvalue: " << v_value << std::endl;
}

void t_image::f_build(const t_resolve& a_resolve)
{
	if (v_built) return;
	if (v_type == L"data")
		throw std::runtime_error("not implemented");
	else
		gl::f_load_rgba(v_value, v_width, v_height, v_data);
	v_built = true;
}

void t_material::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"material" << std::endl;
	t_unique::f_dump(a_out, a_indent + L'\t');
	a_out << a_indent << L"\tinstance_effect: " << v_instance_effect_id << std::endl;
}

void t_matrix_transform::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"matrix" << std::endl;
	t_transform::f_dump(a_out, a_indent + L'\t');
	a_out << a_indent << L"\tvalues: " << std::endl;
	for (auto x : v_array) a_out << L' ' << x;
	a_out << std::endl;
}

void t_matrix_transform::operator()(t_matrix4f& a_x)
{
	a_x *= *this;
}

void t_translate::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"translate" << std::endl;
	t_transform::f_dump(a_out, a_indent + L'\t');
	a_out << a_indent << L"\tvalues: " << v_value.v_x << L' ' << v_value.v_y << L' ' << v_value.v_z << std::endl;
}

void t_translate::operator()(t_matrix4f& a_x)
{
	a_x *= *this;
}

void t_rotate::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"rotate" << std::endl;
	t_transform::f_dump(a_out, a_indent + L'\t');
	a_out << a_indent << L"\tvalues: " << v_axis.v_x << L' ' << v_axis.v_y << L' ' << v_axis.v_z << L' ' << v_angle << std::endl;
}

void t_rotate::operator()(t_matrix4f& a_x)
{
	a_x *= *this;
}

void t_scale::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"scale" << std::endl;
	t_transform::f_dump(a_out, a_indent + L'\t');
	a_out << a_indent << L"\tvalues: " << v_value.v_x << L' ' << v_value.v_y << L' ' << v_value.v_z << std::endl;
}

void t_scale::operator()(t_matrix4f& a_x)
{
	a_x *= *this;
}

void t_instance_material::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"instance material" << std::endl;
	a_out << a_indent << L"\ttarget: " << v_target_id << std::endl;
	a_out << a_indent << L"\tsymbol: " << v_symbol << std::endl;
	a_out << a_indent << L"\tbind vertex inputs" << std::endl;
	auto indent = a_indent + L"\t\t";
	for (const auto& x : v_bind_vertex_inputs) a_out << indent << x.first << L": (" << std::get<0>(x.second) << L", " << std::get<1>(x.second) << L')' << std::endl;
}

void t_instance_material::f_build(const t_resolve& a_resolve)
{
	v_target = &dynamic_cast<t_material&>(*a_resolve[v_target_id]);
	v_target->f_build(a_resolve);
}

t_shading_model* t_instance_material::f_model()
{
	return v_target->f_model();
}

void t_instance_material_fallback::f_build(const t_resolve& a_resolve)
{
}

t_shading_model* t_instance_material_fallback::f_model()
{
	return this;
}

std::unique_ptr<t_shading_model::t_mesh_shader> t_instance_material_fallback::f_mesh_shader(gl::t_shaders& a_shaders)
{
	return std::make_unique<t_mesh_color_shader>(a_shaders.f_constant_color());
}

std::unique_ptr<t_shading_model::t_skin_shader> t_instance_material_fallback::f_skin_shader(gl::t_shaders& a_shaders, size_t a_joints, size_t a_weights)
{
	return std::make_unique<t_skin_color_shader>(a_shaders.f_skin_color_constant(a_joints, a_weights));
}

void t_instance_shape::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"url: " << v_url << std::endl;
	a_out << a_indent << L"materials" << std::endl;
	auto indent = a_indent + L'\t';
	auto indent0 = indent + L'\t';
	for (const auto& x : v_materials) {
		a_out << indent << x.first << std::endl;
		x.second->f_dump(a_out, indent0);
	}
}

void t_instance_geometry::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"instance geometry" << std::endl;
	t_instance_shape::f_dump(a_out, a_indent + L'\t');
}

void t_instance_geometry::f_build_render(const t_resolve& a_resolve, gl::t_shaders& a_shaders)
{
	for (const auto& x : v_geometry->v_primitives) {
		auto material = v_materials.at(x->v_material);
		material->f_build(a_resolve);
		auto shader = material->f_model()->f_mesh_shader(a_shaders);
		shader->f_setup(material->f_model(), *x, material->v_bind_vertex_inputs);
		v_shaders.push_back(std::move(shader));
	}
}

void t_instance_geometry::f_build(const t_resolve& a_resolve, gl::t_shaders& a_shaders)
{
	v_geometry = &dynamic_cast<t_mesh&>(*a_resolve[v_url]);
	v_geometry->f_build(a_resolve);
	f_build_render(a_resolve, a_shaders);
}

void t_instance_geometry::f_create(const t_resolve& a_resolve, gl::t_shaders& a_shaders, t_mesh* a_geometry, const std::map<std::wstring, std::wstring>& a_materials)
{
	v_geometry = a_geometry;
	for (const auto& x : a_materials) {
		auto material = std::make_unique<t_instance_material>();
		material->v_target_id = x.second;
		v_materials.emplace(x.first, material.get());
		v_instance_materials.push_back(std::move(material));
	}
	f_build_render(a_resolve, a_shaders);
}

void t_node::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"node" << std::endl;
	auto indent = a_indent + L'\t';
	t_unique::f_dump(a_out, indent);
	t_with_sid::f_dump(a_out, indent);
	a_out << a_indent << L"\tjoint: " << v_joint << std::endl;
	a_out << a_indent << L"\tlayer: " << v_layer << std::endl;
	a_out << a_indent << L"\ttransforms" << std::endl;
	indent += L'\t';
	for (const auto& x : v_transforms) x->f_dump(a_out, indent);
	a_out << a_indent << L"\tcontrollers" << std::endl;
	for (const auto& x : v_controllers) x->f_dump(a_out, indent);
	a_out << a_indent << L"\tgeometries" << std::endl;
	for (const auto& x : v_geometries) x->f_dump(a_out, indent);
	a_out << a_indent << L"\tinstance nodes" << std::endl;
	for (const auto& x : v_instance_node_ids) a_out << indent << x << std::endl;
	a_out << a_indent << L"\tnodes" << std::endl;
	for (const auto& x : v_nodes) x->f_dump(a_out, indent);
}

void t_node::f_build(const t_resolve& a_resolve, gl::t_shaders& a_shaders)
{
	if (v_built) return;
	for (const auto& x : v_controllers) x->f_build(a_resolve, a_shaders);
	for (const auto& x : v_geometries) x->f_build(a_resolve, a_shaders);
	for (const auto& x : v_instance_node_ids) {
		auto node = &dynamic_cast<t_node&>(*a_resolve[x]);
		node->f_build(a_resolve, a_shaders);
		v_instance_nodes.push_back(node);
	}
	for (const auto& x : v_nodes) x->f_build(a_resolve, a_shaders);
	v_built = true;
}

void t_node::f_postbuild(std::set<t_instance_controller*>& a_ancestors, std::vector<t_instance_controller*>& a_controllers)
{
	for (const auto& x : v_controllers) a_controllers.push_back(x.get());
	if (v_joint) {
		for (auto x : v_referring_controllers) {
			if (a_ancestors.count(x) > 0) continue;
			a_ancestors.insert(x);
			v_root_controllers.push_back(x);
		}
	}
	for (auto x : v_instance_nodes) x->f_postbuild(a_ancestors, a_controllers);
	for (const auto& x : v_nodes) x->f_postbuild(a_ancestors, a_controllers);
	if (v_joint) for (auto x : v_root_controllers) a_ancestors.erase(x);
}

void t_node::f_render_transformed(const t_matrix4f& a_projection, const t_matrix4f& a_viewing, std::vector<std::map<std::wstring, t_matrix4f*>*>& a_joints)
{
	for (const auto& x : v_controllers) x->v_joints[0] = a_viewing;
	if (v_joint) {
		for (auto x : v_root_controllers) a_joints.push_back(&x->v_joint2matrix);
		if (!v_sid.empty()) {
			for (auto x : a_joints) {
				auto i = x->find(v_sid);
				if (i != x->end()) *i->second = a_viewing;
			}
		}
	} else {
		for (const auto& x : v_geometries) x->f_render(a_projection, a_viewing);
	}
	for (auto x : v_instance_nodes) x->f_render(a_projection, a_viewing, a_joints);
	for (const auto& x : v_nodes) x->f_render(a_projection, a_viewing, a_joints);
	for (auto x : v_root_controllers) a_joints.pop_back();
}

void t_node::f_render(const t_matrix4f& a_projection, const t_matrix4f& a_viewing, std::vector<std::map<std::wstring, t_matrix4f*>*>& a_joints)
{
	if (v_transforms.empty()) {
		f_render_transformed(a_projection, a_viewing, a_joints);
	} else {
		auto viewing = a_viewing;
		for (const auto& x : v_transforms) (*x)(viewing);
		f_render_transformed(a_projection, viewing, a_joints);
	}
}

void t_skin::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"skin" << std::endl;
	t_unique::f_dump(a_out, a_indent + L'\t');
	a_out << a_indent << L"\tsource: " << v_source_id << std::endl;
	a_out << a_indent << L"\tbind shape matrix:";
	for (auto x : v_bind_shape_matrix.v_array) a_out << L' ' << x;
	a_out << std::endl << a_indent << L"\tsources" << std::endl;
	auto indent = a_indent + L"\t\t";
	for (const auto& x : v_sources) x->f_dump(a_out, indent);
	a_out << a_indent << L"\tjoints" << std::endl;
	for (const auto& x : v_joints) a_out << indent << x.first << L": " << x.second << std::endl;
	a_out << a_indent << L"\tvertex weights" << std::endl;
	for (const auto& x : v_vertex_weights) a_out << indent << x.first << L": " << std::get<0>(x.second) << L", " << std::get<1>(x.second) << std::endl;
	a_out << a_indent << L"\tvcount:";
	for (auto x : v_vcount) a_out << L' ' << x;
	a_out << std::endl << a_indent << L"\tv:";
	for (auto x : v_v) a_out << L' ' << x;
	a_out << std::endl;
}

void t_skin::f_build(const t_resolve& a_resolve)
{
	if (v_source) return;
	v_source = &dynamic_cast<t_mesh&>(*a_resolve[v_source_id]);
	auto& position = dynamic_cast<t_source&>(*a_resolve[v_source->v_vertices->v_inputs.at(L"POSITION")]);
	position.f_build(a_resolve);
	auto& pa = dynamic_cast<t_accessor_of<float>&>(*position.v_accessor);
	size_t ii = 0;
	size_t ij = std::get<0>(v_vertex_weights.at(L"JOINT"));
	size_t iw = std::get<0>(v_vertex_weights.at(L"WEIGHT"));
	auto& weights = dynamic_cast<t_source&>(*a_resolve[std::get<1>(v_vertex_weights.at(L"WEIGHT"))]);
	weights.f_build(a_resolve);
	auto& wa = dynamic_cast<t_accessor_of<float>&>(*weights.v_accessor);
	std::vector<std::tuple<t_vector3f, std::vector<std::tuple<size_t, float>>>> vertices;
	size_t n = v_vcount.size();
	for (size_t i = 0; i < n; ++i) {
		size_t m = v_vcount[i];
		if (m > v_weights_per_vertex) v_weights_per_vertex = m;
		std::vector<std::tuple<size_t, float>> bones(m);
		for (size_t j = 0; j < m; ++j) {
			size_t joint = v_v[ii + ij];
			size_t weight = v_v[ii + iw];
			bones[j] = std::make_tuple(joint, wa[weight]);
			ii += v_vertex_weights.size();
		}
		std::array<float, 3> x;
		pa.f_copy(i, x.begin());
		vertices.push_back(std::make_tuple(t_vector3f(x[0], x[1], x[2]), std::move(bones)));
	}
	for (const auto& x : v_source->v_primitives) {
		auto y = std::make_unique<t_skin_primitive>(*x);
		y->f_build(a_resolve, vertices, v_weights_per_vertex);
		v_primitives.push_back(std::move(y));
	}
}

void t_instance_controller::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"instance controller" << std::endl;
	t_instance_shape::f_dump(a_out, a_indent + L'\t');
	a_out << a_indent << L"\tskeletons:";
	for (const auto& x : v_skeletons) a_out << L' ' << x;
	a_out << std::endl;
}

void t_instance_controller::f_build(const t_resolve& a_resolve, gl::t_shaders& a_shaders)
{
	v_controller = &dynamic_cast<t_skin&>(*a_resolve[v_url]);
	v_controller->f_build(a_resolve);
	auto& joints = dynamic_cast<t_source&>(*a_resolve[v_controller->v_joints.at(L"JOINT")]);
	joints.f_build(a_resolve);
	auto& ja = dynamic_cast<t_accessor_of<std::wstring>&>(*joints.v_accessor);
	auto& matrices = dynamic_cast<t_source&>(*a_resolve[v_controller->v_joints.at(L"INV_BIND_MATRIX")]);
	matrices.f_build(a_resolve);
	auto& ma = dynamic_cast<t_accessor_of<t_matrix4f>&>(*matrices.v_accessor);
	v_ibms.resize(joints.v_count + 1);
	v_ibms[0] = v_controller->v_bind_shape_matrix;
	v_joints.resize(joints.v_count + 1);
	for (size_t i = 0; i < joints.v_count; ++i) {
		v_ibms[i + 1] = ma[i] * v_controller->v_bind_shape_matrix;
		v_joint2matrix.emplace(ja[i], &v_joints[i + 1]);
	}
	for (const auto& x : v_skeletons) {
		auto& skeleton = dynamic_cast<t_node&>(*a_resolve[x]);
		skeleton.f_build(a_resolve, a_shaders);
		skeleton.v_referring_controllers.push_back(this);
	}
	for (const auto& x : v_controller->v_primitives) {
		auto material = v_materials.at(x->v_material);
		material->f_build(a_resolve);
		auto shader = material->f_model()->f_skin_shader(a_shaders, joints.v_count + 1, v_controller->v_weights_per_vertex);
		shader->f_setup(material->f_model(), v_joints, *x, material->v_bind_vertex_inputs);
		v_shaders.push_back(std::move(shader));
	}
}

void t_channel::f_build(const t_resolve& a_resolve)
{
	if (v_source) return;
	v_source = &dynamic_cast<t_sampler&>(*a_resolve[v_source_id]);
	v_source->f_build(a_resolve);
	size_t i = v_target_path.find(L'/');
	v_node = &dynamic_cast<t_node&>(*a_resolve[L'#' + v_target_path.substr(0, i)]);
	size_t j = v_target_path.find(L'(', ++i);
	auto target = v_node->v_sids.find(v_target_path.substr(i, j - i));
	if (target == v_node->v_sids.end()) return;
	v_target = &dynamic_cast<t_matrix_transform&>(*target->second);
	if (j >= v_target_path.size()) return;
	i = v_target_path.find(L')', ++j);
	size_t column = std::stoul(v_target_path.substr(j, i - j));
	j = v_target_path.find(L'(', i + 1) + 1;
	i = v_target_path.find(L')', j);
	size_t row = std::stoul(v_target_path.substr(j, i - j));
	v_ij = column * 4 + row;
}

void t_sampler::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"sampler" << std::endl;
	a_out << a_indent << L"\tinputs" << std::endl;
	auto indent = a_indent + L"\t\t";
	for (const auto& x : v_inputs) a_out << indent << x.first << L": " << x.second << std::endl;
}

void t_sampler::f_build(const t_resolve& a_resolve)
{
	if (v_input) return;
	v_input = &dynamic_cast<t_source&>(*a_resolve[v_inputs.at(L"INPUT")]);
	v_input->f_build(a_resolve);
	v_output = &dynamic_cast<t_source&>(*a_resolve[v_inputs.at(L"OUTPUT")]);
	v_output->f_build(a_resolve);
	v_interpolation = &dynamic_cast<t_source&>(*a_resolve[v_inputs.at(L"INTERPOLATION")]);
	v_interpolation->f_build(a_resolve);
}

void t_animation::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"animation" << std::endl;
	t_unique::f_dump(a_out, a_indent + L'\t');
	a_out << a_indent << L"\tanimations" << std::endl;
	auto indent = a_indent + L"\t\t";
	for (const auto& x : v_animations) x->f_dump(a_out, indent);
	a_out << a_indent << L"\tsources" << std::endl;
	for (const auto& x : v_sources) x->f_dump(a_out, indent);
	a_out << a_indent << L"\tsamplers" << std::endl;
	for (const auto& x : v_samplers) x->f_dump(a_out, indent);
	a_out << a_indent << L"\tchannels" << std::endl;
	for (const auto& x : v_channels) {
		a_out << a_indent << L"\t\tchannel" << std::endl;
		a_out << a_indent << L"\t\t\tsource: " << x.v_source_id << std::endl;
		a_out << a_indent << L"\t\t\ttarget: " << x.v_target_path << std::endl;
	}
}

void t_animation::f_iterators(const t_resolve& a_resolve, std::map<t_sampler*, std::unique_ptr<t_sampler::t_iterator>>& a_iterators, const std::function<bool (const t_channel&)>& a_use)
{
	for (const auto& x : v_animations) x->f_iterators(a_resolve, a_iterators, a_use);
	for (auto& x : v_channels) {
		x.f_build(a_resolve);
		if (!a_use(x)) continue;
		auto i = a_iterators.lower_bound(x.v_source);
		if (i == a_iterators.end() || i->first != x.v_source) i = a_iterators.emplace_hint(i, x.v_source, x.v_source->f_iterator());
		i->second->v_channels.push_back(&x);
	}
}

void t_visual_scene::f_dump(std::wostream& a_out, const std::wstring& a_indent) const
{
	a_out << a_indent << L"visual scene" << std::endl;
	t_unique::f_dump(a_out, a_indent + L'\t');
	auto indent = a_indent + L"\t\t";
	for (const auto& x : v_nodes) x->f_dump(a_out, indent);
}

void t_visual_scene::f_build(const t_resolve& a_resolve, gl::t_shaders& a_shaders)
{
	if (v_built) return;
	for (const auto& x : v_nodes) x->f_build(a_resolve, a_shaders);
	std::set<t_instance_controller*> ancestors;
	for (const auto& x : v_nodes) x->f_postbuild(ancestors, v_controllers);
	v_built = true;
}

void t_document::f_load(t_reader& a_reader, const std::wstring& a_base)
{
	auto f_read_optional_integer = [&](const wchar_t* a_name, size_t a_default)
	{
		auto x = a_reader.f_get_attribute(a_name);
		return x.empty() ? a_default : std::stoul(x);
	};
	auto f_read_empty_elements = [&](const wchar_t* a_name, auto a_callback)
	{
		for (; a_reader.f_is_start_element(a_name); a_reader.f_read_element_text()) a_callback();
	};
	auto f_read_asset_x_extra = [&](auto a_callback)
	{
		a_reader.f_read_next();
		if (a_reader.f_is_start_element(L"asset")) a_reader.f_read_element_text();
		a_callback();
		while (a_reader.f_is_start_element(L"extra")) a_reader.f_read_element_text();
		a_reader.f_end_element();
	};
	auto f_read_asset_0x_extra = [&](const wchar_t* a_name, auto a_callback)
	{
		if (a_reader.f_is_empty_element()) return a_reader.f_read_next();
		f_read_asset_x_extra([&]
		{
			while (a_reader.f_is_start_element(a_name)) a_callback();
		});
	};
	auto f_read_asset_1x_extra = [&](const wchar_t* a_name, auto a_callback)
	{
		f_read_asset_x_extra([&]
		{
			a_reader.f_check_start_element(a_name);
			while (true) {
				a_callback();
				if (!a_reader.f_is_start_element(a_name)) break;
			}
		});
	};
	const std::map<std::wstring, std::function<void(t_asset&)>> asset_elements{
		{L"unit", [&](t_asset& a_x)
			{
				a_x.v_unit_meter = std::stod(a_reader.f_get_attribute(L"meter"));
				a_x.v_unit_name = a_reader.f_get_attribute(L"name");
				a_reader.f_read_element_text();
			}
		},
		{L"up_axis", [&](t_asset& a_x)
			{
				a_x.v_up_axis = a_reader.f_read_element_text();
			}
		}
	};
	auto f_read_array = [&](auto a_parse)
	{
		size_t count = std::stoul(a_reader.f_get_attribute(L"count"));
		auto id = a_reader.f_get_attribute(L"id");
		auto array = std::make_unique<t_array_of<decltype(a_parse(std::wstring()))>>();
		f_parse_vector(count, a_reader.f_read_element_text(), *array);
		array->v_id = id;
		if (!id.empty()) v_ids.emplace(id, array.get());
		return array;
	};
	const std::map<std::wstring, std::function<void(t_source&)>> source_elements{
		{L"Name_array", [&](t_source& a_x)
			{
				size_t count = std::stoul(a_reader.f_get_attribute(L"count"));
				auto id = a_reader.f_get_attribute(L"id");
				auto array = std::make_unique<t_array_of<std::wstring>>();
				array->reserve(count);
				f_parse_texts(a_reader.f_read_element_text(), f_is_whitespace, [&](auto a_i, auto a_j)
				{
					array->emplace_back(a_i, a_j);
				});
				if (array->size() != count) throw std::runtime_error("wrong count");
				array->v_id = id;
				if (!id.empty()) v_ids.emplace(id, array.get());
				a_x.v_array = std::move(array);
			}
		},
		{L"float_array", [&](t_source& a_x)
			{
				a_x.v_array = f_read_array(f_to<float>);
			}
		},
		{L"int_array", [&](t_source& a_x)
			{
				a_x.v_array = f_read_array(f_to<int>);
			}
		},
		{L"technique_common", [&](t_source& a_x)
			{
				a_reader.f_read_next();
				a_reader.f_check_start_element(L"accessor");
				a_x.v_count = std::stoul(a_reader.f_get_attribute(L"count"));
				a_x.v_offset = f_read_optional_integer(L"offset", 0);
				a_x.v_source_id = a_reader.f_get_attribute(L"source");
				a_x.v_stride = f_read_optional_integer(L"stride", 1);
				a_reader.f_read_next();
				std::vector<std::tuple<std::wstring, std::wstring>> params;
				f_read_empty_elements(L"param", [&]
				{
					auto name = a_reader.f_get_attribute(L"name");
					auto type = a_reader.f_get_attribute(L"type");
					params.emplace_back(name, type);
				});
				if (!params.empty()) {
					if (params.size() == 1 && std::get<1>(params[0]) == L"float4x4") {
						a_x.v_params.push_back(0);
						a_x.v_type = L"float4x4";
					} else {
						size_t i = 0;
						for (auto param : params) {
							if (!std::get<0>(param).empty()) a_x.v_params.push_back(i);
							++i;
						}
						a_x.v_type = std::get<1>(params[0]);
					}
				}
				a_reader.f_end_element();
				a_reader.f_end_element();
			}
		}
	};
	auto f_read_source = [&]
	{
		auto source = std::make_unique<t_source>();
		auto id = a_reader.f_get_attribute(L"id");
		source->v_id = id;
		a_reader.f_parse_elements(source_elements, *source);
		v_ids.emplace(id, source.get());
		return source;
	};
	std::function<std::unique_ptr<t_animation>()> f_read_animation;
	const std::map<std::wstring, std::function<void(t_animation&)>> animation_elements{
		{L"animation", [&](t_animation& a_x)
			{
				a_x.v_animations.push_back(f_read_animation());
			}
		},
		{L"source", [&](t_animation& a_x)
			{
				a_x.v_sources.push_back(f_read_source());
			}
		},
		{L"sampler", [&](t_animation& a_x)
			{
				auto sampler = std::make_unique<t_sampler>();
				auto id = a_reader.f_get_attribute(L"id");
				sampler->v_id = id;
				a_reader.f_read_next();
				f_read_empty_elements(L"input", [&]
				{
					sampler->v_inputs.emplace(a_reader.f_get_attribute(L"semantic"), a_reader.f_get_attribute(L"source"));
				});
				a_reader.f_end_element();
				if (!id.empty()) v_ids.emplace(id, sampler.get());
				a_x.v_samplers.push_back(std::move(sampler));
			}
		},
		{L"channel", [&](t_animation& a_x)
			{
				auto source = a_reader.f_get_attribute(L"source");
				auto target = a_reader.f_get_attribute(L"target");
				a_reader.f_read_element_text();
				a_x.v_channels.emplace_back(source, target);
			}
		}
	};
	f_read_animation = [&]
	{
		auto animation = std::make_unique<t_animation>();
		auto id = a_reader.f_get_attribute(L"id");
		animation->v_id = id;
		a_reader.f_parse_elements(animation_elements, *animation);
		if (!id.empty()) v_ids.emplace(id, animation.get());
		return animation;
	};
	auto f_read_matrix = [&]
	{
		std::vector<float> xs;
		f_parse_vector(16, a_reader.f_read_element_text(), xs);
		t_matrix4f m;
		for (size_t i = 0; i < 4; ++i)
			for (size_t j = 0; j < 4; ++j)
				m[j][i] = xs[i * 4 + j];
		return m;
	};
	auto f_read_skin = [&]
	{
		auto skin = std::make_unique<t_skin>();
		skin->v_source_id = a_reader.f_get_attribute(L"source");
		a_reader.f_read_next();
		if (a_reader.f_is_start_element(L"bind_shape_matrix")) skin->v_bind_shape_matrix = f_read_matrix();
		for (size_t i = 0; i < 3; ++i) {
			a_reader.f_check_start_element(L"source");
			skin->v_sources.push_back(f_read_source());
		}
		while (a_reader.f_is_start_element(L"source")) skin->v_sources.push_back(f_read_source());
		a_reader.f_check_start_element(L"joints");
		a_reader.f_read_next();
		f_read_empty_elements(L"input", [&]
		{
			skin->v_joints.emplace(a_reader.f_get_attribute(L"semantic"), a_reader.f_get_attribute(L"source"));
		});
		while (a_reader.f_is_start_element(L"extra")) a_reader.f_read_element_text();
		a_reader.f_end_element();
		a_reader.f_check_start_element(L"vertex_weights");
		size_t count = std::stoul(a_reader.f_get_attribute(L"count"));
		a_reader.f_read_next();
		f_read_empty_elements(L"input", [&]
		{
			skin->v_vertex_weights.emplace(a_reader.f_get_attribute(L"semantic"), std::make_tuple(skin->v_vertex_weights.size(), a_reader.f_get_attribute(L"source")));
		});
		a_reader.f_check_start_element(L"vcount");
		f_parse_vector(count, a_reader.f_read_element_text(), skin->v_vcount);
		size_t n = 0;
		for (size_t x : skin->v_vcount) n += x;
		a_reader.f_check_start_element(L"v");
		f_parse_vector(n * skin->v_vertex_weights.size(), a_reader.f_read_element_text(), skin->v_v);
		while (a_reader.f_is_start_element(L"extra")) a_reader.f_read_element_text();
		a_reader.f_end_element();
		while (a_reader.f_is_start_element(L"extra")) a_reader.f_read_element_text();
		a_reader.f_end_element();
		return skin;
	};
	const std::map<std::wstring, std::function<void(t_surface&)>> profile_COMMON_newparam_surface_elements{
		{L"init_from", [&](t_surface& a_x)
			{
				a_x.v_mip = a_reader.f_get_attribute(L"mip");
				a_x.v_slice = a_reader.f_get_attribute(L"slice");
				a_x.v_face = a_reader.f_get_attribute(L"face");
				a_x.v_from_id = a_reader.f_read_element_text();
			}
		},
		{L"format", [&](t_surface& a_x)
			{
				a_x.v_format = a_reader.f_read_element_text();
			}
		}
	};
	const std::map<std::wstring, std::function<void(t_sampler2d&)>> profile_COMMON_newparam_sampler2D_elements{
		{L"source", [&](t_sampler2d& a_x)
			{
				a_x.v_source_sid = a_reader.f_read_element_text();
			}
		},
		{L"minfilter", [&](t_sampler2d& a_x)
			{
				a_x.v_minfilter = a_reader.f_read_element_text();
			}
		},
		{L"magfilter", [&](t_sampler2d& a_x)
			{
				a_x.v_magfilter = a_reader.f_read_element_text();
			}
		}
	};
	const std::map<std::wstring, std::function<void(std::unique_ptr<t_with_sid>&)>> profile_COMMON_newparam_elements{
		{L"surface", [&](std::unique_ptr<t_with_sid>& a_x)
			{
				auto p = new t_surface();
				a_x.reset(p);
				p->v_type = a_reader.f_get_attribute(L"type");
				a_reader.f_parse_elements(profile_COMMON_newparam_surface_elements, *p);
			}
		},
		{L"sampler2D", [&](std::unique_ptr<t_with_sid>& a_x)
			{
				auto p = new t_sampler2d();
				a_x.reset(p);
				a_reader.f_parse_elements(profile_COMMON_newparam_sampler2D_elements, *p);
			}
		}
	};
	auto f_read_common_color_or_texture = [&]
	{
		a_reader.f_read_next();
		std::unique_ptr<t_common> value;
		if (a_reader.f_is_start_element(L"color")) {
			std::vector<float> xs;
			f_parse_vector(4, a_reader.f_read_element_text(), xs);
			value.reset(new t_common_color(xs[0], xs[1], xs[2], xs[3]));
		} else if (a_reader.f_is_start_element(L"param")) {
			auto p = new t_common_param();
			value.reset(p);
			p->v_ref_sid = a_reader.f_get_attribute(L"ref");
			a_reader.f_read_element_text();
		} else if (a_reader.f_is_start_element(L"texture")) {
			auto p = new t_common_texture();
			value.reset(p);
			p->v_texture_sid = a_reader.f_get_attribute(L"texture");
			p->v_texcoord = a_reader.f_get_attribute(L"texcoord");
			a_reader.f_read_element_text();
		}
		a_reader.f_end_element();
		return value;
	};
	auto f_read_float_or_param = [&]
	{
		a_reader.f_read_next();
		std::unique_ptr<t_common> value;
		if (a_reader.f_is_start_element(L"float")) {
			value.reset(new t_common_float(f_to<float>(a_reader.f_read_element_text())));
		} else if (a_reader.f_is_start_element(L"param")) {
			auto p = new t_common_param();
			value.reset(p);
			p->v_ref_sid = a_reader.f_get_attribute(L"ref");
			a_reader.f_read_element_text();
		}
		a_reader.f_end_element();
		return value;
	};
	const std::map<std::wstring, std::function<void(t_shading_model&)>> shader_elements{
		{L"emission", [&](t_shading_model& a_x)
			{
				a_x.v_emission = f_read_common_color_or_texture();
			}
		},
		{L"ambient", [&](t_shading_model& a_x)
			{
				a_x.v_ambient = f_read_common_color_or_texture();
			}
		},
		{L"diffuse", [&](t_shading_model& a_x)
			{
				a_x.v_diffuse = f_read_common_color_or_texture();
			}
		},
		{L"specular", [&](t_shading_model& a_x)
			{
				a_x.v_specular = f_read_common_color_or_texture();
			}
		},
		{L"shininess", [&](t_shading_model& a_x)
			{
				a_x.v_shininess = f_read_float_or_param();
			}
		},
		{L"reflective", [&](t_shading_model& a_x)
			{
				a_x.v_reflective = f_read_common_color_or_texture();
			}
		},
		{L"reflectivity", [&](t_shading_model& a_x)
			{
				a_x.v_reflectivity = f_read_float_or_param();
			}
		},
		{L"transparent", [&](t_shading_model& a_x)
			{
				a_x.v_transparent = f_read_common_color_or_texture();
			}
		},
		{L"transparency", [&](t_shading_model& a_x)
			{
				a_x.v_transparency = f_read_float_or_param();
			}
		},
		{L"index_of_refraction", [&](t_shading_model& a_x)
			{
				a_x.v_index_of_refraction = f_read_float_or_param();
			}
		}
	};
	const std::map<std::wstring, std::function<void(t_technique_fx&)>> profile_COMMON_technique_elements{
		{L"blinn", [&](t_technique_fx& a_x)
			{
				a_x.v_model.reset(new t_blinn());
				a_reader.f_parse_elements(shader_elements, *a_x.v_model);
			}
		},
		{L"constant", [&](t_technique_fx& a_x)
			{
				a_x.v_model.reset(new t_constant());
				a_reader.f_parse_elements(shader_elements, *a_x.v_model);
			}
		},
		{L"lambert", [&](t_technique_fx& a_x)
			{
				a_x.v_model.reset(new t_lambert());
				a_reader.f_parse_elements(shader_elements, *a_x.v_model);
			}
		},
		{L"phong", [&](t_technique_fx& a_x)
			{
				a_x.v_model.reset(new t_phong());
				a_reader.f_parse_elements(shader_elements, *a_x.v_model);
			}
		}
	};
	const std::map<std::wstring, std::function<void(t_effect&)>> effect_elements{
		{L"profile_COMMON", [&](t_effect& a_x)
			{
				auto profile = std::make_unique<t_profile_common>();
				auto id = a_reader.f_get_attribute(L"id");
				profile->v_id = id;
				f_read_asset_x_extra([&]
				{
					while (a_reader.f_is_start_element(L"image")) a_reader.f_read_element_text();
					while (a_reader.f_is_start_element(L"newparam")) {
						std::unique_ptr<t_with_sid> newparam;
						auto sid = a_reader.f_get_attribute(L"sid");
						a_reader.f_parse_elements(profile_COMMON_newparam_elements, newparam);
						newparam->v_sid = sid;
						profile->v_sids.emplace(sid, newparam.get());
						profile->v_newparams.push_back(std::move(newparam));
					}
					a_reader.f_check_start_element(L"technique");
					auto technique = std::make_unique<t_technique_fx>();
					auto id = a_reader.f_get_attribute(L"id");
					auto sid = a_reader.f_get_attribute(L"sid");
					technique->v_id = id;
					technique->v_sid = sid;
					a_reader.f_parse_elements(profile_COMMON_technique_elements, *technique);
					profile->v_sids.emplace(sid, technique.get());
					if (!id.empty()) v_ids.emplace(id, technique.get());
					profile->v_technique = std::move(technique);
				});
				if (!id.empty()) v_ids.emplace(id, profile.get());
				a_x.v_profiles.push_back(std::move(profile));
			}
		}
	};
	auto f_read_primitive_common = [&](const auto& a_primitive, auto a_callback)
	{
		a_primitive->v_count = std::stoul(a_reader.f_get_attribute(L"count"));
		a_primitive->v_material = a_reader.f_get_attribute(L"material");
		a_reader.f_read_next();
		size_t stride = 0;
		f_read_empty_elements(L"input", [&]
		{
			t_input input;
			input.v_offset = std::stoul(a_reader.f_get_attribute(L"offset"));
			if (input.v_offset + 1 > stride) stride = input.v_offset + 1;
			auto semantic = a_reader.f_get_attribute(L"semantic");
			input.v_semantic = semantic;
			input.v_source_id = a_reader.f_get_attribute(L"source");
			auto set = a_reader.f_get_attribute(L"set");
			size_t i = set.empty() ? 0 : std::stoul(set);
			a_primitive->v_inputs.emplace(std::make_tuple(semantic, i), input);
		});
		a_primitive->v_stride = stride;
		a_callback();
		while (a_reader.f_is_start_element(L"extra")) a_reader.f_read_element_text();
		a_reader.f_end_element();
	};
	auto f_read_primitive = [&](const auto& a_primitive)
	{
		f_read_primitive_common(a_primitive, [&]
		{
			if (a_reader.f_is_start_element(L"p")) f_parse_vector(a_primitive->v_count * a_primitive->v_stride * a_primitive->v_unit, a_reader.f_read_element_text(), a_primitive->v_indices);
		});
	};
	auto f_read_polylist = [&]
	{
		auto primitive = std::make_unique<t_mesh_primitive>(GL_TRIANGLES, 3);
		f_read_primitive_common(primitive, [&]
		{
			size_t count = primitive->v_count;
			size_t stride = primitive->v_stride;
			a_reader.f_check_start_element(L"vcount");
			std::vector<size_t> vcount;
			f_parse_vector(count, a_reader.f_read_element_text(), vcount);
			size_t n = 0;
			size_t m = 0;
			for (size_t x : vcount) {
				n += x;
				m += (x - 2) * 3;
			}
			a_reader.f_check_start_element(L"p");
			std::vector<size_t> indices0;
			f_parse_vector(n * stride, a_reader.f_read_element_text(), indices0);
			std::vector<size_t>& indices1 = primitive->v_indices;
			indices1.resize(m * stride);
			n = m = 0;
			size_t triangles = 0;
			for (size_t x : vcount) {
				size_t c = x - 2;
				for (size_t j = 0; j < c; ++j) {
					for (size_t k = 0; k < stride; ++k) {
						indices1[m + j * 3 * stride + k] = indices0[n + k];
						indices1[m + (j * 3 + 1) * stride + k] = indices0[n + (j + 1) * stride + k];
						indices1[m + (j * 3 + 2) * stride + k] = indices0[n + (j + 2) * stride + k];
					}
				}
				n += x * stride;
				m += c * 3 * stride;
				triangles += c;
			}
			primitive->v_count = triangles;
		});
		return primitive;
	};
	const std::map<std::wstring, std::function<void(t_mesh&)>> mesh_elements{
		{L"source", [&](t_mesh& a_x)
			{
				a_x.v_sources.push_back(f_read_source());
			}
		},
		{L"vertices", [&](t_mesh& a_x)
			{
				auto id = a_reader.f_get_attribute(L"id");
				a_x.v_vertices->v_id = id;
				a_reader.f_read_next();
				f_read_empty_elements(L"input", [&]
				{
					a_x.v_vertices->v_inputs.emplace(a_reader.f_get_attribute(L"semantic"), a_reader.f_get_attribute(L"source"));
				});
				while (a_reader.f_is_start_element(L"extra")) a_reader.f_read_element_text();
				a_reader.f_end_element();
				v_ids.emplace(id, a_x.v_vertices.get());
			}
		},
		{L"lines", [&](t_mesh& a_x)
			{
				auto primitive = std::make_unique<t_mesh_primitive>(GL_LINES, 2);
				f_read_primitive(primitive);
				a_x.v_primitives.push_back(std::move(primitive));
			}
		},
		{L"triangles", [&](t_mesh& a_x)
			{
				auto primitive = std::make_unique<t_mesh_primitive>(GL_TRIANGLES, 3);
				f_read_primitive(primitive);
				a_x.v_primitives.push_back(std::move(primitive));
			}
		},
		{L"polylist", [&](t_mesh& a_x)
			{
				a_x.v_primitives.push_back(f_read_polylist());
			}
		}
	};
	const std::map<std::wstring, std::function<void(std::unique_ptr<t_geometry>&)>> geometry_elements{
		{L"mesh", [&](std::unique_ptr<t_geometry>& a_x)
			{
				auto p = new t_mesh();
				a_x.reset(p);
				a_reader.f_parse_elements(mesh_elements, *p);
			}
		}
	};
	auto f_read_instance_material = [&](auto& a_x)
	{
		auto material = std::make_unique<t_instance_material>();
		material->v_target_id = a_reader.f_get_attribute(L"target");
		material->v_symbol = a_reader.f_get_attribute(L"symbol");
		bool b = a_reader.f_is_empty_element();
		a_reader.f_read_next();
		if (!b) {
			while (a_reader.f_is_start_element(L"bind")) a_reader.f_read_element_text();
			while (a_reader.f_is_start_element(L"bind_vertex_input")) {
				auto semantic = a_reader.f_get_attribute(L"semantic");
				auto input_semantic = a_reader.f_get_attribute(L"input_semantic");
				auto input_set = a_reader.f_get_attribute(L"input_set");
				size_t set = input_set.empty() ? 0 : f_to<size_t>(input_set);
				a_reader.f_read_element_text();
				material->v_bind_vertex_inputs.emplace(semantic, std::make_tuple(input_semantic, set));
			}
			while (a_reader.f_is_start_element(L"extra")) a_reader.f_read_element_text();
			a_reader.f_end_element();
		}
		a_x.v_materials.emplace(material->v_symbol, material.get());
		a_x.v_instance_materials.push_back(std::move(material));
	};
	auto f_read_bind_material = [&](auto& a_x)
	{
		a_reader.f_read_next();
		while (a_reader.f_is_start_element(L"param")) a_reader.f_read_element_text();
		a_reader.f_check_start_element(L"technique_common");
		a_reader.f_read_next();
		a_reader.f_check_start_element(L"instance_material");
		f_read_instance_material(a_x);
		while (a_reader.f_is_start_element(L"instance_material")) f_read_instance_material(a_x);
		a_reader.f_end_element();
		while (a_reader.f_is_start_element(L"technique")) a_reader.f_read_element_text();
		while (a_reader.f_is_start_element(L"extra")) a_reader.f_read_element_text();
		a_reader.f_end_element();
	};
	std::function<std::unique_ptr<t_node>()> f_read_node;
	const std::map<std::wstring, std::function<void(t_node&)>> node_elements{
		{L"matrix", [&](t_node& a_x)
			{
				auto sid = a_reader.f_get_attribute(L"sid");
				auto transform = std::make_unique<t_matrix_transform>(f_read_matrix());
				transform->v_sid = sid;
				if (!sid.empty()) a_x.v_sids.emplace(sid, transform.get());
				a_x.v_transforms.push_back(std::move(transform));
			}
		},
		{L"rotate", [&](t_node& a_x)
			{
				auto sid = a_reader.f_get_attribute(L"sid");
				std::vector<float> xs;
				f_parse_vector(4, a_reader.f_read_element_text(), xs);
				auto transform = std::make_unique<t_rotate>(xs[0], xs[1], xs[2], xs[3] * static_cast<float>(M_PI / 180.0));
				transform->v_sid = sid;
				if (!sid.empty()) a_x.v_sids.emplace(sid, transform.get());
				a_x.v_transforms.push_back(std::move(transform));
			}
		},
		{L"scale", [&](t_node& a_x)
			{
				auto sid = a_reader.f_get_attribute(L"sid");
				std::vector<float> xs;
				f_parse_vector(3, a_reader.f_read_element_text(), xs);
				auto transform = std::make_unique<t_scale>(xs[0], xs[1], xs[2]);
				transform->v_sid = sid;
				if (!sid.empty()) a_x.v_sids.emplace(sid, transform.get());
				a_x.v_transforms.push_back(std::move(transform));
			}
		},
		{L"translate", [&](t_node& a_x)
			{
				auto sid = a_reader.f_get_attribute(L"sid");
				std::vector<float> xs;
				f_parse_vector(3, a_reader.f_read_element_text(), xs);
				auto transform = std::make_unique<t_translate>(xs[0], xs[1], xs[2]);
				transform->v_sid = sid;
				if (!sid.empty()) a_x.v_sids.emplace(sid, transform.get());
				a_x.v_transforms.push_back(std::move(transform));
			}
		},
		{L"instance_controller", [&](t_node& a_x)
			{
				auto controller = std::make_unique<t_instance_controller>(&v_instance_material_fallback);
				controller->v_url = a_reader.f_get_attribute(L"url");
				if (a_reader.f_is_empty_element()) {
					a_reader.f_read_next();
				} else {
					a_reader.f_read_next();
					while (a_reader.f_is_start_element(L"skeleton")) controller->v_skeletons.push_back(a_reader.f_read_element_text());
					if (a_reader.f_is_start_element(L"bind_material")) f_read_bind_material(*controller);
					while (a_reader.f_is_start_element(L"extra")) a_reader.f_read_element_text();
					a_reader.f_end_element();
				}
				a_x.v_controllers.push_back(std::move(controller));
			}
		},
		{L"instance_geometry", [&](t_node& a_x)
			{
				auto geometry = std::make_unique<t_instance_geometry>(&v_instance_material_fallback);
				geometry->v_url = a_reader.f_get_attribute(L"url");
				if (a_reader.f_is_empty_element()) {
					a_reader.f_read_next();
				} else {
					a_reader.f_read_next();
					if (a_reader.f_is_start_element(L"bind_material")) f_read_bind_material(*geometry);
					while (a_reader.f_is_start_element(L"extra")) a_reader.f_read_element_text();
					a_reader.f_end_element();
				}
				a_x.v_geometries.push_back(std::move(geometry));
			}
		},
		{L"instance_node", [&](t_node& a_x)
			{
				a_x.v_instance_node_ids.push_back(a_reader.f_get_attribute(L"url"));
				a_reader.f_read_element_text();
			}
		},
		{L"node", [&](t_node& a_x)
			{
				a_x.v_nodes.push_back(f_read_node());
			}
		}
	};
	f_read_node = [&]
	{
		auto node = std::make_unique<t_node>();
		auto id = a_reader.f_get_attribute(L"id");
		node->v_id = id;
		node->v_sid = a_reader.f_get_attribute(L"sid");
		node->v_joint = a_reader.f_get_attribute(L"type") == L"JOINT";
		node->v_layer = a_reader.f_get_attribute(L"layer");
		a_reader.f_parse_elements(node_elements, *node);
		if (!id.empty()) v_ids.emplace(id, node.get());
		return node;
	};
	const std::map<std::wstring, std::function<void()>> root_elements{
		{L"asset", [&]
			{
				a_reader.f_parse_elements(asset_elements, v_asset);
			}
		},
		{L"library_animations", [&]
			{
				f_read_asset_1x_extra(L"animation", [&]
				{
					v_library_animations.push_back(f_read_animation());
				});
			}
		},
		{L"library_controllers", [&]
			{
				f_read_asset_1x_extra(L"controller", [&]
				{
					std::unique_ptr<t_controller> controller;
					auto id = a_reader.f_get_attribute(L"id");
					f_read_asset_x_extra([&]
					{
						if (a_reader.f_is_start_element(L"skin"))
							controller = f_read_skin();
						else if (a_reader.f_is_start_element(L"morph"))
							a_reader.f_read_element_text();
					});
					controller->v_id = id;
					v_ids.emplace(id, controller.get());
					v_library_controllers.push_back(std::move(controller));
				});
			}
		},
		{L"library_effects", [&]()
			{
				f_read_asset_1x_extra(L"effect", [&]
				{
					auto effect = std::make_unique<t_effect>();
					auto id = a_reader.f_get_attribute(L"id");
					effect->v_id = id;
					a_reader.f_parse_elements(effect_elements, *effect);
					v_ids.emplace(id, effect.get());
					v_library_effects.push_back(std::move(effect));
				});
			}
		},
		{L"library_geometries", [&]
			{
				f_read_asset_1x_extra(L"geometry", [&]
				{
					std::unique_ptr<t_geometry> geometry;
					auto id = a_reader.f_get_attribute(L"id");
					a_reader.f_parse_elements(geometry_elements, geometry);
					geometry->v_id = id;
					if (!id.empty()) v_ids.emplace(id, geometry.get());
					v_library_geometries.push_back(std::move(geometry));
				});
			}
		},
		{L"library_images", [&]
			{
				//f_read_asset_1x_extra(L"image", [&]
				f_read_asset_0x_extra(L"image", [&]
				{
					auto image = std::make_unique<t_image>();
					auto id = a_reader.f_get_attribute(L"id");
					image->v_id = id;
					f_read_asset_x_extra([&]
					{
						if (a_reader.f_is_start_element(L"data")) {
							image->v_type = L"data";
							image->v_value = a_reader.f_read_element_text();
						} else if (a_reader.f_is_start_element(L"init_from")) {
							image->v_type = L"path";
							image->v_value = a_base + a_reader.f_read_element_text();
						}
					});
					if (!id.empty()) v_ids.emplace(id, image.get());
					v_library_images.push_back(std::move(image));
				});
			}
		},
		{L"library_materials", [&]
			{
				f_read_asset_1x_extra(L"material", [&]
				{
					auto material = std::make_unique<t_material>();
					auto id = a_reader.f_get_attribute(L"id");
					material->v_id = id;
					f_read_asset_x_extra([&]
					{
						a_reader.f_check_start_element(L"instance_effect");
						material->v_instance_effect_id = a_reader.f_get_attribute(L"url");
						a_reader.f_read_element_text();
					});
					if (!id.empty()) v_ids.emplace(id, material.get());
					v_library_materials.push_back(std::move(material));
				});
			}
		},
		{L"library_nodes", [&]
			{
				f_read_asset_1x_extra(L"node", [&]
				{
					v_library_nodes.push_back(f_read_node());
				});
			}
		},
		{L"library_visual_scenes", [&]
			{
				f_read_asset_1x_extra(L"visual_scene", [&]
				{
					auto scene = std::make_unique<t_visual_scene>();
					auto id = a_reader.f_get_attribute(L"id");
					scene->v_id = id;
					f_read_asset_x_extra([&]
					{
						a_reader.f_check_start_element(L"node");
						do scene->v_nodes.push_back(f_read_node()); while (a_reader.f_is_start_element(L"node"));
						while (a_reader.f_is_start_element(L"evaluate_scene")) a_reader.f_read_element_text();
					});
					if (!id.empty()) v_ids.emplace(id, scene.get());
					v_library_visual_scenes.push_back(std::move(scene));
				});
			}
		},
		{L"scene", [&]
			{
				a_reader.f_read_next();
				while (a_reader.f_is_start_element(L"instance_physics_scene")) a_reader.f_read_element_text();
				if (a_reader.f_is_start_element(L"instance_visual_scene")) {
					v_scene.v_instance_visual_scene.v_url = a_reader.f_get_attribute(L"url");
					a_reader.f_read_element_text();
				}
				while (a_reader.f_is_start_element(L"extra")) a_reader.f_read_element_text();
				a_reader.f_end_element();
			}
		}
	};
	a_reader.f_read_next();
	a_reader.f_move_to_tag();
	a_reader.f_check_start_element(L"COLLADA");
	v_version = a_reader.f_get_attribute(L"version");
	a_reader.f_parse_elements(root_elements);
}

void t_document::f_dump(std::wostream& a_out) const
{
	a_out << L"version: " << v_version << std::endl;
	v_asset.f_dump(a_out, std::wstring());
	a_out << L"library animations" << std::endl;
	for (const auto& x : v_library_animations) x->f_dump(a_out, L"\t");
	a_out << L"library controllers" << std::endl;
	for (const auto& x : v_library_controllers) x->f_dump(a_out, L"\t");
	a_out << L"library effects" << std::endl;
	for (const auto& x : v_library_effects) x->f_dump(a_out, L"\t");
	a_out << L"library geometries" << std::endl;
	for (const auto& x : v_library_geometries) x->f_dump(a_out, L"\t");
	a_out << L"library images" << std::endl;
	for (const auto& x : v_library_images) x->f_dump(a_out, L"\t");
	a_out << L"library materials" << std::endl;
	for (const auto& x : v_library_materials) x->f_dump(a_out, L"\t");
	a_out << L"library nodes" << std::endl;
	for (const auto& x : v_library_nodes) x->f_dump(a_out, L"\t");
	a_out << L"library visual scenes" << std::endl;
	for (const auto& x : v_library_visual_scenes) x->f_dump(a_out, L"\t");
	a_out << L"scene" << std::endl;
	a_out << L"\tinstance visual scene" << std::endl;
	a_out << L"\t\turl: " << v_scene.v_instance_visual_scene.v_url << std::endl;
}
