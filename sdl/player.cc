#include "player.h"

#include "main.h"

float f_reach_range(const t_vector3f& a_ball, const t_vector3f& a_velocity, const t_vector3f& a_player, float a_speed, float a_t0, float a_sign)
{
	auto qp = a_ball - a_player;
	qp.v_y = 0.0;
	t_vector3f v(a_velocity.v_x, 0.0, a_velocity.v_z);
	float ss = a_speed * a_speed;
	float a = v * v - ss;
	float b = v * qp - ss * a_t0;
	float c = qp * qp - ss * a_t0 * a_t0;
	float d = b * b - a * c;
	return d < 0.0 ? -b / a : (-b + a_sign * sqrt(d)) / a;
}

t_vector3f f_shot_direction(const t_vector3f& a_ball, float a_end, bool a_left, bool a_right, bool a_forward, bool a_backward)
{
	float vx = -a_ball.v_x;
	if (a_left) vx -= 12 * 12 * 0.0254 * a_end;
	if (a_right) vx += 12 * 12 * 0.0254 * a_end;
	float vz = -24 * 12 * 0.0254 * a_end - a_ball.v_z;
	if (a_forward) vz -= 16 * 12 * 0.0254 * a_end;
	if (a_backward) vz += 10 * 12 * 0.0254 * a_end;
	return t_vector3f(vx, 0.0, vz);
}

void t_player::t_action::f_initialize(t_document& a_scene, float a_start, float a_duration, const std::function<bool (const t_channel&)>& a_use)
{
	v_start = a_start;
	v_end = a_start + a_duration;
	auto iterators = a_scene.f_iterators(a_use);
	for (auto& x : iterators) {
		x.second->f_rewind(a_start);
		size_t i = x.second->f_index();
		v_iterators.emplace_back(std::move(x.second), i);
	}
}

void t_player::t_action::f_rewind()
{
	for (const auto& x : v_iterators) std::get<0>(x)->f_rewind(v_start, std::get<1>(x));
}

void t_player::t_action::f_forward(float a_t)
{
	for (const auto& x : v_iterators) std::get<0>(x)->f_forward(a_t);
}

void t_player::t_swing::f_initialize(t_document& a_scene, t_node* a_skeleton, float a_start, float a_duration, float a_impact, float a_speed, const t_vector3f& a_spin)
{
	t_action::f_initialize(a_scene, a_start, a_duration);
	v_impact = a_start + a_impact;
	v_speed = a_speed;
	v_spin = a_spin;
	auto iterators = a_scene.f_iterators();
	{
		for (auto& x : iterators) x.second->f_rewind(v_impact);
		std::map<std::wstring, t_matrix4f*> joints{{L"Spot", &v_spot}};
		std::vector<std::map<std::wstring, t_matrix4f*>*> js{&joints};
		a_skeleton->f_render(t_matrix4f(1.0), t_matrix4f(1.0), js);
	}
	{
		for (auto& x : iterators) x.second->f_rewind(v_end);
		t_matrix4f root;
		std::map<std::wstring, t_matrix4f*> joints{{L"Root", &root}};
		std::vector<std::map<std::wstring, t_matrix4f*>*> js{&joints};
		a_skeleton->f_render(t_matrix4f(1.0), t_matrix4f(1.0), js);
		v_end_position = t_vector3f(root[0][3], 0.0, root[2][3]);
		v_end_toward = t_vector3f(root[0][2], 0.0, root[2][2]);
	}
}

void t_player::t_swing::f_merge(t_player& a_player) const
{
	auto& placement = *a_player.v_placement;
	placement.f_validate();
	placement.v_position = f_affine(placement, v_end_position);
	placement.v_toward = t_matrix3f(placement) * v_end_toward;
	placement.v_valid = false;
}

void t_player::t_run::f_initialize(t_document& a_scene, t_node* a_skeleton, float a_start, float a_duration, const std::function<bool (const t_channel&)>& a_use)
{
	t_action::f_initialize(a_scene, a_start, a_duration, a_use);
	auto iterators = a_scene.f_iterators([](auto a_x)
	{
		return a_x.v_node->v_id == L"Root";
	});
	for (auto& x : iterators) x.second->f_rewind(a_start);
	t_matrix4f root;
	std::map<std::wstring, t_matrix4f*> joints{{L"Root", &root}};
	std::vector<std::map<std::wstring, t_matrix4f*>*> js{&joints};
	a_skeleton->f_render(t_matrix4f(1.0), t_matrix4f(1.0), js);
	v_toward = t_vector3f(root[0][2], 0.0, root[2][2]);
}

void t_player::t_motion::operator()()
{
	v_action.f_forward(v_time);
	if (v_time < v_action.v_end) v_time += 1.0 / 60.0;
}

constexpr float t_player::t_run_motion::c_duration;

t_player::t_run_motion::t_run_motion(t_run& a_run, const t_vector3f& a_toward, t_player& a_player) : t_motion(a_run), v_toward(a_toward), v_placement(a_player.v_placement), v_toward0(a_player.v_placement->v_toward)
{
	static_cast<t_matrix_transform&>(*a_player.v_root->v_transforms[1]) = a_player.v_root_transform;
	auto t0 = v_toward.f_normalized();
	const auto& t1 = a_run.v_toward;
	v_toward1 = t_vector3f(t0.v_x * t1.v_z + t0.v_z * t1.v_x, 0.0, t0.v_z * t1.v_z - t0.v_x * t1.v_x);
	v_duration = v_toward0 * v_toward1 < -0.75 ? 0.0 : c_duration;
}

void t_player::t_run_motion::operator()()
{
	t_motion::operator()();
	if (v_duration > 0.0) v_duration -= 1.0 / 64.0;
	float t = v_duration / c_duration;
	v_placement->v_toward = v_toward0 * t + v_toward1 * (1.0 - t);
}

const std::set<std::wstring> t_player::v_lowers{
	L"Center",
	L"Leg0_R",
	L"Leg1_R",
	L"Foot_R",
	L"Toe_R",
	L"Leg0_L",
	L"Leg1_L",
	L"Foot_L",
	L"Toe_L"
};

void t_player::f_load(t_document& a_scene, t_node* a_skeleton, const std::wstring& a_source)
{
	float source_fps;
	float fps = 64.0;
	auto input = v_stage.v_main.f_input(a_source);
	t_reader reader(input.get());
	auto f_read_action = [&](t_action& a_action, const std::function<bool (const t_channel&)>& a_use = [](auto a_x) { return true; })
	{
		float start = std::stof(reader.f_get_attribute(L"start")) / source_fps;
		float duration = std::stof(reader.f_get_attribute(L"duration")) / source_fps;
		reader.f_read_element_text();
		a_action.f_initialize(a_scene, start, duration, a_use);
	};
	auto f_read_swing = [&](t_swing& a_swing)
	{
		float start = std::stof(reader.f_get_attribute(L"start")) / source_fps;
		float duration = std::stof(reader.f_get_attribute(L"duration")) / source_fps;
		float impact = std::stof(reader.f_get_attribute(L"impact")) / source_fps;
		float speed = std::stof(reader.f_get_attribute(L"speed")) / fps;
		std::vector<float> spin;
		f_parse_vector(3, reader.f_get_attribute(L"spin"), spin);
		reader.f_read_element_text();
		a_swing.f_initialize(a_scene, a_skeleton, start, duration, impact, speed, t_vector3f(spin[0], spin[1], spin[2]));
	};
	auto f_read_run = [&](t_run& a_run, const std::function<bool (const t_channel&)>& a_use)
	{
		float start = std::stof(reader.f_get_attribute(L"start")) / source_fps;
		float duration = std::stof(reader.f_get_attribute(L"duration")) / source_fps;
		reader.f_read_element_text();
		a_run.f_initialize(a_scene, a_skeleton, start, duration, a_use);
	};
	const std::map<std::wstring, std::function<void(t_shots&)>> shot_swing_elements{
		{L"flat", [&](t_shots& a_x)
			{
				f_read_swing(a_x.v_flat);
			}
		},
		{L"topspin", [&](t_shots& a_x)
			{
				f_read_swing(a_x.v_topspin);
			}
		},
		{L"lob", [&](t_shots& a_x)
			{
				f_read_swing(a_x.v_lob);
			}
		},
		{L"slice", [&](t_shots& a_x)
			{
				f_read_swing(a_x.v_slice);
			}
		}
	};
	const std::map<std::wstring, std::function<void()>> serve_elements{
		{L"set", [&]
			{
				f_read_action(v_actions.v_serve.v_set);
			}
		},
		{L"toss", [&]
			{
				f_read_action(v_actions.v_serve.v_toss);
			}
		},
		{L"swing", [&]
			{
				reader.f_parse_elements(shot_swing_elements, v_actions.v_serve.v_swing);
			}
		}
	};
	const std::map<std::wstring, std::function<void(t_readies&)>> ready_action_elements{
		{L"stroke", [&](t_readies& a_x)
			{
				f_read_run(a_x.v_stroke, f_is_not_root);
			}
		},
		{L"volley", [&](t_readies& a_x)
			{
				f_read_run(a_x.v_volley, f_is_not_root);
			}
		},
		{L"smash", [&](t_readies& a_x)
			{
				f_read_run(a_x.v_smash, f_is_not_root);
			}
		}
	};
	const std::map<std::wstring, std::function<void()>> ready_elements{
		{L"default", [&]
			{
				f_read_run(v_actions.v_ready.v_default, f_is_not_root);
			}
		},
		{L"forehand", [&]
			{
				reader.f_parse_elements(ready_action_elements, v_actions.v_ready.v_forehand);
			}
		},
		{L"backhand", [&]
			{
				reader.f_parse_elements(ready_action_elements, v_actions.v_ready.v_backhand);
			}
		}
	};
	const std::map<std::wstring, std::function<void(t_run*)>> run_lowers_elements{
		{L"left", [&](t_run* a_x)
			{
				f_read_run(a_x[1], f_is_lower);
			}
		},
		{L"right", [&](t_run* a_x) 
			{
				f_read_run(a_x[2], f_is_lower);
			}
		},
		{L"forward", [&](t_run* a_x) 
			{
				f_read_run(a_x[4], f_is_lower);
			}
		},
		{L"forward_left", [&](t_run* a_x) 
			{
				f_read_run(a_x[5], f_is_lower);
			}
		},
		{L"forward_right", [&](t_run* a_x) 
			{
				f_read_run(a_x[6], f_is_lower);
			}
		},
		{L"backward", [&](t_run* a_x) 
			{
				f_read_run(a_x[8], f_is_lower);
			}
		},
		{L"backward_left", [&](t_run* a_x) 
			{
				f_read_run(a_x[9], f_is_lower);
			}
		},
		{L"backward_right", [&](t_run* a_x) 
			{
				f_read_run(a_x[10], f_is_lower);
			}
		}
	};
	const std::map<std::wstring, std::function<void(t_runs&)>> run_action_elements{
		{L"lowers", [&](t_runs& a_x)
			{
				reader.f_parse_elements(run_lowers_elements, a_x.v_lowers);
			}
		},
		{L"stroke", [&](t_runs& a_x)
			{
				f_read_action(a_x.v_stroke, f_is_upper);
			}
		},
		{L"volley", [&](t_runs& a_x)
			{
				f_read_action(a_x.v_volley, f_is_upper);
			}
		},
		{L"smash", [&](t_runs& a_x)
			{
				f_read_action(a_x.v_smash, f_is_upper);
			}
		}
	};
	const std::map<std::wstring, std::function<void()>> run_elements{
		{L"lower", [&]
			{
				f_read_run(v_actions.v_run.v_lower, f_is_lower);
			}
		},
		{L"default", [&]
			{
				f_read_action(v_actions.v_run.v_default, f_is_upper);
			}
		},
		{L"forehand", [&]
			{
				reader.f_parse_elements(run_action_elements, v_actions.v_run.v_forehand);
			}
		},
		{L"backhand", [&]
			{
				reader.f_parse_elements(run_action_elements, v_actions.v_run.v_backhand);
			}
		}
	};
	const std::map<std::wstring, std::function<void(t_swings&)>> swing_action_elements{
		{L"stroke", [&](t_swings& a_x)
			{
				reader.f_parse_elements(shot_swing_elements, a_x.v_stroke);
			}
		},
		{L"volley", [&](t_swings& a_x)
			{
				reader.f_parse_elements(shot_swing_elements, a_x.v_volley);
			}
		},
		{L"smash", [&](t_swings& a_x)
			{
				f_read_swing(a_x.v_smash);
			}
		},
		{L"reach_volley", [&](t_swings& a_x)
			{
				f_read_swing(a_x.v_reach_volley);
			}
		}
	};
	const std::map<std::wstring, std::function<void()>> swing_elements{
		{L"forehand", [&]
			{
				reader.f_parse_elements(swing_action_elements, v_actions.v_swing.v_forehand);
			}
		},
		{L"backhand", [&]
			{
				reader.f_parse_elements(swing_action_elements, v_actions.v_swing.v_backhand);
			}
		},
		{L"toss", [&]
			{
				f_read_swing(v_actions.v_swing.v_toss);
			}
		},
		{L"toss_lob", [&]
			{
				f_read_swing(v_actions.v_swing.v_toss_lob);
			}
		}
	};
	const std::map<std::wstring, std::function<void()>> root_elements{
		{L"serve", [&]
			{
				reader.f_parse_elements(serve_elements);
			}
		},
		{L"ready", [&]
			{
				reader.f_parse_elements(ready_elements);
			}
		},
		{L"run", [&]
			{
				v_actions.v_run.v_speed = std::stof(reader.f_get_attribute(L"speed")) / fps;
				reader.f_parse_elements(run_elements);
			}
		},
		{L"swing", [&]
			{
				reader.f_parse_elements(swing_elements);
			}
		}
	};
	reader.f_read_next();
	reader.f_move_to_tag();
	reader.f_check_start_element(L"player");
	source_fps = std::stof(reader.f_get_attribute(L"fps"));
	reader.f_parse_elements(root_elements);
}

t_player::t_player(t_stage& a_stage, const std::wstring& a_model) : v_stage(a_stage), v_ball(*a_stage.v_ball)
{
	a_stage.v_main.f_load(v_scene, a_model + L".dae");
	v_scene.f_build(a_stage.v_main.v_shaders);
	v_node = &dynamic_cast<t_node&>(*v_scene[L"#Armature"]);
	v_node->v_transforms.clear();
	auto placement = std::make_unique<t_placement>();
	v_placement = placement.get();
	v_node->v_transforms.push_back(std::move(placement));
	auto zup = std::make_unique<t_posture>();
	zup->v_toward = t_vector3f(0.0, 1.0, 0.0);
	zup->v_upward = t_vector3f(0.0, 0.0, -1.0);
	zup->f_validate();
	v_root = &dynamic_cast<t_node&>(*v_scene[L"#Root"]);
	v_root_transform = dynamic_cast<t_matrix_transform&>(*v_root->v_transforms[0]);
	v_root->v_transforms.insert(v_root->v_transforms.begin(), std::move(zup));
	f_load(v_scene, v_root, a_model + L".player");
	v_actions.v_serve.v_set.f_rewind();
	v_lefty = dynamic_cast<t_matrix_transform&>(*v_root->v_transforms[1])[0][3] < 0.0 ? -1.0 : 1.0;
	v_smash_hand = -0.25 * v_lefty;
	f_reset(1.0, v_state_default);
	auto scene = a_stage.v_scene.v_scene.v_instance_visual_scene.v_scene;
	scene->v_instance_nodes.push_back(v_node);
	for (auto x : v_scene.v_scene.v_instance_visual_scene.v_scene->v_controllers) scene->v_controllers.push_back(x);
}

const t_player::t_state t_player::v_state_default{
	[](t_player& a_player)
	{
		auto v = a_player.v_ball.v_position - a_player.v_placement->v_position;
		v.v_y = 0.0;
		v.f_normalize();
		a_player.v_placement->v_toward = v;
		a_player.v_motion = std::make_unique<t_run_motion>(a_player.v_actions.v_ready.v_default, v, a_player);
	},
	[](t_player& a_player)
	{
		t_vector3f d(0.0, 0.0, 0.0);
		const float speed = a_player.v_actions.v_run.v_speed;
		if (a_player.v_left) d.v_x = -speed * a_player.v_end;
		if (a_player.v_right) d.v_x = speed * a_player.v_end;
		if (a_player.v_forward) d.v_z = -speed * a_player.v_end;
		if (a_player.v_backward) d.v_z = speed * a_player.v_end;
		auto f = [&](auto& actions, auto run_done, auto run_default, auto run_takeback)
		{
			if (a_player.v_ball.v_done) {
				run_done(t_vector3f(0.0, 0.0, -a_player.v_end), actions.v_default);
			} else if (a_player.v_ball.v_hitter == nullptr || a_player.v_ball.v_hitter->v_end == a_player.v_end) {
				auto v = a_player.v_ball.v_position - a_player.v_placement->v_position;
				v.v_y = 0.0;
				v.f_normalize();
				run_default(v, actions, actions.v_default);
			} else {
				auto v = a_player.f_direction();
				v.f_normalize();
				float whichhand = a_player.f_whichhand(v);
				float t = f_reach_range(a_player.v_ball.v_position, a_player.v_ball.v_velocity, a_player.v_placement->v_position, 0.0, 0.0, 1.0);
				float y = a_player.v_ball.v_position.v_y + (a_player.v_ball.v_velocity.v_y - 0.5 * G * t) * t;
				if (y > a_player.f_smash_height()) {
					auto& hand = whichhand > a_player.v_smash_hand ? actions.v_forehand : actions.v_backhand;
					run_takeback(v, hand, hand.v_smash);
				} else {
					auto& hand = whichhand > 0.0 ? actions.v_forehand : actions.v_backhand;
					run_takeback(v, hand, a_player.v_ball.v_in || y < 0.0 ? hand.v_stroke : hand.v_volley);
				}
			}
		};
		if (d.v_x == 0.0 && d.v_z == 0.0) {
			auto g = [&](auto v, auto& action)
			{
				a_player.v_motion = std::make_unique<t_run_motion>(action, v, a_player);
			};
			f(a_player.v_actions.v_ready, [&](auto v, auto& action)
			{
				g(v, action);
			}, [&](auto v, auto& actions, auto& action)
			{
				g(v, action);
			}, [&](auto v, auto& hand, auto& action)
			{
				g(v, action);
			});
		} else {
			auto g = [&](auto& action, auto& run)
			{
				if (&a_player.v_motion->v_action != &run || d != static_cast<t_run_motion&>(*a_player.v_motion).v_toward) a_player.v_motion = std::make_unique<t_run_motion>(run, d, a_player);
				if (a_player.v_motion->v_time >= run.v_end) a_player.v_motion->f_rewind();
				action.f_rewind();
				a_player.v_placement->v_position = a_player.v_placement->v_position + d;
			};
			f(a_player.v_actions.v_run, [&](auto v, auto& action)
			{
				g(action, a_player.v_actions.v_run.v_lower);
			}, [&](auto v, auto& actions, auto& action)
			{
				if (a_player.v_forward)
					g(action, a_player.v_actions.v_run.v_lower);
				else if (a_player.v_left)
					g(action, actions.v_backhand.v_lowers[9]);
				else if (a_player.v_right)
					g(action, actions.v_forehand.v_lowers[10]);
				else if (a_player.v_backward)
					g(action, v.v_x * a_player.v_end > 0.0 ? actions.v_forehand.v_lowers[9] : actions.v_backhand.v_lowers[10]);
				else
					g(action, a_player.v_actions.v_run.v_lower);
			}, [&](auto v, auto& hand, auto& action)
			{
				g(action, hand.v_lowers[(a_player.v_left ? 1 : a_player.v_right ? 2 : 0) + (a_player.v_forward ? 4 : a_player.v_backward ? 8 : 0)]);
			});
		}
		(*a_player.v_motion)();
		a_player.v_placement->v_valid = false;
	},
	[](t_player& a_player, t_swing t_shots::* a_shot)
	{
		a_player.v_placement->v_toward = a_player.f_shot_direction();
		a_player.v_placement->v_valid = false;
		auto& actions = a_player.v_actions.v_swing;
		float whichhand = a_player.f_whichhand(a_player.f_direction().f_normalized());
		float t = a_player.v_ball.f_projected_time_for_y(a_player.f_smash_height(), 1.0);
		if (!isnan(t)) {
			auto& swing = whichhand > a_player.v_smash_hand ? actions.v_forehand.v_smash : actions.v_backhand.v_smash;
			float impact = (swing.v_impact - swing.v_start) * 60.0;
			if (t > impact) {
				auto ball = a_player.f_relative_ball(swing, a_player.v_ball.v_position + a_player.v_ball.v_velocity * impact);
				if (fabs(ball.v_x) < 0.5) {
					a_player.v_motion = std::make_unique<t_motion>(swing);
					return a_player.f_transit(a_player.v_state_smash_swing);
				}
			}
		}
		t = a_player.v_ball.v_in ? 0.0 : a_player.v_ball.f_projected_time_for_y(t_ball::c_radius, 1.0);
		auto& hand = whichhand > 0.0 ? actions.v_forehand : actions.v_backhand;
		if (a_player.v_ball.v_done) {
			a_player.v_motion = std::make_unique<t_motion>((a_player.v_placement->v_position.v_z * a_player.v_end > 21 * 12 * 0.0254 ? hand.v_stroke : hand.v_volley).*a_shot);
		} else {
			auto& volley = hand.v_volley.*a_shot;
			float impact = (volley.v_impact - volley.v_start) * 60.0;
			if (t < impact) {
				a_player.v_motion = std::make_unique<t_motion>(hand.v_stroke.*a_shot);
			} else {
				auto ball = a_player.f_relative_ball(volley, a_player.v_ball.v_position + a_player.v_ball.v_velocity * impact);
				if (ball.v_x < -0.5 || (whichhand > 0.0 ? ball.v_z > 0.5 : ball.v_z < -0.5)) {
					a_player.v_motion = std::make_unique<t_motion>(hand.v_reach_volley);
					return a_player.f_transit(a_player.v_state_reach_volley_swing);
				}
				a_player.v_motion = std::make_unique<t_motion>(volley);
			}
		}
		a_player.f_transit(a_player.v_state_swing);
	}
};

const t_player::t_state t_player::v_state_serve_set{
	[](t_player& a_player)
	{
		a_player.v_motion = std::make_unique<t_motion>(a_player.v_actions.v_serve.v_set);
	},
	[](t_player& a_player)
	{
		const float speed = 2.0 / 64.0;
		if (a_player.v_left) a_player.v_ball.v_position.v_x -= speed * a_player.v_end;
		if (a_player.v_right) a_player.v_ball.v_position.v_x += speed * a_player.v_end;
		float es = a_player.v_end * a_player.v_stage.v_side;
		float xes = a_player.v_ball.v_position.v_x * es;
		const float center = 12 * 0.0254;
		const float wide = 14 * 12 * 0.0254;
		if (xes < center) a_player.v_ball.v_position.v_x = center * es;
		if (xes > wide) a_player.v_ball.v_position.v_x = wide * es;
		a_player.v_ball.v_position.v_y = 0.875;
		a_player.v_ball.v_velocity = t_vector3f(0.0, 0.0, 0.0);
		a_player.v_ball.v_spin = t_vector3f(0.0, 0.0, 0.0);
		a_player.v_placement->v_position = t_vector3f(a_player.v_ball.v_position.v_x, 0.0, a_player.v_ball.v_position.v_z);
		a_player.v_placement->v_toward = t_vector3f((6 * 12 + 9) * -0.0254 * es + 2 * 12 * 0.0254 * a_player.v_lefty * a_player.v_end, 0.0, 21 * 12 * -0.0254 * a_player.v_end) - a_player.v_placement->v_position;
		a_player.v_placement->v_valid = false;
		(*a_player.v_motion)();
	},
	[](t_player& a_player, t_swing t_shots::* a_shot)
	{
		a_player.f_transit(a_player.v_state_serve_toss);
	}
};

const t_player::t_state t_player::v_state_serve_toss{
	[](t_player& a_player)
	{
		a_player.v_ball.v_position.v_y = 1.5;
		a_player.v_placement->f_validate();
		const auto& toward = a_player.v_placement->v_toward;
		t_vector3f left(toward.v_z, 0.0, -toward.v_x);
		a_player.v_ball.v_velocity = left * 0.0075 * a_player.v_lefty + toward * 0.01;
		a_player.v_ball.v_velocity.v_y = 0.085;
		a_player.v_ball.v_spin = t_vector3f(0.0, 0.0, 0.0);
		a_player.v_motion = std::make_unique<t_motion>(a_player.v_actions.v_serve.v_toss);
	},
	[](t_player& a_player)
	{
		if (a_player.v_ball.v_position.v_y <= 1.5) {
			a_player.v_ball.v_position.v_x = a_player.v_placement->v_position.v_x;
			a_player.v_ball.v_position.v_z = a_player.v_placement->v_position.v_z;
			a_player.v_ball.v_velocity = t_vector3f(0.0, 0.0, 0.0);
			a_player.f_transit(a_player.v_state_serve_set);
		}
		if (a_player.v_left) a_player.v_placement->v_toward.v_x -= 1.0 / 64.0 * a_player.v_end;
		if (a_player.v_right) a_player.v_placement->v_toward.v_x += 1.0 / 64.0 * a_player.v_end;
		a_player.v_placement->v_valid = false;
		(*a_player.v_motion)();
	},
	[](t_player& a_player, t_swing t_shots::* a_shot)
	{
		a_player.v_motion = std::make_unique<t_motion>(a_player.v_actions.v_serve.v_swing.*a_shot);
		a_player.f_transit(a_player.v_state_serve_swing);
	}
};

const t_player::t_state t_player::v_state_serve_swing{
	[](t_player& a_player)
	{
		a_player.v_stage.v_sound_swing.f_play();
	},
	[](t_player& a_player)
	{
		const auto& action = static_cast<t_swing&>(a_player.v_motion->v_action);
		if (fabs(a_player.v_motion->v_time - action.v_impact) < 0.5 / 60.0) {
			auto ball = a_player.f_relative_ball(action);
			if (fabs(ball.v_y) < 0.3) {
				float d = 58 * 12 * 0.0254 + ball.v_y * 10.0;
				const auto& spin = action.v_spin;
				d *= pow(2.0, -spin.v_x * (4.0 / 64.0));
				float speed = action.v_speed + ball.v_y * 0.125;
				a_player.v_ball.f_impact(a_player.v_placement->v_toward.v_x, a_player.v_placement->v_toward.v_z, speed, G * d / (2.0 * speed) - a_player.v_ball.v_position.v_y * speed / d, spin);
				a_player.v_ball.v_hitter = &a_player;
				a_player.v_stage.v_sound_hit.f_play();
			}
		}
		(*a_player.v_motion)();
		if (a_player.v_motion->v_time < action.v_end) return;
		action.f_merge(a_player);
		a_player.f_transit(a_player.v_state_default);
	},
	[](t_player& a_player, t_swing t_shots::* a_shot)
	{
	}
};

void t_player::f_swing_impact(const t_vector3f& a_v)
{
	const auto& action = static_cast<t_swing&>(v_motion->v_action);
	if (fabs(v_motion->v_time - action.v_impact) < 0.5 / 60.0) {
		auto ball = f_relative_ball(action);
		if (fabs(ball.v_x) < 0.5 && ball.v_y < 1.0 && fabs(ball.v_z) < 1.0) {
			float d = a_v.f_length();
			float n = -d * v_ball.v_position.v_z / a_v.v_z;
			float b = v_ball.v_position.v_y * (d - n) / d;
			float a = d / (60 * 12 * 0.0254);
			float speed = action.v_speed * (a > 1.25 ? 1.25 : a < 0.85 ? 0.85 : a);
			const auto& spin = action.v_spin;
			float dd = v_ball.v_position.v_z * v_end;
			d = dd + (d - dd) * pow(2.0, -spin.v_x * ((spin.v_x > 0.0 ? 12.0 : 8.0) / 64.0));
			float nh = (36 + 42) * 0.5 * 0.0254 + t_ball::c_radius;
			if (b < nh) {
				float vm = sqrt(G * (d - n) * n * 0.5 / (nh - b));
				if (vm < speed) speed = vm;
			}
			d -= ball.v_x * 2.0;
			speed -= ball.v_x * 0.125;
			float dx = a_v.v_x + a_v.v_z * ball.v_z * 0.0625;
			float dz = a_v.v_z - a_v.v_x * ball.v_z * 0.0625;
			v_ball.f_impact(dx, dz, speed, G * d / (2.0 * speed) - v_ball.v_position.v_y * speed / d, spin);
			v_ball.f_hit(this);
			v_stage.v_sound_hit.f_play();
		}
	}
	(*v_motion)();
	if (v_motion->v_time < action.v_end) return;
	action.f_merge(*this);
	f_transit(v_state_default);
}

const t_player::t_state t_player::v_state_swing{
	[](t_player& a_player)
	{
		a_player.v_stage.v_sound_swing.f_play();
	},
	[](t_player& a_player)
	{
		auto v = a_player.f_shot_direction();
		const auto& action = static_cast<t_swing&>(a_player.v_motion->v_action);
		if (a_player.v_motion->v_time <= action.v_impact) {
			a_player.v_placement->v_toward = v;
			a_player.v_placement->v_valid = false;
		}
		a_player.f_swing_impact(v);
	},
	[](t_player& a_player, t_swing t_shots::* a_shot)
	{
	}
};

const t_player::t_state t_player::v_state_smash_swing{
	[](t_player& a_player)
	{
		a_player.v_stage.v_sound_swing.f_play();
	},
	[](t_player& a_player)
	{
		auto v = a_player.f_shot_direction();
		const auto& action = static_cast<t_swing&>(a_player.v_motion->v_action);
		if (a_player.v_motion->v_time <= action.v_impact) {
			a_player.v_placement->v_toward = v;
			a_player.v_placement->v_valid = false;
		}
		if (fabs(a_player.v_motion->v_time - action.v_impact) < 0.5 / 60.0) {
			auto ball = a_player.f_relative_ball(action);
			if (fabs(ball.v_x) < 0.5 && fabs(ball.v_y) < 0.5 && fabs(ball.v_z) < 1.0) {
				float d = v.f_length() + (ball.v_y - ball.v_z) * 2.0;
				float speed = action.v_speed + ball.v_y * 0.125;
				float dx = v.v_x + v.v_z * ball.v_x * 0.0625;
				float dz = v.v_z - v.v_x * ball.v_x * 0.0625;
				a_player.v_ball.f_impact(dx, dz, speed, G * d / (2.0 * speed) - a_player.v_ball.v_position.v_y * speed / d, action.v_spin);
				a_player.v_ball.f_hit(&a_player);
				a_player.v_stage.v_sound_hit.f_play();
			}
		}
		(*a_player.v_motion)();
		if (a_player.v_motion->v_time < action.v_end) return;
		action.f_merge(a_player);
		a_player.f_transit(a_player.v_state_default);
	},
	[](t_player& a_player, t_swing t_shots::* a_shot)
	{
	}
};

const t_player::t_state t_player::v_state_reach_volley_swing{
	[](t_player& a_player)
	{
		const auto& action = static_cast<t_swing&>(a_player.v_motion->v_action);
		float impact = (action.v_impact - a_player.v_motion->v_time) * 60.0;
		float vx = a_player.v_ball.v_position.v_x + a_player.v_ball.v_velocity.v_x * impact - a_player.v_placement->v_position.v_x;
		float vz = a_player.v_ball.v_position.v_z + a_player.v_ball.v_velocity.v_z * impact - a_player.v_placement->v_position.v_z;
		a_player.v_placement->v_toward = t_vector3f(vx, 0.0, vz);
		a_player.v_placement->v_valid = false;
		a_player.v_stage.v_sound_swing.f_play();
	},
	[](t_player& a_player)
	{
		a_player.f_swing_impact(a_player.f_shot_direction());
	},
	[](t_player& a_player, t_swing t_shots::* a_shot)
	{
	}
};
