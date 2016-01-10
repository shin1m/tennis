#include "main.h"

#include <cstring>
#include <SDL_main.h>

#include "computer.h"

//#define MEASURE_FPS

void t_component::f_step()
{
}

void t_component::f_render(const t_matrix4f& a_viewing)
{
}

void t_component::f_key_press(SDL_Keycode a_key)
{
	switch (a_key) {
	case SDLK_ESCAPE:
	case SDLK_1:
	case SDLK_VOLUMEUP:
		v_back();
		break;
	}
}

void t_component::f_finger_down(const t_matrix4f& a_viewing, const SDL_TouchFingerEvent& a_event)
{
}

void t_component::f_finger_up(const t_matrix4f& a_viewing, const SDL_TouchFingerEvent& a_event)
{
	if (a_event.x < 0.125f) v_back();
}

void t_component::f_finger_motion(const t_matrix4f& a_viewing, const SDL_TouchFingerEvent& a_event)
{
}

t_main::t_main(const std::wstring& a_prefix, bool a_show_pad) : v_prefix(a_prefix), v_show_pad(a_show_pad)
{
	v_font.f_create(f_path(L"font.ttf.png"));
	f_load(v_sound_cursor, L"cursor.wav");
	f_load(v_sound_select, L"select.wav");
#ifndef __ANDROID__
	int n = SDL_GameControllerAddMappingsFromFile(f_convert(f_path(L"gamecontrollerdb.txt")).c_str());
	if (n == -1) throw std::runtime_error((std::string("SDL_GameControllerAddMappingsFromFile Error: ") + SDL_GetError()).c_str());
#endif
	f_setup_controllers();
	std::array<float, 12> array{
		0.0f, 2.0f, 0.0f,
		-1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 2.0f, 0.0f
	};
	v_pad.v_triangle.f_create();
	gl::f_bind_buffer(GL_ARRAY_BUFFER, v_pad.v_triangle);
	gl::f_buffer_data(GL_ARRAY_BUFFER, array.size() * sizeof(float), array.data(), GL_STATIC_DRAW);
	gl::f_bind_buffer(GL_ARRAY_BUFFER, 0);
	v_pad.v_uniforms.v_stride = 3 * sizeof(float);
	v_pad.v_uniforms.v_projection = v_projection.v_array;
	v_pad.v_uniforms.v_color = t_vector4f(1.0f, 1.0f, 1.0f, 1.0f);
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

#ifdef __ANDROID__
namespace
{
std::vector<std::string> joysticks;
}
#endif

void t_main::f_setup_controllers()
{
	for (auto& x : v_controllers) x.f_close();
#ifdef __ANDROID__
joysticks.clear();
#endif
	size_t j = 0;
	int n = SDL_NumJoysticks();
	for (int i = 0; i < n; ++i) {
#ifndef __ANDROID__
char guid[33];
SDL_JoystickGetGUIDString(SDL_JoystickGetDeviceGUID(i), guid, sizeof(guid));
SDL_Log("joystick guid: %s", guid);
		if (SDL_IsGameController(i) != SDL_TRUE) continue;
SDL_Log("is game controller");
#endif
		try {
			v_controllers[j].f_open(i);
#ifdef __ANDROID__
			int b = SDL_JoystickNumButtons(v_controllers[j]);
			int h = SDL_JoystickNumHats(v_controllers[j]);
char cs[32];
std::sprintf(cs, "buttons: %d, hats: %d", b, h);
joysticks.push_back(cs);
			if (b <= 0 || h <= 0) {
				v_controllers[j].f_close();
				continue;
			}
#endif
			if (++j >= sizeof(v_controllers) / sizeof(v_controllers[0])) break;
		} catch (std::exception& e) {
			SDL_Log("caught: %s", e.what());
		}
	}
}

#ifdef __ANDROID__
void t_main::f_hat(const SDL_JoyHatEvent& a_hat)
{
	if (v_controllers[0] && a_hat.which == v_controllers[0].f_id()) {
		if ((v_hats[0] & SDL_HAT_UP) != 0) v_screen->f_key_release(SDLK_UP);
		if ((v_hats[0] & SDL_HAT_RIGHT) != 0) v_screen->f_key_release(SDLK_RIGHT);
		if ((v_hats[0] & SDL_HAT_DOWN) != 0) v_screen->f_key_release(SDLK_DOWN);
		if ((v_hats[0] & SDL_HAT_LEFT) != 0) v_screen->f_key_release(SDLK_LEFT);
		v_hats[0] = a_hat.value;
		if ((v_hats[0] & SDL_HAT_UP) != 0) v_screen->f_key_press(SDLK_UP);
		if ((v_hats[0] & SDL_HAT_RIGHT) != 0) v_screen->f_key_press(SDLK_RIGHT);
		if ((v_hats[0] & SDL_HAT_DOWN) != 0) v_screen->f_key_press(SDLK_DOWN);
		if ((v_hats[0] & SDL_HAT_LEFT) != 0) v_screen->f_key_press(SDLK_LEFT);
	} else if (v_controllers[1] && a_hat.which == v_controllers[1].f_id()) {
		if ((v_hats[1] & SDL_HAT_UP) != 0) v_screen->f_key_release(SDLK_8);
		if ((v_hats[1] & SDL_HAT_RIGHT) != 0) v_screen->f_key_release(SDLK_6);
		if ((v_hats[1] & SDL_HAT_DOWN) != 0) v_screen->f_key_release(SDLK_5);
		if ((v_hats[1] & SDL_HAT_LEFT) != 0) v_screen->f_key_release(SDLK_4);
		v_hats[1] = a_hat.value;
		if ((v_hats[1] & SDL_HAT_UP) != 0) v_screen->f_key_press(SDLK_8);
		if ((v_hats[1] & SDL_HAT_RIGHT) != 0) v_screen->f_key_press(SDLK_6);
		if ((v_hats[1] & SDL_HAT_DOWN) != 0) v_screen->f_key_press(SDLK_5);
		if ((v_hats[1] & SDL_HAT_LEFT) != 0) v_screen->f_key_press(SDLK_4);
	}
}
#endif

void t_main::f_resize(size_t a_width, size_t a_height)
{
	v_width = a_width;
	v_height = a_height;
	float a = static_cast<float>(v_width) / v_height;
	v_aspect = a;
	v_projection = f_orthographic(-a, a, -1.0f, 1.0f, -1.0f, 1.0f);
	v_text_scale = v_width < v_height ? t_scale3f(a, a, 1.0f) : t_scale3f(1.0f, 1.0f, 1.0f);
	v_pad.v_inverse = ~v_projection;
	float size = v_pad.v_size = std::min(a * 0.875f, 1.0f);
	v_pad.v_left_center = t_vector3f(size * 0.5f - a, size * 0.5f - 1.0f, 0.0f);
	t_matrix4f triangle = t_translate3f(0.0f, size * 0.25f, 0.0f);
	triangle *= t_scale3f(size * 0.1f, size * 0.1f, 1.0f);
	t_matrix4f center = t_translate3f(v_pad.v_left_center);
	for (size_t i = 0; i < 8; ++i) v_pad.v_left_marks[i] = center * t_rotate3f(t_vector3f(0.0f, 0.0f, 1.0f), static_cast<float>(M_PI / 4.0f) * i) * triangle;
	v_pad.v_right_center = t_vector3f(a - size * 0.5f, size * 0.5f - 1.0f, 0.0f);
	center = t_translate3f(v_pad.v_right_center);
	for (size_t i = 0; i < 4; ++i) v_pad.v_right_marks[i] = center * t_rotate3f(t_vector3f(0.0f, 0.0f, 1.0f), static_cast<float>(M_PI / 2.0) * i) * triangle;
};

void t_match::f_new_game()
{
	v_player0->v_point = v_player1->v_point = 0;
	v_second = false;
	size_t games = v_player0->v_game + v_player1->v_game;
	v_end = games % 4 < 2 ? 1.0f : -1.0f;
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
	f_transit_replay();
	v_sound_ace.f_play();

}
void t_match::f_ball_let()
{
	v_mark->f_mark(*v_ball);
	v_message = std::vector<std::wstring>{L"LET"};
	v_duration = 2 * 64;
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
	v_duration = 2 * 64;
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
	v_duration = 2 * 64;
	v_sound_miss.f_play();
}

std::wstring t_match::v_points0[] = {L" 0", L"15", L"30", L"40"};
std::wstring t_match::v_points1[] = {L"0 ", L"15", L"30", L"40"};

void t_match::f_transit_ready()
{
	f_reset();
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
	v_duration = 64;
	f_step_things();
}

const t_stage::t_state t_match::v_state_close{
	[](t_stage& a_stage)
	{
		a_stage.f_step_things();
	},
	[](t_stage& a_stage, SDL_Keycode a_key)
	{
		auto& stage = static_cast<t_match&>(a_stage);
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
	[](t_stage& a_stage)
	{
	},
	[](t_stage& a_stage, const SDL_TouchFingerEvent& a_event)
	{
	},
	[](t_stage& a_stage, const SDL_TouchFingerEvent& a_event)
	{
		if (a_event.y > 0.25f) return;
		auto& stage = static_cast<t_match&>(a_stage);
		if (a_event.x < 0.5f)
			a_stage.f_back();
		else
			stage.f_new_set();
	},
	[](t_stage& a_stage, const SDL_TouchFingerEvent& a_event)
	{
	}
};

void t_match::f_transit_close()
{
	v_player0->f_reset();
	v_player1->f_reset();
	f_reset_cameras();
	f_set_cameras();
	v_state = &v_state_close;
	v_message = std::vector<std::wstring>{
		std::wstring(v_player0->v_game > v_player1->v_game ? L"P1" : L"P2") + L" WON!",
		L"P1 " + std::to_wstring(v_player0->v_game) + L" - " + std::to_wstring(v_player1->v_game) + L" P2",
		L"PRESS START",
		L"TO PLAY AGAIN"
	};
	v_sound_ace.f_play();
}

const t_stage::t_state t_match::v_state_replay{
	[](t_stage& a_stage)
	{
		if (a_stage.v_duration <= 0) {
			a_stage.f_next();
			return;
		}
		if (--a_stage.v_duration % 2 == 0) return;
		auto& stage = static_cast<t_match&>(a_stage);
		auto record = stage.v_records.front();
		stage.v_records.pop_front();
		auto& ball = *stage.v_ball;
		ball.f_replay(record.v_ball);
		stage.v_mark->f_replay(record.v_mark);
		stage.v_player0->f_replay(record.v_player0);
		stage.v_player1->f_replay(record.v_player1);
		stage.v_records.push_back(record);
		stage.v_camera0.v_position = t_vector3f((ball.v_position.v_x + ball.v_hitter->f_root_position().v_x) * 0.5f, 4.0f, (ball.v_position.v_z + 40.0f * ball.v_hitter->v_opponent->v_end) * 0.5f);
		stage.v_camera0.v_toward = t_vector3f(0.0f, -6.0f, -40.0f * ball.v_hitter->v_opponent->v_end);
		stage.v_camera1.v_position = t_vector3f((ball.v_position.v_x + ball.v_hitter->v_opponent->f_root_position().v_x) * 0.5f, 4.0f, (ball.v_position.v_z + 40.0f * ball.v_hitter->v_end) * 0.5f);
		stage.v_camera1.v_toward = t_vector3f(0.0f, -6.0f, -40.0f * ball.v_hitter->v_end);
	},
	[](t_stage& a_stage, SDL_Keycode a_key)
	{
		switch (a_key) {
		case SDLK_RETURN:
			a_stage.f_next();
			break;
		case SDLK_ESCAPE:
			a_stage.f_back();
			break;
		}
	},
	[](t_stage& a_stage, SDL_Keycode a_key)
	{
	},
	[](t_stage& a_stage)
	{
	},
	[](t_stage& a_stage, const SDL_TouchFingerEvent& a_event)
	{
	},
	[](t_stage& a_stage, const SDL_TouchFingerEvent& a_event)
	{
		if (a_event.y > 0.25f) return;
		if (a_event.x < 0.5f)
			a_stage.f_back();
		else
			a_stage.f_next();
	},
	[](t_stage& a_stage, const SDL_TouchFingerEvent& a_event)
	{
	}
};

void t_match::f_transit_replay()
{
	v_state = &v_state_replay;
	v_duration = v_records.size() * 2;
}

void t_match::f_step_things()
{
	t_stage::f_step_things();
	auto record = v_records.front();
	v_records.pop_front();
	v_ball->f_record(record.v_ball);
	v_mark->f_record(record.v_mark);
	v_player0->f_record(record.v_player0);
	v_player1->f_record(record.v_player1);
	v_records.push_back(record);
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
		f_transit_ready();
	}
}

void t_match::f_back()
{
	v_back();
}

void t_match::f_transit_play()
{
	v_state = &v_state_play;
	v_message.clear();
}

void t_match::f_reset_cameras()
{
	v_camera0.v_position = t_vector3f(0.0f, 14.0f, 0.0f);
	v_camera0.v_toward = t_vector3f(0.0f, -12.0f, -40.0f * (v_fixed ? 1.0f : v_player0->v_end));
	v_camera1.v_position = t_vector3f(0.0f, 14.0f, 0.0f);
	v_camera1.v_toward = t_vector3f(0.0f, -12.0f, -40.0f * (v_fixed ? -1.0f : v_player1->v_end));
}

void t_match::f_reset()
{
	v_side = (v_player0->v_point + v_player1->v_point) % 2 == 0 ? 1.0f : -1.0f;
	v_ball->f_reset(v_side, 2 * 12 * 0.0254f * v_end * v_side, 0.875f, 39 * 12 * 0.0254f * v_end);
	v_mark->v_duration = 0;
	v_server->f_reset(v_end, t_player::v_state_serve_set);
	v_receiver->v_placement->v_position = t_vector3f(-9 * 12 * 0.0254f * v_end * v_side, 0.0f, -39 * 12 * 0.0254f * v_end);
	v_receiver->v_placement->v_valid = false;
	v_receiver->f_reset(-v_end, t_player::v_state_default);
	f_reset_cameras();
}

void t_match::f_new_set()
{
	v_closed = false;
	v_player0->v_game = v_player1->v_game = 0;
	f_new_game();
	f_transit_ready();
}

const std::vector<std::wstring> t_training::v_toss_message{
	L"     CHANGE SIDES: START",
	L"         POSITION:   +  ",
	L"PLACEMENT & SWING: + & *",
	L"",
	L"            LOB         ",
	L"     TOPSPIN * FLAT     ",
	L"           SLICE        "
};

void t_training::f_ball_ace()
{
	v_duration = 32;
}

void t_training::f_ball_let()
{
	v_mark->f_mark(*v_ball);
	v_text_viewing = t_scale3f(0.25f, 0.25f, 1.0f);
	v_message = std::vector<std::wstring>{L"LET"};
	v_duration = 32;
}

void t_training::f_serve_miss()
{
	v_text_viewing = t_scale3f(0.25f, 0.25f, 1.0f);
	v_message = std::vector<std::wstring>{L"FAULT"};
	v_duration = 32;
	v_sound_miss.f_play();
}

void t_training::f_ball_serve_air()
{
	f_serve_miss();
}

void t_training::f_miss(const std::wstring& a_message)
{
	v_text_viewing = t_scale3f(0.25f, 0.25f, 1.0f);
	v_message = std::vector<std::wstring>{a_message};
	v_duration = 32;
	v_sound_miss.f_play();
}

void t_training::f_ball_in()
{
	v_mark->f_mark(*v_ball);
	if (v_ball->v_hitter != v_player0.get()) return;
	v_text_viewing = t_scale3f(0.25f, 0.25f, 1.0f);
	v_message = std::vector<std::wstring>{L"IN"};
}

void t_training::f_step_things()
{
	t_stage::f_step_things();
	v_camera1.v_position = t_vector3f((v_ball->v_position.v_x + v_player0->f_root_position().v_x) * 0.5f, 4.0f, (v_ball->v_position.v_z + 40.0f * v_player1->v_end) * 0.5f);
	v_camera1.v_toward = t_vector3f(0.0f, -6.0f, -40.0f * v_player1->v_end);
}

t_training::t_training(t_main& a_main, const std::function<void (t_stage::t_state&, t_player&)>& a_controller0, const std::wstring& a_player0, const std::wstring& a_player1, const std::function<void ()>& a_back) : t_stage(a_main, true, false, a_controller0, a_player0, [](t_stage::t_state& a_state, t_player& a_player)
{
	a_state.v_step = [step = std::move(a_state.v_step), &a_player](t_stage& a_stage)
	{
		if (a_player.v_state != &t_player::v_state_swing) a_player.v_left = a_player.v_right = a_player.v_forward = a_player.v_backward = false;
		step(a_stage);
	};
}, a_player1), v_menu(a_main)
{
	auto f = [this](t_state& a_state)
	{
		a_state.v_key_press = [this, key_press = std::move(a_state.v_key_press)](t_stage& a_stage, SDL_Keycode a_key)
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
		a_state.v_finger_up = [this, finger_up = std::move(a_state.v_finger_up)](t_stage& a_stage, const SDL_TouchFingerEvent& a_event)
		{
			if (a_event.x > 0.5f && a_event.y < 0.5f) {
				v_side = -v_side;
				f_transit_ready();
			} else {
				finger_up(a_stage, a_event);
			}
		};
	};
	f(v_state_ready);
	f(v_state_play);
	v_camera0.v_position = t_vector3f(0.0f, 14.0f, 0.0f);
	v_camera0.v_toward = t_vector3f(0.0f, -12.0f, -40.0f * v_player0->v_end);
	v_menu.v_back = a_back;
	v_menu.v_items.assign({
		t_item{L" SERVE ", [this]
		{
			f_transit_ready();
		}, [this]
		{
			v_ball->f_reset(v_side, 2 * 12 * 0.0254f * v_side, 0.875f, 39 * 12 * 0.0254f);
			v_mark->v_duration = 0;
			v_player0->f_reset(1.0f, t_player::v_state_serve_set);
			v_player1->v_placement->v_position = t_vector3f(-9 * 12 * 0.0254f * v_side, 0.0f, -39 * 12 * 0.0254f);
			v_player1->v_placement->v_valid = false;
			v_player1->f_reset(-1.0f, t_player::v_state_default);
			f_step_things();
			v_text_viewing = t_matrix4f(1.0f) * t_translate3f(0.0f, -0.5f, 0.0f) * t_scale3f(1.5f / 16.0f, 1.5f / 16.0f, 1.0f);
			v_message = std::vector<std::wstring>{
				L"CHANGE SIDES: START",
				L"    POSITION: < + >",
				L"        TOSS:   *  ",
				L"   DIRECTION: < + >",
				L"       SWING:   *  ",
				L"",
				L"       SECOND      ",
				L"    SPIN * FLAT    ",
				L"       SLICE       "
			};
			v_duration = 0;
		}, [] {}},
		t_item{L" STROKE", [this]
		{
			f_transit_ready();
		}, [this]
		{
			f_reset(3 * 12 * 0.0254f * v_side, 1.0f, -39 * 12 * 0.0254f, t_vector3f((0.0f - 3.2f * v_side) * 12 * 0.0254f, 0.0f, 39 * 12 * 0.0254f), v_player1->v_actions.v_swing.v_toss);
			v_text_viewing = t_matrix4f(1.0f) * t_translate3f(0.0f, -0.5f, 0.0f) * t_scale3f(1.5f / 16.0f, 1.5f / 16.0f, 1.0f);
			v_message = v_toss_message;
			v_duration = 32;
		}, [this]
		{
			f_toss(v_player1->v_actions.v_swing.v_toss);
		}},
		t_item{L" VOLLEY", [this]
		{
			f_transit_ready();
		}, [this]
		{
			f_reset(3 * 12 * 0.0254f * v_side, 1.0f, -39 * 12 * 0.0254f, t_vector3f((0.1f - 2.0f * v_side) * 12 * 0.0254f, 0.0f, 13 * 12 * 0.0254f), v_player1->v_actions.v_swing.v_toss);
			v_text_viewing = t_matrix4f(1.0f) * t_translate3f(0.0f, -0.5f, 0.0f) * t_scale3f(1.5f / 16.0f, 1.5f / 16.0f, 1.0f);
			v_message = v_toss_message;
			v_duration = 32;
		}, [this]
		{
			f_toss(v_player1->v_actions.v_swing.v_toss);
		}},
		t_item{L" SMASH ", [this]
		{
			f_transit_ready();
		}, [this]
		{
			f_reset(3 * 12 * 0.0254f * v_side, 1.0f, -39 * 12 * 0.0254f, t_vector3f((0.4f - 0.4f * v_side) * 12 * 0.0254f, 0.0f, 9 * 12 * 0.0254f), v_player1->v_actions.v_swing.v_toss_lob);
			v_text_viewing = t_matrix4f(1.0f) * t_translate3f(0.0f, -0.5f, 0.0f) * t_scale3f(1.5f / 16.0f, 1.5f / 16.0f, 1.0f);
			v_message = v_toss_message;
			v_duration = 32;
		}, [this]
		{
			f_toss(v_player1->v_actions.v_swing.v_toss_lob);
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
	v_mark->v_duration = 0;
	v_player0->v_placement->v_position = a_position;
	v_player0->v_placement->v_valid = false;
	v_player0->f_reset(1.0f, t_player::v_state_default);
	t_posture posture;
	posture.f_validate();
	v_player1->v_placement->v_position = v_ball->v_position - f_affine(posture * a_shot.v_spot, t_vector3f(0.0f, 0.0f, 0.0f));
	v_player1->v_placement->v_position.v_y = 0.0f;
	v_player1->v_placement->v_valid = false;
	v_player1->f_reset(-1.0f, t_player::v_state_default);
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
	f_transit_select();
}

const t_stage::t_state t_training::v_state_select{
	[](t_stage& a_stage) {},
	[](t_stage& a_stage, SDL_Keycode a_key)
	{
		auto& stage = static_cast<t_training&>(a_stage);
		stage.v_menu.f_key_press(a_key);
	},
	[](t_stage& a_stage, SDL_Keycode a_key)
	{
	},
	[](t_stage& a_stage)
	{
		auto& stage = static_cast<t_training&>(a_stage);
		stage.v_menu.f_render(stage.f_transform());
	},
	[](t_stage& a_stage, const SDL_TouchFingerEvent& a_event)
	{
		auto& stage = static_cast<t_training&>(a_stage);
		stage.v_menu.f_finger_down(stage.f_transform(), a_event);
	},
	[](t_stage& a_stage, const SDL_TouchFingerEvent& a_event)
	{
		auto& stage = static_cast<t_training&>(a_stage);
		stage.v_menu.f_finger_up(stage.f_transform(), a_event);
	},
	[](t_stage& a_stage, const SDL_TouchFingerEvent& a_event)
	{
		auto& stage = static_cast<t_training&>(a_stage);
		stage.v_menu.f_finger_motion(stage.f_transform(), a_event);
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
	v_side = 1.0f;
	v_ball->f_reset(v_side, 2 * 12 * 0.0254f, t_ball::c_radius + 0.01f, 2 * 12 * 0.0254f);
	v_mark->v_duration = 0;
	v_player0->v_placement->v_position = t_vector3f((0.1f - 2.0f * v_side) * 12 * 0.0254f, 0.0f, 13 * 12 * 0.0254f);
	v_player0->v_placement->v_valid = false;
	v_player0->f_reset(1.0f, t_player::v_state_default);
	v_player1->v_placement->v_position = t_vector3f((0.1f + 2.0f * v_side) * 12 * 0.0254f, 0.0f, -13 * 12 * 0.0254f);
	v_player1->v_placement->v_valid = false;
	v_player1->f_reset(-1.0f, t_player::v_state_default);
	f_step_things();
	v_message.clear();
	v_duration = 0;
}

void t_training::f_transit_ready()
{
	v_state = &v_state_ready;
	v_menu.f_selected().v_ready();
}

void f_controller0(t_stage::t_state& a_state, t_player& a_player)
{
	a_state.v_key_press = [key_press = std::move(a_state.v_key_press), &a_player](t_stage& a_stage, SDL_Keycode a_key)
	{
		switch (a_key) {
		case SDLK_1:
		case SDLK_j:
			a_player.f_do(a_stage.v_main.v_pad.v_shot = &t_player::t_shots::v_topspin);
			break;
		case SDLK_SPACE:
		case SDLK_2:
		case SDLK_l:
			a_player.f_do(a_stage.v_main.v_pad.v_shot = &t_player::t_shots::v_flat);
			break;
		case SDLK_VOLUMEUP:
		case SDLK_i:
			a_player.f_do(a_stage.v_main.v_pad.v_shot = &t_player::t_shots::v_lob);
			break;
		case SDLK_VOLUMEDOWN:
		case SDLK_m:
			a_player.f_do(a_stage.v_main.v_pad.v_shot = &t_player::t_shots::v_slice);
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
	a_state.v_key_release = [key_release = std::move(a_state.v_key_release), &a_player](t_stage& a_stage, SDL_Keycode a_key)
	{
		switch (a_key) {
		case SDLK_1:
		case SDLK_j:
		case SDLK_SPACE:
		case SDLK_2:
		case SDLK_l:
		case SDLK_VOLUMEUP:
		case SDLK_i:
		case SDLK_VOLUMEDOWN:
		case SDLK_m:
			a_stage.v_main.v_pad.v_shot = nullptr;
			break;
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
	if (a_player.v_stage.v_main.v_show_pad) {
		a_state.v_render = [render = std::move(a_state.v_render), &a_player](t_stage& a_stage)
		{
			if (a_stage.v_main.v_controllers[0]) {
				render(a_stage);
				return;
			}
			auto& shader = a_stage.v_main.v_shaders.f_constant_color();
			auto& pad = a_stage.v_main.v_pad;
			size_t pressed;
			if (a_player.v_forward)
				pressed = a_player.v_left ? 1 : a_player.v_right ? 7 : 0;
			else if (a_player.v_backward)
				pressed = a_player.v_left ? 3 : a_player.v_right ? 5 : 4;
			else
				pressed = a_player.v_left ? 2 : a_player.v_right ? 6 : 8;
			for (size_t i = 0; i < 8; ++i) {
				pad.v_uniforms.v_vertex = pad.v_left_marks[i].v_array;
				shader(pad.v_uniforms, pad.v_triangle, i == pressed ? GL_TRIANGLES : GL_LINE_STRIP, 0, i == pressed ? 3 : 4);
			}
			if (pad.v_shot == &t_player::t_shots::v_lob)
				pressed = 0;
			else if (pad.v_shot == &t_player::t_shots::v_topspin)
				pressed = 1;
			else if (pad.v_shot == &t_player::t_shots::v_slice)
				pressed = 2;
			else if (pad.v_shot == &t_player::t_shots::v_flat)
				pressed = 3;
			else
				pressed = 4;
			for (size_t i = 0; i < 4; ++i) {
				pad.v_uniforms.v_vertex = pad.v_right_marks[i].v_array;
				shader(pad.v_uniforms, pad.v_triangle, i == pressed ? GL_TRIANGLES : GL_LINE_STRIP, 0, i == pressed ? 3 : 4);
			}
			render(a_stage);
		};
	}
	auto left_pad = [&a_player](t_stage& a_stage, const SDL_TouchFingerEvent& a_event)
	{
		auto& pad = a_stage.v_main.v_pad;
		auto u = f_affine(pad.v_inverse, t_vector3f(a_event.x * 2.0f - 1.0f, a_event.y * -2.0f + 1.0f, 0.0f)) - pad.v_left_center;
		a_player.v_left = a_player.v_right = a_player.v_forward = a_player.v_backward = false;
		if (u.f_length() < pad.v_size / 8.0f) return;
		if (u.v_x == 0.0f) {
			(u.v_y < 0.0f ? a_player.v_backward : a_player.v_forward) = true;
		} else {
			float a = std::fabs(u.v_y / u.v_x);
			if (a < std::tan(static_cast<float>(M_PI) * 0.375f)) (u.v_x < 0.0f ? a_player.v_left : a_player.v_right) = true;
			if (a > std::tan(static_cast<float>(M_PI) * 0.125f)) (u.v_y < 0.0f ? a_player.v_backward : a_player.v_forward) = true;
		}
	};
	a_state.v_finger_down = [left_pad, finger_down = std::move(a_state.v_finger_down), &a_player](t_stage& a_stage, const SDL_TouchFingerEvent& a_event)
	{
		if (a_event.x < 0.5f) {
			left_pad(a_stage, a_event);
		} else {
			auto& pad = a_stage.v_main.v_pad;
			auto u = f_affine(pad.v_inverse, t_vector3f(a_event.x * 2.0f - 1.0f, a_event.y * -2.0f + 1.0f, 0.0f)) - pad.v_right_center;
			pad.v_shot = std::fabs(u.v_x) < std::fabs(u.v_y) ? (u.v_y < 0.0f ? &t_player::t_shots::v_slice : &t_player::t_shots::v_lob) : (u.v_x < 0.0f ? &t_player::t_shots::v_topspin : &t_player::t_shots::v_flat);
			a_player.f_do(pad.v_shot);
		}
		if (a_event.y < 0.5f) finger_down(a_stage, a_event);
	};
	a_state.v_finger_up = [finger_up = std::move(a_state.v_finger_up), &a_player](t_stage& a_stage, const SDL_TouchFingerEvent& a_event)
	{
		if (a_event.x < 0.5f)
			a_player.v_left = a_player.v_right = a_player.v_forward = a_player.v_backward = false;
		else
			a_stage.v_main.v_pad.v_shot = nullptr;
		if (a_event.y < 0.5f) finger_up(a_stage, a_event);
	};
	a_state.v_finger_motion = [left_pad, finger_motion = std::move(a_state.v_finger_motion), &a_player](t_stage& a_stage, const SDL_TouchFingerEvent& a_event)
	{
		if (a_event.x < 0.5f) left_pad(a_stage, a_event);
		if (a_event.y < 0.5f) finger_motion(a_stage, a_event);
	};
}

void f_controller1(t_stage::t_state& a_state, t_player& a_player)
{
	a_state.v_key_press = [key_press = std::move(a_state.v_key_press), &a_player](t_stage& a_stage, SDL_Keycode a_key)
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
	a_state.v_key_release = [key_release = std::move(a_state.v_key_release), &a_player](t_stage& a_stage, SDL_Keycode a_key)
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

void t_background::f_render(const t_matrix4f& a_viewing)
{
	float a = v_main.v_aspect * v_image.v_height / v_image.v_width;
	float s = a < 1.0f ? a : 1.0f;
	float t = a < 1.0f ? 1.0f : 1.0f / a;
	v_image(v_main.v_projection, a_viewing, std::array<float, 20>{
		-v_main.v_aspect, -1.0f, 0.0f, (1.0f - s) * 0.5f, (1.0f + t) * 0.5f,
		v_main.v_aspect, -1.0f, 0.0f, (1.0f + s) * 0.5f, (1.0f + t) * 0.5f,
		-v_main.v_aspect, 1.0f, 0.0f, (1.0f - s) * 0.5f, (1.0f - t) * 0.5f,
		v_main.v_aspect, 1.0f, 0.0f, (1.0f + s) * 0.5f, (1.0f - t) * 0.5f
	});
	t_container::f_render(a_viewing);
}

void t_titled::f_render(const t_matrix4f& a_viewing)
{
	t_container::f_render(a_viewing);
	auto viewing = a_viewing * v_main.v_text_scale * t_translate3f(0.0f, 0.5f, 0.0f) * t_scale3f(1.125f / 4.0f, 1.125f / 4.0f, 1.0f) * t_translate3f(v_title.size() * -0.25f, 0.0f, 0.0f);
	v_main.v_font(v_main.v_projection, viewing, v_title);
}

t_stage_menu::t_stage_menu(t_main_screen& a_screen, const std::wstring& a_title, const std::function<void (const std::wstring&, const std::wstring&)>& a_done, const std::function<void ()>& a_back) : t_titled(a_screen.v_main, a_title), v_player0(a_screen.v_main, 2), v_player1(a_screen.v_main, 2), v_ready(a_screen.v_main)
{
	v_player0.v_back = a_back;
	v_player1.v_back = [this]
	{
		f_transit(&v_player0, 2.0f * v_main.v_aspect);
	};
	for (const auto& x : a_screen.v_players) {
		v_player0.v_items.push_back(t_menu_item{std::get<0>(x), [this]
		{
			f_transit(&v_player1, -2.0f * v_main.v_aspect);
		}});
		v_player1.v_items.push_back(t_menu_item{std::get<0>(x), [this]
		{
			f_transit(&v_ready, -2.0f * v_main.v_aspect);
		}});
	}
	v_ready.v_back = [this]
	{
		f_transit(&v_player1, 2.0f * v_main.v_aspect);
	};
	v_ready.v_items = std::vector<t_menu_item>{
		t_menu_item{L"START", [this, &a_screen, a_done]
		{
			a_done(std::get<1>(a_screen.v_players[v_player0.v_selected]), std::get<1>(a_screen.v_players[v_player1.v_selected]));
		}}
	};
	v_content = &v_player0;
}

void t_stage_menu::f_render(const t_matrix4f& a_viewing)
{
	t_titled::f_render(a_viewing);
	auto message = v_player0.v_items[v_player0.v_selected].v_label;
	if (v_content == &v_player0) message += L'?';
	message += L" vs " + v_player1.v_items[v_player1.v_selected].v_label;
	if (v_content == &v_player1) message += L'?';
	auto viewing = a_viewing * v_main.v_text_scale * t_translate3f(0.0f, 0.25f, 0.0f) * t_scale3f(1.0f / 4.0f, 1.0f / 4.0f, 1.0f) * t_translate3f(message.size() * -0.25f, -0.5f, 0.0f);
	v_main.v_font(v_main.v_projection, viewing, message);
}

t_credits::t_credits(t_main_screen& a_screen, const std::function<void ()>& a_back) : t_titled(a_screen.v_main, L"CREDITS")
{
	v_back = a_back;
}

void t_credits::f_render(const t_matrix4f& a_viewing)
{
	t_titled::f_render(a_viewing);
	std::wstring message[] = {
		L"Copyright (c) shin1m",
		L"",
		L"The background image is",
		L"\"Arthur Ashe Stadium 2010\"",
		L"by manalahmadkhan,",
		L"used under CC BY 2.0.",
		L"",
		L"The sound effects are",
		L"by freeSFX and others."
	};
	auto viewing = a_viewing * v_main.v_text_scale * t_translate3f(0.0f, 0.25f, 0.0f) * t_scale3f(0.125f, 0.125f, 1.0f);
	float y = 0.0f;
	for (const auto& line : message) {
		v_main.v_font(v_main.v_projection, viewing * t_translate3f(line.size() * -0.25f, y, 0.0f), line);
		y -= 1.0f;
	}
}

t_main_menu::t_main_menu(t_main_screen& a_screen, const std::function<void ()>& a_back) : t_titled(a_screen.v_main, L"WakuWakuTennis"), v_menu(a_screen.v_main)
{
	auto back2this = [&a_screen, a_back]
	{
		a_screen.v_container.f_transit(std::make_unique<t_main_menu>(a_screen, a_back), 2.0f * a_screen.v_main.v_aspect);
	};
	v_menu.v_back = []
	{
		SDL_Event event;
		event.type = SDL_QUIT;
		SDL_PushEvent(&event);
	};
	v_menu.v_items.push_back(t_menu_item{L"  1P vs COM  ", [&a_screen, a_back, back2this]
	{
		a_screen.v_container.f_transit(std::make_unique<t_stage_menu>(a_screen, L"1P vs COM", [&main = a_screen.v_main, a_back](const std::wstring& a_player0, const std::wstring& a_player1)
		{
			main.v_screen = std::make_unique<t_match>(main, false, false, f_controller0, a_player0, f_computer, a_player1, a_back);
		}, back2this), -2.0f * a_screen.v_main.v_aspect);
	}});
#ifdef __ANDROID__
	if (v_main.v_controllers[1])
#endif
	v_menu.v_items.push_back(t_menu_item{L"  1P vs 2P   ", [&a_screen, a_back, back2this]
	{
		a_screen.v_container.f_transit(std::make_unique<t_stage_menu>(a_screen, L"1P vs 2P", [&main = a_screen.v_main, a_back](const std::wstring& a_player0, const std::wstring& a_player1)
		{
			main.v_screen = std::make_unique<t_match>(main, true, false, f_controller0, a_player0, f_controller1, a_player1, a_back);
		}, back2this), -2.0f * a_screen.v_main.v_aspect);
	}});
	v_menu.v_items.push_back(t_menu_item{L" COM vs COM  ", [&a_screen, a_back, back2this]
	{
		a_screen.v_container.f_transit(std::make_unique<t_stage_menu>(a_screen, L"COM vs COM", [&main = a_screen.v_main, a_back](const std::wstring& a_player0, const std::wstring& a_player1)
		{
			main.v_screen = std::make_unique<t_match>(main, false, true, f_computer, a_player0, f_computer, a_player1, a_back);
		}, back2this), -2.0f * a_screen.v_main.v_aspect);
	}});
	v_menu.v_items.push_back(t_menu_item{L"  TRAINING   ", [&a_screen, a_back, back2this]
	{
		a_screen.v_container.f_transit(std::make_unique<t_stage_menu>(a_screen, L"TRAINING", [&main = a_screen.v_main, a_back](const std::wstring& a_player0, const std::wstring& a_player1)
		{
			main.v_screen = std::make_unique<t_training>(main, f_controller0, a_player0, a_player1, a_back);
		}, back2this), -2.0f * a_screen.v_main.v_aspect);
	}});
	v_menu.v_items.push_back(t_menu_item{L"   CREDITS   ", [&a_screen, a_back, back2this]
	{
		a_screen.v_container.f_transit(std::make_unique<t_credits>(a_screen, back2this), -2.0f * a_screen.v_main.v_aspect);
	}});
	v_content = &v_menu;
}

t_main_screen::t_main_screen(t_main& a_main) : t_screen(a_main), v_container(v_main, L"main-background.jpg", L"main-background.wav")
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
	v_container.v_content = std::make_unique<t_main_menu>(*this, [&a_main]
	{
		a_main.v_screen = std::make_unique<t_main_screen>(a_main);
	});
}

void t_main_screen::f_step()
{
	v_container.f_step();
}

void t_main_screen::f_render()
{
	glViewport(0, 0, v_main.v_width, v_main.v_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	v_container.f_render(t_matrix4f(1.0f));
#if 0
	auto viewing = static_cast<t_matrix4f>(t_translate(-1.0f, 0.5f, 0.0f)) * t_scale3f(0.1f, 0.1f, 0.1f);
	float y = 0.0f;
	for (const auto& s : joysticks) {
		v_main.v_font(v_main.v_projection, viewing * t_translate(0.0f, y, 0.0f), f_convert(s));
		y -= 1.0f;
	}
#endif
}

void t_main_screen::f_key_press(SDL_Keycode a_key)
{
	v_container.f_key_press(a_key);
}

void t_main_screen::f_key_release(SDL_Keycode a_key)
{
}

void t_main_screen::f_finger_down(const SDL_TouchFingerEvent& a_event)
{
	v_container.f_finger_down(t_matrix4f(1.0f), a_event);
}

void t_main_screen::f_finger_up(const SDL_TouchFingerEvent& a_event)
{
	v_container.f_finger_up(t_matrix4f(1.0f), a_event);
}

void t_main_screen::f_finger_motion(const SDL_TouchFingerEvent& a_event)
{
	v_container.f_finger_motion(t_matrix4f(1.0f), a_event);
}

void f_loop(SDL_Window* a_window, const std::wstring& a_prefix, bool a_show_pad)
{
	t_main main(a_prefix, a_show_pad);
	main.v_screen = std::make_unique<t_main_screen>(main);
	SDL_GL_SetSwapInterval(1);
	int width;
	int height;
	SDL_GetWindowSize(a_window, &width, &height);
	main.f_resize(width, height);
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
#ifdef MEASURE_FPS
	Uint64 start = SDL_GetPerformanceCounter();
	size_t count = 0;
#endif
	while (true) {
#ifdef MEASURE_FPS
		++count;
#endif
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				return;
			case SDL_WINDOWEVENT:
				switch (event.window.event) {
				case SDL_WINDOWEVENT_RESIZED:
					main.f_resize(event.window.data1, event.window.data2);
					break;
				}
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				case SDLK_q:
					return;
#ifdef MEASURE_FPS
				case SDLK_r:
					start = SDL_GetPerformanceCounter();
					count = 0;
					break;
#endif
				case SDLK_AC_BACK:
					main.v_screen->f_key_press(SDLK_ESCAPE);
					break;
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
					tfinger.x = static_cast<float>(event.motion.x) / main.v_width;
					tfinger.y = static_cast<float>(event.motion.y) / main.v_height;
					main.v_screen->f_finger_motion(tfinger);
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (!dragging) {
					dragging = true;
					SDL_TouchFingerEvent tfinger;
					tfinger.x = static_cast<float>(event.button.x) / main.v_width;
					tfinger.y = static_cast<float>(event.button.y) / main.v_height;
					main.v_screen->f_finger_down(tfinger);
				}
				break;
			case SDL_MOUSEBUTTONUP:
				if (dragging) {
					dragging = false;
					SDL_TouchFingerEvent tfinger;
					tfinger.x = static_cast<float>(event.button.x) / main.v_width;
					tfinger.y = static_cast<float>(event.button.y) / main.v_height;
					main.v_screen->f_finger_up(tfinger);
				}
				break;
#endif
#ifdef __ANDROID__
			case SDL_JOYHATMOTION:
				main.f_hat(event.jhat);
				break;
			case SDL_JOYBUTTONDOWN:
				main.v_screen->f_key_press(main.f_keycode(event.jbutton));
				break;
			case SDL_JOYBUTTONUP:
				main.v_screen->f_key_release(main.f_keycode(event.jbutton));
				break;
			case SDL_JOYDEVICEADDED:
			case SDL_JOYDEVICEREMOVED:
				main.f_setup_controllers();
				break;
#else
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
#endif
			case SDL_FINGERDOWN:
				main.v_screen->f_finger_down(event.tfinger);
#ifdef MEASURE_FPS
				if (event.tfinger.x > 0.5f && event.tfinger.y < 0.5f) {
					start = SDL_GetPerformanceCounter();
					count = 0;
				}
#endif
				break;
			case SDL_FINGERUP:
				main.v_screen->f_finger_up(event.tfinger);
				break;
			case SDL_FINGERMOTION:
				main.v_screen->f_finger_motion(event.tfinger);
				break;
			}
		}
		main.v_screen->f_render();
#ifdef MEASURE_FPS
		{
			auto viewing = static_cast<t_matrix4f>(t_translate(-0.5f, 0.5f, 0.0f)) * t_scale3f(0.1f, 0.1f, 0.1f);
			float fps = static_cast<float>(count) * SDL_GetPerformanceFrequency() / (SDL_GetPerformanceCounter() - start);
			wchar_t cs[32];
			std::swprintf(cs, sizeof(cs) / sizeof(wchar_t), L"%.1f fps", fps);
			main.v_font(main.v_projection, viewing, cs);
		}
#endif
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
//		t_window window("Tennis", 0, 0, 640, 360, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
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
