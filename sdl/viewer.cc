#include "path.h"
#include "sdl_core.h"
#include "collada.h"

void f_loop(SDL_Window* a_window, const std::wstring& a_path)
{
	t_document document;
	document.f_load(a_path);
	gl::t_shaders shaders;
	document.f_build(shaders);
	t_translate3f translate(0.0, 0.0, -100.0);
	//t_translate3f translate(0.0, 0.0, -900.0);
	bool z_up = document.v_asset.v_up_axis == L"Z_UP";
	t_rotate3f rotate_x(1.0, 0.0, 0.0, z_up ? -90.0 * M_PI / 180.0 : 0.0);
	t_rotate3f rotate_y(0.0, 1.0, 0.0, 0.0);
	t_rotate3f rotate_z(0.0, 0.0, 1.0, 0.0);
	auto iterators = document.f_iterators();
	float duration = 0.0;
	for (const auto& x : iterators) {
		float d = x.second->f_duration();
		if (d > duration) duration = d;
	}
	float time = 0.0;
	auto step = [&]
	{
		for (auto& x : iterators) x.second->f_forward(time);
		time += 1.0 / 60.0;
		while (time >= duration) {
			time -= duration;
			for (auto& x : iterators) x.second->f_rewind(time);
		}
	};
	SDL_GL_SetSwapInterval(1);
	int width;
	int height;
	SDL_GetWindowSize(a_window, &width, &height);
	auto resize = [](int a_width, int a_height)
	{
		glViewport(0, 0, a_width, a_height);
		float w = static_cast<float>(a_width) / a_height;
		float h = 1.0f;
		return f_frustum(-w, w, -h, h, 10.0f, 500.0f);
	};
	auto projection = resize(width, height);
	bool pressed = false;
	t_vector2f pressed_at;
	t_vector2f origin;
	std::function<void (float, float)> handler;
	bool animating = false;
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
					projection = resize(event.window.data1, event.window.data2);
					break;
				}
				break;
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym) {
				case SDLK_q:
					return;
				case SDLK_SPACE:
					animating ^= true;
					break;
				case SDLK_BACKSPACE:
					time = 0.0;
					for (auto& x : iterators) x.second->f_rewind(0.0);
					break;
				case SDLK_RETURN:
					step();
					break;
				}
				break;
			case SDL_MOUSEMOTION:
				if (pressed) handler(event.motion.x, event.motion.y);
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (!pressed) {
					pressed = true;
					pressed_at = t_vector2f(event.button.x, event.button.y);
					switch (event.button.button) {
					case SDL_BUTTON_LEFT:
						origin = t_vector2f(rotate_x.v_angle, z_up ? rotate_z.v_angle : rotate_y.v_angle);
						handler = [&](float a_x, float a_y)
						{
							rotate_x.v_angle = origin.v_x + (a_y - pressed_at.v_y) * M_PI / 180.0;
							(z_up ? rotate_z : rotate_y).v_angle = origin.v_y + (a_x - pressed_at.v_x) * M_PI / 180.0;
						};
						break;
					case SDL_BUTTON_MIDDLE:
						origin.v_x = translate.v_value.v_z;
						handler = [&](float a_x, float a_y)
						{
							translate.v_value.v_z = origin.v_x - a_y + pressed_at.v_y;
						};
						break;
					case SDL_BUTTON_RIGHT:
						origin = t_vector2f(translate.v_value.v_x, translate.v_value.v_y);
						handler = [&](float a_x, float a_y)
						{
							float a = 0.05;
							translate.v_value.v_x = origin.v_x + (a_x - pressed_at.v_x) * a;
							translate.v_value.v_y = origin.v_y - (a_y - pressed_at.v_y) * a;
						};
						break;
					}
				}
				break;
			case SDL_MOUSEBUTTONUP:
				pressed = false;
				break;
			}
		}
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		t_matrix4f viewing(1.0);
		viewing *= translate;
		viewing *= rotate_x;
		viewing *= rotate_y;
		viewing *= rotate_z;
		//viewing *= t_translate4f(0.0, 0.0, -80.0);
		//viewing *= t_translate4f(0.0, -10.0, 0.0);
		//meter = document.v_asset.v_unit.v_meter;
		//viewing *= t_scale3f(meter, meter, meter);
		//viewing *= t_scale3f(0.1, 0.1, 0.1);
		document.f_render(projection, viewing);
		SDL_GL_SwapWindow(a_window);
		if (animating) step();
	}
}

int main(int argc, char* argv[])
{
	if (argc < 2) return -1;
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	t_sdl sdl(SDL_INIT_VIDEO);
	t_sdl_image image(IMG_INIT_PNG | IMG_INIT_JPG);
	t_window window("OpenGL Collada", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	t_gl_context context(window);
	f_loop(window, f_convert(std::string(argv[1])));
	return 0;
}
