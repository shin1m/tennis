#ifndef SDL_CORE_H
#define SDL_CORE_H

#include <stdexcept>
#include <cassert>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

struct t_sdl
{
	t_sdl(Uint32 a_flags)
	{
		if (SDL_Init(a_flags) != 0) throw std::runtime_error(std::string("SDL_Init Error: ") + SDL_GetError());
	}
	~t_sdl()
	{
		SDL_Quit();
	}
};

class t_window
{
	SDL_Window* v_p;

public:
	t_window(const char* a_title, int a_x, int a_y, int a_width, int a_height, Uint32 a_flags) : v_p(SDL_CreateWindow(a_title, a_x, a_y, a_width, a_height, a_flags))
	{
		if (v_p == NULL) throw std::runtime_error(std::string("SDL_CreateWindow Error: ") + SDL_GetError());
	}
	~t_window()
	{
		if (v_p != NULL) SDL_DestroyWindow(v_p);
	}
	operator SDL_Window*() const
	{
		return v_p;
	}
};

class t_game_controller
{
	SDL_GameController* v_p = NULL;

public:
	~t_game_controller()
	{
		if (v_p != NULL) SDL_GameControllerClose(v_p);
	}
	void f_open(int a_index)
	{
		f_close();
		v_p = SDL_GameControllerOpen(a_index);
		if (v_p == NULL) throw std::runtime_error(std::string("SDL_GameControllerOpen Error: ") + SDL_GetError());
	}
	void f_close()
	{
		if (v_p == NULL) return;
		SDL_GameControllerClose(v_p);
		v_p = NULL;
	}
	operator SDL_GameController*() const
	{
		return v_p;
	}
	operator bool() const
	{
		return v_p != NULL && SDL_GameControllerGetAttached(v_p) == SDL_TRUE;
	}
	SDL_JoystickID f_joystick_id() const
	{
		return SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(v_p));
	}
};

class t_gl_context
{
	SDL_GLContext v_p;

public:
	t_gl_context(SDL_Window* a_window) : v_p(SDL_GL_CreateContext(a_window))
	{
		if (v_p == NULL) throw std::runtime_error(std::string("SDL_GL_CreateContext Error: ") + SDL_GetError());
	}
	~t_gl_context()
	{
		if (v_p != NULL) SDL_GL_DeleteContext(v_p);
	}
};

struct t_sdl_image
{
	t_sdl_image(int a_flags)
	{
		if (IMG_Init(a_flags) != a_flags) throw std::runtime_error(std::string("IMG_Init Error: ") + IMG_GetError());
	}
	~t_sdl_image()
	{
		IMG_Quit();
	}
};

struct t_sdl_ttf
{
	t_sdl_ttf()
	{
		if (TTF_Init() != 0) throw std::runtime_error(std::string("TTF_Init Error: ") + TTF_GetError());
	}
	~t_sdl_ttf()
	{
		TTF_Quit();
	}
};

class t_font
{
	TTF_Font* v_p;

public:
	t_font(const char* a_path, int a_size) : v_p(TTF_OpenFont(a_path, a_size))
	{
		if (v_p == NULL) throw std::runtime_error(std::string("TTF_OpenFont Error: ") + TTF_GetError());
	}
	~t_font()
	{
		if (v_p != NULL) TTF_CloseFont(v_p);
	}
	operator TTF_Font*() const
	{
		return v_p;
	}
};

struct t_sdl_mixer
{
	t_sdl_mixer(int a_flags)
	{
		Mix_Init(a_flags);
	}
	~t_sdl_mixer()
	{
		Mix_Quit();
	}
};

struct t_sdl_audio
{
	t_sdl_audio(int a_frequency, Uint16 a_format, int a_channels, int a_chunk)
	{
		if (Mix_OpenAudio(a_frequency, a_format, a_channels, a_chunk) != 0) throw std::runtime_error(std::string("Mix_OpenAudio Error: ") + Mix_GetError());
	}
	~t_sdl_audio()
	{
		Mix_CloseAudio();
	}
};

class t_chunk
{
	Mix_Chunk* v_p = NULL;

public:
	~t_chunk()
	{
		if (v_p != NULL) Mix_FreeChunk(v_p);
	}
	void f_create(const char* a_path)
	{
		assert(v_p == NULL);
		v_p = Mix_LoadWAV(a_path);
		if (v_p == NULL) throw std::runtime_error(std::string("Mix_LoadWAV Error: ") + Mix_GetError());
	}
	operator Mix_Chunk*() const
	{
		return v_p;
	}
	int f_play(int a_channel = -1, int a_loops = 0)
	{
		int channel = Mix_PlayChannel(a_channel, v_p, a_loops);
		if (channel == -1) throw std::runtime_error(std::string("Mix_PlayChannel Error: ") + Mix_GetError());
		return channel;
	}
};

class t_music
{
	Mix_Music* v_p = NULL;

public:
	~t_music()
	{
		if (v_p != NULL) Mix_FreeMusic(v_p);
	}
	void f_create(const char* a_path)
	{
		assert(v_p == NULL);
		v_p = Mix_LoadMUS(a_path);
		if (v_p == NULL) throw std::runtime_error(std::string("Mix_LoadMUS Error: ") + Mix_GetError());
	}
	operator Mix_Music*() const
	{
		return v_p;
	}
	void f_play(int a_loops)
	{
		if (Mix_PlayMusic(v_p, a_loops) != 0) throw std::runtime_error(std::string("Mix_PlayMusic Error: ") + Mix_GetError());
	}
	void f_stop()
	{
		Mix_HaltMusic();
	}
};

#endif
