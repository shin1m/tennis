#include "stage.h"

#include "path.h"
#include "main.h"

void t_stage::f_ball_in()
{
	v_mark->f_mark(*v_ball);
}

void t_stage::f_step_things()
{
	v_ball->f_step();
	v_mark->f_step();
	v_player0->f_step();
	v_player1->f_step();
	auto target = v_ball->v_position * 0.25;
	if (v_fixed) {
		v_camera0.v_position.v_x = target.v_x;
		v_camera0.v_position.v_z = 48.0 + target.v_z;
		v_camera1.v_position.v_x = target.v_x;
		v_camera1.v_position.v_z = -48.0 + target.v_z;
	} else {
		v_camera0.v_position.v_x = target.v_x + v_player0->f_root_position().v_x * 0.5;
		v_camera0.v_position.v_z = 48.0 * v_player0->v_end + target.v_z;
		v_camera1.v_position.v_x = target.v_x + v_player1->f_root_position().v_x * 0.5;
		v_camera1.v_position.v_z = 48.0 * v_player1->v_end + target.v_z;
	}
}

t_stage::t_stage(t_main& a_main, bool a_dual, bool a_fixed, const std::function<void (t_stage::t_state&, t_player&)>& a_controller0, const std::wstring& a_player0, const std::function<void (t_stage::t_state&, t_player&)>& a_controller1, const std::wstring& a_player1) : t_screen(a_main), v_dual(a_dual), v_fixed(a_fixed)
{
	a_main.f_load(v_sound_bounce, L"data/bounce.wav");
	a_main.f_load(v_sound_net, L"data/net.wav");
	a_main.f_load(v_sound_chip, L"data/chip.wav");
	a_main.f_load(v_sound_hit, L"data/hit.wav");
	a_main.f_load(v_sound_swing, L"data/swing.wav");
	a_main.f_load(v_sound_ace, L"data/ace.wav");
	a_main.f_load(v_sound_miss, L"data/miss.wav");
	v_scene.f_load(t_path(a_main.v_root) / L"../data/court.dae");
	v_scene.f_build(a_main.v_shaders);
	v_ball = std::make_unique<t_ball>(*this, L"#Material-Shadow", L"#Material-Ball");
	v_mark = std::make_unique<t_mark>(*this, L"#Material-Shadow");
	v_player0 = std::make_unique<t_player>(*this, a_player0);
	v_player1 = std::make_unique<t_player>(*this, a_player1);
	v_player0->v_opponent = v_player1.get();
	v_player1->v_opponent = v_player0.get();
	v_state_ready.v_step = [](t_stage& a_stage)
	{
		if (a_stage.v_duration > 0.0)
			a_stage.v_duration -= 1.0;
		else
			a_stage.f_transit_play();
	};
	v_state_ready.v_key_press = [](t_stage& a_stage, SDL_Keycode a_key)
	{
		switch (a_key) {
		case SDLK_RETURN:
			a_stage.f_transit_play();
			break;
		case SDLK_ESCAPE:
			a_stage.f_back();
			break;
		}
	};
	v_state_ready.v_key_release = [](t_stage& a_stage, SDL_Keycode a_key)
	{
	};
	v_state_play.v_step = [](t_stage& a_stage)
	{
		a_stage.f_step_things();
		if (!a_stage.v_ball->v_done) return;
		if (a_stage.v_duration > 0.0)
			a_stage.v_duration -= 1.0;
		else
			a_stage.f_next();
	};
	v_state_play.v_key_press = [](t_stage& a_stage, SDL_Keycode a_key)
	{
		switch (a_key) {
		case SDLK_RETURN:
			a_stage.f_next();
			break;
		case SDLK_ESCAPE:
			a_stage.f_back();
			break;
		}
	};
	v_state_play.v_key_release = [](t_stage& a_stage, SDL_Keycode a_key)
	{
	};
	a_controller0(v_state_play, *v_player0);
	a_controller1(v_state_play, *v_player1);
}

void t_stage::f_step()
{
	v_state->v_step(*this);
}

void t_stage::f_render(size_t a_width, size_t a_height)
{
	glViewport(0, 0, a_width, a_height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	v_ball->f_setup();
	v_mark->f_setup();
	v_player0->f_setup();
	v_player1->f_setup();
	glEnable(GL_DEPTH_TEST);
	if (v_dual) glViewport(0, 0, a_width / 2, a_height);
	float pw = a_width * (v_dual ? 0.5 : 1.0) / a_height;
	float ph = 1.0;
	auto projection = f_frustum(-pw, pw, -ph, ph, 10.0f, 200.0f);
	v_scene.f_render(projection, v_camera0.f_viewing());
	if (v_dual) {
		glViewport(a_width / 2, 0, a_width - a_width / 2, a_height);
		v_scene.f_render(projection, v_camera1.f_viewing());
		glViewport(0, 0, a_width, a_height);
	}
	glDisable(GL_DEPTH_TEST);
	float y = v_message.size() * 0.5 - 1.0;
	for (const auto& line : v_message) {
		auto viewing = v_text_viewing * t_translate3f(line.size() * -0.25, y, 0.0);
		v_main.v_text_renderer(v_main.v_text_projection, viewing, line);
		y -= 1.0;
	}
}

void t_stage::f_key_press(SDL_Keycode a_key)
{
	v_state->v_key_press(*this, a_key);
}

void t_stage::f_key_release(SDL_Keycode a_key)
{
	v_state->v_key_release(*this, a_key);
}