#ifndef BALL_H
#define BALL_H

#include "placement.h"

std::vector<std::array<std::tuple<t_vector3f, t_vector3f>, 3>> f_circle(size_t a_n, float a_sign = 1.0f);
void f_divide(size_t a_n, const t_vector3f& a_a, const t_vector3f& a_b, const t_vector3f& a_c, std::vector<std::array<std::tuple<t_vector3f, t_vector3f>, 3>>& a_triangles);
std::vector<std::array<std::tuple<t_vector3f, t_vector3f>, 3>> f_sphere(size_t a_n);
std::unique_ptr<t_node> f_node(t_document& a_document, gl::t_shaders& a_shaders, const std::vector<std::array<std::tuple<t_vector3f, t_vector3f>, 3>>& a_triangles, const std::wstring& a_material);

const float G = 9.8f / (64.0f * 64.0f);

inline float f_projected_time_for_y(float a_py, float a_vy, float a_y, float a_sign)
{
	float a = a_vy * a_vy + 2.0f * G * (a_py - a_y);
	return a < 0.0f ? NAN : (a_vy + a_sign * sqrt(a)) / G;
}

struct t_player;
struct t_stage;

struct t_ball
{
	struct t_record
	{
		t_vector3f v_position{0.0f, 0.0f, 0.0f};
		t_vector3f v_velocity{0.0f, 0.0f, 0.0f};
		t_vector3f v_spin{0.0f, 0.0f, 0.0f};
	};

	static constexpr float c_radius = 0.0625f;
	static std::array<float, 3> v_rally;

	t_stage& v_stage;
	t_vector3f v_position{0.0f, 0.0f, 0.0f};
	t_vector3f v_velocity{0.0f, 0.0f, 0.0f};
	t_vector3f v_spin{0.0f, 0.0f, 0.0f};
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
		auto d = t_vector3f(a_dx, 0.0f, a_dz).f_normalized();
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
	float f_projected_y_in(float a_t) const
	{
		return v_position.v_y + (v_velocity.v_y - 0.5f * G * a_t) * a_t;
	}
	void f_record(t_record& a_to) const
	{
		a_to.v_position = v_position;
		a_to.v_velocity = v_velocity;
		a_to.v_spin = v_spin;
	}
	void f_replay(const t_record& a_from)
	{
		v_position = a_from.v_position;
		v_velocity = a_from.v_velocity;
		v_spin = a_from.v_spin;
	}
};

struct t_mark
{
	struct t_record
	{
		size_t v_duration = 0;
		float v_stretch = 1.0f;
		t_placement v_placement;
	};

	static constexpr float c_radius = 0.0625f;

	size_t v_duration = 0;
	float v_stretch = 1.0f;
	std::unique_ptr<t_node> v_node;
	t_placement* v_placement;
	t_scale* v_scale;

	t_mark(t_stage& a_stage, const std::wstring& a_shadow);
	void f_setup();
	void f_step()
	{
		if (v_duration > 0) --v_duration;
	}
	void f_mark(const t_ball& a_ball);
	void f_record(t_record& a_to) const
	{
		a_to.v_duration = v_duration;
		a_to.v_stretch = v_stretch;
		a_to.v_placement = *v_placement;
	}
	void f_replay(const t_record& a_from)
	{
		v_duration = a_from.v_duration;
		v_stretch = a_from.v_stretch;
		*v_placement = a_from.v_placement;
	}
};

#endif
