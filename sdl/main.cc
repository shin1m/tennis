#include "main.h"

#include <cstring>
#include <SDL_main.h>

#include "computer.h"

t_main::t_main(const std::wstring& a_prefix, bool a_show_pad) : v_prefix(a_prefix), v_show_pad(a_show_pad)
{
	v_font.f_create(f_path(L"font.ttf.png"));
	f_load(v_sound_cursor, L"cursor.wav");
	f_load(v_sound_select, L"select.wav");
	std::array<float, 12> array{
		0.0, 2.0, 0.0,
		-1.0, 0.0, 0.0,
		1.0, 0.0, 0.0,
		0.0, 2.0, 0.0
	};
	v_triangle.f_create();
	gl::f_bind_buffer(GL_ARRAY_BUFFER, v_triangle);
	gl::f_buffer_data(GL_ARRAY_BUFFER, array.size() * sizeof(float), array.data(), GL_STATIC_DRAW);
	gl::f_bind_buffer(GL_ARRAY_BUFFER, 0);
	int n = SDL_GameControllerAddMappingsFromFile(f_convert(f_path(L"gamecontrollerdb.txt")).c_str());
	if (n == -1) throw std::runtime_error((std::string("SDL_GameControllerAddMappingsFromFile Error: ") + SDL_GetError()).c_str());
	f_setup_controllers();
}

std::unique_ptr<xmlParserInputBuffer, void (*)(xmlParserInputBufferPtr)> t_main::f_input(const std::wstring& a_name)
{
	SDL_RWops* p = SDL_RWFromFile(f_convert(f_path(a_name)).c_str(), "rb");
	if (p == NULL) throw std::runtime_error((std::string("SDL_RWFromFile Error: ") + SDL_GetError()).c_str());
	return std::unique_ptr<xmlParserInputBuffer, void (*)(xmlParserInputBufferPtr)>(xmlParserInputBufferCreateIO([](void* a_p, char* a_buffer, int a_length) -> int
	{
		auto p = static_cast<SDL_RWops*>(a_p);
		return SDL_RWread(p, a_buffer, 1, a_length);
	}, [](void* a_p) -> int
	{
		auto p = static_cast<SDL_RWops*>(a_p);
		return SDL_RWclose(p) == 0 ? 0 : -1;
	}, p, XML_CHAR_ENCODING_NONE), xmlFreeParserInputBuffer);
}

void t_main::f_load(t_document& a_document, const std::wstring& a_name)
{
	auto input = f_input(a_name);
	t_reader reader(input.get());
	size_t n = a_name.find_last_of(L'/');
	a_document.f_load(reader, f_path(n == std::wstring::npos ? std::wstring() : a_name.substr(0, n + 1)));
	a_document.v_texture_format = GL_RGB5_A1;
	a_document.f_build(v_shaders);
}

void t_main::f_load(gl::t_image& a_image, const std::wstring& a_name)
{
	a_image.f_create(f_path(a_name), GL_RGB5_A1);
}

std::vector<std::string> joysticks;

void t_main::f_setup_controllers()
{
	for (auto& x : v_controllers) x.f_close();
	size_t j = 0;
	int n = SDL_NumJoysticks();
joysticks.clear();
	for (int i = 0; i < n; ++i) {
char guid[33];
SDL_JoystickGetGUIDString(SDL_JoystickGetDeviceGUID(i), guid, sizeof(guid));
SDL_Log("joystick guid: %s", guid);
joysticks.push_back(std::string(guid) + (SDL_IsGameController(i) == SDL_TRUE ? " is gc" : " is not gc"));
		if (SDL_IsGameController(i) != SDL_TRUE) continue;
SDL_Log("is game controller");
		try {
			v_controllers[j].f_open(i);
			if (++j >= sizeof(v_controllers) / sizeof(t_game_controller)) break;
		} catch (std::exception& e) {
			SDL_Log("caught: %s", e.what());
		}
	}
}

SDL_Keycode t_main::f_keycode(const SDL_ControllerButtonEvent& a_button) const
{
	if (v_controllers[0] && a_button.which == v_controllers[0].f_joystick_id()) {
		switch (a_button.button) {
		case SDL_CONTROLLER_BUTTON_A:
			return SDLK_VOLUMEDOWN;
		case SDL_CONTROLLER_BUTTON_B:
			return SDLK_2;
		case SDL_CONTROLLER_BUTTON_X:
			return SDLK_1;
		case SDL_CONTROLLER_BUTTON_Y:
			return SDLK_VOLUMEUP;
		case SDL_CONTROLLER_BUTTON_BACK:
		case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
		case SDL_CONTROLLER_BUTTON_LEFTSTICK:
			return SDLK_ESCAPE;
		case SDL_CONTROLLER_BUTTON_GUIDE:
			return SDLK_TAB;
		case SDL_CONTROLLER_BUTTON_START:
		case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
		case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
			return SDLK_RETURN;
		case SDL_CONTROLLER_BUTTON_DPAD_UP:
			return SDLK_UP;
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
			return SDLK_DOWN;
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
			return SDLK_LEFT;
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
			return SDLK_RIGHT;
		}
	} else if (v_controllers[1] && a_button.which == v_controllers[1].f_joystick_id()) {
		switch (a_button.button) {
		case SDL_CONTROLLER_BUTTON_A:
			return SDLK_PERIOD;
		case SDL_CONTROLLER_BUTTON_B:
			return SDLK_9;
		case SDL_CONTROLLER_BUTTON_X:
			return SDLK_7;
		case SDL_CONTROLLER_BUTTON_Y:
			return SDLK_COMMA;
		case SDL_CONTROLLER_BUTTON_BACK:
		case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
		case SDL_CONTROLLER_BUTTON_LEFTSTICK:
			return SDLK_ESCAPE;
		case SDL_CONTROLLER_BUTTON_GUIDE:
			return SDLK_TAB;
		case SDL_CONTROLLER_BUTTON_START:
		case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
		case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
			return SDLK_RETURN;
		case SDL_CONTROLLER_BUTTON_DPAD_UP:
			return SDLK_8;
		case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
			return SDLK_5;
		case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
			return SDLK_4;
		case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
			return SDLK_6;
		}
	}
	return SDLK_UNKNOWN;
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
	},
	[](t_stage& a_stage, size_t a_width, size_t a_height)
	{
	},
	[](t_stage& a_stage, const SDL_TouchFingerEvent& a_event, size_t a_width, size_t a_height)
	{
	},
	[](t_stage& a_stage, const SDL_TouchFingerEvent& a_event, size_t a_width, size_t a_height)
	{
		if (a_event.y > 0.25) return;
		t_match& stage = static_cast<t_match&>(a_stage);
		if (a_event.x < 0.5)
			a_stage.f_back();
		else
			stage.f_new_set();
	},
	[](t_stage& a_stage, const SDL_TouchFingerEvent& a_event, size_t a_width, size_t a_height)
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

const std::vector<std::wstring> t_training::v_toss_message{
	L"  CHANGE SIDES: START",
	L"      POSITION: +    ",
	L"COURCE & SWING: + & *"
};

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
}, a_player1), v_menu(a_main)
{
	auto f = [this](t_state& a_state)
	{
		auto key_press = std::move(a_state.v_key_press);
		a_state.v_key_press = [this, key_press](t_stage& a_stage, SDL_Keycode a_key)
		{
			switch (a_key) {
			case SDLK_TAB:
				v_side = -v_side;
			case SDLK_RETURN:
				f_transit_ready();
				break;
			default:
				key_press(a_stage, a_key);
			}
		};
		auto finger_up = std::move(a_state.v_finger_up);
		a_state.v_finger_up = [this, finger_up](t_stage& a_stage, const SDL_TouchFingerEvent& a_event, size_t a_width, size_t a_height)
		{
			if (a_event.x > 0.5 && a_event.y < 0.5) {
				v_side = -v_side;
				f_transit_ready();
			} else {
				finger_up(a_stage, a_event, a_width, a_height);
			}
		};
	};
	f(v_state_ready);
	f(v_state_play);
	v_camera0.v_position = t_vector3f(0.0, 14.0, 0.0);
	v_camera0.v_toward = t_vector3f(0.0, -12.0, -40.0 * v_player0->v_end);
	v_menu.v_back = [this]
	{
		f_exit();
	};
	v_menu.v_items.assign({
		t_item{L" SERVE ", [this]
		{
			v_ball->f_reset(v_side, 2 * 12 * 0.0254 * v_side, 0.875, 39 * 12 * 0.0254);
			v_mark->v_duration = 0.0;
			v_player0->f_reset(1.0, t_player::v_state_serve_set);
			v_player1->v_placement->v_position = t_vector3f(-9 * 12 * 0.0254 * v_side, 0.0, -39 * 12 * 0.0254);
			v_player1->v_placement->v_valid = false;
			v_player1->f_reset(-1.0, t_player::v_state_default);
			f_step_things();
		}, [this]
		{
			f_transit_ready();
		}, [this]
		{
			v_text_viewing = t_matrix4f(1.0) * t_translate3f(0.0, -0.625, 0.0) * t_scale3f(0.125, 0.125, 1.0);
			v_message = std::vector<std::wstring>{
				L"CHANGE SIDES: START",
				L"    POSITION: < + >",
				L"        TOSS:   *  ",
				L"      COURCE: < + >",
				L"       SWING:   *  "
			};
			v_duration = 0.0 * 64.0;
		}, []
		{
		}, [this]
		{
			f_transit_select();
		}},
		t_item{L" STROKE", [this]
		{
			f_reset(3 * 12 * 0.0254 * v_side, 1.0, -39 * 12 * 0.0254, t_vector3f((0.0 - 3.2 * v_side) * 12 * 0.0254, 0.0, 39 * 12 * 0.0254), v_player1->v_actions.v_swing.v_toss);
		}, [this]
		{
			f_transit_ready();
		}, [this]
		{
			v_text_viewing = t_matrix4f(1.0) * t_translate3f(0.0, -0.75, 0.0) * t_scale3f(0.125, 0.125, 1.0);
			v_message = v_toss_message;
			v_duration = 0.5 * 64.0;
		}, [this]
		{
			f_toss(v_player1->v_actions.v_swing.v_toss);
		}, [this]
		{
			f_transit_select();
		}},
		t_item{L" VOLLEY", [this]
		{
			f_reset(3 * 12 * 0.0254 * v_side, 1.0, -39 * 12 * 0.0254, t_vector3f((0.1 - 2.0 * v_side) * 12 * 0.0254, 0.0, 13 * 12 * 0.0254), v_player1->v_actions.v_swing.v_toss);
		}, [this]
		{
			f_transit_ready();
		}, [this]
		{
			v_text_viewing = t_matrix4f(1.0) * t_translate3f(0.0, -0.75, 0.0) * t_scale3f(0.125, 0.125, 1.0);
			v_message = v_toss_message;
			v_duration = 0.5 * 64.0;
		}, [this]
		{
			f_toss(v_player1->v_actions.v_swing.v_toss);
		}, [this]
		{
			f_transit_select();
		}},
		t_item{L" SMASH ", [this]
		{
			f_reset(3 * 12 * 0.0254 * v_side, 1.0, -39 * 12 * 0.0254, t_vector3f((0.4 - 0.4 * v_side) * 12 * 0.0254, 0.0, 9 * 12 * 0.0254), v_player1->v_actions.v_swing.v_toss_lob);
		}, [this]
		{
			f_transit_ready();
		}, [this]
		{
			v_text_viewing = t_matrix4f(1.0) * t_translate3f(0.0, -0.75, 0.0) * t_scale3f(0.125, 0.125, 1.0);
			v_message = v_toss_message;
			v_duration = 0.5 * 64.0;
		}, [this]
		{
			f_toss(v_player1->v_actions.v_swing.v_toss_lob);
		}, [this]
		{
			f_transit_select();
		}},
		t_item{L" BACK  ", [this]
		{
			v_ball->f_reset(v_side, 2 * 12 * 0.0254, t_ball::c_radius + 0.01, 2 * 12 * 0.0254);
			v_mark->v_duration = 0.0;
			v_player0->v_placement->v_position = t_vector3f((0.1 - 2.0 * v_side) * 12 * 0.0254, 0.0, 13 * 12 * 0.0254);
			v_player0->v_placement->v_valid = false;
			v_player0->f_reset(1.0, t_player::v_state_default);
			v_player1->v_placement->v_position = t_vector3f((0.1 + 2.0 * v_side) * 12 * 0.0254, 0.0, -13 * 12 * 0.0254);
			v_player1->v_placement->v_valid = false;
			v_player1->f_reset(-1.0, t_player::v_state_default);
			f_step_things();
		}, [this]
		{
			f_back();
		}, []
		{
		}, []
		{
		}, [this]
		{
			f_exit();
		}}
	});
	f_transit_select();
}

void t_training::f_next()
{
	if (v_ball->v_done) f_transit_ready();
}

void t_training::f_reset(float a_x, float a_y, float a_z, const t_vector3f& a_position, const t_player::t_swing& a_shot)
{
	v_ball->f_reset(v_side, a_x, a_y, a_z, false);
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
	f_step_things();
}

void t_training::f_toss(t_player::t_swing& a_shot)
{
	v_player1->v_placement->v_toward = v_player1->f_shot_direction();
	v_player1->v_placement->v_valid = false;
	v_player1->v_motion = std::make_unique<t_player::t_motion>(a_shot);
	v_player1->f_transit(t_player::v_state_swing);
}

void t_training::f_back()
{
	v_menu.f_selected().v_back();
}

void t_training::f_exit()
{
	v_main.v_screen = std::make_unique<t_main_screen>(v_main);
}

const t_stage::t_state t_training::v_state_select{
	[](t_stage& a_stage)
	{
	},
	[](t_stage& a_stage, SDL_Keycode a_key)
	{
		t_training& stage = static_cast<t_training&>(a_stage);
		stage.v_menu.f_key_press(a_key);
	},
	[](t_stage& a_stage, SDL_Keycode a_key)
	{
	},
	[](t_stage& a_stage, size_t a_width, size_t a_height)
	{
		t_training& stage = static_cast<t_training&>(a_stage);
		stage.v_menu.f_render(stage.f_transform());
	},
	[](t_stage& a_stage, const SDL_TouchFingerEvent& a_event, size_t a_width, size_t a_height)
	{
		t_training& stage = static_cast<t_training&>(a_stage);
		stage.v_menu.f_finger_down(a_event, stage.f_transform());
	},
	[](t_stage& a_stage, const SDL_TouchFingerEvent& a_event, size_t a_width, size_t a_height)
	{
		t_training& stage = static_cast<t_training&>(a_stage);
		stage.v_menu.f_finger_up(a_event, stage.f_transform());
	},
	[](t_stage& a_stage, const SDL_TouchFingerEvent& a_event, size_t a_width, size_t a_height)
	{
		t_training& stage = static_cast<t_training&>(a_stage);
		stage.v_menu.f_finger_motion(a_event, stage.f_transform());
	}
};

void t_training::f_transit_play()
{
	v_state = &v_state_play;
	v_menu.f_selected().v_play();
}

void t_training::f_transit_select()
{
	v_state = &v_state_select;
	v_side = 1.0;
	v_menu.f_selected().v_select();
	v_message.clear();
	v_duration = 0.0 * 64.0;
}

void t_training::f_transit_ready()
{
	v_state = &v_state_ready;
	v_menu.f_selected().v_select();
	v_menu.f_selected().v_ready();
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
	const float pad_size = 1.0;
	if (a_player.v_stage.v_main.v_show_pad) {
		auto render = std::move(a_state.v_render);
		a_state.v_render = [render, &a_player, pad_size](t_stage& a_stage, size_t a_width, size_t a_height)
		{
			if (a_stage.v_main.v_controllers[0]) {
				render(a_stage, a_width, a_height);
				return;
			}
			auto& shader = a_stage.v_main.v_shaders.f_constant_color();
			std::remove_reference<decltype(shader)>::type::t_uniforms uniforms;
			uniforms.v_projection = a_stage.v_main.v_projection.v_array;
			uniforms.v_color = t_vector4f(1.0, 1.0, 1.0, 1.0);
			std::remove_reference<decltype(shader)>::type::t_attributes attributes;
			attributes.v_vertices = a_stage.v_main.v_triangle;
			float a = static_cast<float>(a_width) / a_height;
			float size = std::min(a, pad_size);
			t_matrix4f triangle = t_translate3f(0.0, size * 0.25, 0.0);
			triangle *= t_scale3f(size * 0.1, size * 0.1, 1.0);
			t_matrix4f pad = t_translate3f(size * 0.5 - a, size * 0.5 - 1.0, 0.0);
			for (size_t i = 0; i < 8; ++i) {
				auto m = pad * t_rotate3f(t_vector3f(0.0, 0.0, 1.0), M_PI * i / 4.0) * triangle;
				uniforms.v_vertex = m.v_array;
				shader(uniforms, attributes, GL_LINE_STRIP, 0, 4);
			}
			pad = t_translate3f(a - size * 0.5, size * 0.5 - 1.0, 0.0);
			for (size_t i = 0; i < 4; ++i) {
				auto m = pad * t_rotate3f(t_vector3f(0.0, 0.0, 1.0), M_PI * i / 2.0) * triangle;
				uniforms.v_vertex = m.v_array;
				shader(uniforms, attributes, GL_LINE_STRIP, 0, 4);
			}
			render(a_stage, a_width, a_height);
		};
	}
	auto left_pad = [&a_player, pad_size](t_stage& a_stage, const SDL_TouchFingerEvent& a_event, size_t a_width, size_t a_height)
	{
		float a = static_cast<float>(a_width) / a_height;
		float size = std::min(a, pad_size);
		auto m = ~a_stage.v_main.v_projection;
		auto v = f_affine(m, t_vector3f(a_event.x * 2.0 - 1.0, a_event.y * -2.0 + 1.0, 0.0));
		auto u = v - t_vector3f(size * 0.5 - a, size * 0.5 - 1.0, 0.0);
		a_player.v_left = a_player.v_right = a_player.v_forward = a_player.v_backward = false;
		if (u.f_length() < size / 8.0) return;
		if (u.v_x == 0.0) {
			(u.v_y < 0.0 ? a_player.v_backward : a_player.v_forward) = true;
		} else {
			float a = std::fabs(u.v_y / u.v_x);
			if (a < std::tan(M_PI * 0.375)) (u.v_x < 0.0 ? a_player.v_left : a_player.v_right) = true;
			if (a > std::tan(M_PI * 0.125)) (u.v_y < 0.0 ? a_player.v_backward : a_player.v_forward) = true;
		}
	};
	auto finger_down = std::move(a_state.v_finger_down);
	a_state.v_finger_down = [left_pad, finger_down, &a_player, pad_size](t_stage& a_stage, const SDL_TouchFingerEvent& a_event, size_t a_width, size_t a_height)
	{
		if (a_event.x < 0.5) {
			left_pad(a_stage, a_event, a_width, a_height);
		} else {
			float a = static_cast<float>(a_width) / a_height;
			float size = std::min(a, pad_size);
			auto m = ~a_stage.v_main.v_projection;
			auto v = f_affine(m, t_vector3f(a_event.x * 2.0 - 1.0, a_event.y * -2.0 + 1.0, 0.0));
			auto u = v - t_vector3f(a - size * 0.5, size * 0.5 - 1.0, 0.0);
			auto shot = std::fabs(u.v_x) < std::fabs(u.v_y) ? (u.v_y < 0.0 ? &t_player::t_shots::v_slice : &t_player::t_shots::v_lob) : (u.v_x < 0.0 ? &t_player::t_shots::v_topspin : &t_player::t_shots::v_flat);
			a_player.f_do(shot);
		}
		if (a_event.y < 0.5) finger_down(a_stage, a_event, a_width, a_height);
	};
	auto finger_up = std::move(a_state.v_finger_up);
	a_state.v_finger_up = [finger_up, &a_player](t_stage& a_stage, const SDL_TouchFingerEvent& a_event, size_t a_width, size_t a_height)
	{
		if (a_event.x < 0.5) a_player.v_left = a_player.v_right = a_player.v_forward = a_player.v_backward = false;
		if (a_event.y < 0.5) finger_up(a_stage, a_event, a_width, a_height);
	};
	auto finger_motion = std::move(a_state.v_finger_motion);
	a_state.v_finger_motion = [left_pad, finger_motion, &a_player](t_stage& a_stage, const SDL_TouchFingerEvent& a_event, size_t a_width, size_t a_height)
	{
		if (a_event.x < 0.5) left_pad(a_stage, a_event, a_width, a_height);
		if (a_event.y < 0.5) finger_motion(a_stage, a_event, a_width, a_height);
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

t_dialog::t_dialog(t_main& a_main, const std::wstring& a_image, const std::wstring& a_sound, const std::wstring& a_title) : v_main(a_main), v_title(a_title)
{
	v_main.f_load(v_image, a_image);
	v_main.f_load(v_sound, a_sound);
	v_sound.f_play(-1);
}

void t_dialog::f_render(size_t a_width, size_t a_height, const t_matrix4f& a_viewing)
{
	float w = static_cast<float>(a_width) / a_height;
	float a = a_width * v_image.v_height / (a_height * v_image.v_width);
	float s = a < 1.0 ? a : 1.0;
	float t = a < 1.0 ? 1.0 : 1.0 / a;
	v_image(v_main.v_projection, a_viewing, std::array<float, 20>{
		-w, -1.0, 0.0, (1.0f - s) * 0.5f, (1.0f + t) * 0.5f,
		w, -1.0, 0.0, (1.0f + s) * 0.5f, (1.0f + t) * 0.5f,
		-w, 1.0, 0.0, (1.0f - s) * 0.5f, (1.0f - t) * 0.5f,
		w, 1.0, 0.0, (1.0f + s) * 0.5f, (1.0f - t) * 0.5f
	});
	auto viewing = a_viewing * v_main.v_text_scale * t_translate3f(0.0, 0.5, 0.0) * t_scale3f(1.5 / 4.0, 1.5 / 4.0, 1.0) * t_translate3f(v_title.size() * -0.25, 0.0, 0.0);
	v_main.v_font(v_main.v_projection, viewing, v_title);
}

t_stage_menu::t_stage_menu(t_main_screen& a_screen, const std::wstring& a_image, const std::wstring& a_sound, const std::wstring& a_title, const std::function<void (const std::wstring&, const std::wstring&)>& a_done) : t_dialog(a_screen.v_main, a_image, a_sound, a_title), v_player0(a_screen.v_main, 2), v_player1(a_screen.v_main, 2), v_ready(a_screen.v_main), v_menu(&v_player0)
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
		v_player0.v_items.push_back(t_menu_item{std::get<0>(x), []{}, [this]
		{
			f_transit(v_player1, -1.0);
		}});
		v_player1.v_items.push_back(t_menu_item{std::get<0>(x), []{}, [this]
		{
			f_transit(v_ready, -1.0);
		}});
	}
	v_ready.v_back = [this]
	{
		f_transit(v_player1, 1.0);
	};
	v_ready.v_items = std::vector<t_menu_item>{
		t_menu_item{L"START", []{}, [this, &a_screen, a_done]
		{
			a_done(std::get<1>(a_screen.v_players[v_player0.v_selected]), std::get<1>(a_screen.v_players[v_player1.v_selected]));
		}}
	};
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
	if (v_menu == &v_player0) message += L'?';
	message += L" vs " + v_player1.v_items[v_player1.v_selected].v_label;
	if (v_menu == &v_player1) message += L'?';
	auto viewing = a_viewing * v_main.v_text_scale * t_translate3f(0.0, 0.25, 0.0) * t_scale3f(1.5 / 8.0, 1.5 / 8.0, 1.0) * t_translate3f(message.size() * -0.25, -0.5, 0.0);
	v_main.v_font(v_main.v_projection, viewing, message);
	viewing = a_viewing;
	if (v_transit) {
		float a = 2.0 * v_direction * a_width / a_height;
		float t = v_t / v_duration;
		v_transit->f_render(viewing * t_translate3f(a * t, 0.0, 0.0) * v_main.v_text_scale);
		viewing *= t_translate3f(a * (t - 1.0), 0.0, 0.0);
	}
	v_menu->f_render(f_transform(viewing));
}

void t_stage_menu::f_key_press(SDL_Keycode a_key)
{
	if (!v_transit) v_menu->f_key_press(a_key);
}

void t_stage_menu::f_finger_down(const SDL_TouchFingerEvent& a_event)
{
	if (!v_transit) v_menu->f_finger_down(a_event, f_transform(t_matrix4f(1.0)));
}

void t_stage_menu::f_finger_up(const SDL_TouchFingerEvent& a_event)
{
	if (!v_transit) v_menu->f_finger_up(a_event, f_transform(t_matrix4f(1.0)));
}

void t_stage_menu::f_finger_motion(const SDL_TouchFingerEvent& a_event)
{
	if (!v_transit) v_menu->f_finger_motion(a_event, f_transform(t_matrix4f(1.0)));
}

void t_stage_menu::f_transit(t_menu<t_menu_item>& a_menu, float a_direction)
{
	v_transit = v_menu;
	v_menu = &a_menu;
	v_direction = a_direction;
	v_duration = 30.0;
	v_t = 0.0;
}

t_main_menu::t_main_menu(t_main_screen& a_screen) : t_dialog(a_screen.v_main, L"main-background.jpg", L"main-background.wav", L"TENNIS"), t_menu<t_menu_item>(a_screen.v_main)
{
	v_back = []
	{
		SDL_Event event;
		event.type = SDL_QUIT;
		SDL_PushEvent(&event);
	};
	v_items = std::vector<t_menu_item>{
		t_menu_item{L"  1P vs COM  ", []{}, [this, &a_screen]
		{
			v_sound.f_stop();
			a_screen.f_transit(std::make_unique<t_stage_menu>(a_screen, L"main-background.jpg", L"main-background.wav", L"1P vs COM", [&a_screen](const std::wstring& a_player0, const std::wstring& a_player1)
			{
				a_screen.v_main.v_screen = std::make_unique<t_match>(a_screen.v_main, false, false, f_controller0, a_player0, f_computer, a_player1);
			}), -1.0);
		}},
		t_menu_item{L"  1P vs 2P   ", []{}, [this, &a_screen]
		{
			v_sound.f_stop();
			a_screen.f_transit(std::make_unique<t_stage_menu>(a_screen, L"main-background.jpg", L"main-background.wav", L"1P vs 2P", [&a_screen](const std::wstring& a_player0, const std::wstring& a_player1)
			{
				a_screen.v_main.v_screen = std::make_unique<t_match>(a_screen.v_main, true, false, f_controller0, a_player0, f_controller1, a_player1);
			}), -1.0);
		}},
		t_menu_item{L" COM vs COM  ", []{}, [this, &a_screen]
		{
			v_sound.f_stop();
			a_screen.f_transit(std::make_unique<t_stage_menu>(a_screen, L"main-background.jpg", L"main-background.wav", L"COM vs COM", [&a_screen](const std::wstring& a_player0, const std::wstring& a_player1)
			{
				a_screen.v_main.v_screen = std::make_unique<t_match>(a_screen.v_main, false, true, f_computer, a_player0, f_computer, a_player1);
			}), -1.0);
		}},
		t_menu_item{L"  TRAINING   ", []{}, [this, &a_screen]
		{
			v_sound.f_stop();
			a_screen.f_transit(std::make_unique<t_stage_menu>(a_screen, L"main-background.jpg", L"training-background.wav", L"TRAINING", [&a_screen](const std::wstring& a_player0, const std::wstring& a_player1)
			{
				a_screen.v_main.v_screen = std::make_unique<t_training>(a_screen.v_main, f_controller0, a_player0, a_player1);
			}), -1.0);
		}},
		t_menu_item{L"    EXIT     ", []{}, v_back}
	};
}

void t_main_menu::f_step()
{
}

void t_main_menu::f_render(size_t a_width, size_t a_height, const t_matrix4f& a_viewing)
{
	t_dialog::f_render(a_width, a_height, a_viewing);
	t_menu<t_menu_item>::f_render(f_transform(a_viewing));
#if 0
	auto viewing = a_viewing * t_translate(-1.0, 0.5, 0.0) * t_scale3f(0.1, 0.1, 0.1);
	float y = 0.0;
	for (const auto& s : joysticks) {
		t_dialog::v_main.v_font(t_dialog::v_main.v_projection, viewing * t_translate(0.0, y, 0.0), f_convert(s));
		y -= 1.0;
	}
#endif
}

void t_main_menu::f_key_press(SDL_Keycode a_key)
{
	t_menu<t_menu_item>::f_key_press(a_key);
}

void t_main_menu::f_finger_down(const SDL_TouchFingerEvent& a_event)
{
	t_menu<t_menu_item>::f_finger_down(a_event, f_transform(t_matrix4f(1.0)));
}

void t_main_menu::f_finger_up(const SDL_TouchFingerEvent& a_event)
{
	t_menu<t_menu_item>::f_finger_up(a_event, f_transform(t_matrix4f(1.0)));
}

void t_main_menu::f_finger_motion(const SDL_TouchFingerEvent& a_event)
{
	t_menu<t_menu_item>::f_finger_motion(a_event, f_transform(t_matrix4f(1.0)));
}

t_main_screen::t_main_screen(t_main& a_main) : t_screen(a_main)
{
	{
		auto input = a_main.f_input(L"players");
		t_reader reader(input.get());
		reader.f_read_next();
		reader.f_move_to_tag();
		reader.f_check_start_element(L"players");
		reader.f_read_next();
		while (reader.f_is_start_element(L"player")) {
			auto name = reader.f_get_attribute(L"name");
			auto path = reader.f_get_attribute(L"path");
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

void t_main_screen::f_finger_down(const SDL_TouchFingerEvent& a_event, size_t a_width, size_t a_height)
{
	if (!v_transit) v_dialog->f_finger_down(a_event);
}

void t_main_screen::f_finger_up(const SDL_TouchFingerEvent& a_event, size_t a_width, size_t a_height)
{
	if (!v_transit) v_dialog->f_finger_up(a_event);
}

void t_main_screen::f_finger_motion(const SDL_TouchFingerEvent& a_event, size_t a_width, size_t a_height)
{
	if (!v_transit) v_dialog->f_finger_motion(a_event);
}

void t_main_screen::f_transit(std::unique_ptr<t_dialog>&& a_dialog, float a_direction)
{
	v_transit = std::move(v_dialog);
	v_dialog = std::move(a_dialog);
	v_direction = a_direction;
	v_duration = 30.0;
	v_t = 0.0;
}

void f_loop(SDL_Window* a_window, const std::wstring& a_prefix, bool a_show_pad)
{
	t_main main(a_prefix, a_show_pad);
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
		main.v_projection = f_orthographic(-w, w, -1.0f, 1.0f, -1.0f, 1.0f);
		main.v_text_scale = width < height ? t_scale3f(w, w, 1.0) : t_scale3f(1.0, 1.0, 1.0);
	};
	f_resize(width, height);
#ifdef __ANDROID__
	SDL_SetEventFilter([](void* a_user, SDL_Event* a_event)
	{
		switch (a_event->type) {
		case SDL_APP_WILLENTERBACKGROUND:
			Mix_PauseMusic();
			return 0;
		case SDL_APP_DIDENTERFOREGROUND:
			Mix_ResumeMusic();
			return 0;
		default:
			return 1;
		}
	}, NULL);
#endif
	glEnable(GL_CULL_FACE);
	SDL_Event event;
	bool dragging = false;
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
#ifndef __ANDROID__
			case SDL_MOUSEMOTION:
				if (dragging) {
					SDL_TouchFingerEvent tfinger;
					tfinger.x = static_cast<float>(event.motion.x) / width;
					tfinger.y = static_cast<float>(event.motion.y) / height;
					main.v_screen->f_finger_motion(tfinger, width, height);
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (!dragging) {
					dragging = true;
					SDL_TouchFingerEvent tfinger;
					tfinger.x = static_cast<float>(event.button.x) / width;
					tfinger.y = static_cast<float>(event.button.y) / height;
					main.v_screen->f_finger_down(tfinger, width, height);
				}
				break;
			case SDL_MOUSEBUTTONUP:
				if (dragging) {
					dragging = false;
					SDL_TouchFingerEvent tfinger;
					tfinger.x = static_cast<float>(event.button.x) / width;
					tfinger.y = static_cast<float>(event.button.y) / height;
					main.v_screen->f_finger_up(tfinger, width, height);
				}
				break;
#endif
			case SDL_CONTROLLERBUTTONDOWN:
				main.v_screen->f_key_press(main.f_keycode(event.cbutton));
				break;
			case SDL_CONTROLLERBUTTONUP:
				main.v_screen->f_key_release(main.f_keycode(event.cbutton));
				break;
			case SDL_CONTROLLERDEVICEADDED:
			case SDL_CONTROLLERDEVICEREMOVED:
			case SDL_CONTROLLERDEVICEREMAPPED:
				main.f_setup_controllers();
				break;
			case SDL_FINGERDOWN:
				main.v_screen->f_finger_down(event.tfinger, width, height);
				break;
			case SDL_FINGERUP:
				main.v_screen->f_finger_up(event.tfinger, width, height);
				break;
			case SDL_FINGERMOTION:
				main.v_screen->f_finger_motion(event.tfinger, width, height);
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
	try {
		t_sdl sdl(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER);
		t_sdl_image image(IMG_INIT_PNG | IMG_INIT_JPG);
		t_sdl_ttf ttf;
		t_sdl_audio audio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 1024);
		t_window window("Tennis", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		t_gl_context context(window);
#ifdef __ANDROID__
		f_loop(window, std::wstring(), true);
#else
		auto path = f_absolute_path(f_convert(argv[0]));
		size_t n = path.find_last_of(v_directory_separator);
		auto prefix = path.substr(0, n + 1) + L"data" + v_directory_separator;
		bool show_pad = argc >= 2 && std::strcmp(argv[1], "--show-pad") == 0;
		f_loop(window, prefix, show_pad);
#endif
		return EXIT_SUCCESS;
	} catch (std::exception& e) {
		SDL_Log("caught: %s", e.what());
		return EXIT_FAILURE;
	}
}
