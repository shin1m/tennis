#include "computer.h"

#include <chrono>

#include "main.h"

int f_random()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

void f_computer(t_stage::t_state& a_state, t_player& a_player)
{
	struct t_state
	{
		float v_duration = 1.0 * 64.0;
		bool v_decided0 = false;
		bool v_decided1 = false;
		bool v_left = false;
		bool v_right = false;
		bool v_forward = false;
		bool v_backward = false;
		t_player::t_swing t_player::t_shots::* v_shot = &t_player::t_shots::v_flat;
		bool v_net = false;

		void f_reset_decision()
		{
			v_decided0 = v_decided1 = v_left = v_right = v_forward = v_backward = false;
			v_shot = &t_player::t_shots::v_flat;
		}
	};
	auto f_reset_move = [&a_player]
	{
		a_player.v_left = a_player.v_right = a_player.v_forward = a_player.v_backward = false;
	};
	auto step = std::move(a_state.v_step);
	a_state.v_step = [state = t_state(), f_reset_move, step, &a_player](t_stage& a_stage) mutable
	{
		auto& stage = dynamic_cast<t_match&>(a_stage);
		auto& ball = *stage.v_ball;
		if (ball.v_done) {
			f_reset_move();
			state.f_reset_decision();
			state.v_net = false;
			step(stage);
			return;
		} else if (ball.v_hitter == nullptr) {
			if (&a_player == stage.v_server) {
				if (a_player.v_state == &t_player::v_state_serve_set) {
					f_reset_move();
					if (state.v_duration <= 0.0) {
						a_player.f_do(&t_player::t_shots::v_flat);
						state.v_duration = 1.0 * 64.0;
					} else {
						state.v_duration -= 1.0;
					}
				} else if (a_player.v_state == &t_player::v_state_serve_toss) {
					if (stage.v_second) {
						state.v_shot = &t_player::t_shots::v_lob;
					} else {
						int i = f_random() % 10;
						if (i > 6)
							state.v_shot = &t_player::t_shots::v_topspin;
						else if (i > 4)
							state.v_shot = &t_player::t_shots::v_slice;
						else
							state.v_shot = &t_player::t_shots::v_flat;
					}
					const auto& swing = a_player.v_actions.v_serve.v_swing.*state.v_shot;
					float t = ball.f_projected_time_for_y(swing.v_spot[1][3], 1.0);
					float dt = stage.v_second ? 0.0 : 1.0;
					if (f_random() % 2 == 0) dt += 1.0;
					if (t < (swing.v_impact - swing.v_start) * 60.0 + dt) {
						state.v_net = f_random() % 10 > (stage.v_second ? 7 : 4);
						a_player.f_do(state.v_shot);
					} else if (t < (swing.v_impact - swing.v_start) * 60.0 + 8.0) {
						if (!a_player.v_left && !a_player.v_right) {
							int i = f_random();
							if (i % 8 < (stage.v_second ? 1 : 2))
								a_player.v_left = true;
							else if (i % 8 > (stage.v_second ? 5 : 4))
								a_player.v_right = true;
						}
					}
				}
			} else {
				f_reset_move();
			}
		} else if (ball.v_hitter->v_end == a_player.v_end) {
			if (!state.v_decided0) {
				state.v_decided0 = true;
				if (!state.v_net) state.v_net = f_random() % 10 > 6;
			}
			f_reset_move();
			t_vector3f point(ball.v_position.v_x, 0.0, ball.v_position.v_z);
			t_vector3f side0(-(13 * 12 + 6) * 0.0254, 0.0, 21 * 12 * 0.0254 * a_player.v_end);
			t_vector3f side1((13 * 12 + 6) * 0.0254, 0.0, 21 * 12 * 0.0254 * a_player.v_end);
			auto v = (side0 - point).f_normalized() + (side1 - point).f_normalized();
			auto a = t_vector3f(-v.v_z, 0.0, v.v_x) * (a_player.v_placement->v_position - point);
			float epsilon = 1.0 / 1.0;
			if (a < -epsilon)
				a_player.v_left = true;
			else if (a > epsilon)
				a_player.v_right = true;
			float z = a_player.v_placement->v_position.v_z * a_player.v_end;
			float zt = state.v_net ? 21 * 12 * 0.0254 : 39 * 12 * 0.0254;
			epsilon = 1.0 / 2.0;
			if (z < zt - epsilon)
				a_player.v_backward = true;
			else if (z > zt + epsilon)
				a_player.v_forward = true;
		} else if (a_player.v_state == &t_player::v_state_default) {
			if (!state.v_decided1) {
				state.v_decided1 = true;
				int i = f_random();
				if (i % 3 == 1)
					state.v_left = true;
				else if (i % 3 == 2)
					state.v_right = true;
				if (i % 10 > 5)
					state.v_forward = true;
				else if (i % 10 == 0)
					state.v_backward = true;
				i = f_random() % 10;
				if (i > 6)
					state.v_shot = &t_player::t_shots::v_topspin;
				else if (i > 4)
					state.v_shot = &t_player::t_shots::v_slice;
				else if (i > 3)
					state.v_shot = &t_player::t_shots::v_lob;
				else
					state.v_shot = &t_player::t_shots::v_flat;
			}
			auto position = ball.v_position;
			auto velocity = ball.v_velocity;
			float bound_t;
			t_vector3f bound_position;
			if (!ball.v_in) {
				bound_t = ceil(ball.f_projected_time_for_y(t_ball::c_radius, 1.0));
				bound_position = t_vector3f(position.v_x + velocity.v_x * bound_t, t_ball::c_radius, position.v_z + velocity.v_z * bound_t);
			}
			auto v = a_player.f_direction().f_normalized();
			float whichhand = a_player.f_whichhand(v);
			const auto& actions = a_player.v_actions.v_swing;
			const t_player::t_swing* swing = nullptr;
			float t = f_projected_time_for_y(position.v_y, velocity.v_y, t_player::c_smash_height, 1.0);
			float ix;
			float iz;
			float t0;
			if (!isnan(t)) {
				float d = (t_vector3f(position.v_x + velocity.v_x * t, 0.0, position.v_z + velocity.v_z * t) - a_player.v_placement->v_position).f_length();
				if (d < 0.5 || d / a_player.v_actions.v_run.v_speed <= t) {
					const auto& hand = whichhand > a_player.v_smash_hand ? actions.v_forehand : actions.v_backhand;
					swing = &hand.v_smash;
					ix = swing->v_spot[0][3];
					iz = swing->v_spot[2][3];
					t0 = 0.0;
					t = f_projected_time_for_y(position.v_y, velocity.v_y, swing->v_spot[1][3], 1.0);
					if (isnan(t)) t = velocity.v_y / G;
					t -= 2.0;
				}
			}
			if (swing == nullptr) {
				const auto& hand = whichhand > 0.0 ? actions.v_forehand : actions.v_backhand;
				swing = state.v_net && !ball.v_in ? &hand.v_volley : &(hand.v_stroke.*state.v_shot);
				ix = swing->v_spot[0][3];
				iz = swing->v_spot[2][3];
				if (state.v_net || ball.v_in) {
					t0 = 0.0;
				} else {
					t0 = bound_t;
					position = bound_position;
					velocity = t_vector3f(velocity.v_x, velocity.v_y - G * bound_t, velocity.v_z);
					auto spin = ball.v_spin;
					ball.f_calculate_bounce(velocity, spin);
				}
				if (state.v_net && !ball.v_in) {
					auto point = a_player.v_placement->v_position - t_vector3f(-v.v_z, 0.0, v.v_x) * ix + v * iz;
					t = f_reach_range(position, velocity, point, a_player.v_actions.v_run.v_speed, 0.0, -1.0) + 1.0;
				} else {
					t = f_projected_time_for_y(position.v_y, velocity.v_y, 1.25, -1.0);
					if (isnan(t)) t = velocity.v_y / G;
				}
			}
			auto point = t_vector3f(position.v_x + velocity.v_x * t, 0.0, position.v_z + velocity.v_z * t);
			v = f_shot_direction(point, a_player.v_end, state.v_left, state.v_right, state.v_forward, state.v_backward).f_normalized();
			point = a_player.v_placement->v_position - t_vector3f(-v.v_z, 0.0, v.v_x) * ix + v * iz;
			float tt = t0 + t;
			float t1 = t0 + f_reach_range(position, velocity, point, a_player.v_actions.v_run.v_speed, t0, 1.0);
			if (t1 >= 0.0 && t1 < tt) tt = t1;
			if (tt < -1.0) {
			} else if ((ball.v_in || bound_position.v_x > -(13 * 12 + 6) * 0.0254 - 0.125 && bound_position.v_x < (13 * 12 + 6) * 0.0254 + 0.125 && bound_position.v_z * a_player.v_end < 39 * 12 * 0.0254 + 0.5) && tt < (swing->v_impact - swing->v_start) * 60.0 + 1.0) {
				f_reset_move();
				a_player.v_left = state.v_left;
				a_player.v_right = state.v_right;
				a_player.v_forward = state.v_forward;
				a_player.v_backward = state.v_backward;
				a_player.f_do(state.v_shot);
				state.f_reset_decision();
				state.v_net = a_player.v_placement->v_position.v_z * a_player.v_end < 22 * 12 * 0.0254;
			} else {
				f_reset_move();
				point = t_vector3f(position.v_x + velocity.v_x * t, 0.0, position.v_z + velocity.v_z * t);
				v = f_shot_direction(point, a_player.v_end, state.v_left, state.v_right, state.v_forward, state.v_backward).f_normalized();
				auto target = point + t_vector3f(-v.v_z, 0.0, v.v_x) * ix - v * iz;
				float epsilon = tt > 32.0 ? 1.0 / 8.0 : 1.0 / 32.0;
				if (a_player.v_placement->v_position.v_x * a_player.v_end < target.v_x * a_player.v_end - epsilon)
					a_player.v_right = true;
				else if (a_player.v_placement->v_position.v_x * a_player.v_end > target.v_x * a_player.v_end + epsilon)
					a_player.v_left = true;
				epsilon = tt > 32.0 ? 1.0 : 1.0 / 4.0;
				if (a_player.v_placement->v_position.v_z * a_player.v_end < target.v_z * a_player.v_end - epsilon)
					a_player.v_backward = true;
				else if (a_player.v_placement->v_position.v_z * a_player.v_end > target.v_z * a_player.v_end + epsilon)
					a_player.v_forward = true;
			}
		}
		step(stage);
	};
}
