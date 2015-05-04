math = Module("math");
gl = Module("gl");
glmatrix = Module("glmatrix");
collada = Module("collada");
placement = Module("placement");

Vector3 = glmatrix.Vector3;
Placement = placement.Placement;

circle = @(n, sign = 1.0) {
	normal = Vector3(0.0, sign, 0.0);
	unit = sign * 2.0 * math.PI / n;
	triangles = [];
	for (i = 0; i < n; i = i + 1) {
		a = unit * i;
		b = a + unit;
		triangles.push('('(Vector3(0.0, 0.0, 0.0), normal), '(Vector3(math.cos(a), 0.0, -math.sin(a)), normal), '(Vector3(math.cos(b), 0.0, -math.sin(b)), normal)));
	}
	triangles;
};

divide = @(n, a, b, c, triangles) {
	if (n > 0) {
		n = n - 1;
		ab = (a + b).normalized();
		bc = (b + c).normalized();
		ca = (c + a).normalized();
		divide(n, a, ab, ca, triangles);
		divide(n, b, bc, ab, triangles);
		divide(n, c, ca, bc, triangles);
		divide(n, ab, bc, ca, triangles);
	} else {
		triangles.push('('(a, a), '(b, b), '(c, c)));
	}
};

sphere = @(n) {
	triangles = [];
	divide(n, Vector3(1.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0), Vector3(0.0, 0.0, 1.0), triangles);
	divide(n, Vector3(0.0, 0.0, 1.0), Vector3(0.0, 1.0, 0.0), Vector3(-1.0, 0.0, 0.0), triangles);
	divide(n, Vector3(-1.0, 0.0, 0.0), Vector3(0.0, 1.0, 0.0), Vector3(0.0, 0.0, -1.0), triangles);
	divide(n, Vector3(0.0, 0.0, -1.0), Vector3(0.0, 1.0, 0.0), Vector3(1.0, 0.0, 0.0), triangles);
	divide(n, Vector3(1.0, 0.0, 0.0), Vector3(0.0, -1.0, 0.0), Vector3(0.0, 0.0, -1.0), triangles);
	divide(n, Vector3(0.0, 0.0, -1.0), Vector3(0.0, -1.0, 0.0), Vector3(-1.0, 0.0, 0.0), triangles);
	divide(n, Vector3(-1.0, 0.0, 0.0), Vector3(0.0, -1.0, 0.0), Vector3(0.0, 0.0, 1.0), triangles);
	divide(n, Vector3(0.0, 0.0, 1.0), Vector3(0.0, -1.0, 0.0), Vector3(1.0, 0.0, 0.0), triangles);
	triangles;
};

node = @(resolve, shaders, triangles, material) {
	bytes0 = Bytes(triangles.size() * 9 * gl.Float32Array.BYTES_PER_ELEMENT);
	bytes1 = Bytes(triangles.size() * 9 * gl.Float32Array.BYTES_PER_ELEMENT);
	array0 = gl.Float32Array(bytes0);
	array1 = gl.Float32Array(bytes1);
	for (i = 0; i < triangles.size(); i = i + 1) {
		triangle = triangles[i];
		for (j = 0; j < 3; j = j + 1) {
			k = i * 9 + j * 3;
			vertex = triangle[j][0];
			normal = triangle[j][1];
			array0[k] = vertex.x;
			array0[k + 1] = vertex.y;
			array0[k + 2] = vertex.z;
			array1[k] = normal.x;
			array1[k + 1] = normal.y;
			array1[k + 2] = normal.z;
		}
	}
	mesh = collada.Mesh();
	mesh.primitives.push(collada.Triangles().create(triangles.size(), "Symbol", bytes0, bytes1, {}));
	node = collada.Node().create();
	node.geometries.push(collada.InstanceGeometry(null).create(resolve, shaders, mesh, {"Symbol": material}));
	node;
};

$G = G = 9.8 / (64.0 * 64.0);

$projected_time_for_y = projected_time_for_y = @(py, vy, y, sign) {
	a = vy * vy + 2.0 * G * (py - y);
	a < 0.0 ? null : (vy + sign * math.sqrt(a)) / G;
};

$Ball = Class() :: @{
	$radius = radius = 0.0625;

	$__initialize = @(stage, resolve, shaders, shadow, body) {
		$stage = stage;
		$position = Vector3(0.0, 0.0, 0.0);
		$velocity = Vector3(0.0, 0.0, 0.0);
		$spin = Vector3(0.0, 0.0, 0.0);
		$node = collada.Node().create();
		$translate = collada.Translate(0.0, 0.0, 0.0);
		$node.transforms.push($translate);
		shadow = node(resolve, shaders, circle(8), shadow);
		shadow.transforms.push(collada.Translate(0.0, 1.0 / 64.0, 0.0));
		shadow.transforms.push(collada.Scale(radius, 1.0, radius));
		$node.nodes.push(shadow);
		body = node(resolve, shaders, sphere(2), body);
		$body_translate = collada.Translate(0.0, 0.0, 0.0);
		body.transforms.push($body_translate);
		body.transforms.push(collada.Scale(radius, radius, radius));
		$node.nodes.push(body);
	};
	$setup = @{
		$translate.x = $position.x;
		$body_translate.y = $position.y;
		$translate.z = $position.z;
	};
	$netin_part = @(x0, y0, x1, y1) {
		ex = x1 - x0;
		ey = y1 - y0;
		l = math.sqrt(ex * ex + ey * ey);
		ex = ex / l;
		ey = ey / l;
		dx = $position.x - x0;
		dy = $position.y - y0;
		y = -ey * dx + ex * dy;
		if (y > radius) return;
		if (y < 0.0) {
			$position.z = $velocity.z < 0.0 ? radius : -radius;
			$velocity.z = $velocity.z * -1.0;
			$velocity.scale(0.125);
			$stage.ball_net();
		} else {
			x = ex * dx + ey * dy;
			$position.x = x0 + ex * x - ey * radius;
			$position.y = y0 + ey * x + ex * radius;
			if ($velocity.z < 0.0) {
				if ($position.z < 0.0) $position.z = 0.0;
			} else {
				if ($position.z > 0.0) $position.z = 0.0;
			}
			v = Vector3(ex, ey, 0.0);
			p = Vector3(dx, dy, $position.z);
			n = p ^ v;
			n.normalize();
			m = v ^ n;
			m.normalize();
			vv = v * $velocity * 0.375;
			vn = n * $velocity * 0.375;
			vm = m * $velocity * 0.0;
			$velocity = v * vv + n * vn + m * vm;
			$stage.ball_chip();
		}
		$net = true;
	};
	$netin = @{
		if ($position.x < -21 * 12 * 0.0254) return;
		if ($position.x < -0.0254)
			$netin_part(-21 * 12 * 0.0254, (3 * 12 + 6) * 0.0254, -0.0254, 3 * 12 * 0.0254);
		else if ($position.x < 0.0254)
			$netin_part(-0.0254, 3 * 12 * 0.0254, 0.0254, 3 * 12 * 0.0254);
		else if ($position.x < 21 * 12 * 0.0254)
			$netin_part(0.0254, 3 * 12 * 0.0254, 21 * 12 * 0.0254, (3 * 12 + 6) * 0.0254);
	};
	$emit_ace = @{
		$done = true;
		$stage.ball_ace();
	};
	$emit_out = @{
		$done = true;
		$stage.ball_out();
	};
	$emit_bounce = @() $stage.ball_bounce();
	$wall = @{
		if ($done) return;
		$in ? $emit_ace() : $emit_out();
	};
	$step = @{
		last = $position;
		$position = last + $velocity;
		$position.y = $position.y - 0.5 * G;
		$velocity.y = $velocity.y - G;
		$velocity = $velocity + ($spin ^ $velocity) * (1.0 / 4096.0);
		if ($position.y - radius <= 0.0) {
			$position.y = radius;
			$bounce();
			if (!$done) {
				if ($in) {
					$emit_ace();
				} else if ($hitter === null) {
					$done = true;
					$stage.ball_serve_air();
				} else {
					x = $hitter.end * $position.x;
					z = $hitter.end * $position.z;
					if (z > 0.0) {
						$done = true;
						$stage.ball_miss();
					} else if (x < $target[0] || x > $target[1] || z < $target[2]) {
						$emit_out();
					} else {
						$in = true;
						if ($serving() && $net) {
							$done = true;
							$stage.ball_let();
						} else {
							$stage.ball_in();
						}
					}
				}
			}
			if ($velocity.y > 1.0 / 64.0) $emit_bounce();
		}
		if ($position.x - radius <= -30 * 12 * 0.0254) {
			$position.x = radius - 30 * 12 * 0.0254;
			$velocity.x = $velocity.x * -0.5;
			$wall();
			if (math.fabs($velocity.x) > 1.0 / 64.0) $emit_bounce();
		} else if ($position.x + radius >= 30 * 12 * 0.0254) {
			$position.x = 30 * 12 * 0.0254 - radius;
			$velocity.x = $velocity.x * -0.5;
			$wall();
			if (math.fabs($velocity.x) > 1.0 / 64.0) $emit_bounce();
		}
		if ($position.z - radius <= -60 * 12 * 0.0254) {
			$position.z = radius - 60 * 12 * 0.0254;
			$velocity.z = $velocity.z * -0.5;
			$wall();
			if (math.fabs($velocity.z) > 1.0 / 64.0) $emit_bounce();
		} else if ($position.z + radius >= 60 * 12 * 0.0254) {
			$position.z = 60 * 12 * 0.0254 - radius;
			$velocity.z = $velocity.z * -0.5;
			$wall();
			if (math.fabs($velocity.z) > 1.0 / 64.0) $emit_bounce();
		}
		if ($velocity.z < 0.0) {
			if (last.z > radius && $position.z <= radius) $netin();
		} else {
			if (last.z < -radius && $position.z >= -radius) $netin();
		}
	};
	rally = '(-(13 * 12 + 6) * 0.0254, (13 * 12 + 6) * 0.0254, -39 * 12 * 0.0254);
	$set = @(hitter, target) {
		$hitter = hitter;
		$target = target;
		$in = $net = false;
	};
	$reset = @(side, x, y, z) {
		$position = Vector3(x, y, z);
		$velocity = Vector3(0.0, 0.0, 0.0);
		$spin = Vector3(0.0, 0.0, 0.0);
		$done = false;
		x0 = 1 * 0.0254 * side;
		x1 = -(13 * 12 + 6) * 0.0254 * side;
		$set(null, '(x0 < x1 ? x0 : x1, x0 < x1 ? x1 : x0, -21 * 12 * 0.0254));
	};
	$hit = @(hitter) $done || $set(hitter, rally);
	$serving = @() $target !== rally;
	$impact = @(dx, dz, speed, vy, spin) {
		dl = 1.0 / math.sqrt(dx * dx + dz * dz);
		dx = dx * dl;
		dz = dz * dl;
		$velocity = Vector3(dx * speed, vy, dz * speed);
		$spin = Vector3(-dz * spin.x + dx * spin.z, spin.y, dx * spin.x + dz * spin.z);
	};
	$calculate_bounce = @(velocity, spin) {
		f = 0.0;
		v0 = velocity;
		w0 = spin;
		v1x = v0.x + radius * w0.z;
		v1z = v0.z - radius * w0.x;
		e = 1.25 + v0.y * 10.0;
		if (e < 0.25) e = 0.0;
		if (e > 1.0) e = 1.0;
		b = (e + 2.0 / 3.0) * radius;
		w1x = (1.0 - e) * v0.z + b * w0.x + f * v1z;
		w1z = (e - 1.0) * v0.x + b * w0.z - f * v1x;
		velocity.x = e * v1x - 3.0 / 5.0 * w1z;
		velocity.y = v0.y * -0.75;
		velocity.z = e * v1z + 3.0 / 5.0 * w1x;
		d = 3.0 / 5.0 / radius;
		spin.x = w1x * d;
		spin.z = w1z * d;
	};
	$bounce = @() $calculate_bounce($velocity, $spin);
	$projected_time_for_y = @(y, sign) projected_time_for_y($position.y, $velocity.y, y, sign);
};

$Mark = Class() :: @{
	radius = 0.0625;

	$__initialize = @(resolve, shaders, shadow) {
		$duration = 0.0;
		$stretch = 1.0;
		$node = node(resolve, shaders, circle(8), shadow);
		$placement = Placement();
		$placement.position.y = 1.0 / 64.0;
		$node.transforms.push($placement);
		$scale = collada.Scale(radius, 1.0, radius);
		$node.transforms.push($scale);
		node__render = $node.render;
		$node.render = @(projection, viewing, joints) {
			if ($duration > 0.0) node__render(projection, viewing, joints);
		}[$];
	};
	$setup = @{
		$placement.validate();
		$scale.z = radius * $stretch;
	};
	$step = @() $duration > 0.0 && ($duration = $duration - 1.0);
	$mark = @(ball) {
		$duration = 2.0 * 64.0;
		$placement.position.x = ball.position.x;
		$placement.position.z = ball.position.z;
		$placement.toward = Vector3(ball.velocity.x, 0.0, ball.velocity.z);
		$placement.valid = false;
		$stretch = 1.0 + $placement.toward.length() * 8.0;
	};
};
