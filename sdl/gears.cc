#include <array>
#include <xanadu/transform>
#include <gl/core.h>

#include "sdl_core.h"

using namespace xanadu;
using namespace gl;

void f_build(t_program& a_program, const GLchar* a_vshader, const GLchar* a_fshader)
{
	a_program.f_create();
	t_shader vs;
	vs.f_create(GL_VERTEX_SHADER);
	vs.f_compile(a_vshader);
	t_shader fs;
	fs.f_create(GL_FRAGMENT_SHADER);
	fs.f_compile(a_fshader);
	a_program.f_attach(vs);
	a_program.f_attach(fs);
	a_program.f_link();
}

class t_vertices
{
	static void f_transfer(const std::vector<float>& a_floats, GLuint a_buffer)
	{
		glBindBuffer(GL_ARRAY_BUFFER, a_buffer);
		glBufferData(GL_ARRAY_BUFFER, a_floats.size() * sizeof(float), &a_floats[0], GL_STATIC_DRAW);
	}

	float v_normal[3];
	std::vector<float> v_vertices;
	std::vector<float> v_normals;

public:
	t_vertices() : v_normal{0.0, 0.0, 1.0}
	{
	}
	void f_vertex(float a_x, float a_y, float a_z)
	{
		v_vertices.push_back(a_x);
		v_vertices.push_back(a_y);
		v_vertices.push_back(a_z);
		v_normals.push_back(v_normal[0]);
		v_normals.push_back(v_normal[1]);
		v_normals.push_back(v_normal[2]);
	}
	void f_normal(float a_x, float a_y, float a_z)
	{
		v_normal[0] = a_x;
		v_normal[1] = a_y;
		v_normal[2] = a_z;
	}
	size_t f_vertices(GLuint a_buffer)
	{
		f_transfer(v_vertices, a_buffer);
		return v_vertices.size() / 3;
	}
	size_t f_normals(GLuint a_buffer)
	{
		f_transfer(v_normals, a_buffer);
		return v_normals.size() / 3;
	}
};

class t_gear
{
	// flat shading + uniform color + uniform mvp
	const char* FACE_VSHADER =
"attribute vec3 vertex;\n"
"uniform   mat4 mvp;\n"
"\n"
"void main()\n"
"{\n"
"	gl_Position = mvp * vec4(vertex, 1.0);\n"
"}\n";

	const char* FACE_FSHADER =
"#ifdef GL_ES\n"
"precision mediump float;\n"
"precision mediump int;\n"
"#endif\n"
"\n"
"uniform vec4 color;\n"
"\n"
"void main()\n"
"{\n"
"	gl_FragColor = color;\n"
"}\n";

	// flat shading - each normal across polygon is constant
	// per-vertex normal + uniform color + uniform mvp
	const char* OUTWARD_VSHADER =
"attribute vec3 vertex;\n"
"attribute vec3 normal;\n"
"uniform   vec4 color;\n"
"uniform   mat3 nm;\n"
"uniform   mat4 mvp;\n"
"varying   vec4 varying_color;\n"
"\n"
"void main()\n"
"{\n"
"	vec4 ambient        = vec4(0.2, 0.2, 0.2, 1.0);\n"
"	vec3 light_position = vec3(5.0, 5.0, 10.0);\n"
"	light_position      = normalize(light_position);\n"
"	vec3 nm_normal      = normalize(nm * normal);\n"
"\n"
"	float ndotlp = dot(nm_normal, light_position);\n"
"	if(ndotlp > 0.0) {\n"
"		vec4 diffuse  = vec4(ndotlp, ndotlp, ndotlp, 0.0);\n"
"		varying_color = color * (ambient + diffuse);\n"
"	} else {\n"
"		varying_color = color * ambient;\n"
"	}\n"
"	gl_Position = mvp * vec4(vertex, 1.0);\n"
"}\n";

	const char* OUTWARD_FSHADER =
"#ifdef GL_ES\n"
"precision mediump float;\n"
"precision mediump int;\n"
"#endif\n"
"\n"
"varying vec4 varying_color;\n"
"\n"
"void main()\n"
"{\n"
"	gl_FragColor = varying_color;\n"
"}\n";

	// smooth shading + per-vertex normal + uniform color + uniform mvp
	const char* CYLINDER_VSHADER =
"attribute vec3 vertex;\n"
"attribute vec3 normal;\n"
"uniform   mat3 nm;\n"
"uniform   mat4 mvp;\n"
"varying   vec4 varying_normal;\n"
"\n"
"void main()\n"
"{\n"
"	varying_normal = vec4(normalize(nm * normal), 1.0);\n"
"	gl_Position    = mvp * vec4(vertex, 1.0);\n"
"}\n";

	const char* CYLINDER_FSHADER =
"#ifdef GL_ES\n"
"precision mediump float;\n"
"precision mediump int;\n"
"#endif\n"
"\n"
"uniform mat3 nm;\n"
"uniform vec4 color;\n"
"varying vec4 varying_normal;\n"
"\n"
"void main()\n"
"{\n"
"	vec4 frag_color;\n"
"	vec4 ambient       = vec4(0.2, 0.2, 0.2, 1.0);\n"
"	vec3 light_position = vec3(5.0, 5.0, 10.0);\n"
"	light_position      = normalize(light_position);\n"
"\n"
"	float ndotlp  = dot(vec3(varying_normal), light_position);\n"
"	if(ndotlp > 0.0) {\n"
"		vec4 diffuse = vec4(ndotlp, ndotlp, ndotlp, 0.0);\n"
"		frag_color   = color * (ambient + diffuse);\n"
"	} else {\n"
"		frag_color = color * ambient;\n"
"	}\n"
"	gl_FragColor = frag_color;\n"
"}\n";

	static std::array<double, 4> f_angle(size_t a_i, size_t a_teeth)
	{
		double angle = a_i * 2.0 * M_PI / a_teeth;
		double da = 2.0 * M_PI / a_teeth / 4.0;
		return std::array<double, 4>{angle, angle + 1.0 * da, angle + 2.0 * da, angle + 3.0 * da};
	}

	t_vector4f v_color;
	t_buffer v_front_vertices;
	size_t v_front_vertices_count;
	t_buffer v_front_teeth_vertices;
	size_t v_front_teeth_vertices_count;
	t_buffer v_back_vertices;
	size_t v_back_vertices_count;
	t_buffer v_back_teeth_vertices;
	size_t v_back_teeth_vertices_count;
	t_buffer v_outward_vertices;
	size_t v_outward_vertices_count;
	t_buffer v_outward_normals;
	size_t v_outward_normals_count;
	t_buffer v_cylinder_vertices;
	size_t v_cylinder_vertices_count;
	t_buffer v_cylinder_normals;
	size_t v_cylinder_normals_count;
	// face shaders
	t_program v_face_program;
	GLint v_face_attribute_vertex;
	t_uniform_location v_face_uniform_color;
	t_uniform_location v_face_uniform_mvp;
	// outward teeth shaders
	t_program v_outward_program;
	GLint v_outward_attribute_vertex;
	GLint v_outward_attribute_normal;
	t_uniform_location v_outward_uniform_color;
	t_uniform_location v_outward_uniform_nm;
	t_uniform_location v_outward_uniform_mvp;
	// cylinder shaders
	t_program v_cylinder_program;
	GLint v_cylinder_attribute_vertex;
	GLint v_cylinder_attribute_normal;
	t_uniform_location v_cylinder_uniform_color;
	t_uniform_location v_cylinder_uniform_nm;
	t_uniform_location v_cylinder_uniform_mvp;

	//  Generate a gear wheel.
	//  Input:  inner - radius of hole at center
	//          outer - radius at center of teeth
	//          width - width of gear
	//          teeth - number of teeth
	//          depth - depth of tooth
	void f_generate(double a_inner, double a_outer, double a_width, size_t a_teeth, double a_depth)
	{
		double r0 = a_inner;
		double r1 = a_outer - a_depth / 2.0;
		double r2 = a_outer + a_depth / 2.0;
		double dz = 0.5 * a_width;
		{
			// draw front face
			// GL_TRIANGLE_STRIP
			t_vertices vertices;
			for (size_t i = 0; i < a_teeth; ++i) {
				auto as = f_angle(i, a_teeth);
				vertices.f_vertex(r0 * std::cos(as[0]), r0 * std::sin(as[0]), dz);
				vertices.f_vertex(r1 * std::cos(as[0]), r1 * std::sin(as[0]), dz);
				vertices.f_vertex(r0 * std::cos(as[0]), r0 * std::sin(as[0]), dz);
				vertices.f_vertex(r1 * std::cos(as[3]), r1 * std::sin(as[3]), dz);
			}
			vertices.f_vertex(r0 * std::cos(0.0), r0 * std::sin(0.0), dz);
			vertices.f_vertex(r1 * std::cos(0.0), r1 * std::sin(0.0), dz);
			v_front_vertices.f_create();
			v_front_vertices_count = vertices.f_vertices(v_front_vertices);
		}
		{
			// draw front sides of teeth
			// GL_TRIANGLES
			t_vertices vertices;
			for (size_t i = 0; i < a_teeth; ++i) {
				auto as = f_angle(i, a_teeth);
				vertices.f_vertex(r1 * std::cos(as[0]), r1 * std::sin(as[0]), dz);   // 0
				vertices.f_vertex(r2 * std::cos(as[1]), r2 * std::sin(as[1]), dz);   // 1
				vertices.f_vertex(r2 * std::cos(as[2]), r2 * std::sin(as[2]), dz);   // 2
				vertices.f_vertex(r1 * std::cos(as[0]), r1 * std::sin(as[0]), dz);   // 0
				vertices.f_vertex(r2 * std::cos(as[2]), r2 * std::sin(as[2]), dz);   // 2
				vertices.f_vertex(r1 * std::cos(as[3]), r1 * std::sin(as[3]), dz);   // 3
			}
			v_front_teeth_vertices.f_create();
			v_front_teeth_vertices_count = vertices.f_vertices(v_front_teeth_vertices);
		}
		{
			// draw back face
			// GL_TRIANGLE_STRIP
			t_vertices vertices;
			for (size_t i = 0; i < a_teeth; ++i) {
				auto as = f_angle(i, a_teeth);
				vertices.f_vertex(r1 * std::cos(as[0]), r1 * std::sin(as[0]), -dz);
				vertices.f_vertex(r0 * std::cos(as[0]), r0 * std::sin(as[0]), -dz);
				vertices.f_vertex(r1 * std::cos(as[3]), r1 * std::sin(as[3]), -dz);
				vertices.f_vertex(r0 * std::cos(as[0]), r0 * std::sin(as[0]), -dz);
			}
			vertices.f_vertex(r1 * std::cos(0.0), r1 * std::sin(0.0), -dz);
			vertices.f_vertex(r0 * std::cos(0.0), r0 * std::sin(0.0), -dz);
			v_back_vertices.f_create();
			v_back_vertices_count = vertices.f_vertices(v_back_vertices);
		}
		{
			// draw back sides of teeth
			// GL_TRIANGLES
			t_vertices vertices;
			for (size_t i = 0; i < a_teeth; ++i) {
				auto as = f_angle(i, a_teeth);
				vertices.f_vertex(r1 * std::cos(as[3]), r1 * std::sin(as[3]), -dz);   // 0
				vertices.f_vertex(r2 * std::cos(as[2]), r2 * std::sin(as[2]), -dz);   // 1
				vertices.f_vertex(r2 * std::cos(as[1]), r2 * std::sin(as[1]), -dz);   // 2
				vertices.f_vertex(r1 * std::cos(as[3]), r1 * std::sin(as[3]), -dz);   // 0
				vertices.f_vertex(r2 * std::cos(as[1]), r2 * std::sin(as[1]), -dz);   // 2
				vertices.f_vertex(r1 * std::cos(as[0]), r1 * std::sin(as[0]), -dz);   // 3
			}
			v_back_teeth_vertices.f_create();
			v_back_teeth_vertices_count = vertices.f_vertices(v_back_teeth_vertices);
		}
		{
			// draw outward faces of teeth
			// GL_TRIANGLE_STRIP
			// repeated vertices are necessary to achieve flat shading in ES2
			t_vertices vertices;
			for (size_t i = 0; i < a_teeth; ++i) {
				auto as = f_angle(i, a_teeth);
				if (i > 0) {
					vertices.f_vertex(r1 * std::cos(as[0]), r1 * std::sin(as[0]), dz);
					vertices.f_vertex(r1 * std::cos(as[0]), r1 * std::sin(as[0]), -dz);
				}
				double u = r2 * std::cos(as[1]) - r1 * std::cos(as[0]);
				double v = r2 * std::sin(as[1]) - r1 * std::sin(as[0]);
				double l = std::sqrt(u * u + v * v);
				u /= l;
				v /= l;
				vertices.f_normal(v, -u, 0.0);
				vertices.f_vertex(r1 * std::cos(as[0]), r1 * std::sin(as[0]), dz);
				vertices.f_vertex(r1 * std::cos(as[0]), r1 * std::sin(as[0]), -dz);
				vertices.f_vertex(r2 * std::cos(as[1]), r2 * std::sin(as[1]), dz);
				vertices.f_vertex(r2 * std::cos(as[1]), r2 * std::sin(as[1]), -dz);
				vertices.f_normal(std::cos(as[0]), std::sin(as[0]), 0.0);
				vertices.f_vertex(r2 * std::cos(as[1]), r2 * std::sin(as[1]), dz);
				vertices.f_vertex(r2 * std::cos(as[1]), r2 * std::sin(as[1]), -dz);
				vertices.f_vertex(r2 * std::cos(as[2]), r2 * std::sin(as[2]), dz);
				vertices.f_vertex(r2 * std::cos(as[2]), r2 * std::sin(as[2]), -dz);
				u = r1 * std::cos(as[3]) - r2 * std::cos(as[2]);
				v = r1 * std::sin(as[3]) - r2 * std::sin(as[2]);
				vertices.f_normal(v, -u, 0.0);
				vertices.f_vertex(r2 * std::cos(as[2]), r2 * std::sin(as[2]), dz);
				vertices.f_vertex(r2 * std::cos(as[2]), r2 * std::sin(as[2]), -dz);
				vertices.f_vertex(r1 * std::cos(as[3]), r1 * std::sin(as[3]), dz);
				vertices.f_vertex(r1 * std::cos(as[3]), r1 * std::sin(as[3]), -dz);
				vertices.f_normal(std::cos(as[0]), std::sin(as[0]), 0.0);
				vertices.f_vertex(r1 * std::cos(as[3]), r1 * std::sin(as[3]), dz);
				vertices.f_vertex(r1 * std::cos(as[3]), r1 * std::sin(as[3]), -dz);
			}
			vertices.f_vertex(r1 * std::cos(0.0), r1 * std::sin(0.0), dz);
			vertices.f_vertex(r1 * std::cos(0.0), r1 * std::sin(0.0), -dz);
			v_outward_vertices.f_create();
			v_outward_vertices_count = vertices.f_vertices(v_outward_vertices);
			v_outward_normals.f_create();
			v_outward_normals_count = vertices.f_normals(v_outward_normals);
		}
		{
			// draw inside radius cylinder
			// GL_TRIANGLE_STRIP
			t_vertices vertices;
			for (size_t i = 0; i < a_teeth; ++i) {
				auto as = f_angle(i, a_teeth);
				vertices.f_normal(-std::cos(as[0]), -std::sin(as[0]), 0.0);
				vertices.f_vertex(r0 * std::cos(as[0]), r0 * std::sin(as[0]), -dz);
				vertices.f_vertex(r0 * std::cos(as[0]), r0 * std::sin(as[0]), dz);
			}
			vertices.f_normal(-std::cos(0.0), -std::sin(0.0), 0.0);
			vertices.f_vertex(r0 * std::cos(0.0), r0 * std::sin(0.0), -dz);
			vertices.f_vertex(r0 * std::cos(0.0), r0 * std::sin(0.0), dz);
			v_cylinder_vertices.f_create();
			v_cylinder_vertices_count = vertices.f_vertices(v_cylinder_vertices);
			v_cylinder_normals.f_create();
			v_cylinder_normals_count = vertices.f_normals(v_cylinder_normals);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	void f_load()
	{
		// face shaders
		f_build(v_face_program, FACE_VSHADER, FACE_FSHADER);
		v_face_attribute_vertex = v_face_program.f_get_attrib_location("vertex");
		v_face_uniform_color = v_face_program.f_get_uniform_location("color");
		v_face_uniform_mvp = v_face_program.f_get_uniform_location("mvp");

		// outward teeth shaders
		f_build(v_outward_program, OUTWARD_VSHADER, OUTWARD_FSHADER);
		v_outward_attribute_vertex = v_outward_program.f_get_attrib_location("vertex");
		v_outward_attribute_normal = v_outward_program.f_get_attrib_location("normal");
		v_outward_uniform_color = v_outward_program.f_get_uniform_location("color");
		v_outward_uniform_nm = v_outward_program.f_get_uniform_location("nm");
		v_outward_uniform_mvp = v_outward_program.f_get_uniform_location("mvp");

		// cylinder shaders
		f_build(v_cylinder_program, CYLINDER_VSHADER, CYLINDER_FSHADER);
		v_cylinder_attribute_vertex = v_cylinder_program.f_get_attrib_location("vertex");
		v_cylinder_attribute_normal = v_cylinder_program.f_get_attrib_location("normal");
		v_cylinder_uniform_color = v_cylinder_program.f_get_uniform_location("color");
		v_cylinder_uniform_nm = v_cylinder_program.f_get_uniform_location("nm");
		v_cylinder_uniform_mvp = v_cylinder_program.f_get_uniform_location("mvp");
	}

public:
	t_gear(const t_vector4f& a_color, double a_inner, double a_outer, double a_width, size_t a_teeth, double a_depth) : v_color(a_color)
	{
		f_generate(a_inner, a_outer, a_width, a_teeth, a_depth);
		f_load();
	}
	void f_draw(const t_matrix4f& a_projection, const t_matrix4f& a_viewing)
	{
		auto mvp = a_projection * a_viewing;
		auto nm = (~t_matrix3f(a_viewing)).f_transposition();
		auto light_position = t_vector3f(5.0, 5.0, 10.0).f_normalized();

		// front, back, front teeth and back teeth
		glUseProgram(v_face_program);
		glEnableVertexAttribArray(v_face_attribute_vertex);

		// compute color for flat shaded surface
		auto normal_front = (nm * t_vector3f(0.0, 0.0, 1.0)).f_normalized();
		t_vector4f color(0.2, 0.2, 0.2, 1.0);   // ambient
		auto ndotlp = normal_front * light_position;
		if (ndotlp > 0.0) color += t_vector4f(ndotlp, ndotlp, ndotlp, 0.0); // ambient + diffuse
		v_face_uniform_color.f_uniform(color.v_x * v_color.v_x, color.v_y * v_color.v_y, color.v_z * v_color.v_z, color.v_w * v_color.v_w); // color * (ambient + diffuse)
		v_face_uniform_mvp.f_matrix4(mvp.v_array);
		glBindBuffer(GL_ARRAY_BUFFER, v_front_vertices);
		glVertexAttribPointer(v_face_attribute_vertex, 3, GL_FLOAT, false, 0, 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, v_front_vertices_count);

		glBindBuffer(GL_ARRAY_BUFFER, v_front_teeth_vertices);
		glVertexAttribPointer(v_face_attribute_vertex, 3, GL_FLOAT, false, 0, 0);
		glDrawArrays(GL_TRIANGLES, 0, v_front_teeth_vertices_count);

		// compute color for flat shaded surface
		auto normal_back = (nm * t_vector3f(0.0, 0.0, -1.0)).f_normalized();
		color = t_vector4f(0.2, 0.2, 0.2, 1.0);   // reload ambient
		ndotlp = normal_back * light_position;
		if (ndotlp > 0.0) color += t_vector4f(ndotlp, ndotlp, ndotlp, 0.0); // ambient + diffuse
		v_face_uniform_color.f_uniform(color.v_x * v_color.v_x, color.v_y * v_color.v_y, color.v_z * v_color.v_z, color.v_w * v_color.v_w); // color * (ambient + diffuse)
		glBindBuffer(GL_ARRAY_BUFFER, v_back_vertices);
		glVertexAttribPointer(v_face_attribute_vertex, 3, GL_FLOAT, false, 0, 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, v_back_vertices_count);
		glBindBuffer(GL_ARRAY_BUFFER, v_back_teeth_vertices);
		glVertexAttribPointer(v_face_attribute_vertex, 3, GL_FLOAT, false, 0, 0);
		glDrawArrays(GL_TRIANGLES, 0, v_back_teeth_vertices_count);

		glDisableVertexAttribArray(v_face_attribute_vertex);

		// outward teeth
		glUseProgram(v_outward_program);
		glEnableVertexAttribArray(v_outward_attribute_vertex);
		glEnableVertexAttribArray(v_outward_attribute_normal);
		v_outward_uniform_color.f_uniform(v_color.v_x, v_color.v_y, v_color.v_z, v_color.v_w);
		v_outward_uniform_nm.f_matrix3(nm.v_array);
		v_outward_uniform_mvp.f_matrix4(mvp.v_array);
		glBindBuffer(GL_ARRAY_BUFFER, v_outward_vertices);
		glVertexAttribPointer(v_outward_attribute_vertex, 3, GL_FLOAT, false, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, v_outward_normals);
		glVertexAttribPointer(v_outward_attribute_normal, 3, GL_FLOAT, false, 0, 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, v_outward_vertices_count);
		glDisableVertexAttribArray(v_outward_attribute_normal);
		glDisableVertexAttribArray(v_outward_attribute_vertex);

		// cylinder
		glUseProgram(v_cylinder_program);
		glEnableVertexAttribArray(v_cylinder_attribute_vertex);
		glEnableVertexAttribArray(v_cylinder_attribute_normal);
		v_cylinder_uniform_color.f_uniform(v_color.v_x, v_color.v_y, v_color.v_z, v_color.v_w);
		v_cylinder_uniform_nm.f_matrix3(nm.v_array);
		v_cylinder_uniform_mvp.f_matrix4(mvp.v_array);
		glBindBuffer(GL_ARRAY_BUFFER, v_cylinder_vertices);
		glVertexAttribPointer(v_cylinder_attribute_vertex, 3, GL_FLOAT, false, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, v_cylinder_normals);
		glVertexAttribPointer(v_cylinder_attribute_normal, 3, GL_FLOAT, false, 0, 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, v_cylinder_vertices_count);
		glDisableVertexAttribArray(v_cylinder_attribute_normal);
		glDisableVertexAttribArray(v_cylinder_attribute_vertex);

		glUseProgram(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
};

t_matrix4f f_resize(int a_width, int a_height)
{
	glViewport(0, 0, a_width, a_height);
	float w = static_cast<float>(a_width) / a_height;
	float h = 1.0f;
	return f_frustum(-w, w, -h, h, 5.0f, 60.0f);
}

void f_loop(SDL_Window* a_window)
{
	SDL_GL_SetSwapInterval(1);
	t_gear gear1(t_vector4f(0.8, 0.1, 0.0, 1.0), 1.0, 4.0, 1.0, 20, 0.7);
	t_gear gear2(t_vector4f(0.0, 0.8, 0.2, 1.0), 0.5, 2.0, 2.0, 10, 0.7);
	t_gear gear3(t_vector4f(0.2, 0.2, 1.0, 1.0), 1.3, 2.0, 0.5, 10, 0.7);
	int width;
	int height;
	SDL_GetWindowSize(a_window, &width, &height);
	t_matrix4f projection = f_resize(width, height);
	t_tuple3f rotate(M_PI * 20.0 / 180.0, M_PI * 30.0 / 180.0, 0.0);
	float angle = 0.0;
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	SDL_Event event;
	while (true) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				return;
			case SDL_WINDOWEVENT:
				switch (event.window.event) {
				case SDL_WINDOWEVENT_RESIZED:
					projection = f_resize(event.window.data1, event.window.data2);
					break;
				}
				break;
			}
		}
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		t_matrix4f viewing = t_translate3f(0.0, 0.0, -40.0);
		viewing *= t_rotate3f(t_vector3f(1.0, 0.0, 0.0), rotate.v_x);
		viewing *= t_rotate3f(t_vector3f(0.0, 1.0, 0.0), rotate.v_y);
		viewing *= t_rotate3f(t_vector3f(0.0, 0.0, 1.0), rotate.v_z);
		{
			auto v = viewing * t_translate3f(-3.0, -2.0, 0.0);
			v *= t_rotate3f(t_vector3f(0.0, 0.0, 1.0), angle);
			gear1.f_draw(projection, v);
		}
		{
			auto v = viewing * t_translate3f(3.1, -2.0, 0.0);
			v *= t_rotate3f(t_vector3f(0.0, 0.0, 1.0), -2.0 * angle - M_PI * 9.0 / 180.0);
			gear2.f_draw(projection, v);
		}
		{
			auto v = viewing * t_translate3f(-3.1, 2.2, -1.8);
			v *= t_rotate3f(t_vector3f(1.0, 0.0, 0.0), M_PI * 0.5);
			v *= t_rotate3f(t_vector3f(0.0, 0.0, 1.0), 2.0 * angle - M_PI * 2.0 / 180.0);
			gear3.f_draw(projection, v);
		}
		SDL_GL_SwapWindow(a_window);
		angle += M_PI * 2.0 / 180.0;
	}
}

int main(int argc, char* argv[])
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	t_sdl sdl(SDL_INIT_VIDEO);
	t_window window("Gears", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	t_gl_context context(window);
	f_loop(window);
	return 0;
}
