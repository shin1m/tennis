#ifndef MAIN_H
#define MAIN_H

#include <gl/image.h>

#include "portable.h"
#include "stage.h"

struct t_main
{
	std::wstring v_prefix;
	bool v_show_pad;
	gl::t_shaders v_shaders;
	gl::t_font v_font;
	t_chunk v_sound_cursor;
	t_chunk v_sound_select;
	t_matrix4f v_projection;
	t_scale3f v_text_scale{1.0, 1.0, 1.0};
	gl::t_buffer v_triangle;
	std::unique_ptr<t_screen> v_screen;

	t_main(const std::wstring& a_prefix, bool a_show_pad);
	std::wstring f_path(const std::wstring& a_name) const
	{
		auto name = a_name;
		std::replace(name.begin(), name.end(), L'/', wchar_t(v_directory_separator));
		return v_prefix + name;
	}
	std::unique_ptr<xmlParserInputBuffer, void (*)(xmlParserInputBufferPtr)> f_input(const std::wstring& a_name);
	void f_load(t_document& a_document, const std::wstring& a_name);
	void f_load(gl::t_image& a_image, const std::wstring& a_name);
	template<typename T>
	void f_load(T& a_sound, const std::wstring& a_name)
	{
		a_sound.f_create(f_convert(f_path(a_name)).c_str());
	}
};

template<typename T_item>
struct t_menu
{
	t_main& v_main;
	size_t v_columns;
	std::function<void ()> v_back;
	std::vector<T_item> v_items;
	size_t v_selected = 0;

	template<typename T_callback>
	void f_each(const t_matrix4f& a_viewing, T_callback a_callback)
	{
		size_t n = (v_items.size() + v_columns - 1) / v_columns;
		float scale = 1.0 / std::max(n, size_t(4));
		auto viewing = a_viewing * t_scale3f(scale, scale, 1.0);
		float dx = 4.0 * 2.0 / v_columns;
		float x = dx * 0.5 - 4.0;
		size_t k = 0;
		for (size_t i = 0; i < v_columns; ++i) {
			float y = 0.0;
			for (size_t j = 0; j < n && k < v_items.size(); ++j) {
				y -= 1.0;
				a_callback(k, viewing * t_translate3f(x, y, 0.0));
				++k;
			}
			x += dx;
		}
	}
	int f_nearest(const SDL_TouchFingerEvent& a_event, const t_matrix4f& a_viewing)
	{
		int i = -1;
		float d = 2.0;
		t_vector3f v(a_event.x * 2.0 - 1.0, a_event.y * -2.0 + 1.0, 0.0);
		f_each(a_viewing, [&](size_t a_i, const t_matrix4f& a_viewing)
		{
			float a = f_affine(~(v_main.v_projection * a_viewing * t_translate3f(0.0, 0.5, 0.0) * t_scale3f(4.0, 1.0, 1.0)), v).f_length();
			if (a > d) return;
			i = a_i;
			d = a;
		});
		return i;
	}

	t_menu(t_main& a_main, size_t a_columns = 1) : v_main(a_main), v_columns(a_columns)
	{
	}
	void f_render(const t_matrix4f& a_viewing)
	{
		f_each(a_viewing, [&](size_t a_i, const t_matrix4f& a_viewing)
		{
			const auto& item = v_items[a_i];
			auto text = (a_i == v_selected ? L"*" : L" ") + item.v_label;
			v_main.v_font(v_main.v_projection, a_viewing * t_translate3f(text.size() * -0.25, 0.0, 0.0), text);
		});
	}
	T_item& f_selected()
	{
		return v_items[v_selected];
	}
	void f_select(size_t a_i)
	{
		v_selected = a_i;
		v_main.v_sound_cursor.f_play();
		v_items[v_selected].v_select();
	}
	void f_up()
	{
		if (v_selected > 0) f_select(v_selected - 1);
	}
	void f_down()
	{
		if (v_selected < v_items.size() - 1) f_select(v_selected + 1);
	}
	void f_left()
	{
		size_t n = (v_items.size() + v_columns - 1) / v_columns;
		if (v_selected >= n) f_select(v_selected - n);
	}
	void f_right()
	{
		size_t n = (v_items.size() + v_columns - 1) / v_columns;
		if (v_selected < v_items.size() - n) f_select(v_selected + n);
	}
	void f_do()
	{
		v_main.v_sound_select.f_play();
		v_items[v_selected].v_do();
	}
	void f_key_press(SDL_Keycode a_key)
	{
		switch (a_key) {
		case SDLK_ESCAPE:
			v_back();
			break;
		case SDLK_SPACE:
		case SDLK_2:
			f_do();
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
	void f_finger_down(const SDL_TouchFingerEvent& a_event, const t_matrix4f& a_viewing)
	{
		int i = f_nearest(a_event, a_viewing);
		if (i >= 0 && i != v_selected) f_select(i);
	}
	void f_finger_up(const SDL_TouchFingerEvent& a_event, const t_matrix4f& a_viewing)
	{
		int i = f_nearest(a_event, a_viewing);
		if (i < 0) {
			if (a_event.x < 0.125) v_back();
		} else {
			v_selected = i;
			f_do();
		}
	}
	void f_finger_motion(const SDL_TouchFingerEvent& a_event, const t_matrix4f& a_viewing)
	{
		int i = f_nearest(a_event, a_viewing);
		if (i >= 0 && i != v_selected) f_select(i);
	}
};

struct t_menu_item
{
	std::wstring v_label;
	std::function<void ()> v_select;
	std::function<void ()> v_do;
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
	struct t_item
	{
		std::wstring v_label;
		std::function<void ()> v_select;
		std::function<void ()> v_do;
		std::function<void ()> v_ready;
		std::function<void ()> v_play;
		std::function<void ()> v_back;
	};

	static const std::vector<std::wstring> v_toss_message;

	t_menu<t_item> v_menu;

	t_matrix4f f_transform() const
	{
		return static_cast<t_matrix4f>(v_main.v_text_scale) * t_translate3f(0.0, 0.5, 0.0);
	}
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
	virtual void f_back();
	void f_exit();
	static const t_state v_state_select;
	virtual void f_transit_play();
	void f_transit_select();
	void f_transit_ready();
};

struct t_dialog
{
	t_main& v_main;
	gl::t_image v_image;
	t_music v_sound;
	std::wstring v_title;

	t_dialog(t_main& a_main, const std::wstring& a_image, const std::wstring& a_sound, const std::wstring& a_title);
	virtual ~t_dialog() = default;
	virtual void f_step() = 0;
	virtual void f_render(size_t a_width, size_t a_height, const t_matrix4f& a_viewing);
	virtual void f_key_press(SDL_Keycode a_key) = 0;
	virtual void f_finger_down(const SDL_TouchFingerEvent& a_event) = 0;
	virtual void f_finger_up(const SDL_TouchFingerEvent& a_event) = 0;
	virtual void f_finger_motion(const SDL_TouchFingerEvent& a_event) = 0;
};

struct t_main_screen;

struct t_stage_menu : t_dialog
{
	t_menu<t_menu_item> v_player0;
	t_menu<t_menu_item> v_player1;
	t_menu<t_menu_item> v_ready;
	t_menu<t_menu_item>* v_menu;
	t_menu<t_menu_item>* v_transit = nullptr;
	float v_direction;
	float v_duration;
	float v_t;

	t_matrix4f f_transform(const t_matrix4f& a_viewing) const
	{
		return a_viewing * v_main.v_text_scale;
	}

	t_stage_menu(t_main_screen& a_screen, const std::wstring& a_image, const std::wstring& a_sound, const std::wstring& a_title, const std::function<void (const std::wstring&, const std::wstring&)>& a_done);
	virtual void f_step();
	virtual void f_render(size_t a_width, size_t a_height, const t_matrix4f& a_viewing);
	virtual void f_key_press(SDL_Keycode a_key);
	virtual void f_finger_down(const SDL_TouchFingerEvent& a_event);
	virtual void f_finger_up(const SDL_TouchFingerEvent& a_event);
	virtual void f_finger_motion(const SDL_TouchFingerEvent& a_event);
	void f_transit(t_menu<t_menu_item>& a_menu, float a_direction);
};

struct t_main_menu : t_dialog, t_menu<t_menu_item>
{
	t_matrix4f f_transform(const t_matrix4f& a_viewing) const
	{
		return a_viewing * t_dialog::v_main.v_text_scale * t_translate3f(0.0, 0.125, 0.0);
	}

	t_main_menu(t_main_screen& a_screen);
	virtual void f_step();
	virtual void f_render(size_t a_width, size_t a_height, const t_matrix4f& a_viewing);
	virtual void f_key_press(SDL_Keycode a_key);
	virtual void f_finger_down(const SDL_TouchFingerEvent& a_event);
	virtual void f_finger_up(const SDL_TouchFingerEvent& a_event);
	virtual void f_finger_motion(const SDL_TouchFingerEvent& a_event);
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
	virtual void f_finger_down(const SDL_TouchFingerEvent& a_event, size_t a_width, size_t a_height);
	virtual void f_finger_up(const SDL_TouchFingerEvent& a_event, size_t a_width, size_t a_height);
	virtual void f_finger_motion(const SDL_TouchFingerEvent& a_event, size_t a_width, size_t a_height);
	void f_transit(std::unique_ptr<t_dialog>&& a_dialog, float a_direction);
};

#endif
