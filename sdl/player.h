#ifndef PLAYER_H
#define PLAYER_H

#include "ball.h"

float f_reach_range(const t_vector3f& a_ball, const t_vector3f& a_velocity, const t_vector3f& a_player, float a_speed, float a_t0, float a_sign);
t_vector3f f_shot_direction(const t_vector3f& a_ball, float a_end, bool a_left, bool a_right, bool a_forward, bool a_backward);

struct t_player
{
	struct t_action
	{
		float v_start;
		float v_end;
		std::vector<std::tuple<std::unique_ptr<t_sampler::t_iterator>, size_t>> v_iterators;

		void f_initialize(t_document& a_scene, float a_start, float a_duration, const std::function<bool (const t_channel&)>& a_use = [](auto a_x) { return true; });
		void f_rewind();
		void f_forward(float a_t);
	};
	struct t_swing : t_action
	{
		float v_impact;
		float v_speed;
		t_vector3f v_spin;
		t_matrix4f v_spot;
		t_vector3f v_end_position;
		t_vector3f v_end_toward;

		void f_initialize(t_document& a_scene, t_node* a_skeleton, float a_start, float a_duration, float a_impact, float a_speed, const t_vector3f& a_spin = t_vector3f(0.0, 0.0, 0.0));
		void f_merge(t_player& a_player) const;
	};
	struct t_run : t_action
	{
		t_vector3f v_toward;

		void f_initialize(t_document& a_scene, t_node* a_skeleton, float a_start, float a_duration, const std::function<bool (const t_channel&)>& a_use);
	};
	struct t_shots
	{
		t_swing v_flat;
		t_swing v_topspin;
		t_swing v_lob;
		t_swing v_slice;
		t_swing v_reach;
	};
	struct t_readies
	{
		t_run v_stroke;
		t_run v_volley;
		t_run v_smash;
	};
	struct t_runs
	{
		t_run v_lowers[11];
		t_action v_stroke;
		t_action v_volley;
		t_action v_smash;
	};
	struct t_swings
	{
		t_shots v_stroke;
		t_shots v_volley;
		t_swing v_smash;
	};
	struct t_motion
	{
		t_action& v_action;
		float v_time;

		t_motion(t_action& a_action) : v_action(a_action)
		{
			f_rewind();
		}
		virtual ~t_motion() = default;
		void f_rewind()
		{
			v_time = v_action.v_start;
			v_action.f_rewind();
		}
		virtual void operator()();
	};
	struct t_run_motion : t_motion
	{
		static constexpr float c_duration = 4.0 / 64.0;

		t_vector3f v_toward;
		t_placement* v_placement;
		t_vector3f v_toward0;
		t_vector3f v_toward1;
		float v_duration;

		t_run_motion(t_run& a_run, const t_vector3f& a_toward, t_player& a_player);
		virtual void operator()();
	};
	struct t_state
	{
		std::function<void (t_player&)> v_enter;
		std::function<void (t_player&)> v_step;
		std::function<void (t_player&, t_swing t_shots::*)> v_do;
	};

	static const std::set<std::wstring> v_lowers;

	static bool f_is_not_root(const t_channel& a_x)
	{
		return a_x.v_node->v_id != L"Root";
	}
	static bool f_is_lower(const t_channel& a_x)
	{
		return v_lowers.count(a_x.v_node->v_id) > 0;
	}
	static bool f_is_upper(const t_channel& a_x)
	{
		return f_is_not_root(a_x) && !f_is_lower(a_x);
	}

	void f_load(t_document& a_scene, t_node* a_skeleton, const std::wstring& a_source);

	t_stage& v_stage;
	t_ball& v_ball;
	t_document v_scene;
	t_node* v_node;
	t_placement* v_placement;
	t_node* v_root;
	t_matrix4f v_root_transform;
	struct
	{
		struct
		{
			t_action v_set;
			t_action v_toss;
			t_shots v_swing;
		} v_serve;
		struct
		{
			t_run v_default;
			t_readies v_forehand;
			t_readies v_backhand;
		} v_ready;
		struct
		{
			float v_speed;
			t_run v_lower;
			t_action v_default;
			t_runs v_forehand;
			t_runs v_backhand;
		} v_run;
		struct
		{
			t_swings v_forehand;
			t_swings v_backhand;
			t_swing v_toss;
			t_swing v_toss_lob;
		} v_swing;
	} v_actions;
	float v_lefty;
	float v_smash_hand;
	std::unique_ptr<t_motion> v_motion;
	bool v_left;
	bool v_right;
	bool v_forward;
	bool v_backward;
	float v_end;
	const t_state* v_state;
	t_player* v_opponent;
	size_t v_point;
	size_t v_game;

	t_player(t_stage& a_stage, const std::wstring& a_model);
	void f_transit(const t_state& a_state)
	{
		v_state = &a_state;
		v_state->v_enter(*this);
	}
	void f_reset(float a_end, const t_state& a_state)
	{
		v_left = v_right = v_forward = v_backward = false;
		v_end = a_end;
		f_transit(a_state);
	}
	void f_setup()
	{
		v_placement->f_validate();
	}
	t_vector3f f_root_position() const
	{
		v_placement->f_validate();
		const auto& m = static_cast<t_matrix_transform&>(*v_root->v_transforms[1]);
		return f_affine(*v_placement, f_affine(static_cast<t_matrix_transform&>(*v_root->v_transforms[0]), t_vector3f(m[3][0], m[3][1], m[3][2])));
	}
	t_vector3f f_direction() const
	{
		const auto& v = v_ball.v_velocity;
		float e = (v.v_z < 0.0 ? 1.0 : -1.0) * v_end;
		t_vector3f d(v.v_x * e, 0.0, v.v_z * e);
		return d.f_length() > 0.01 / 64.0 ? d : t_vector3f(0.0, 0.0, -v_end);
	}
	float f_whichhand(const t_vector3f& a_v) const
	{
		return t_vector3f(-a_v.v_z, 0.0, a_v.v_x) * (v_ball.v_position - v_placement->v_position);
	}
	t_vector3f f_relative_ball(const t_swing& a_swing, const t_vector3f& a_ball) const
	{
		v_placement->f_validate();
		auto p = a_ball - v_placement->v_position;
		const auto& v = v_placement->v_toward;
		float x = v.v_z * p.v_x - v.v_x * p.v_z - a_swing.v_spot[3][0];
		float y = p.v_y - a_swing.v_spot[3][1];
		float z = v.v_x * p.v_x + v.v_z * p.v_z - a_swing.v_spot[3][2];
		return a_swing.v_spot[1][0] > 0.0 ? t_vector3f(-x, y, -z) : t_vector3f(x, y, z);
	}
	t_vector3f f_relative_ball(const t_swing& a_swing) const
	{
		return f_relative_ball(a_swing, v_ball.v_position);
	}
	void f_step()
	{
		v_state->v_step(*this);
		auto& position = v_placement->v_position;
		if (position.v_x < -30 * 12 * 0.0254)
			position.v_x = -30 * 12 * 0.0254;
		else if (position.v_x > 30 * 12 * 0.0254)
			position.v_x = 30 * 12 * 0.0254;
		if (position.v_z * v_end < 1.0)
			position.v_z = 1.0 * v_end;
		else if (position.v_z * v_end > 60 * 12 * 0.0254)
			position.v_z = 60 * 12 * 0.0254 * v_end;
	}
	void f_do(t_swing t_shots::* a_shot)
	{
		v_state->v_do(*this, a_shot);
	}
	t_vector3f f_shot_direction() const
	{
		return v_ball.v_position.v_z * v_end < 0.0 ? t_vector3f(0.0, 0.0, -v_end) : ::f_shot_direction(v_ball.v_position, v_end, v_left, v_right, v_forward, v_backward);
	}
	float f_smash_height() const
	{
		return v_actions.v_swing.v_forehand.v_smash.v_spot[3][1] - 0.25;
	}
	static const t_state v_state_default;
	static const t_state v_state_serve_set;
	static const t_state v_state_serve_toss;
	static const t_state v_state_serve_swing;
	void f_swing_impact(const t_vector3f& a_v);
	static const t_state v_state_swing;
	static const t_state v_state_smash_swing;
	static const t_state v_state_reach_swing;
};

#endif
