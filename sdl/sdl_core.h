#ifndef SDL_CORE_H
#define SDL_CORE_H

#include <stdexcept>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

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

#endif
