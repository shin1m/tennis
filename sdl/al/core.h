#ifndef AL__CORE_H
#define AL__CORE_H

#include <array>
#include <AL/al.h>
#include <AL/alut.h>

namespace al
{

struct t_alut
{
	t_alut(int* argc, char** argv)
	{
		if (alutInit(argc, argv) != AL_TRUE) throw std::runtime_error(std::string("alutInit Error: ") + alutGetErrorString(alutGetError()));
	}
	~t_alut()
	{
		if (alutExit() != AL_TRUE) throw std::runtime_error(std::string("alutExit Error: ") + alutGetErrorString(alutGetError()));
	}
};

inline void f_check()
{
	ALenum error = alGetError();
	if (error != AL_NO_ERROR) throw std::runtime_error(alGetString(error));
}

class t_buffer
{
	ALuint v_id = AL_NONE;

public:
	~t_buffer()
	{
		if (v_id == AL_NONE) return;
		alDeleteBuffers(1, &v_id);
		f_check();
	}
	void f_create()
	{
		assert(v_id == AL_NONE);
		alGenBuffers(1, &v_id);
		f_check();
	}
	void f_create_from_file(const char* a_path)
	{
		assert(v_id == AL_NONE);
		v_id = alutCreateBufferFromFile(a_path);
		if (v_id == AL_NONE) throw std::runtime_error(std::string("alutCreateBufferFromFile Error: ") + alutGetErrorString(alutGetError()));
	}
	operator ALuint() const
	{
		return v_id;
	}
	void f_data(ALenum a_format, const ALvoid* a_data, ALsizei a_size, ALsizei a_frequency)
	{
		alBufferData(v_id, a_format, a_data, a_size, a_frequency);
		f_check();
	}
	void f_set(ALenum a_parameter, ALfloat a_value)
	{
		alBufferf(v_id, a_parameter, a_value);
		f_check();
	}
	void f_set(ALenum a_parameter, ALfloat a_value1, ALfloat a_value2, ALfloat a_value3)
	{
		alBuffer3f(v_id, a_parameter, a_value1, a_value2, a_value3);
		f_check();
	}
	void f_set(ALenum a_parameter, ALint a_value)
	{
		alBufferi(v_id, a_parameter, a_value);
		f_check();
	}
	void f_set(ALenum a_parameter, ALint a_value1, ALint a_value2, ALint a_value3)
	{
		alBuffer3i(v_id, a_parameter, a_value1, a_value2, a_value3);
		f_check();
	}
	ALfloat f_getf(ALenum a_parameter) const
	{
		ALfloat value;
		alGetBufferf(v_id, a_parameter, &value);
		f_check();
		return value;
	}
	std::array<ALfloat, 3> f_get3f(ALenum a_parameter) const
	{
		ALfloat value1;
		ALfloat value2;
		ALfloat value3;
		alGetBuffer3f(v_id, a_parameter, &value1, &value2, &value3);
		f_check();
		return std::array<ALfloat, 3>{value1, value2, value3};
	}
	ALint f_geti(ALenum a_parameter) const
	{
		ALint value;
		alGetBufferi(v_id, a_parameter, &value);
		f_check();
		return value;
	}
	std::array<ALint, 3> f_get3i(ALenum a_parameter) const
	{
		ALint value1;
		ALint value2;
		ALint value3;
		alGetBuffer3i(v_id, a_parameter, &value1, &value2, &value3);
		f_check();
		return std::array<ALint, 3>{value1, value2, value3};
	}
};

class t_source
{
	ALuint v_id = AL_NONE;

public:
	~t_source()
	{
		if (v_id == AL_NONE) return;
		alDeleteSources(1, &v_id);
		f_check();
	}
	void f_create()
	{
		assert(v_id == AL_NONE);
		alGenSources(1, &v_id);
		f_check();
	}
	operator ALuint() const
	{
		return v_id;
	}
	void f_set(ALenum a_parameter, ALfloat a_value)
	{
		alSourcef(v_id, a_parameter, a_value);
		f_check();
	}
	void f_set(ALenum a_parameter, ALfloat a_value1, ALfloat a_value2, ALfloat a_value3)
	{
		alSource3f(v_id, a_parameter, a_value1, a_value2, a_value3);
		f_check();
	}
	void f_set(ALenum a_parameter, ALint a_value)
	{
		alSourcei(v_id, a_parameter, a_value);
		f_check();
	}
	void f_set(ALenum a_parameter, ALint a_value1, ALint a_value2, ALint a_value3)
	{
		alSource3i(v_id, a_parameter, a_value1, a_value2, a_value3);
		f_check();
	}
	void f_set(ALenum a_parameter, bool a_value)
	{
		alSourcei(v_id, a_parameter, a_value ? AL_TRUE : AL_FALSE);
		f_check();
	}
	ALfloat f_getf(ALenum a_parameter) const
	{
		ALfloat value;
		alGetSourcef(v_id, a_parameter, &value);
		f_check();
		return value;
	}
	std::array<ALfloat, 3> f_get3f(ALenum a_parameter) const
	{
		ALfloat value1;
		ALfloat value2;
		ALfloat value3;
		alGetSource3f(v_id, a_parameter, &value1, &value2, &value3);
		f_check();
		return std::array<ALfloat, 3>{value1, value2, value3};
	}
	ALint f_geti(ALenum a_parameter) const
	{
		ALint value;
		alGetSourcei(v_id, a_parameter, &value);
		f_check();
		return value;
	}
	std::array<ALint, 3> f_get3i(ALenum a_parameter) const
	{
		ALint value1;
		ALint value2;
		ALint value3;
		alGetSource3i(v_id, a_parameter, &value1, &value2, &value3);
		f_check();
		return std::array<ALint, 3>{value1, value2, value3};
	}
	bool f_getb(ALenum a_parameter) const
	{
		ALint value;
		alGetSourcei(v_id, a_parameter, &value);
		f_check();
		return value != AL_FALSE;
	}
	void f_play()
	{
		alSourcePlay(v_id);
		f_check();
	}
	void f_stop()
	{
		alSourceStop(v_id);
		f_check();
	}
	void f_rewind()
	{
		alSourceRewind(v_id);
		f_check();
	}
	void f_pause()
	{
		alSourcePause(v_id);
		f_check();
	}
	void f_queue_buffers(ALsizei a_n, ALuint* a_buffers)
	{
		alSourceQueueBuffers(v_id, a_n, a_buffers);
		f_check();
	}
	void f_unqueue_buffers(ALsizei a_n, ALuint* a_buffers)
	{
		alSourceUnqueueBuffers(v_id, a_n, a_buffers);
		f_check();
	}
};

}

#endif
