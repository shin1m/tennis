#include "main.h"

#include "path.h"
#include "xml_reader.h"
#include "sdl_core.h"
#include "computer.h"

void t_main::f_load(t_sound& a_sound, const std::wstring& a_name)
{
	a_sound.f_create_from_file(f_convert((t_path(v_root) / L".." / a_name)).c_str());
	a_sound.f_create();
	a_sound.f_set(AL_BUFFER, static_cast<ALint>(static_cast<al::t_buffer&>(a_sound)));
}

void t_match::f_new_game()
{
	v_player0->v_point = v_player1->v_point = 0;
	v_second = false;
	size_t games = v_player0->v_game + v_player1->v_game;
	v_end = games % 4 < 2 ? 1.0 : -1.0;
	v_server = (games % 2 == 0 ? v_player0 : v_player1).get();
	v_receiver = (games % 2 == 0 ? v_player1 : v_player0).get();
}

void t_match::f_point(t_player& a_player)
{
	++a_player.v_point;
	if (a_player.v_point < 4 || a_player.v_point - a_player.v_opponent->v_point < 2) return;
	++a_player.v_game;
	if (a_player.v_game < 6 || a_player.v_game - a_player.v_opponent->v_game < 2)
		f_new_game();
	else
		v_closed = true;
}

void t_match::f_ball_ace()
{
	v_second = false;
	f_point(*v_ball->v_hitter);
	v_duration = 2.0 * 64.0;
	v_sound_ace.f_play();

}
void t_match::f_ball_let()
{
	v_mark->f_mark(*v_ball);
	v_message = std::vector<std::wstring>{L"LET"};
	v_duration = 2.0 * 64.0;
}

void t_match::f_serve_miss()
{
	if (v_second) {
		v_message = std::vector<std::wstring>{L"DOUBLE FAULT"};
		v_second = false;
		f_point(*v_receiver);
	} else {
		v_message = std::vector<std::wstring>{L"FAULT"};
		v_second = true;
	}
	v_duration = 2.0 * 64.0;
	v_sound_miss.f_play();
}

void t_match::f_ball_serve_air()
{
	f_serve_miss();
}

void t_match::f_miss(const std::wstring& a_message)
{
	v_message = std::vector<std::wstring>{a_message};
	v_second = false;
	f_point(*v_ball->v_hitter->v_opponent);
	v_duration = 2.0 * 64.0;
	v_sound_miss.f_play();
}

std::wstring t_match::v_points0[] = {L" 0", L"15", L"30", L"40"};
std::wstring t_match::v_points1[] = {L"0 ", L"15", L"30", L"40"};

void t_match::f_transit_ready()
{
	v_state = &v_state_ready;
	std::wstring game;
	if (v_player0->v_point + v_player1->v_point < 6)
		game = v_points0[v_player0->v_point] + L" - " + v_points1[v_player1->v_point];
	else if (v_player0->v_point > v_player1->v_point)
		game = L" A -   ";
	else if (v_player0->v_point < v_player1->v_point)
		game = L"   - A ";
	else
		game = L" DEUCE ";
	v_message = std::vector<std::wstring>{
		L"P1 " + std::to_wstring(v_player0->v_game) + L" - " + std::to_wstring(v_player1->v_game) + L" P2",
		(v_player0.get() == v_server ? L"* " : L"  ") + game + (v_player1.get() == v_server ? L" *" : L"  ")
	};
	v_duration = 1.0 * 64.0;
	f_step_things();
}

const t_stage::t_state t_match::v_state_close{
	[](t_stage& a_stage)
	{
		a_stage.f_step_things();
	},
	[](t_stage& a_stage, SDL_Keycode a_key)
	{
		t_match& stage = static_cast<t_match&>(a_stage);
		switch (a_key) {
		case SDLK_RETURN:
			stage.f_new_set();
			break;
		case SDLK_ESCAPE:
			a_stage.f_back();
			break;
		}
	},
	[](t_stage& a_stage, SDL_Keycode a_key)
	{
	}
};

void t_match::f_transit_close()
{
	v_state = &v_state_close;
	v_message = std::vector<std::wstring>{
		std::wstring(v_player0->v_game > v_player1->v_game ? L"P1" : L"P2") + L" WON!",
		L"P1 " + std::to_wstring(v_player0->v_game) + L" - " + std::to_wstring(v_player1->v_game) + L" P2",
		L"PRESS START",
		L"TO PLAY AGAIN"
	};
	v_sound_ace.f_play();
}

void t_match::f_next()
{
	if (!v_ball->v_done) return;
	if (v_second || v_ball->f_serving() && v_ball->v_in && v_ball->v_net) {
		f_reset();
		v_message.clear();
		f_step_things();
	} else if (v_closed) {
		f_transit_close();
	} else {
		f_reset();
		f_transit_ready();
	}
}

void t_match::f_back()
{
	v_main.v_screen = std::make_unique<t_main_screen>(v_main);
}

void t_match::f_transit_play()
{
	v_state = &v_state_play;
	v_message.clear();
}

void t_match::f_reset()
{
	v_side = (v_player0->v_point + v_player1->v_point) % 2 == 0 ? 1.0 : -1.0;
	v_ball->f_reset(v_side, 2 * 12 * 0.0254 * v_end * v_side, 0.875, 39 * 12 * 0.0254 * v_end);
	v_mark->v_duration = 0.0;
	v_server->f_reset(v_end, t_player::v_state_serve_set);
	v_receiver->v_placement->v_position = t_vector3f(-9 * 12 * 0.0254 * v_end * v_side, 0.0, -39 * 12 * 0.0254 * v_end);
	v_receiver->v_placement->v_valid = false;
	v_receiver->f_reset(-v_end, t_player::v_state_default);
	v_camera0.v_position = t_vector3f(0.0, 14.0, 0.0);
	v_camera0.v_toward = t_vector3f(0.0, -12.0, -40.0 * (v_fixed ? 1.0 : v_player0->v_end));
	v_camera1.v_position = t_vector3f(0.0, 14.0, 0.0);
	v_camera1.v_toward = t_vector3f(0.0, -12.0, -40.0 * (v_fixed ? -1.0 : v_player1->v_end));
}

void t_match::f_new_set()
{
	v_closed = false;
	v_player0->v_game = v_player1->v_game = 0;
	f_new_game();
	f_reset();
	f_transit_ready();
}

void t_training::f_ball_ace()
{
	v_duration = 0.5 * 64.0;
}

void t_training::f_ball_let()
{
	v_mark->f_mark(*v_ball);
	v_text_viewing = t_scale3f(0.25, 0.25, 1.0);
	v_message = std::vector<std::wstring>{L"LET"};
	v_duration = 0.5 * 64.0;
}

void t_training::f_serve_miss()
{
	v_text_viewing = t_scale3f(0.25, 0.25, 1.0);
	v_message = std::vector<std::wstring>{L"FAULT"};
	v_duration = 0.5 * 64.0;
	v_sound_miss.f_play();
}

void t_training::f_ball_serve_air()
{
	f_serve_miss();
}

void t_training::f_miss(const std::wstring& a_message)
{
	v_text_viewing = t_scale3f(0.25, 0.25, 1.0);
	v_message = std::vector<std::wstring>{a_message};
	v_duration = 0.5 * 64.0;
	v_sound_miss.f_play();
}

void t_training::f_ball_in()
{
	v_mark->f_mark(*v_ball);
	if (v_ball->v_hitter != v_player0.get()) return;
	v_text_viewing = t_scale3f(0.25, 0.25, 1.0);
	v_message = std::vector<std::wstring>{L"IN"};
}

void t_training::f_step_things()
{
	t_stage::f_step_things();
	v_camera1.v_position = t_vector3f((v_ball->v_position.v_x + v_player0->f_root_position().v_x) * 0.5, 4.0, (v_ball->v_position.v_z + 40.0 * v_player1->v_end) * 0.5);
	v_camera1.v_toward = t_vector3f(0.0, -6.0, -40.0 * v_player1->v_end);
}

t_training::t_training(t_main& a_main, const std::function<void (t_stage::t_state&, t_player&)>& a_controller0, const std::wstring& a_player0, const std::wstring& a_player1) : t_stage(a_main, true, false, a_controller0, a_player0, [](t_stage::t_state& a_state, t_player& a_player)
{
	auto step = std::move(a_state.v_step);
	a_state.v_step = [step, &a_player](t_stage& a_stage)
	{
		if (a_player.v_state != &t_player::v_state_swing) a_player.v_left = a_player.v_right = a_player.v_forward = a_player.v_backward = false;
		step(a_stage);
	};
}, a_player1)
{
	auto f = [this](t_state& a_state)
	{
		auto key_press = std::move(a_state.v_key_press);
		a_state.v_key_press = [this, key_press](t_stage& a_stage, SDL_Keycode a_key)
		{
			switch (a_key) {
			case SDLK_RETURN:
				v_side = -v_side;
				f_transit_ready();
				break;
			default:
				key_press(a_stage, a_key);
			}
		};
	};
	f(v_state_ready);
	f(v_state_play);
	v_camera0.v_position = t_vector3f(0.0, 14.0, 0.0);
	v_camera0.v_toward = t_vector3f(0.0, -12.0, -40.0 * v_player0->v_end);
	f_transit_select();
}

void t_training::f_next()
{
	if (v_ball->v_done) f_transit_ready();
}

void t_training::f_reset(float a_x, float a_y, float a_z, const t_vector3f& a_position, const t_player::t_swing& a_shot)
{
	v_ball->f_reset(v_side, a_x, a_y, a_z);
	v_mark->v_duration = 0.0;
	v_player0->v_placement->v_position = a_position;
	v_player0->v_placement->v_valid = false;
	v_player0->f_reset(1.0, t_player::v_state_default);
	t_posture posture;
	posture.f_validate();
	v_player1->v_placement->v_position = v_ball->v_position - f_affine(posture * a_shot.v_spot, t_vector3f(0.0, 0.0, 0.0));
	v_player1->v_placement->v_position.v_y = 0.0;
	v_player1->v_placement->v_valid = false;
	v_player1->f_reset(-1.0, t_player::v_state_default);
}

void t_training::f_toss(t_player::t_swing& a_shot)
{
	v_player1->v_placement->v_toward = v_player1->f_shot_direction();
	v_player1->v_placement->v_valid = false;
	v_player1->v_motion = std::make_unique<t_player::t_motion>(a_shot);
	v_player1->f_transit(t_player::v_state_swing);
}

const std::vector<std::wstring> t_training::v_toss_message{
	L"  CHANGE SIDES: START",
	L"      POSITION: +    ",
	L"COURCE & SWING: + & *"
};

const std::vector<t_training::t_item> t_training::v_items{
	t_item{L" SERVE ", [](t_training& a_stage)
	{
		a_stage.v_ball->f_reset(a_stage.v_side, 2 * 12 * 0.0254 * a_stage.v_side, 0.875, 39 * 12 * 0.0254);
		a_stage.v_mark->v_duration = 0.0;
		a_stage.v_player0->f_reset(1.0, t_player::v_state_serve_set);
		a_stage.v_player1->v_placement->v_position = t_vector3f(-9 * 12 * 0.0254 * a_stage.v_side, 0.0, -39 * 12 * 0.0254);
		a_stage.v_player1->v_placement->v_valid = false;
		a_stage.v_player1->f_reset(-1.0, t_player::v_state_default);
	}, [](t_training& a_stage)
	{
		a_stage.f_transit_ready();
	}, [](t_training& a_stage)
	{
		a_stage.v_text_viewing = t_matrix4f(1.0) * t_translate3f(0.0, -0.625, 0.0) * t_scale3f(0.125, 0.125, 1.0);
		a_stage.v_message = std::vector<std::wstring>{
			L"CHANGE SIDES: START",
			L"    POSITION: < + >",
			L"        TOSS:   *  ",
			L"      COURCE: < + >",
			L"       SWING:   *  "
		};
		a_stage.v_duration = 0.0 * 64.0;
	}, [](t_training& a_stage)
	{
	}, [](t_training& a_stage)
	{
		a_stage.f_transit_select();
	}},
	t_item{L" STROKE", [](t_training& a_stage)
	{
		a_stage.f_reset(3 * 12 * 0.0254 * a_stage.v_side, 1.0, -39 * 12 * 0.0254, t_vector3f((0.0 - 3.2 * a_stage.v_side) * 12 * 0.0254, 0.0, 39 * 12 * 0.0254), a_stage.v_player1->v_actions.v_swing.v_toss);
	}, [](t_training& a_stage)
	{
		a_stage.f_transit_ready();
	}, [](t_training& a_stage)
	{
		a_stage.v_text_viewing = t_matrix4f(1.0) * t_translate3f(0.0, -0.75, 0.0) * t_scale3f(0.125, 0.125, 1.0);
		a_stage.v_message = v_toss_message;
		a_stage.v_duration = 0.5 * 64.0;
	}, [](t_training& a_stage)
	{
		a_stage.f_toss(a_stage.v_player1->v_actions.v_swing.v_toss);
	}, [](t_training& a_stage)
	{
		a_stage.f_transit_select();
	}},
	t_item{L" VOLLEY", [](t_training& a_stage)
	{
		a_stage.f_reset(3 * 12 * 0.0254 * a_stage.v_side, 1.0, -39 * 12 * 0.0254, t_vector3f((0.1 - 2.0 * a_stage.v_side) * 12 * 0.0254, 0.0, 13 * 12 * 0.0254), a_stage.v_player1->v_actions.v_swing.v_toss);
	}, [](t_training& a_stage)
	{
		a_stage.f_transit_ready();
	}, [](t_training& a_stage)
	{
		a_stage.v_text_viewing = t_matrix4f(1.0) * t_translate3f(0.0, -0.75, 0.0) * t_scale3f(0.125, 0.125, 1.0);
		a_stage.v_message = v_toss_message;
		a_stage.v_duration = 0.5 * 64.0;
	}, [](t_training& a_stage)
	{
		a_stage.f_toss(a_stage.v_player1->v_actions.v_swing.v_toss);
	}, [](t_training& a_stage)
	{
		a_stage.f_transit_select();
	}},
	t_item{L" SMASH ", [](t_training& a_stage)
	{
		a_stage.f_reset(3 * 12 * 0.0254 * a_stage.v_side, 1.0, -39 * 12 * 0.0254, t_vector3f((0.4 - 0.4 * a_stage.v_side) * 12 * 0.0254, 0.0, 11.5 * 12 * 0.0254), a_stage.v_player1->v_actions.v_swing.v_toss_lob);
	}, [](t_training& a_stage)
	{
		a_stage.f_transit_ready();
	}, [](t_training& a_stage)
	{
		a_stage.v_text_viewing = t_matrix4f(1.0) * t_translate3f(0.0, -0.75, 0.0) * t_scale3f(0.125, 0.125, 1.0);
		a_stage.v_message = v_toss_message;
		a_stage.v_duration = 0.5 * 64.0;
	}, [](t_training& a_stage)
	{
		a_stage.f_toss(a_stage.v_player1->v_actions.v_swing.v_toss_lob);
	}, [](t_training& a_stage)
	{
		a_stage.f_transit_select();
	}},
	t_item{L" BACK  ", [](t_training& a_stage)
	{
		a_stage.v_ball->f_reset(a_stage.v_side, 2 * 12 * 0.0254, t_ball::c_radius + 0.01, 2 * 12 * 0.0254);
		a_stage.v_mark->v_duration = 0.0;
		a_stage.v_player0->v_placement->v_position = t_vector3f((0.1 - 2.0 * a_stage.v_side) * 12 * 0.0254, 0.0, 13 * 12 * 0.0254);
		a_stage.v_player0->v_placement->v_valid = false;
		a_stage.v_player0->f_reset(1.0, t_player::v_state_default);
		a_stage.v_player1->v_placement->v_position = t_vector3f((0.1 + 2.0 * a_stage.v_side) * 12 * 0.0254, 0.0, -13 * 12 * 0.0254);
		a_stage.v_player1->v_placement->v_valid = false;
		a_stage.v_player1->f_reset(-1.0, t_player::v_state_default);
	}, [](t_training& a_stage)
	{
		a_stage.f_back();
	}, [](t_training& a_stage)
	{
	}, [](t_training& a_stage)
	{
	}, [](t_training& a_stage)
	{
		a_stage.f_exit();
	}}
};

void t_training::f_update_select()
{
	v_text_viewing = t_matrix4f(1.0) * t_translate3f(0.0, 0.0, 0.0) * t_scale3f(0.25, 0.25, 1.0);
	v_message.clear();
	for (const auto& item : v_items) v_message.push_back((v_message.size() == v_selected ? L"*" : L" ") + item.v_label);
	f_step_things();
}

void t_training::f_back()
{
	v_items[v_selected].v_back(*this);
}

void t_training::f_exit()
{
	v_main.v_screen = std::make_unique<t_main_screen>(v_main);
}

void t_training::f_up()
{
	if (v_selected <= 0) return;
	--v_selected;
	v_main.v_sound_cursor.f_play();
	v_items[v_selected].v_reset(*this);
	f_update_select();
}

void t_training::f_down()
{
	if (v_selected >= v_items.size() - 1) return;
	++v_selected;
	v_main.v_sound_cursor.f_play();
	v_items[v_selected].v_reset(*this);
	f_update_select();
}

void t_training::f_select()
{
	v_main.v_sound_select.f_play();
	v_items[v_selected].v_do(*this);
}

const t_stage::t_state t_training::v_state_select{
	[](t_stage& a_stage)
	{
	},
	[](t_stage& a_stage, SDL_Keycode a_key)
	{
		t_training& stage = static_cast<t_training&>(a_stage);
		switch (a_key) {
		case SDLK_ESCAPE:
			stage.f_exit();
			break;
		case SDLK_SPACE:
		case SDLK_2:
			stage.f_select();
			break;
		case SDLK_UP:
		case SDLK_e:
			stage.f_up();
			break;
		case SDLK_DOWN:
		case SDLK_c:
			stage.f_down();
			break;
		}
	},
	[](t_stage& a_stage, SDL_Keycode a_key)
	{
	}
};

void t_training::f_transit_play()
{
	v_state = &v_state_play;
	v_items[v_selected].v_play(*this);
}

void t_training::f_transit_select()
{
	v_state = &v_state_select;
	v_side = 1.0;
	v_items[v_selected].v_reset(*this);
	v_duration = 0.0 * 64.0;
	f_update_select();
}

void t_training::f_transit_ready()
{
	v_state = &v_state_ready;
	v_items[v_selected].v_reset(*this);
	v_items[v_selected].v_ready(*this);
	f_step_things();
}

void f_controller0(t_stage::t_state& a_state, t_player& a_player)
{
	auto key_press = std::move(a_state.v_key_press);
	a_state.v_key_press = [key_press, &a_player](t_stage& a_stage, SDL_Keycode a_key)
	{
		switch (a_key) {
		case SDLK_1:
		case SDLK_j:
			a_player.f_do(&t_player::t_shots::v_topspin);
			break;
		case SDLK_SPACE:
		case SDLK_2:
		case SDLK_l:
			a_player.f_do(&t_player::t_shots::v_flat);
			break;
		case SDLK_VOLUMEUP:
		case SDLK_i:
			a_player.f_do(&t_player::t_shots::v_lob);
			break;
		case SDLK_VOLUMEDOWN:
		case SDLK_m:
			a_player.f_do(&t_player::t_shots::v_slice);
			break;
		case SDLK_LEFT:
		case SDLK_s:
			a_player.v_left = true;
			break;
		case SDLK_RIGHT:
		case SDLK_f:
			a_player.v_right = true;
			break;
		case SDLK_UP:
		case SDLK_e:
			a_player.v_forward = true;
			break;
		case SDLK_DOWN:
		case SDLK_c:
			a_player.v_backward = true;
			break;
		default:
			key_press(a_stage, a_key);
		}
	};
	auto key_release = std::move(a_state.v_key_release);
	a_state.v_key_release = [key_release, &a_player](t_stage& a_stage, SDL_Keycode a_key)
	{
		switch (a_key) {
		case SDLK_LEFT:
		case SDLK_s:
			a_player.v_left = false;
			break;
		case SDLK_RIGHT:
		case SDLK_f:
			a_player.v_right = false;
			break;
		case SDLK_UP:
		case SDLK_e:
			a_player.v_forward = false;
			break;
		case SDLK_DOWN:
		case SDLK_c:
			a_player.v_backward = false;
			break;
		default:
			key_release(a_stage, a_key);
		}
	};
}

void f_controller1(t_stage::t_state& a_state, t_player& a_player)
{
	auto key_press = std::move(a_state.v_key_press);
	a_state.v_key_press = [key_press, &a_player](t_stage& a_stage, SDL_Keycode a_key)
	{
		switch (a_key) {
		case SDLK_7:
			a_player.f_do(&t_player::t_shots::v_topspin);
			break;
		case SDLK_9:
			a_player.f_do(&t_player::t_shots::v_flat);
			break;
		case SDLK_COMMA:
			a_player.f_do(&t_player::t_shots::v_lob);
			break;
		case SDLK_PERIOD:
			a_player.f_do(&t_player::t_shots::v_slice);
			break;
		case SDLK_4:
			a_player.v_left = true;
			break;
		case SDLK_6:
			a_player.v_right = true;
			break;
		case SDLK_8:
			a_player.v_forward = true;
			break;
		case SDLK_5:
			a_player.v_backward = true;
			break;
		default:
			key_press(a_stage, a_key);
		}
	};
	auto key_release = std::move(a_state.v_key_release);
	a_state.v_key_release = [key_release, &a_player](t_stage& a_stage, SDL_Keycode a_key)
	{
		switch (a_key) {
		case SDLK_4:
			a_player.v_left = false;
			break;
		case SDLK_6:
			a_player.v_right = false;
			break;
		case SDLK_8:
			a_player.v_forward = false;
			break;
		case SDLK_5:
			a_player.v_backward = false;
			break;
		default:
			key_release(a_stage, a_key);
		}
	};
}

void t_menu::f_render(const t_matrix4f& a_viewing)
{
	size_t n = (v_items.size() + v_columns - 1) / v_columns;
	auto viewing = a_viewing * t_scale3f(1.0 / n, 1.0 / n, 1.0);
	float dx = 8.0 * 2.0 / v_columns;
	float x = dx * 0.5 - 8.0;
	size_t k = 0;
	for (size_t i = 0; i < v_columns; ++i) {
		float y = 0.0;
		for (size_t j = 0; j < n && k < v_items.size(); ++j) {
			const auto& item = v_items[k];
			auto text = (k == v_selected ? L"*" : L" ") + item.v_label;
			y -= 1.0;
			v_main.v_text_renderer(v_main.v_text_projection, viewing * t_translate3f(x - text.size() * 0.25, y, 0.0), text);
			++k;
		}
		x += dx;
	}
}

void t_menu::f_up()
{
	if (v_selected <= 0) return;
	--v_selected;
	v_main.v_sound_cursor.f_play();
}

void t_menu::f_down()
{
	if (v_selected >= v_items.size() - 1) return;
	++v_selected;
	v_main.v_sound_cursor.f_play();
}

void t_menu::f_left()
{
	size_t n = (v_items.size() + v_columns - 1) / v_columns;
	if (v_selected < n) return;
	v_selected -= n;
	v_main.v_sound_cursor.f_play();
}

void t_menu::f_right()
{
	size_t n = (v_items.size() + v_columns - 1) / v_columns;
	if (v_selected >= v_items.size() - n) return;
	v_selected += n;
	v_main.v_sound_cursor.f_play();
}

void t_menu::f_select()
{
	v_main.v_sound_select.f_play();
	v_items[v_selected].v_do();
}

void t_menu::f_key_press(SDL_Keycode a_key)
{
	switch (a_key) {
	case SDLK_ESCAPE:
		v_back();
		break;
	case SDLK_SPACE:
	case SDLK_2:
		f_select();
		break;
	case SDLK_LEFT:
	case SDLK_s:
		f_left();
		break;
	case SDLK_RIGHT:
	case SDLK_f:
		f_right();
		break;
	case SDLK_UP:
	case SDLK_e:
		f_up();
		break;
	case SDLK_DOWN:
	case SDLK_c:
		f_down();
		break;
	}
}

t_dialog::t_dialog(t_main& a_main, const std::wstring& a_image, const std::wstring& a_sound, const std::wstring& a_title) : v_main(a_main), v_title(a_title)
{
	v_image.f_create_from_file(t_path(v_main.v_root) / L".." / a_image, 1);
	v_main.f_load(v_sound, a_sound);
	v_sound.f_set(AL_LOOPING, true);
	v_sound.f_play();
}

void t_dialog::f_render(size_t a_width, size_t a_height, const t_matrix4f& a_viewing)
{
	float a = a_width / (a_height * v_image.v_unit);
	a = 2.0 * (a < 1.0 ? 1.0 : a);
	auto viewing = a_viewing * t_scale3f(a, a, 1.0) * t_translate3f(-0.5 * v_image.v_unit, -0.5, 0.0);
	v_image(v_main.v_text_projection, viewing, std::wstring(1, L'\0'));
	viewing = a_viewing * t_translate3f(0.0, 0.5, 0.0) * t_scale3f(1.5 / 4.0, 1.5 / 4.0, 1.0) * t_translate3f(v_title.size() * -0.25, 0.0, 0.0);
	v_main.v_text_renderer(v_main.v_text_projection, viewing, v_title);
}

t_stage_menu::t_stage_menu(t_main_screen& a_screen, const std::wstring& a_image, const std::wstring& a_sound, const std::wstring& a_title, const std::function<void (const std::wstring&, const std::wstring&)>& a_done) : t_dialog(a_screen.v_main, a_image, a_sound, a_title), v_player0(a_screen.v_main, 2), v_player1(a_screen.v_main, 2), v_menu(&v_player0)
{
	v_player0.v_back = [this, &a_screen]
	{
		v_sound.f_stop();
		a_screen.f_transit(std::make_unique<t_main_menu>(a_screen), 1.0);
	};
	v_player1.v_back = [this]
	{
		f_transit(v_player0, 1.0);
	};
	for (const auto& x : a_screen.v_players) {
		v_player0.v_items.push_back(t_menu::t_item{std::get<0>(x), [this]
		{
			f_transit(v_player1, -1.0);
		}});
		v_player1.v_items.push_back(t_menu::t_item{std::get<0>(x), [this, &a_screen, a_done]
		{
			a_done(std::get<1>(a_screen.v_players[v_player0.v_selected]), std::get<1>(a_screen.v_players[v_player1.v_selected]));
		}});
	}
}

void t_stage_menu::f_step()
{
	if (!v_transit) return;
	if (v_t < v_duration)
		v_t += 1.0;
	else
		v_transit = nullptr;
}

void t_stage_menu::f_render(size_t a_width, size_t a_height, const t_matrix4f& a_viewing)
{
	t_dialog::f_render(a_width, a_height, a_viewing);
	auto message = v_player0.v_items[v_player0.v_selected].v_label;
	message += v_menu == &v_player0 ? L"? vs     " : L" vs " + v_player1.v_items[v_player1.v_selected].v_label + L"?";
	auto viewing = a_viewing * t_translate3f(0.0, 0.25, 0.0) * t_scale3f(1.0 / 4.0, 1.0 / 4.0, 1.0) * t_translate3f(message.size() * -0.25, -0.5, 0.0);
	v_main.v_text_renderer(v_main.v_text_projection, viewing, message);
	viewing = a_viewing;
	if (v_transit) {
		float a = 2.0 * v_direction * a_width / a_height;
		float t = v_t / v_duration;
		v_transit->f_render(viewing * t_translate3f(a * t, 0.0, 0.0));
		viewing *= t_translate3f(a * (t - 1.0), 0.0, 0.0);
	}
	v_menu->f_render(viewing * t_translate3f(0.0, 0.0, 0.0));
}

void t_stage_menu::f_key_press(SDL_Keycode a_key)
{
	if (!v_transit) v_menu->f_key_press(a_key);
}

void t_stage_menu::f_transit(t_menu& a_menu, float a_direction)
{
	v_transit = v_menu;
	v_menu = &a_menu;
	v_direction = a_direction;
	v_duration = 30.0;
	v_t = 0.0;
}

t_main_menu::t_main_menu(t_main_screen& a_screen) : t_dialog(a_screen.v_main, L"data/main-background.jpg", L"data/main-background.wav", L"TENNIS"), t_menu(a_screen.v_main)
{
	v_back = []
	{
		SDL_Event event;
		event.type = SDL_QUIT;
		SDL_PushEvent(&event);
	};
	v_items = std::vector<t_item>{
		t_item{L"  1P vs COM  ", [this, &a_screen]
		{
			v_sound.f_stop();
			a_screen.f_transit(std::make_unique<t_stage_menu>(a_screen, L"data/main-background.jpg", L"data/main-background.wav", L"1P vs COM", [&a_screen](const std::wstring& a_player0, const std::wstring& a_player1)
			{
				a_screen.v_main.v_screen = std::make_unique<t_match>(a_screen.v_main, false, false, f_controller0, a_player0, f_computer, a_player1);
			}), -1.0);
		}},
		t_item{L"  1P vs 2P   ", [this, &a_screen]
		{
			v_sound.f_stop();
			a_screen.f_transit(std::make_unique<t_stage_menu>(a_screen, L"data/main-background.jpg", L"data/main-background.wav", L"1P vs 2P", [&a_screen](const std::wstring& a_player0, const std::wstring& a_player1)
			{
				a_screen.v_main.v_screen = std::make_unique<t_match>(a_screen.v_main, true, false, f_controller0, a_player0, f_controller1, a_player1);
			}), -1.0);
		}},
		t_item{L" COM vs COM  ", [this, &a_screen]
		{
			v_sound.f_stop();
			a_screen.f_transit(std::make_unique<t_stage_menu>(a_screen, L"data/main-background.jpg", L"data/main-background.wav", L"COM vs COM", [&a_screen](const std::wstring& a_player0, const std::wstring& a_player1)
			{
				a_screen.v_main.v_screen = std::make_unique<t_match>(a_screen.v_main, false, true, f_computer, a_player0, f_computer, a_player1);
			}), -1.0);
		}},
		t_item{L"  TRAINING   ", [this, &a_screen]
		{
			v_sound.f_stop();
			a_screen.f_transit(std::make_unique<t_stage_menu>(a_screen, L"data/main-background.jpg", L"data/training-background.wav", L"TRAINING", [&a_screen](const std::wstring& a_player0, const std::wstring& a_player1)
			{
				a_screen.v_main.v_screen = std::make_unique<t_training>(a_screen.v_main, f_controller0, a_player0, a_player1);
			}), -1.0);
		}},
		t_item{L"    EXIT     ", v_back}
	};
}

void t_main_menu::f_step()
{
}

void t_main_menu::f_render(size_t a_width, size_t a_height, const t_matrix4f& a_viewing)
{
	t_dialog::f_render(a_width, a_height, a_viewing);
	t_menu::f_render(a_viewing * t_translate3f(0.0, 0.125, 0.0));
}

void t_main_menu::f_key_press(SDL_Keycode a_key)
{
	t_menu::f_key_press(a_key);
}

t_main_screen::t_main_screen(t_main& a_main) : t_screen(a_main)
{
	{
		auto source = t_path(a_main.v_root) / L"../data/players";
		t_reader reader(source);
		reader.f_read_next();
		reader.f_move_to_tag();
		reader.f_check_start_element(L"players");
		reader.f_read_next();
		while (reader.f_is_start_element(L"player")) {
			auto name = reader.f_get_attribute(L"name");
			std::wstring path = source / L".." / reader.f_get_attribute(L"path");
			reader.f_read_element_text();
			v_players.emplace_back(name, path);
		}
		reader.f_end_element();
	}
	v_dialog = std::make_unique<t_main_menu>(*this);
}

void t_main_screen::f_step()
{
	if (v_transit) {
		if (v_t < v_duration)
			v_t += 1.0;
		else
			v_transit = nullptr;
	}
	v_dialog->f_step();
}

void t_main_screen::f_render(size_t a_width, size_t a_height)
{
	glViewport(0, 0, a_width, a_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	auto viewing = t_matrix4f(1.0);
	if (v_transit) {
		float a = 2.0 * v_direction * a_width / a_height;
		float t = v_t / v_duration;
		v_transit->f_render(a_width, a_height, viewing * t_translate3f(a * t, 0.0, 0.0));
		viewing *= t_translate3f(a * (t - 1.0), 0.0, 0.0);
	}
	v_dialog->f_render(a_width, a_height, viewing);
}

void t_main_screen::f_key_press(SDL_Keycode a_key)
{
	if (!v_transit) v_dialog->f_key_press(a_key);
}

void t_main_screen::f_key_release(SDL_Keycode a_key)
{
}

void t_main_screen::f_transit(std::unique_ptr<t_dialog>&& a_dialog, float a_direction)
{
	v_transit = std::move(v_dialog);
	v_dialog = std::move(a_dialog);
	v_direction = a_direction;
	v_duration = 30.0;
	v_t = 0.0;
}

void f_loop(SDL_Window* a_window, const std::wstring& a_root)
{
	t_main main{a_root};
	main.v_text_renderer.f_create_from_font(t_path(a_root) / L"../font.ttf");
	main.f_load(main.v_sound_cursor, L"data/cursor.wav");
	main.f_load(main.v_sound_select, L"data/select.wav");
	main.v_screen = std::make_unique<t_main_screen>(main);
	SDL_GL_SetSwapInterval(1);
	int width;
	int height;
	SDL_GetWindowSize(a_window, &width, &height);
	auto f_resize = [&](int a_width, int a_height)
	{
		width = a_width;
		height = a_height;
		float w = static_cast<float>(width) / height;
		main.v_text_projection = f_orthographic(-w, w, -1.0f, 1.0f, -1.0f, 1.0f);
	};
	f_resize(width, height);
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
					f_resize(event.window.data1, event.window.data2);
					break;
				}
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				case SDLK_q:
					return;
				default:
					main.v_screen->f_key_press(event.key.keysym.sym);
				}
				break;
			case SDL_KEYUP:
				main.v_screen->f_key_release(event.key.keysym.sym);
				break;
			case SDL_MOUSEMOTION:
				break;
			case SDL_MOUSEBUTTONDOWN:
				break;
			case SDL_MOUSEBUTTONUP:
				break;
			}
		}
		main.v_screen->f_render(width, height);
		SDL_GL_SwapWindow(a_window);
		main.v_screen->f_step();
	}
}

int main(int argc, char* argv[])
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	t_sdl sdl(SDL_INIT_VIDEO);
	t_sdl_image image(IMG_INIT_PNG | IMG_INIT_JPG);
	t_sdl_ttf ttf;
	al::t_alut alut(NULL, NULL);
	t_window window("Tennis", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	t_gl_context context(window);
	f_loop(window, f_convert(argv[0]));
	return 0;
}
