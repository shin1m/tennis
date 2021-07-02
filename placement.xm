glmatrix = Module("glmatrix"
collada = Module("collada"

Matrix4 = glmatrix.Matrix4
Vector3 = glmatrix.Vector3

Posture = collada.Matrix + @
	$valid
	$toward
	$upward
	$__initialize = @(*xs)
		collada.Matrix.__initialize[$](*xs
		$valid = false
		$toward = Vector3(0.0, 0.0, 1.0
		$upward = Vector3(0.0, 1.0, 0.0
	$setup = @
		left = $upward ^ $toward
		$toward.normalize(
		left.normalize(
		$upward = $toward ^ left
		v = $v
		v[0] = left.x
		v[1] = left.y
		v[2] = left.z
		v[4] = $upward.x
		v[5] = $upward.y
		v[6] = $upward.z
		v[8] = $toward.x
		v[9] = $toward.y
		v[10] = $toward.z
	$validate = @
		if !$valid
			$setup(
			$valid = true
		$
	$viewing = @
		left = $upward ^ $toward
		$toward.normalize(
		left.normalize(
		$upward = $toward ^ left
		m = Matrix4(
		v = m.v
		v[0] = -left.x
		v[1] = $upward.x
		v[2] = -$toward.x
		v[4] = -left.y
		v[5] = $upward.y
		v[6] = -$toward.y
		v[8] = -left.z
		v[9] = $upward.z
		v[10] = -$toward.z
		m
$Posture = Posture

$Placement = Posture + @
	$position
	$__initialize = @
		Posture.__initialize[$](
		$position = Vector3(0.0, 0.0, 0.0
	$setup = @
		Posture.setup[$](
		v = $v
		v[12] = $position.x
		v[13] = $position.y
		v[14] = $position.z
	$viewing = @
		m = Posture.viewing[$](
		v = m.v
		v[12] = -(v[0] * $position.x + v[4] * $position.y + v[8] * $position.z)
		v[13] = -(v[1] * $position.x + v[5] * $position.y + v[9] * $position.z)
		v[14] = -(v[2] * $position.x + v[6] * $position.y + v[10] * $position.z)
		m
	$copy = @(to)
		to.toward = +$toward
		to.upward = +$upward
		to.position = +$position
		to.valid = false
