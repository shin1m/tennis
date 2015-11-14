#ifndef STAGE_H
#define STAGE_H

#include <gl/image.h>

#include "sdl_core.h"
#include "player.h"

struct t_main;

struct t_screen
{
	t_main& v_main;

	t_screen(t_main& a_main) : v_main(a_main)
	{
	}
	virtual ~t_screen() = default;
	virtual void f_step() = 0;
	virtual void f_render() = 0;
	virtual void f_key_press(SDL_Keycode a_key) = 0;
	virtual void f_key_release(SDL_Keycode a_key) = 0;
	virtual void f_finger_down(const SDL_TouchFingerEvent& a_event) = 0;
	virtual void f_finger_up(const SDL_TouchFingerEvent& a_event) = 0;
	virtual void f_finger_motion(const SDL_TouchFingerEvent& a_event) = 0;
};

struct t_stage : t_screen
{
	struct t_state
	{
		std::function<void (t_stage&)> v_step;
		std::function<void (t_stage&, SDL_Keycode)> v_key_press;
		std::function<void (t_stage&, SDL_Keycode)> v_key_release;
		std::function<void (t_stage&)> v_render;
		std::function<void (t_stage&, const SDL_TouchFingerEvent&)> v_finger_down;
		std::function<void (t_stage&, const SDL_TouchFingerEvent&)> v_finger_up;
		std::function<void (t_stage&, const SDL_TouchFingerEvent&)> v_finger_motion;
	};

	bool v_dual;
	bool v_fixed;
	t_chunk v_sound_bounce;
	t_chunk v_sound_net;
	t_chunk v_sound_chip;
	t_chunk v_sound_hit;
	t_chunk v_sound_swing;
	t_chunk v_sound_ace;
	t_chunk v_sound_miss;
	t_document v_scene;
	t_placement v_camera0;
	t_placement v_camera1;
	std::unique_ptr<t_ball> v_ball;
	std::unique_ptr<t_mark> v_mark;
	float v_side;
	std::unique_ptr<t_player> v_player0;
	std::unique_ptr<t_player> v_player1;
	const t_state* v_state;
	float v_duration;
	t_state v_state_ready;
	t_state v_state_play;
	t_matrix4f v_text_viewing = static_cast<t_matrix4f>(t_scale3f(0.25f, 0.25f, 1.0f));
	std::vector<std::wstring> v_message;

	virtual void f_ball_ace() = 0;
	virtual void f_ball_let() = 0;
	virtual void f_serve_miss() = 0;
	virtual void f_miss(const std::wstring& a_message) = 0;
	virtual void f_ball_serve_air() = 0;
	void f_ball_bounce()
	{
		v_sound_bounce.f_play();
	}
	virtual void f_ball_in();
	void f_ball_net()
	{
		v_sound_net.f_play();
	}
	void f_ball_chip()
	{
		v_sound_chip.f_play();
	}
	void f_ball_miss()
	{
		v_ball->f_serving() ? f_serve_miss() : f_miss(L"MISS");
	}
	void f_ball_out()
	{
		v_mark->f_mark(*v_ball);
		v_ball->f_serving() ? f_serve_miss() : f_miss(L"OUT");
	}
	virtual void f_step_things();
	virtual void f_next() = 0;
	virtual void f_back() = 0;
	virtual void f_transit_play() = 0;

	t_stage(t_main& a_main, bool a_dual, bool a_fixed, const std::function<void (t_stage::t_state&, t_player&)>& a_controller0, const std::wstring& a_player0, const std::function<void (t_stage::t_state&, t_player&)>& a_controller1, const std::wstring& a_player1);
	virtual void f_step();
	virtual void f_render();
	virtual void f_key_press(SDL_Keycode a_key);
	virtual void f_key_release(SDL_Keycode a_key);
	virtual void f_finger_down(const SDL_TouchFingerEvent& a_event);
	virtual void f_finger_up(const SDL_TouchFingerEvent& a_event);
	virtual void f_finger_motion(const SDL_TouchFingerEvent& a_event);
};

#endif
