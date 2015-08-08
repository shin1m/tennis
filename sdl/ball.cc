#include "ball.h"

#include "main.h"

std::vector<std::array<std::tuple<t_vector3f, t_vector3f>, 3>> f_circle(size_t a_n, float a_sign)
{
	t_vector3f normal(0.0, a_sign, 0.0);
	float unit = a_sign * 2.0 * M_PI / a_n;
	std::vector<std::array<std::tuple<t_vector3f, t_vector3f>, 3>> triangles(a_n);
	for (size_t i = 0; i < a_n; ++i) {
		float a = unit * i;
		float b = a + unit;
		triangles[i][0] = std::make_tuple(t_vector3f(0.0, 0.0, 0.0), normal);
		triangles[i][1] = std::make_tuple(t_vector3f(cos(a), 0.0, -sin(a)), normal);
		triangles[i][2] = std::make_tuple(t_vector3f(cos(b), 0.0, -sin(b)), normal);
	}
	return triangles;
}

void f_divide(size_t a_n, const t_vector3f& a_a, const t_vector3f& a_b, const t_vector3f& a_c, std::vector<std::array<std::tuple<t_vector3f, t_vector3f>, 3>>& a_triangles)
{
	if (a_n > 0) {
		--a_n;
		auto ab = (a_a + a_b).f_normalized();
		auto bc = (a_b + a_c).f_normalized();
		auto ca = (a_c + a_a).f_normalized();
		f_divide(a_n, a_a, ab, ca, a_triangles);
		f_divide(a_n, a_b, bc, ab, a_triangles);
		f_divide(a_n, a_c, ca, bc, a_triangles);
		f_divide(a_n, ab, bc, ca, a_triangles);
	} else {
		a_triangles.push_back(std::array<std::tuple<t_vector3f, t_vector3f>, 3>{std::make_tuple(a_a, a_a), std::make_tuple(a_b, a_b), std::make_tuple(a_c, a_c)});
	}
}

std::vector<std::array<std::tuple<t_vector3f, t_vector3f>, 3>> f_sphere(size_t a_n)
{
	std::vector<std::array<std::tuple<t_vector3f, t_vector3f>, 3>> triangles(a_n);
	f_divide(a_n, t_vector3f(1.0, 0.0, 0.0), t_vector3f(0.0, 1.0, 0.0), t_vector3f(0.0, 0.0, 1.0), triangles);
	f_divide(a_n, t_vector3f(0.0, 0.0, 1.0), t_vector3f(0.0, 1.0, 0.0), t_vector3f(-1.0, 0.0, 0.0), triangles);
	f_divide(a_n, t_vector3f(-1.0, 0.0, 0.0), t_vector3f(0.0, 1.0, 0.0), t_vector3f(0.0, 0.0, -1.0), triangles);
	f_divide(a_n, t_vector3f(0.0, 0.0, -1.0), t_vector3f(0.0, 1.0, 0.0), t_vector3f(1.0, 0.0, 0.0), triangles);
	f_divide(a_n, t_vector3f(1.0, 0.0, 0.0), t_vector3f(0.0, -1.0, 0.0), t_vector3f(0.0, 0.0, -1.0), triangles);
	f_divide(a_n, t_vector3f(0.0, 0.0, -1.0), t_vector3f(0.0, -1.0, 0.0), t_vector3f(-1.0, 0.0, 0.0), triangles);
	f_divide(a_n, t_vector3f(-1.0, 0.0, 0.0), t_vector3f(0.0, -1.0, 0.0), t_vector3f(0.0, 0.0, 1.0), triangles);
	f_divide(a_n, t_vector3f(0.0, 0.0, 1.0), t_vector3f(0.0, -1.0, 0.0), t_vector3f(1.0, 0.0, 0.0), triangles);
	return triangles;
}

std::unique_ptr<t_node> f_node(t_document& a_document, gl::t_shaders& a_shaders, const std::vector<std::array<std::tuple<t_vector3f, t_vector3f>, 3>>& a_triangles, const std::wstring& a_material)
{
	std::vector<float> array0(a_triangles.size() * 9);
	std::vector<float> array1(a_triangles.size() * 9);
	for (size_t i = 0; i < a_triangles.size(); ++i) {
		const auto& triangle = a_triangles[i];
		for (size_t j = 0; j < 3; ++j) {
			size_t k = i * 9 + j * 3;
			const auto& vertex = std::get<0>(triangle[j]);
			const auto& normal = std::get<1>(triangle[j]);
			array0[k] = vertex.v_x;
			array0[k + 1] = vertex.v_y;
			array0[k + 2] = vertex.v_z;
			array1[k] = normal.v_x;
			array1[k + 1] = normal.v_y;
			array1[k + 2] = normal.v_z;
		}
	}
	auto mesh = std::make_unique<t_mesh>();
	auto triangles = std::make_unique<t_mesh_primitive>(GL_TRIANGLES, 3);
	triangles->f_create(a_triangles.size(), L"Symbol", array0, array1, std::map<std::tuple<std::wstring, size_t>, std::vector<float>>());
	mesh->v_primitives.push_back(std::move(triangles));
	auto node = std::make_unique<t_node>();
	auto geometry = std::make_unique<t_instance_geometry>(nullptr);
	geometry->f_create(a_document, a_shaders, mesh.get(), std::map<std::wstring, std::wstring>{{L"Symbol", a_material}});
	a_document.v_library_geometries.push_back(std::move(mesh));
	node->v_geometries.push_back(std::move(geometry));
	return node;
}

constexpr float t_ball::c_radius;
std::array<float, 3> t_ball::v_rally{-(13 * 12 + 6) * 0.0254, (13 * 12 + 6) * 0.0254, -39 * 12 * 0.0254};

t_ball::t_ball(t_stage& a_stage, const std::wstring& a_shadow, const std::wstring& a_body) : v_stage(a_stage)
{
	auto translate = std::make_unique<t_translate>(0.0, 0.0, 0.0);
	v_translate = translate.get();
	v_node->v_transforms.push_back(std::move(translate));
	auto shadow = f_node(a_stage.v_scene, a_stage.v_main.v_shaders, f_circle(8), a_shadow);
	shadow->v_transforms.push_back(std::make_unique<t_translate>(0.0, 1.0 / 64.0, 0.0));
	shadow->v_transforms.push_back(std::make_unique<t_scale>(c_radius, 1.0, c_radius));
	v_node->v_nodes.push_back(std::move(shadow));
	auto body = f_node(a_stage.v_scene, a_stage.v_main.v_shaders, f_sphere(2), a_body);
	auto body_translate = std::make_unique<t_translate>(0.0, 0.0, 0.0);
	v_body_translate = body_translate.get();
	body->v_transforms.push_back(std::move(body_translate));
	body->v_transforms.push_back(std::make_unique<t_scale>(c_radius, c_radius, c_radius));
	v_node->v_nodes.push_back(std::move(body));
	a_stage.v_scene.v_scene.v_instance_visual_scene.v_scene->v_instance_nodes.push_back(v_node.get());
}

void t_ball::f_netin_part(float a_x0, float a_y0, float a_x1, float a_y1)
{
	auto e = t_vector2f(a_x1 - a_x0, a_y1 - a_y0).f_normalized();
	t_vector2f d(v_position.v_x - a_x0, v_position.v_y - a_y0);
	float y = t_vector2f(-e.v_y, e.v_x) * d;
	if (y > c_radius) return;
	if (y < 0.0) {
		v_position.v_z = v_velocity.v_z < 0.0 ? c_radius : -c_radius;
		v_velocity.v_z *= -1.0;
		v_velocity.f_scale(0.125);
		v_stage.f_ball_net();
	} else {
		float x = e * d;
		v_position.v_x = a_x0 + e.v_x * x - e.v_y * c_radius;
		v_position.v_y = a_y0 + e.v_y * x + e.v_x * c_radius;
		if (v_velocity.v_z < 0.0) {
			if (v_position.v_z < 0.0) v_position.v_z = 0.0;
		} else {
			if (v_position.v_z > 0.0) v_position.v_z = 0.0;
		}
		t_vector3f v(e.v_x, e.v_y, 0.0);
		t_vector3f p(d.v_x, d.v_y, v_position.v_z);
		auto n = p ^ v;
		n.f_normalize();
		auto m = v ^ n;
		m.f_normalize();
		float vv = v * v_velocity * 0.375;
		float vn = n * v_velocity * 0.375;
		float vm = m * v_velocity * 0.0;
		v_velocity = v * vv + n * vn + m * vm;
		v_stage.f_ball_chip();
	}
	v_net = true;
}

void t_ball::f_netin()
{
	if (v_position.v_x < -21 * 12 * 0.0254) return;
	if (v_position.v_x < -0.0254)
		f_netin_part(-21 * 12 * 0.0254, (3 * 12 + 6) * 0.0254, -0.0254, 3 * 12 * 0.0254);
	else if (v_position.v_x < 0.0254)
		f_netin_part(-0.0254, 3 * 12 * 0.0254, 0.0254, 3 * 12 * 0.0254);
	else if (v_position.v_x < 21 * 12 * 0.0254)
		f_netin_part(0.0254, 3 * 12 * 0.0254, 21 * 12 * 0.0254, (3 * 12 + 6) * 0.0254);
}

void t_ball::f_emit_ace()
{
	v_done = true;
	v_stage.f_ball_ace();
}

void t_ball::f_emit_miss()
{
	v_done = true;
	v_stage.f_ball_miss();
}

void t_ball::f_emit_out()
{
	v_done = true;
	v_stage.f_ball_out();
}

void t_ball::f_emit_serve_air()
{
	v_done = true;
	v_stage.f_ball_serve_air();
}

void t_ball::f_emit_bounce()
{
	v_stage.f_ball_bounce();
}

void t_ball::f_wall()
{
	if (v_done) return;
	if (v_in)
		f_emit_ace();
	else
		f_emit_out();
}

void t_ball::f_step()
{
	auto last = v_position;
	v_position = last + v_velocity;
	v_position.v_y -= 0.5 * G;
	v_velocity.v_y -= G;
	v_velocity += (v_spin ^ v_velocity) * (1.0 / 1500.0);
	v_velocity *= 0.999;
	v_spin *= 0.99;
	if (v_position.v_y - c_radius <= 0.0) {
		v_position.v_y = c_radius;
		f_bounce();
		if (!v_done) {
			if (v_in) {
				f_emit_ace();
			} else if (v_hitter == nullptr) {
				f_emit_serve_air();
			} else {
				float x = v_hitter->v_end * v_position.v_x;
				float z = v_hitter->v_end * v_position.v_z;
				if (z > 0.0) {
					f_emit_miss();
				} else if (x < v_target[0] || x > v_target[1] || z < v_target[2]) {
					f_emit_out();
				} else {
					v_in = true;
					if (f_serving() && v_net) {
						v_done = true;
						v_stage.f_ball_let();
					} else {
						v_stage.f_ball_in();
						v_target = v_rally;
					}
				}
			}
		}
		if (v_velocity.v_y > 1.0 / 64.0) f_emit_bounce();
	}
	if (v_position.v_x - c_radius <= -30 * 12 * 0.0254) {
		v_position.v_x = c_radius - 30 * 12 * 0.0254;
		v_velocity.v_x *= -0.5;
		f_wall();
		if (fabs(v_velocity.v_x) > 1.0 / 64.0) f_emit_bounce();
	} else if (v_position.v_x + c_radius >= 30 * 12 * 0.0254) {
		v_position.v_x = 30 * 12 * 0.0254 - c_radius;
		v_velocity.v_x *= -0.5;
		f_wall();
		if (fabs(v_velocity.v_x) > 1.0 / 64.0) f_emit_bounce();
	}
	if (v_position.v_z - c_radius <= -60 * 12 * 0.0254) {
		v_position.v_z = c_radius - 60 * 12 * 0.0254;
		v_velocity.v_z *= -0.5;
		f_wall();
		if (fabs(v_velocity.v_z) > 1.0 / 64.0) f_emit_bounce();
	} else if (v_position.v_z + c_radius >= 60 * 12 * 0.0254) {
		v_position.v_z = 60 * 12 * 0.0254 - c_radius;
		v_velocity.v_z *= -0.5;
		f_wall();
		if (fabs(v_velocity.v_z) > 1.0 / 64.0) f_emit_bounce();
	}
	if (v_velocity.v_z < 0.0) {
		if (last.v_z > c_radius && v_position.v_z <= c_radius) f_netin();
	} else {
		if (last.v_z < -c_radius && v_position.v_z >= -c_radius) f_netin();
	}
}

void t_ball::f_reset(float a_side, float a_x, float a_y, float a_z, bool a_serving)
{
	v_position = t_vector3f(a_x, a_y, a_z);
	v_velocity = t_vector3f(0.0, 0.0, 0.0);
	v_spin = t_vector3f(0.0, 0.0, 0.0);
	v_done = false;
	float x0 = 1 * 0.0254 * a_side;
	float x1 = -(13 * 12 + 6) * 0.0254 * a_side;
	v_target = a_serving ? std::array<float, 3>({x0 < x1 ? x0 : x1, x0 < x1 ? x1 : x0, -21 * 12 * 0.0254}) : v_rally;
	f_set(nullptr);
}

void t_ball::f_calculate_bounce(t_vector3f& a_velocity, t_vector3f& a_spin)
{
	float f = 0.0;
	const auto& v0 = a_velocity;
	const auto& w0 = a_spin;
	float v1x = v0.v_x + c_radius * w0.v_z;
	float v1z = v0.v_z - c_radius * w0.v_x;
	float e = 1.25 + v0.v_y * 10.0;
	if (e < 0.25) e = 0.0;
	if (e > 1.0) e = 1.0;
	float b = (e + 2.0 / 3.0) * c_radius;
	float w1x = (1.0 - e) * v0.v_z + b * w0.v_x + f * v1z;
	float w1z = (e - 1.0) * v0.v_x + b * w0.v_z - f * v1x;
	a_velocity.v_x = e * v1x - 3.0 / 5.0 * w1z;
	a_velocity.v_y = v0.v_y * -0.75;
	a_velocity.v_z = e * v1z + 3.0 / 5.0 * w1x;
	float d = 3.0 / 5.0 / c_radius;
	a_spin.v_x = w1x * d;
	a_spin.v_z = w1z * d;
}

constexpr float t_mark::c_radius;

t_mark::t_mark(t_stage& a_stage, const std::wstring& a_shadow)
{
	v_node = f_node(a_stage.v_scene, a_stage.v_main.v_shaders, f_circle(8), a_shadow);
	auto placement = std::make_unique<t_placement>();
	v_placement = placement.get();
	v_node->v_transforms.push_back(std::move(placement));
	auto scale = std::make_unique<t_scale>(c_radius, 1.0, c_radius);
	v_scale = scale.get();
	v_node->v_transforms.push_back(std::move(scale));
	a_stage.v_scene.v_scene.v_instance_visual_scene.v_scene->v_instance_nodes.push_back(v_node.get());
}

void t_mark::f_setup()
{
	v_placement->v_position.v_y = (v_duration > 0.0 ? 1.0 : -1.0) / 64.0;
	v_placement->v_valid = false;
	v_placement->f_validate();
	v_scale->v_value.v_z = c_radius * v_stretch;
}

void t_mark::f_mark(const t_ball& a_ball)
{
	v_duration = 2.0 * 64.0;
	v_placement->v_position.v_x = a_ball.v_position.v_x;
	v_placement->v_position.v_z = a_ball.v_position.v_z;
	v_placement->v_toward = t_vector3f(a_ball.v_velocity.v_x, 0.0, a_ball.v_velocity.v_z);
	v_placement->v_valid = false;
	v_stretch = 1.0 + v_placement->v_toward.f_length() * 8.0;
}
