glmatrix = Module("glmatrix");
collada = Module("collada");

Matrix4 = glmatrix.Matrix4;
Vector3 = glmatrix.Vector3;

$Posture = Class(collada.Matrix) :: @{
	$__initialize = @{
		:$^__initialize[$]();
		$valid = false;
		$toward = Vector3(0.0, 0.0, 1.0);
		$upward = Vector3(0.0, 1.0, 0.0);
	};
	$setup = @{
		left = $upward ^ $toward;
		$toward.normalize();
		left.normalize();
		$upward = $toward ^ left;
		v = $v;
		v[0] = left.x;
		v[1] = $upward.x;
		v[2] = $toward.x;
		v[4] = left.y;
		v[5] = $upward.y;
		v[6] = $toward.y;
		v[8] = left.z;
		v[9] = $upward.z;
		v[10] = $toward.z;
	};
	$validate = @{
		if ($valid) return $;
		$setup();
		$valid = true;
		$;
	};
	$viewing = @{
		left = $upward ^ $toward;
		$toward.normalize();
		left.normalize();
		$upward = $toward ^ left;
		m = Matrix4();
		v = m.v;
		v[0] = -left.x;
		v[4] = $upward.x;
		v[8] = -$toward.x;
		v[1] = -left.y;
		v[5] = $upward.y;
		v[9] = -$toward.y;
		v[2] = -left.z;
		v[6] = $upward.z;
		v[10] = -$toward.z;
		m;
	};
};

$Placement = Class($Posture) :: @{
	$__initialize = @{
		:$^__initialize[$]();
		$position = Vector3(0.0, 0.0, 0.0);
	};
	$setup = @{
		:$^setup[$]();
		v = $v;
		v[3] = $position.x;
		v[7] = $position.y;
		v[11] = $position.z;
	};
	$viewing = @{
		m = :$^viewing[$]();
		v = m.v;
		v[3] = -(v[0] * $position.x + v[1] * $position.y + v[2] * $position.z);
		v[7] = -(v[4] * $position.x + v[5] * $position.y + v[6] * $position.z);
		v[11] = -(v[8] * $position.x + v[9] * $position.y + v[10] * $position.z);
		m;
	};
};
