#ifndef PLACEMENT_H
#define PLACEMENT_H

#include "collada.h"

struct t_posture : t_matrix_transform
{
	bool v_valid = false;
	t_vector3f v_toward{0.0, 0.0, 1.0};
	t_vector3f v_upward{0.0, 1.0, 0.0};

	void f_setup()
	{
		auto left = (v_upward ^ v_toward).f_normalized();
		v_toward.f_normalize();
		v_upward = v_toward ^ left;
		v[0][0] = left.v_x;
		v[0][1] = v_upward.v_x;
		v[0][2] = v_toward.v_x;
		v[1][0] = left.v_y;
		v[1][1] = v_upward.v_y;
		v[1][2] = v_toward.v_y;
		v[2][0] = left.v_z;
		v[2][1] = v_upward.v_z;
		v[2][2] = v_toward.v_z;
	}
	void f_validate()
	{
		if (v_valid) return;
		f_setup();
		v_valid = true;
	}
	t_matrix4f f_viewing()
	{
		auto left = (v_upward ^ v_toward).f_normalized();
		v_toward.f_normalize();
		v_upward = v_toward ^ left;
		t_matrix4f m(1.0);
		m[0][0] = -left.v_x;
		m[1][0] = v_upward.v_x;
		m[2][0] = -v_toward.v_x;
		m[0][1] = -left.v_y;
		m[1][1] = v_upward.v_y;
		m[2][1] = -v_toward.v_y;
		m[0][2] = -left.v_z;
		m[1][2] = v_upward.v_z;
		m[2][2] = -v_toward.v_z;
		return m;
	}
};

struct t_placement : t_posture
{
	t_vector3f v_position{0.0, 0.0, 0.0};

	void f_setup()
	{
		t_posture::f_setup();
		v[0][3] = v_position.v_x;
		v[1][3] = v_position.v_y;
		v[2][3] = v_position.v_z;
	}
	void f_validate()
	{
		if (v_valid) return;
		f_setup();
		v_valid = true;
	}
	t_matrix4f f_viewing()
	{
		auto m = t_posture::f_viewing();
		m[0][3] = -(m[0][0] * v_position.v_x + m[0][1] * v_position.v_y + m[0][2] * v_position.v_z);
		m[1][3] = -(m[1][0] * v_position.v_x + m[1][1] * v_position.v_y + m[1][2] * v_position.v_z);
		m[2][3] = -(m[2][0] * v_position.v_x + m[2][1] * v_position.v_y + m[2][2] * v_position.v_z);
		return m;
	}
};

#endif
