#ifndef MAIN_H
#define MAIN_H

#include <gl/image.h>

#include "stage.h"

struct t_main
{
	std::wstring v_root;
	gl::t_shaders v_shaders;
	gl::t_text_renderer v_text_renderer;
	t_sound v_sound_cursor;
	t_sound v_sound_select;
	t_matrix4f v_text_projection;
	std::unique_ptr<t_screen> v_screen;

	void f_load(t_sound& a_sound, const std::wstring& a_name);
};

struct t_match : t_stage
{
	bool v_closed;
	bool v_second;
	float v_end;
	t_player* v_server;
	t_player* v_receiver;

	void f_new_game();
	void f_point(t_player& a_player);
	virtual void f_ball_ace();
	virtual void f_ball_let();
	virtual void f_serve_miss();
	virtual void f_miss(const std::wstring& a_message);
	virtual void f_ball_serve_air();
	static std::wstring v_points0[4];
	static std::wstring v_points1[4];
	void f_transit_ready();
	static const t_state v_state_close;
	void f_transit_close();
	virtual void f_next();
	virtual void f_back();
	virtual void f_transit_play();
	void f_reset();

	t_match(t_main& a_main, bool a_dual, bool a_fixed, const std::function<void (t_stage::t_state&, t_player&)>& a_controller0, const std::wstring& a_player0, const std::function<void (t_stage::t_state&, t_player&)>& a_controller1, const std::wstring& a_player1) : t_stage(a_main, a_dual, a_fixed, a_controller0, a_player0, a_controller1, a_player1)
	{
		f_new_set();
	}
	void f_new_set();
};

struct t_training : t_stage
{
	size_t v_selected = 0;

	virtual void f_ball_ace();
	virtual void f_ball_let();
	virtual void f_serve_miss();
	virtual void f_ball_serve_air();
	virtual void f_miss(const std::wstring& a_message);
	virtual void f_ball_in();
	virtual void f_step_things();

	t_training(t_main& a_main, const std::function<void (t_stage::t_state&, t_player&)>& a_controller0, const std::wstring& a_player0, const std::wstring& a_player1);
	virtual void f_next();
	void f_reset(float a_x, float a_y, float a_z, const t_vector3f& a_position, const t_player::t_swing& a_shot);
	void f_toss(t_player::t_swing& a_shot);
	static const std::vector<std::wstring> v_toss_message;
	struct t_item
	{
		std::wstring v_label;
		std::function<void (t_training& a_stage)> v_reset;
		std::function<void (t_training& a_stage)> v_do;
		std::function<void (t_training& a_stage)> v_ready;
		std::function<void (t_training& a_stage)> v_play;
		std::function<void (t_training& a_stage)> v_back;
	};
	static const std::vector<t_item> v_items;
	void f_update_select();
	virtual void f_back();
	void f_exit();
	void f_up();
	void f_down();
	void f_select();
	static const t_state v_state_select;
	virtual void f_transit_play();
	void f_transit_select();
	void f_transit_ready();
};

struct t_menu
{
	struct t_item
	{
		std::wstring v_label;
		std::function<void ()> v_do;
	};

	t_main& v_main;
	size_t v_columns;
	std::function<void ()> v_back;
	std::vector<t_item> v_items;
	size_t v_selected = 0;

	t_menu(t_main& a_main, size_t a_columns = 1) : v_main(a_main), v_columns(a_columns)
	{
	}
	void f_render(const t_matrix4f& a_viewing);
	void f_up();
	void f_down();
	void f_left();
	void f_right();
	void f_select();
	void f_key_press(SDL_Keycode a_key);
};

struct t_dialog
{
	t_main& v_main;
	gl::t_text_renderer v_image;
	t_sound v_sound;
	std::wstring v_title;

	t_dialog(t_main& a_main, const std::wstring& a_image, const std::wstring& a_sound, const std::wstring& a_title);
	virtual ~t_dialog() = default;
	virtual void f_step() = 0;
	virtual void f_render(size_t a_width, size_t a_height, const t_matrix4f& a_viewing);
	virtual void f_key_press(SDL_Keycode a_key) = 0;
};

struct t_main_screen;

struct t_stage_menu : t_dialog
{
	t_menu v_player0;
	t_menu v_player1;
	t_menu* v_menu;
	t_menu* v_transit = nullptr;
	float v_direction;
	float v_duration;
	float v_t;

	t_stage_menu(t_main_screen& a_screen, const std::wstring& a_image, const std::wstring& a_sound, const std::wstring& a_title, const std::function<void (const std::wstring&, const std::wstring&)>& a_done);
	virtual void f_step();
	virtual void f_render(size_t a_width, size_t a_height, const t_matrix4f& a_viewing);
	virtual void f_key_press(SDL_Keycode a_key);
	void f_transit(t_menu& a_menu, float a_direction);
};

struct t_main_menu : t_dialog, t_menu
{
	t_main_menu(t_main_screen& a_screen);
	virtual void f_step();
	virtual void f_render(size_t a_width, size_t a_height, const t_matrix4f& a_viewing);
	virtual void f_key_press(SDL_Keycode a_key);
};

struct t_main_screen : t_screen
{
	std::vector<std::tuple<std::wstring, std::wstring>> v_players;
	std::unique_ptr<t_dialog> v_dialog;
	std::unique_ptr<t_dialog> v_transit;
	float v_direction;
	float v_duration;
	float v_t;

	t_main_screen(t_main& a_main);
	virtual void f_step();
	virtual void f_render(size_t a_width, size_t a_height);
	virtual void f_key_press(SDL_Keycode a_key);
	virtual void f_key_release(SDL_Keycode a_key);
	void f_transit(std::unique_ptr<t_dialog>&& a_dialog, float a_direction);
};

#endif
