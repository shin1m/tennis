#ifndef BALL_H
#define BALL_H

#include "placement.h"

std::vector<std::array<std::tuple<t_vector3f, t_vector3f>, 3>> f_circle(size_t a_n, float a_sign = 1.0);
void f_divide(size_t a_n, const t_vector3f& a_a, const t_vector3f& a_b, const t_vector3f& a_c, std::vector<std::array<std::tuple<t_vector3f, t_vector3f>, 3>>& a_triangles);
std::vector<std::array<std::tuple<t_vector3f, t_vector3f>, 3>> f_sphere(size_t a_n);
std::unique_ptr<t_node> f_node(t_document& a_document, gl::t_shaders& a_shaders, const std::vector<std::array<std::tuple<t_vector3f, t_vector3f>, 3>>& a_triangles, const std::wstring& a_material);

const float G = 9.8 / (64.0 * 64.0);

inline float f_projected_time_for_y(float a_py, float a_vy, float a_y, float a_sign)
{
	float a = a_vy * a_vy + 2.0 * G * (a_py - a_y);
	return a < 0.0 ? NAN : (a_vy + a_sign * sqrt(a)) / G;
}

struct t_player;
struct t_stage;

struct t_ball
{
	static constexpr float c_radius = 0.0625;
	static std::array<float, 3> v_rally;

	t_stage& v_stage;
	t_vector3f v_position{0.0, 0.0, 0.0};
	t_vector3f v_velocity{0.0, 0.0, 0.0};
	t_vector3f v_spin{0.0, 0.0, 0.0};
	std::unique_ptr<t_node> v_node = std::make_unique<t_node>();
	t_translate* v_translate;
	t_translate* v_body_translate;
	t_player* v_hitter = nullptr;
	std::array<float, 3> v_target;
	bool v_done = false;
	bool v_in = false;
	bool v_net = false;

	t_ball(t_stage& a_stage, const std::wstring& a_shadow, const std::wstring& a_body);
	void f_setup()
	{
		v_translate->v_value.v_x = v_position.v_x;
		v_body_translate->v_value.v_y = v_position.v_y;
		v_translate->v_value.v_z = v_position.v_z;
	}
	void f_netin_part(float a_x0, float a_y0, float a_x1, float a_y1);
	void f_netin();
	void f_emit_ace();
	void f_emit_miss();
	void f_emit_out();
	void f_emit_serve_air();
	void f_emit_bounce();
	void f_wall();
	void f_step();
	void f_set(t_player* a_hitter)
	{
		v_hitter = a_hitter;
		v_in = v_net = false;
	}
	void f_reset(float a_side, float a_x, float a_y, float a_z, bool a_serving = true);
	void f_hit(t_player* a_hitter)
	{
		if (v_done) return;
		if (v_target == v_rally) {
			f_set(a_hitter);
		} else {
			v_target = v_rally;
			v_hitter = a_hitter;
			f_emit_miss();
		}
	}
	bool f_serving() const
	{
		return v_target != v_rally;
	}
	void f_impact(float a_dx, float a_dz, float a_speed, float a_vy, const t_vector3f& a_spin)
	{
		auto d = t_vector3f(a_dx, 0.0, a_dz).f_normalized();
		v_velocity = t_vector3f(d.v_x * a_speed, a_vy, d.v_z * a_speed);
		v_spin = t_vector3f(-d.v_z * a_spin.v_x + d.v_x * a_spin.v_z, a_spin.v_y, d.v_x * a_spin.v_x + d.v_z * a_spin.v_z);
	}
	static void f_calculate_bounce(t_vector3f& a_velocity, t_vector3f& a_spin);
	void f_bounce()
	{
		f_calculate_bounce(v_velocity, v_spin);
	}
	float f_projected_time_for_y(float a_y, float a_sign) const
	{
		return ::f_projected_time_for_y(v_position.v_y, v_velocity.v_y, a_y, a_sign);
	}
};

struct t_mark
{
	static constexpr float c_radius = 0.0625;

	float v_duration = 0.0;
	float v_stretch = 1.0;
	std::unique_ptr<t_node> v_node;
	t_placement* v_placement;
	t_scale* v_scale;

	t_mark(t_stage& a_stage, const std::wstring& a_shadow);
	void f_setup();
	void f_step()
	{
		if (v_duration > 0.0) v_duration -= 1.0;
	}
	void f_mark(const t_ball& a_ball);
};

#endif
