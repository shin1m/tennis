#include <gl/image.h>

#include "portable.h"
#include "sdl_core.h"

namespace gl
{

void t_shaders::f_vertex_shader(t_shader& a_shader, const std::string& a_defines)
{
	a_shader.f_create(GL_VERTEX_SHADER);
	a_shader.f_compile((a_defines + "\n"
"uniform mat4 projection;\n"
"uniform mat4 vertexMatrix;\n"
"attribute vec3 vertex;\n"
"#ifdef USE_TEXTURE\n"
"attribute vec2 texcoord;\n"
"varying vec2 varyingTexcoord;\n"
"#endif\n"
"\n"
"void main()\n"
"{\n"
"	gl_Position = projection * (vertexMatrix * vec4(vertex, 1.0));\n"
"#ifdef USE_TEXTURE\n"
"	varyingTexcoord = vec2(texcoord.x, 1.0 - texcoord.y);\n"
"#endif\n"
"}\n"
	).c_str());
}

void t_shaders::f_vertex_shader_normal(t_shader& a_shader, const std::string& a_defines)
{
	a_shader.f_create(GL_VERTEX_SHADER);
	a_shader.f_compile((a_defines + "\n"
"uniform mat4 projection;\n"
"uniform mat4 vertexMatrix;\n"
"attribute vec3 vertex;\n"
"attribute vec3 normal;\n"
"uniform mat3 normalMatrix;\n"
"varying vec3 varyingNormal;\n"
"#ifdef USE_TEXTURE\n"
"attribute vec2 texcoord;\n"
"varying vec2 varyingTexcoord;\n"
"#endif\n"
"\n"
"void main()\n"
"{\n"
"	gl_Position = projection * (vertexMatrix * vec4(vertex, 1.0));\n"
"	varyingNormal = normalize(normal * normalMatrix);\n"
"#ifdef USE_TEXTURE\n"
"	varyingTexcoord = vec2(texcoord.x, 1.0 - texcoord.y);\n"
"#endif\n"
"}\n"
	).c_str());
}

void t_shaders::f_constant_shader(t_shader& a_shader, const std::string& a_defines)
{
	a_shader.f_create(GL_FRAGMENT_SHADER);
	a_shader.f_compile((a_defines + "\n"
"#ifdef GL_ES\n"
"precision mediump float;\n"
"precision mediump int;\n"
"#endif\n"
"\n"
"#ifdef USE_TEXTURE\n"
"uniform sampler2D color;\n"
"varying vec2 varyingTexcoord;\n"
"#else\n"
"uniform vec4 color;\n"
"#endif\n"
"\n"
"void main()\n"
"{\n"
"#ifdef USE_TEXTURE\n"
"	vec4 sample = texture2D(color, varyingTexcoord);\n"
"	if (sample.a < 0.5) discard;\n"
"	gl_FragColor = sample;\n"
"#else\n"
"	gl_FragColor = color;\n"
"#endif\n"
"}\n"
	).c_str());
}

void t_shaders::f_blinn_shader(t_shader& a_shader, const std::string& a_defines)
{
	a_shader.f_create(GL_FRAGMENT_SHADER);
	a_shader.f_compile((a_defines + "\n"
"#ifdef GL_ES\n"
"precision mediump float;\n"
"precision mediump int;\n"
"#endif\n"
"\n"
"uniform vec4 color;\n"
"varying vec3 varyingNormal;\n"
"#ifdef USE_TEXTURE\n"
"uniform sampler2D diffuse;\n"
"varying vec2 varyingTexcoord;\n"
"#else\n"
"uniform vec4 diffuse;\n"
"#endif\n"
"uniform vec4 specular;\n"
"uniform float shininess;\n"
"uniform float refraction;\n"
"const vec3 light = vec3(0.40824829046, 0.40824829046, 0.81649658092);\n"
"\n"
"void main()\n"
"{\n"
"#ifdef USE_TEXTURE\n"
"	vec4 sample = texture2D(diffuse, varyingTexcoord);\n"
"	if (sample.a < 0.5) discard;\n"
"#endif\n"
"	vec3 normal = normalize(varyingNormal);\n"
"//	vec3 light = normalize(vec3(5.0, 5.0, 10.0));\n"
"	vec3 eye = vec3(0.0, 0.0, 1.0);\n"
"	vec3 h = normalize(light + eye);\n"
"	float nh = dot(normal, h);\n"
"	float c2 = shininess * shininess;\n"
"	float D = c2 / (nh * nh * (c2 - 1.0) + 1.0);\n"
"	float eh = dot(eye, h);\n"
"	float ne = dot(normal, eye);\n"
"	float nl = dot(normal, light);\n"
"	float G = min(1.0, 2.0 * nh * min(ne, nl) / eh);\n"
"	float g = sqrt(refraction * refraction + eh * eh - 1.0);\n"
"	float gpc = g + eh;\n"
"	float gmc = g - eh;\n"
"	float cgpcm1 = eh * gpc - 1.0;\n"
"	float cgmcp1 = eh * gmc + 1.0;\n"
"	float F = gmc * gmc * (1.0 + cgpcm1 * cgpcm1 / (cgmcp1 * cgmcp1)) * 0.5 / (gpc * gpc);\n"
"#ifdef USE_TEXTURE\n"
"	gl_FragColor = color + sample\n"
"#else\n"
"	gl_FragColor = color + diffuse\n"
"#endif\n"
"	* max(nl, 0.0) + specular * max(D * D * G * F / ne, 0.0);\n"
"}\n"
	).c_str());
}

void t_shaders::f_lambert_shader(t_shader& a_shader, const std::string& a_defines)
{
	a_shader.f_create(GL_FRAGMENT_SHADER);
	a_shader.f_compile((a_defines + "\n"
"#ifdef GL_ES\n"
"precision mediump float;\n"
"precision mediump int;\n"
"#endif\n"
"\n"
"uniform vec4 color;\n"
"varying vec3 varyingNormal;\n"
"#ifdef USE_TEXTURE\n"
"uniform sampler2D diffuse;\n"
"varying vec2 varyingTexcoord;\n"
"#else\n"
"uniform vec4 diffuse;\n"
"#endif\n"
"const vec3 light = vec3(0.40824829046, 0.40824829046, 0.81649658092);\n"
"\n"
"void main()\n"
"{\n"
"#ifdef USE_TEXTURE\n"
"	vec4 sample = texture2D(diffuse, varyingTexcoord);\n"
"	if (sample.a < 0.5) discard;\n"
"#endif\n"
"	vec3 normal = normalize(varyingNormal);\n"
"//	vec3 light = normalize(vec3(5.0, 5.0, 10.0));\n"
"#ifdef USE_TEXTURE\n"
"	gl_FragColor = color + sample * max(dot(normal, light), 0.0);\n"
"#else\n"
"	gl_FragColor = color + diffuse * max(dot(normal, light), 0.0);\n"
"#endif\n"
"}\n"
	).c_str());
}

void t_shaders::f_phong_shader(t_shader& a_shader, const std::string& a_defines)
{
	a_shader.f_create(GL_FRAGMENT_SHADER);
	a_shader.f_compile((a_defines + "\n"
"#ifdef GL_ES\n"
"precision mediump float;\n"
"precision mediump int;\n"
"#endif\n"
"\n"
"uniform vec4 color;\n"
"varying vec3 varyingNormal;\n"
"#ifdef USE_TEXTURE\n"
"uniform sampler2D diffuse;\n"
"varying vec2 varyingTexcoord;\n"
"#else\n"
"uniform vec4 diffuse;\n"
"#endif\n"
"uniform vec4 specular;\n"
"uniform float shininess;\n"
"const vec3 light = vec3(0.40824829046, 0.40824829046, 0.81649658092);\n"
"\n"
"void main()\n"
"{\n"
"#ifdef USE_TEXTURE\n"
"	vec4 sample = texture2D(diffuse, varyingTexcoord);\n"
"	if (sample.a < 0.5) discard;\n"
"#endif\n"
"	vec3 normal = normalize(varyingNormal);\n"
"//	vec3 light = normalize(vec3(5.0, 5.0, 10.0));\n"
"//	vec3 r = reflect(-light, normal);\n"
"//	vec3 eye = vec3(0.0, 0.0, 1.0);\n"
"	float nl = dot(normal, light);\n"
"	float r = max(2.0 * nl * normal.z - light.z, 0.0);\n"
"	float s = r / (shininess - shininess * r + r);\n"
"#ifdef USE_TEXTURE\n"
"	gl_FragColor = color + sample\n"
"#else\n"
"	gl_FragColor = color + diffuse\n"
"#endif\n"
"//	* max(dot(normal, light), 0.0) + specular * pow(max(dot(r, eye), 0.0), shininess);\n"
"	* max(nl, 0.0) + specular * s;\n"
"}\n"
	).c_str());
}

void t_shaders::f_skin_shader(t_shader& a_shader, size_t a_joints, size_t a_weights, const std::string& a_defines)
{
	a_shader.f_create(GL_VERTEX_SHADER);
	std::string joint;
	std::string weight;
	std::string vertex;
	for (size_t i = 1; i < a_weights; ++i) {
		auto si = std::to_string(i);
		joint += "attribute float joint" + si + ";\n";
		weight += "attribute float weight" + si + ";\n";
		vertex += "if (weight" + si + " > 0.0) { vm += vertexMatrices[int(joint" + si + ")] * weight" + si + ";";
	}
	for (size_t i = 1; i < a_weights; ++i) vertex += " }";
	a_shader.f_compile((a_defines + "\n"
"mat3 invert4to3(const in mat4 m) {\n"
"	//float d = m[0][0] * m[1][1] * m[2][2] + m[1][0] * m[2][1] * m[0][2] + m[2][0] * m[0][1] * m[1][2] - m[0][0] * m[2][1] * m[1][2] - m[2][0] * m[1][1] * m[0][2] - m[1][0] * m[0][1] * m[2][2];\n"
"	return mat3(\n"
"		m[1][1] * m[2][2] - m[1][2] * m[2][1],\n"
"		m[0][2] * m[2][1] - m[0][1] * m[2][2],\n"
"		m[0][1] * m[1][2] - m[0][2] * m[1][1],\n"
"		m[1][2] * m[2][0] - m[1][0] * m[2][2],\n"
"		m[0][0] * m[2][2] - m[0][2] * m[2][0],\n"
"		m[0][2] * m[1][0] - m[0][0] * m[1][2],\n"
"		m[1][0] * m[2][1] - m[1][1] * m[2][0],\n"
"		m[0][1] * m[2][0] - m[0][0] * m[2][1],\n"
"		m[0][0] * m[1][1] - m[0][1] * m[1][0]\n"
"	);\n"
"}\n"
"\n"
"uniform mat4 projection;\n"
"uniform mat4 vertexMatrices[" + std::to_string(a_joints) + "];\n"
"attribute vec3 vertex;\n"
"attribute vec3 normal;\n"
"attribute float joint0;\n"
+ joint +
"attribute float weight0;\n"
+ weight +
"varying vec3 varyingNormal;\n"
"#ifdef USE_TEXTURE\n"
"attribute vec2 texcoord;\n"
"varying vec2 varyingTexcoord;\n"
"#endif\n"
"\n"
"void main()\n"
"{\n"
"	vec4 v = vec4(vertex, 1.0);\n"
"	mat4 vm = vertexMatrices[int(joint0)] * weight0;" + vertex + "\n"
"	gl_Position = projection * (vm * v);\n"
"	varyingNormal = normalize(normal * invert4to3(vm));\n"
"#ifdef USE_TEXTURE\n"
"	varyingTexcoord = vec2(texcoord.x, 1.0 - texcoord.y);\n"
"#endif\n"
"}\n"
	).c_str());
}

void f_load_rgba(const std::wstring& a_path, size_t& a_width, size_t& a_height, std::vector<unsigned char>& a_data)
{
	std::string error;
	auto surface0 = IMG_Load(f_convert(a_path).c_str());
	if (surface0 == NULL) {
		error = std::string("IMG_Load Error: ") + IMG_GetError();
	} else {
		auto surface1 = SDL_ConvertSurfaceFormat(surface0, SDL_PIXELFORMAT_ARGB8888, 0);
		if (surface1 == NULL) {
			error = std::string("SDL_ConvertSurfaceFormat Error: ") + SDL_GetError();
		} else {
			a_width = surface0->w;
			a_height = surface0->h;
			a_data.resize(a_width * a_height * 4);
			if (SDL_ConvertPixels(a_width, a_height, surface1->format->format, surface1->pixels, surface1->pitch, SDL_PIXELFORMAT_ABGR8888, a_data.data(), a_width * 4) != 0) error = std::string("SDL_ConvertPixels Error: ") + SDL_GetError();
			SDL_FreeSurface(surface1);
		}
		SDL_FreeSurface(surface0);
	}
	if (!error.empty()) throw std::runtime_error(error);
}

void t_image::f_create(const std::wstring& a_path, GLint a_format)
{
	size_t width;
	size_t height;
	std::vector<unsigned char> data;
	f_load_rgba(a_path, width, height, data);
	v_width = width;
	v_height = height;
	t_shader vshader;
	vshader.f_create(GL_VERTEX_SHADER);
	vshader.f_compile(
"uniform mat4 projection;\n"
"uniform mat4 vertexMatrix;\n"
"attribute vec3 vertex;\n"
"attribute vec2 texcoord;\n"
"varying vec2 varyingTexcoord;\n"
"\n"
"void main()\n"
"{\n"
"gl_Position = projection * (vertexMatrix * vec4(vertex, 1.0));\n"
"varyingTexcoord = texcoord;\n"
"}\n"
	);
	t_shader fshader;
	fshader.f_create(GL_FRAGMENT_SHADER);
	fshader.f_compile(
"#ifdef GL_ES\n"
"precision mediump float;\n"
"precision mediump int;\n"
"#endif\n"
"\n"
"uniform sampler2D color;\n"
"varying vec2 varyingTexcoord;\n"
"\n"
"void main()\n"
"{\n"
"vec4 sample = texture2D(color, varyingTexcoord);\n"
"if (sample.a < 0.5) discard;\n"
"gl_FragColor = sample;\n"
"}\n"
	);
	v_program.f_create();
	v_program.f_attach(vshader);
	v_program.f_attach(fshader);
	v_program.f_link();
	v_projection = v_program.f_get_uniform_location("projection");
	v_vertex_matrix = v_program.f_get_uniform_location("vertexMatrix");
	v_vertex = v_program.f_get_attrib_location("vertex");
	v_texcoord = v_program.f_get_attrib_location("texcoord");
	v_color = v_program.f_get_uniform_location("color");
	v_texture.f_create();
	f_active_texture(GL_TEXTURE0);
	f_bind_texture(GL_TEXTURE_2D, v_texture);
	f_tex_image2d(GL_TEXTURE_2D, 0, a_format, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
	f_tex_parameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	f_tex_parameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	f_bind_texture(GL_TEXTURE_2D, 0);
	v_vertices.f_create();
}

void t_font::f_create(GLsizei a_width, GLsizei a_height, const GLvoid* a_data, size_t a_count, float a_unit)
{
	v_unit = a_unit;
	t_shader vshader;
	vshader.f_create(GL_VERTEX_SHADER);
	vshader.f_compile(
"uniform mat4 projection;\n"
"uniform mat4 vertexMatrix;\n"
"uniform float offset;\n"
"attribute vec3 vertex;\n"
"varying vec2 varyingTexcoord;\n"
"\n"
"void main()\n"
"{\n"
"gl_Position = projection * (vertexMatrix * vec4(offset + vertex.x, vertex.y, 0.0, 1.0));\n"
"varyingTexcoord = vec2(vertex.z, 1.0 - vertex.y);\n"
"}\n"
	);
	t_shader fshader;
	fshader.f_create(GL_FRAGMENT_SHADER);
	fshader.f_compile(
"#ifdef GL_ES\n"
"precision mediump float;\n"
"precision mediump int;\n"
"#endif\n"
"\n"
"uniform sampler2D color;\n"
"varying vec2 varyingTexcoord;\n"
"\n"
"void main()\n"
"{\n"
"vec4 sample = texture2D(color, varyingTexcoord);\n"
"if (sample.a < 0.5) discard;\n"
"gl_FragColor = sample;\n"
"}\n"
	);
	v_program.f_create();
	v_program.f_attach(vshader);
	v_program.f_attach(fshader);
	v_program.f_link();
	v_projection = v_program.f_get_uniform_location("projection");
	v_vertex_matrix = v_program.f_get_uniform_location("vertexMatrix");
	v_offset = v_program.f_get_uniform_location("offset");
	v_vertex = v_program.f_get_attrib_location("vertex");
	v_color = v_program.f_get_uniform_location("color");
	v_texture.f_create();
	f_active_texture(GL_TEXTURE0);
	f_bind_texture(GL_TEXTURE_2D, v_texture);
	f_tex_image2d(GL_TEXTURE_2D, 0, GL_RGBA, a_width, a_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, a_data);
	f_tex_parameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	f_tex_parameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	f_bind_texture(GL_TEXTURE_2D, 0);
	std::vector<float> array(a_count * 12);
	float division = 1.0f / a_count;
	for (size_t i = 0; i < a_count; ++i) {
		size_t j = i * 12;
		array[j] = array[j + 3] = array[j + 4] = array[j + 10] = 0.0f;
		array[j + 6] = array[j + 9] = v_unit;
		array[j + 1] = array[j + 7] = 1.0f;
		array[j + 2] = array[j + 5] = i * division;
		array[j + 8] = array[j + 11] = (i + 1) * division;
	}
	v_vertices.f_create();
	f_bind_buffer(GL_ARRAY_BUFFER, v_vertices);
	f_buffer_data(GL_ARRAY_BUFFER, array.size() * sizeof(float), array.data(), GL_STATIC_DRAW);
	f_bind_buffer(GL_ARRAY_BUFFER, 0);
}

void t_font::f_create(const std::wstring& a_path)
{
	::t_font font(f_convert(a_path).c_str(), 32);
	std::string text(128, ' ');
	for (size_t i = 32; i < 127; ++i) text[i] = i;
	SDL_Color color = {255, 255, 255, 255};
	auto surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
	if (surface == NULL) throw std::runtime_error(std::string("TTF_RenderUTF8_Blended Error: ") + TTF_GetError());
	int width = surface->w;
	int height = surface->h;
	std::vector<char> data(width * height * 4);
	if (SDL_ConvertPixels(width, height, surface->format->format, surface->pixels, surface->pitch, SDL_PIXELFORMAT_ABGR8888, data.data(), width * 4) != 0) throw std::runtime_error(std::string("SDL_ConvertPixels Error: ") + SDL_GetError());
	SDL_FreeSurface(surface);
	f_create(width, height, data.data(), 128, width / (128.0f * height));
}

void t_font::operator()(const t_matrix4f& a_projection, const t_matrix4f& a_vertex, const std::wstring& a_text)
{
	f_use_program(v_program);
	v_projection.f_matrix4(a_projection.v_array);
	v_vertex_matrix.f_matrix4(a_vertex.v_array);
	f_enable_vertex_attrib_array(v_vertex);
	f_bind_buffer(GL_ARRAY_BUFFER, v_vertices);
	f_vertex_attrib_pointer(v_vertex, 3, GL_FLOAT, false, 0, 0);
	f_active_texture(GL_TEXTURE0);
	f_bind_texture(GL_TEXTURE_2D, v_texture);
	v_color.f_uniform(0);
	for (size_t i = 0; i < a_text.size(); ++i) {
		v_offset.f_uniform(i * v_unit);
		f_draw_arrays(GL_TRIANGLE_STRIP, a_text[i] * 4, 4);
	}
	f_use_program(0);
	f_bind_buffer(GL_ARRAY_BUFFER, 0);
	f_bind_texture(GL_TEXTURE_2D, 0);
}

}
