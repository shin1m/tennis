system = Module("system");
print = system.error.write_line;
io = Module("io");
math = Module("math");
time = Module("time");
gl = Module("gl");
glmatrix = Module("glmatrix");
glshaders = Module("glshaders");
glimage = Module("glimage");
al = Module("al");
cairo = Module("cairo");
xraft = Module("xraft");
collada = Module("collada");

print_time = false;

Matrix3 = glmatrix.Matrix3;
Matrix4 = glmatrix.Matrix4;
Vector3 = glmatrix.Vector3;
Vector4 = glmatrix.Vector4;

Posture = Class(collada.Matrix) :: @{
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

Placement = Class(Posture) :: @{
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

node = @(triangles, shader) {
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
	node.geometries.push(collada.InstanceGeometry(null).create(mesh, {"Symbol": shader}));
	node;
};

G = 9.8 / (64.0 * 64.0);

projected_time_for_y = @(py, vy, y, sign) {
	a = vy * vy + 2.0 * G * (py - y);
	a < 0.0 ? null : (vy + sign * math.sqrt(a)) / G;
};

reach_range = @(ball, velocity, player, speed, t0, sign) {
	qp = ball - player;
	qp.y = 0.0;
	v = Vector3(velocity.x, 0.0, velocity.z);
	ss = speed * speed;
	a = v * v - ss;
	b = v * qp - ss * t0;
	c = qp * qp - ss * t0 * t0;
	d = b * b - a * c;
	d < 0.0 ? -b / a : (-b + sign * math.sqrt(d)) / a;
};

shot_direction = @(ball, end, left, right, forward, backward) {
	vx = -ball.x;
	if (left) vx = vx - 12 * 12 * 0.0254 * end;
	if (right) vx = vx + 12 * 12 * 0.0254 * end;
	vz = -24 * 12 * 0.0254 * end - ball.z;
	if (forward) vz = vz - 14 * 12 * 0.0254 * end;
	if (backward) vz = vz + 10 * 12 * 0.0254 * end;
	Vector3(vx, 0.0, vz);
};

Ball = Class() :: @{
	$radius = radius = 0.0625;

	$__initialize = @(stage, shadow, body) {
		$stage = stage;
		$position = Vector3(0.0, 0.0, 0.0);
		$velocity = Vector3(0.0, 0.0, 0.0);
		$spin = Vector3(0.0, 0.0, 0.0);
		$node = collada.Node().create();
		$translate = collada.Translate(0.0, 0.0, 0.0);
		$node.transforms.push($translate);
		shadow = node(circle(8), shadow);
		shadow.transforms.push(collada.Translate(0.0, 1.0 / 64.0, 0.0));
		shadow.transforms.push(collada.Scale(radius, 1.0, radius));
		$node.nodes.push(shadow);
		body = node(sphere(2), body);
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
	$emit_bounce = @{
		$stage.ball_bounce();
	};
	$wall = @{
		if ($done) return;
		if ($in)
			$emit_ace();
		else
			$emit_out();
	};
	$step = @{
		last = $position;
		$position = last + $velocity;
		$position.y = $position.y - 0.5 * G;
		$velocity.y = $velocity.y - G;
		if ($position.y - radius <= 0.0) {
			$position.y = radius;
			$velocity = $bounced_velocity($velocity);
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
	$reset = @(side) {
		$done = false;
		x0 = 1 * 0.0254 * side;
		x1 = -(13 * 12 + 6) * 0.0254 * side;
		$set(null, '(x0 < x1 ? x0 : x1, x0 < x1 ? x1 : x0, -21 * 12 * 0.0254));
	};
	$hit = @(hitter) {
		if (!$done) $set(hitter, rally);
	};
	$serving = @() $target !== rally;
	$bounced_velocity = @(v) Vector3(v.x * 0.75, v.y * -0.75, v.z * 0.75);
	$projected_time_for_y = @(y, sign) projected_time_for_y($position.y, $velocity.y, y, sign);
};

Mark = Class() :: @{
	radius = 0.0625;

	$__initialize = @(shadow) {
		$duration = 0.0;
		$stretch = 1.0;
		$node = node(circle(8), shadow);
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
	$step = @{
		if ($duration > 0.0) $duration = $duration - 1.0;
	};
	$mark = @(ball) {
		$duration = 2.0 * 64.0;
		$placement.position.x = ball.position.x;
		$placement.position.z = ball.position.z;
		$placement.toward = Vector3(ball.velocity.x, 0.0, ball.velocity.z);
		$placement.valid = false;
		$stretch = 1.0 + $placement.toward.length() * 8.0;
	};
};

Player = Class() :: @{
	State = @(enter, step, do) {
		o = Object();
		o.enter = enter;
		o.step = step;
		o.do = do;
		o;
	};
	Action = Class() :: @{
		$__initialize = @(scene, skeleton, start, duration, use = @(x) true) {
			$start = start;
			$end = start + duration;
			$iterators = scene.iterators(use);
			$iterators.each(@(key, value) {
				value.rewind(start);
				value.index = value.i;
			});
		};
		$rewind = @{
			time = $start;
			$iterators.each(@(key, value) {
				value.i = value.index;
				value.forward(time);
			});
		};
		$forward = @(time) $iterators.each(@(key, value) value.forward(time));
	};
	Swing = Class(Action) :: @{
		$__initialize = @(scene, skeleton, start, duration, impact, speed) {
			:$^__initialize[$](scene, skeleton, start, duration);
			$impact = start + impact;
			$speed = speed;
			iterators = scene.iterators();
			iterators.each((@(key, value) value.rewind($impact))[$]);
			spot = Object();
			skeleton.render(null, Matrix4(), [{"Spot": spot}]);
			$spot = spot.vertex;
			iterators.each((@(key, value) value.rewind($end))[$]);
			root = Object();
			legl = Object();
			legr = Object();
			skeleton.render(null, Matrix4(), [{"Root": root, "Leg0_L": legl, "Leg0_R": legr}]);
			$end_position = Vector3(root.vertex.v[3], 0.0, root.vertex.v[11]);
			$end_toward = Vector3(legr.vertex.v[11] - legl.vertex.v[11], 0.0, legl.vertex.v[3] - legr.vertex.v[3]);
		};
		$merge = @(player) {
			placement = player.placement;
			placement.position = placement.validate() * $end_position;
			placement.v[3] = placement.v[7] = placement.v[11] = 0.0;
			placement.toward = placement * $end_toward;
			placement.valid = false;
		};
	};
	Motion = Class() :: @{
		$__initialize = @(swing) {
			$swing = swing;
			$end = swing.end;
			$rewind();
		};
		$rewind = @() {
			$time = $swing.start;
			$swing.rewind();
		};
		$__call = @{
			$swing.forward($time);
			if ($time < $end) $time = $time + 1.0 / 50.0;
		};
	};
	$Motion = Motion;

	lowers = {
		"Root": true,
		"Center": true,
		"Leg0_R": true,
		"Leg1_R": true,
		"Foot_R": true,
		"Toe_R": true,
		"Leg0_L": true,
		"Leg1_L": true,
		"Foot_L": true,
		"Toe_L": true
	};
	is_lower = @(x) lowers.has(x._node.id);
	is_upper = @(x) !is_lower(x);

	$__initialize = @(stage, model) {
		$stage = stage;
		$ball = stage.ball;
		$scene = collada.load((io.Path(system.script) / "../data" / model).__string());
		$scene.build(stage.main.shaders);
		$node = $scene.ids["Armature"];
		$node.transforms.clear();
		$placement = Placement();
		$node.transforms.push($placement);
		zup = Posture();
		zup.toward = Vector3(0.0, 1.0, 0.0);
		zup.upward = Vector3(0.0, 0.0, -1.0);
		$root = $scene.ids["Root"];
		$root.transforms.unshift(zup.validate());
		$actions = Object();
		$actions.serve = Object();
		$actions.serve.set = Action($scene, $root, 4.0 / 50.0, 0.0);
		$actions.serve.toss = Swing($scene, $root, 4.0 / 50.0, 8.0 / 50.0, 0.0, 0.0);
		$actions.serve.swing = Swing($scene, $root, 16.0 / 50.0, 32.0 / 50.0, 10.0 / 50.0, 24.0 / 64.0);
		$actions.ready = Object();
		$actions.ready.default = Action($scene, $root, 52.0 / 50.0, 0.0);
		$actions.ready.forehand = Object();
		$actions.ready.backhand = Object();
		$actions.ready.forehand.stroke = Action($scene, $root, 394.0 / 50.0, 0.0);
		$actions.ready.backhand.stroke = Action($scene, $root, 398.0 / 50.0, 0.0);
		$actions.ready.forehand.volley = Action($scene, $root, 402.0 / 50.0, 0.0);
		$actions.ready.backhand.volley = Action($scene, $root, 406.0 / 50.0, 0.0);
		$actions.ready.forehand.smash = Action($scene, $root, 410.0 / 50.0, 0.0);
		$actions.ready.backhand.smash = Action($scene, $root, 414.0 / 50.0, 0.0);
		$actions.run = Object();
		#$actions.run.lower = Action($scene, $root, 310.0 / 50.0, 32.0 / 50.0, is_lower);
		$actions.run.lower = Action($scene, $root, 346.0 / 50.0, 24.0 / 50.0, is_lower);
		#$actions.run.lower = Action($scene, $root, 374.0 / 50.0, 16.0 / 50.0, is_lower);
		$actions.run.default = Action($scene, $root, 52.0 / 50.0, 0.0, is_upper);
		$actions.run.forehand = Object();
		$actions.run.backhand = Object();
		$actions.run.forehand.lowers = [
			null,
			Action($scene, $root, 502.0 / 50.0, 24.0 / 50.0, is_lower),
			Action($scene, $root, 418.0 / 50.0, 24.0 / 50.0, is_lower),
			null,
			Action($scene, $root, 446.0 / 50.0, 24.0 / 50.0, is_lower),
			Action($scene, $root, 530.0 / 50.0, 24.0 / 50.0, is_lower),
			Action($scene, $root, 446.0 / 50.0, 24.0 / 50.0, is_lower),
			null,
			Action($scene, $root, 474.0 / 50.0, 24.0 / 50.0, is_lower),
			Action($scene, $root, 558.0 / 50.0, 24.0 / 50.0, is_lower),
			Action($scene, $root, 474.0 / 50.0, 24.0 / 50.0, is_lower)
		];
		$actions.run.backhand.lowers = [
			null,
			Action($scene, $root, 418.0 / 50.0, 24.0 / 50.0, is_lower),
			Action($scene, $root, 502.0 / 50.0, 24.0 / 50.0, is_lower),
			null,
			Action($scene, $root, 474.0 / 50.0, 24.0 / 50.0, is_lower),
			Action($scene, $root, 474.0 / 50.0, 24.0 / 50.0, is_lower),
			Action($scene, $root, 558.0 / 50.0, 24.0 / 50.0, is_lower),
			null,
			Action($scene, $root, 446.0 / 50.0, 24.0 / 50.0, is_lower),
			Action($scene, $root, 446.0 / 50.0, 24.0 / 50.0, is_lower),
			Action($scene, $root, 530.0 / 50.0, 24.0 / 50.0, is_lower)
		];
		$actions.run.forehand.stroke = Action($scene, $root, 394.0 / 50.0, 0.0, is_upper);
		$actions.run.backhand.stroke = Action($scene, $root, 398.0 / 50.0, 0.0, is_upper);
		$actions.run.forehand.volley = Action($scene, $root, 402.0 / 50.0, 0.0, is_upper);
		$actions.run.backhand.volley = Action($scene, $root, 406.0 / 50.0, 0.0, is_upper);
		$actions.run.forehand.smash = Action($scene, $root, 410.0 / 50.0, 0.0, is_upper);
		$actions.run.backhand.smash = Action($scene, $root, 414.0 / 50.0, 0.0, is_upper);
		$actions.swing = Object();
		$actions.swing.forehand = Object();
		$actions.swing.backhand = Object();
		$actions.swing.forehand.stroke = Swing($scene, $root, 58.0 / 50.0, 32.0 / 50.0, 8.0 / 50.0, 16.0 / 64.0);
		$actions.swing.backhand.stroke = Swing($scene, $root, 94.0 / 50.0, 32.0 / 50.0, 8.0 / 50.0, 16.0 / 64.0);
		$actions.swing.forehand.volley = Swing($scene, $root, 130.0 / 50.0, 32.0 / 50.0, 8.0 / 50.0, 16.0 / 64.0);
		$actions.swing.backhand.volley = Swing($scene, $root, 166.0 / 50.0, 32.0 / 50.0, 8.0 / 50.0, 16.0 / 64.0);
		$actions.swing.forehand.smash = Swing($scene, $root, 202.0 / 50.0, 32.0 / 50.0, 10.0 / 50.0, 24.0 / 64.0);
		$actions.swing.backhand.smash = Swing($scene, $root, 238.0 / 50.0, 32.0 / 50.0, 10.0 / 50.0, 18.0 / 64.0);
		$actions.swing.toss = Swing($scene, $root, 274.0 / 50.0, 32.0 / 50.0, 8.0 / 50.0, 18.0 / 64.0);
		$actions.swing.toss_lob = Swing($scene, $root, 274.0 / 50.0, 32.0 / 50.0, 8.0 / 50.0, 11.0 / 64.0);
		$motion = null;
		$reset(1.0, $state_default);
	};
	$transit = @(state) {
		$state = state;
		$state.enter[$]();
	};
	$reset = @(end, state) {
		$left = $right = $forward = $backward = false;
		$end = end;
		$transit(state);
	};
	$setup = @{
		$placement.validate();
	};
	$direction = @{
		v = $ball.velocity;
		e = (v.z < 0.0 ? 1.0 : -1.0) * $end;
		v = Vector3(v.x * e, 0.0, v.z * e);
		v.length() > 0.01 / 64.0 ? v : Vector3(0.0, 0.0, -$end);
	};
	$whichhand = @(v) Vector3(-v.z, 0.0, v.x) * ($ball.position - $placement.position);
	$relative_ball = @(swing) {
		$placement.validate();
		p = $ball.position - $placement.position;
		v = $placement.toward;
		x = v.z * p.x - v.x * p.z - swing.spot[3];
		y = p.y - swing.spot[7];
		z = v.x * p.x + v.z * p.z - swing.spot[11];
		swing.spot[1] > 0.0 ? Vector3(-x, y, -z) : Vector3(x, y, z);
	};
	$step = @() $state.step[$]();
	$do = @() $state.do[$]();
	$shot_direction = @() $ball.position.z * $end < 0.0 ? Vector3(0.0, 0.0, -$end) : shot_direction($ball.position, $end, $left, $right, $forward, $backward);
	$speed = 4.0 / 64.0;
	$smash_height = 2.25;
	$smash_hand = -0.25;
	$state_default = State(@{
		$motion = Motion($actions.ready.default);
	}, @{
		d = Vector3(0.0, 0.0, 0.0);
		if ($left) d.x = -$speed * $end;
		if ($right) d.x = $speed * $end;
		if ($forward) d.z = -$speed * $end;
		if ($backward) d.z = $speed * $end;
		actions = d.x == 0.0 && d.z == 0.0 ? $actions.ready : $actions.run;
		if ($ball.hitter === null || $ball.hitter.end == $end) {
			v = $ball.position - $placement.position;
			v.y = 0.0;
			v.normalize();
			hand = null;
			action = actions.default;
		} else {
			v = $direction();
			v.normalize();
			whichhand = $whichhand(v);
			t = reach_range($ball.position, $ball.velocity, $placement.position, 0.0, 0.0, 1.0);
			y = $ball.position.y + ($ball.velocity.y - 0.5 * G * t) * t;
			if (y > $smash_height) {
				hand = whichhand > $smash_hand ? actions.forehand : actions.backhand;
				action = hand.smash;
			} else {
				hand = whichhand > 0.0 ? actions.forehand : actions.backhand;
				if ($ball.done)
					action = $placement.position.z * $end > 21 * 12 * 0.0254 ? hand.stroke : hand.volley;
				else
					action = $ball.in || y < 0.0 ? hand.stroke : hand.volley;
			}
		}
		if (d.x == 0.0 && d.z == 0.0) {
			$motion = Motion(action);
			d = v;
		} else {
			run = hand === null ? $actions.run.lower : hand.lowers[($left ? 1 : $right ? 2 : 0) + ($forward ? 4 : $backward ? 8 : 0)];
			if ($motion.swing !== run) $motion = Motion(run);
			if ($motion.time >= run.end) $motion.rewind();
			action.rewind();
			$placement.position = $placement.position + d;
		}
		$placement.toward = d;
		$placement.valid = false;
		$motion();
	}, @{
		$placement.toward = $shot_direction();
		$placement.valid = false;
		actions = $actions.swing;
		whichhand = $whichhand($direction().normalized());
		t = $ball.projected_time_for_y($smash_height, 1.0);
		if (t !== null) {
			swing = whichhand > $smash_hand ? actions.forehand.smash : actions.backhand.smash;
			if (t > (swing.impact - swing.start) * 50.0) {
				$motion = Motion(swing);
				return $transit($state_smash_swing);
			}
		}
		t = $ball.in ? 0.0 : $ball.projected_time_for_y(Ball.radius, 1.0);
		hand = whichhand > 0.0 ? actions.forehand : actions.backhand;
		$motion = Motion(t < (hand.volley.impact - hand.volley.start) * 50.0 ? hand.stroke : hand.volley);
		$transit($state_swing);
	});
	$state_serve_set = State(@{
		$motion = Motion($actions.serve.set);
	}, @{
		speed = 2.0 / 64.0;
		if ($left) $ball.position.x = $ball.position.x - speed;
		if ($right) $ball.position.x = $ball.position.x + speed;
		$ball.position.y = 0.875;
		$ball.velocity = Vector3(0.0, 0.0, 0.0);
		$placement.position = Vector3($ball.position.x, 0.0, $ball.position.z);
		$placement.toward = Vector3((6 * 12 + 9) * -0.0254 * $end * $stage.side, 0.0, 21 * 12 * -0.0254 * $end) - $placement.position;
		$placement.valid = false;
		$motion();
	}, @{
		$transit($state_serve_toss);
	});
	$state_serve_toss = State(@{
		$ball.position.y = 1.5;
		toward = $placement.toward;
		left = Vector3(toward.z, 0.0, -toward.x);
		$ball.velocity = left * 0.0075 + toward * 0.01;
		$ball.velocity.y = 0.085;
		$motion = Motion($actions.serve.toss);
	}, @{
		if ($ball.position.y <= 1.5) {
			$ball.position.x = $placement.position.x;
			$ball.position.z = $placement.position.z;
			$transit($state_serve_set);
		}
		if ($left) $placement.toward.x = $placement.toward.x - 1.0 / 64.0 * $end;
		if ($right) $placement.toward.x = $placement.toward.x + 1.0 / 64.0 * $end;
		$placement.valid = false;
		$motion();
	}, @{
		$transit($state_serve_swing);
	});
	$state_serve_swing = State(@{
		$motion = Motion($actions.serve.swing);
		$stage.sound_swing.play();
	}, @{
		if (math.fabs($motion.time - $motion.swing.impact) < 0.5 / 50.0) {
			ball = $relative_ball($motion.swing);
			if (math.fabs(ball.y) < 0.3) {
				d = 58 * 12 * 0.0254 + ball.y * 12.0;
				speed = $motion.swing.speed + ball.y * 0.125;
				$ball.velocity = Vector3($placement.toward.x * speed, G * d / (2.0 * speed) - $ball.position.y * speed / d, $placement.toward.z * speed);
				$ball.hitter = $;
				$stage.sound_hit.play();
			}
		}
		$motion();
		if ($motion.time < $motion.end) return;
		$motion.swing.merge($);
		$transit($state_default);
	}, @{
	});
	$state_swing = State(@{
		$stage.sound_swing.play();
	}, @{
		v = $shot_direction();
		if ($motion.time <= $motion.swing.impact) {
			$placement.toward = v * 1.0;
			$placement.valid = false;
		}
		if (math.fabs($motion.time - $motion.swing.impact) < 0.5 / 50.0) {
			ball = $relative_ball($motion.swing);
			print("x: " + ball.x + ", y: " + ball.y + ", z: " + ball.z);
			if (math.fabs(ball.x) < 0.5 && math.fabs(ball.z) < 1.0) {
				d = v.length();
				n = -d * $ball.position.z / v.z;
				b = $ball.position.y * (d - n) / d;
				speed = $motion.swing.speed;
				if (d > 60 * 12 * 0.0254) {
					a = d / (60 * 12 * 0.0254);
					speed = speed * (a < 1.25 ? a : 1.25);
				}
				if (b < 42 * 0.0254) {
					vm = math.sqrt(G * (d - n) * n * 0.5 / (42 * 0.0254 - b));
					if (vm < speed) speed = vm;
				}
				d = d - ball.x * 2.0;
				speed = speed - ball.x * 0.125;
				dx = v.x + v.z * ball.z * 0.0625;
				dz = v.z - v.x * ball.z * 0.0625;
				a = speed / math.sqrt(dx * dx + dz * dz);
				$ball.velocity = Vector3(dx * a, G * d / (2.0 * speed) - $ball.position.y * speed / d, dz * a);
				$ball.hit($);
				$stage.sound_hit.play();
			}
		}
		$motion();
		if ($motion.time < $motion.end) return;
		$motion.swing.merge($);
		$transit($state_default);
	}, @{
	});
	$state_smash_swing = State(@{
		$stage.sound_swing.play();
	}, @{
		v = $shot_direction();
		if ($motion.time <= $motion.swing.impact) {
			$placement.toward = v * 1.0;
			$placement.valid = false;
		}
		if (math.fabs($motion.time - $motion.swing.impact) < 0.5 / 50.0) {
			ball = $relative_ball($motion.swing);
			print("x: " + ball.x + ", y: " + ball.y + ", z: " + ball.z);
			if (math.fabs(ball.x) < 0.5 && math.fabs(ball.y) < 0.5 && math.fabs(ball.z) < 1.0) {
				d = v.length() + (ball.y - ball.z) * 2.0;
				speed = $motion.swing.speed + ball.y * 0.125;
				dx = v.x + v.z * ball.x * 0.0625;
				dz = v.z - v.x * ball.x * 0.0625;
				a = speed / math.sqrt(dx * dx + dz * dz);
				$ball.velocity = Vector3(dx * a, G * d / (2.0 * speed) - $ball.position.y * speed / d, dz * a);
				$ball.hit($);
				$stage.sound_hit.play();
			}
		}
		$motion();
		if ($motion.time < $motion.end) return;
		$motion.swing.merge($);
		$transit($state_default);
	}, @{
	});
};

Stage = Class() :: @{
	$State = @(step, key_press, key_release) {
		o = Object();
		o.step = step;
		o.key_press = key_press;
		o.key_release = key_release;
		o;
	};

	$shader = @(name) {
		material = $scene.ids[name];
		material.build($scene.resolve, $main.shaders);
		shader = Object();
		shader.model = material.model();
		shader.shader = shader.model.mesh_shader($main.shaders);
		shader.bind_vertex_inputs = {};
		shader;
	};
	$ball_bounce = @() $sound_bounce.play();
	$ball_in = @() $mark.mark($ball);
	$ball_net = @() $sound_net.play();
	$ball_chip = @() $sound_chip.play();
	$ball_miss = @{
		if ($ball.serving())
			$serve_miss();
		else
			$miss("MISS");
	};
	$ball_out = @{
		$mark.mark($ball);
		if ($ball.serving())
			$serve_miss();
		else
			$miss("OUT");
	};
	$step_things = @{
		$ball.step();
		$mark.step();
		$player0.step();
		$player1.step();
		target = $ball.position * 0.25;
		v = $player0.root.transforms[1].v;
		position = $player0.placement.validate() * ($player0.root.transforms[0] * Vector3(v[3], v[7], v[11]));
		$camera0.position.x = target.x + position.x * 0.5;
		$camera0.position.z = 48.0 * $player0.end + target.z;
		v = $player1.root.transforms[1].v;
		position = $player1.placement.validate() * ($player1.root.transforms[0] * Vector3(v[3], v[7], v[11]));
		$camera1.position.x = target.x + position.x * 0.5;
		$camera1.position.z = 48.0 * $player1.end + target.z;
	};

	$__initialize = @(main, dual, controller0, controller1) {
		$main = main;
		$dual = dual;
		$text_viewing = Matrix4().scale(0.25, 0.25, 1.0);
		$scene = collada.load((io.Path(system.script) / "../data/court.dae").__string());
		$scene.build(main.shaders);
		$sound_bounce = main.load_sound("data/bounce.wav");
		$sound_net = main.load_sound("data/net.wav");
		$sound_chip = main.load_sound("data/chip.wav");
		$sound_hit = main.load_sound("data/hit.wav");
		$sound_swing = main.load_sound("data/swing.wav");
		$sound_ace = main.load_sound("data/ace.wav");
		$sound_miss = main.load_sound("data/miss.wav");
		$camera0 = Placement();
		$camera1 = Placement();
		$ball = Ball($, $shader("Material-Shadow"), $shader("Material-Ball"));
		$scene.scene.instance_visual_scene._scene.nodes.push($ball.node);
		$mark = Mark($shader("Material-Shadow"));
		$scene.scene.instance_visual_scene._scene.nodes.push($mark.node);
		$player0 = Player($, "male.dae");
		$scene.scene.instance_visual_scene._scene.nodes.push($player0.node);
		$player0.scene.scene.instance_visual_scene._scene._controllers.each($scene.scene.instance_visual_scene._scene._controllers.push);
		$player1 = Player($, "male.dae");
		$scene.scene.instance_visual_scene._scene.nodes.push($player1.node);
		$player1.scene.scene.instance_visual_scene._scene._controllers.each($scene.scene.instance_visual_scene._scene._controllers.push);
		$player0.opponent = $player1;
		$player1.opponent = $player0;
		$state_ready = $State(@{
			if ($duration <= 0.0) return $transit_play();
			$duration = $duration - 1.0;
		}, {
			xraft.Key.RETURN: @() $transit_play(),
			xraft.Key.ESCAPE: @() $transit_back()
		}, {
		});
		$state_play = $State(@{
			$step_things();
			if (!$ball.done) return;
			if ($duration <= 0.0) return $next();
			$duration = $duration - 1.0;
		}, {
			xraft.Key.RETURN: @() $next(),
			xraft.Key.ESCAPE: @() $transit_back()
		}, {});
		controller0[$]($state_play, $player0);
		controller1[$]($state_play, $player1);
	};
	$destroy = @{
		$scene.destroy();
		$player0.scene.destroy();
		$player1.scene.destroy();
		$sound_bounce.delete();
		$sound_net.delete();
		$sound_chip.delete();
		$sound_hit.delete();
		$sound_swing.delete();
		$sound_ace.delete();
		$sound_miss.delete();
	};
	$step = @() $state.step[$]();
	$render = @{
		extent = $main.geometry();
		w = extent.width();
		h = extent.height();
		gl.viewport(0, 0, w, h);
t0 = time.now();
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
if (print_time) print("\tclear: " + (time.now() - t0));
		$ball.setup();
		$mark.setup();
		$player0.setup();
		$player1.setup();
		gl.enable(gl.DEPTH_TEST);
		if ($dual) gl.viewport(0, 0, w / 2, h);
		pw = extent.width() * ($dual ? 0.5 : 1.0) / extent.height();
		ph = 1.0;
		projection = Matrix4().frustum(-pw, pw, -ph, ph, 10.0, 200.0).bytes;
		$scene.render(projection, $camera0.viewing());
		if ($dual) {
			gl.viewport(w / 2, 0, w - w / 2, h);
			$scene.render(projection, $camera1.viewing());
			gl.viewport(0, 0, w, h);
		}
t0 = time.now();
		gl.disable(gl.DEPTH_TEST);
		y = $message.size() * 0.5 - 1.0;
		$message.each(@(line) {
			viewing = Matrix4($text_viewing).translate(line.size() * -0.25, y, 0.0).bytes;
			$main.text_renderer($main.text_projection, viewing, line);
			:y = y - 1.0;
		}[$]);
if (print_time) print("\ttext: " + (time.now() - t0));
	};
	$key_press = @(modifier, key, ascii) {
		if ($state.key_press.has(key)) $state.key_press[key][$]();
	};
	$key_release = @(modifier, key, ascii) {
		if ($state.key_release.has(key)) $state.key_release[key][$]();
	};
};

MainMenu = null;

Match = Class(Stage) :: @{
	$new_game = @{
		$player0.point = $player1.point = 0;
		$second = false;
		games = $player0.game + $player1.game;
		$end = games % 4 > 2 ? -1.0 : 1.0;
		$server = games % 2 == 0 ? $player0 : $player1;
		$receiver = games % 2 == 0 ? $player1 : $player0;
	};
	$point = @(player) {
		player.point = player.point + 1;
		if (player.point < 4 || player.point - player.opponent.point < 2) return;
		player.game = player.game + 1;
		if (player.game < 6 || player.game - player.opponent.game < 2)
			$new_game();
		else
			$closed = true;
	};
	$ball_ace = @{
		$second = false;
		$point($ball.hitter);
		$duration = 2.0 * 64.0;
		$sound_ace.play();
	};
	$ball_let = @{
		$mark.mark($ball);
		$message = ["LET"];
		$duration = 2.0 * 64.0;
	};
	$serve_miss = @{
		if ($second) {
			$message = ["DOUBLE FAULT"];
			$second = false;
			$point($receiver);
		} else {
			$message = ["FAULT"];
			$second = true;
		}
		$duration = 2.0 * 64.0;
		$sound_miss.play();
	};
	$ball_serve_air = $serve_miss;
	$miss = @(message) {
		$message = [message];
		$second = false;
		$point($ball.hitter.opponent);
		$duration = 2.0 * 64.0;
		$sound_miss.play();
	};
	points0 = [" 0", "15", "30", "40"];
	points1 = ["0 ", "15", "30", "40"];
	$transit_ready = @{
		$state = $state_ready;
		if ($player0.point + $player1.point < 6)
			game = points0[$player0.point] + " - " + points1[$player1.point];
		else if ($player0.point > $player1.point)
			game = " A -   ";
		else if ($player0.point < $player1.point)
			game = "   - A ";
		else
			game = " DEUCE ";
		$message = [
			"P1 " + $player0.game + " - " + $player1.game + " P2",
			($player0 === $server ? "* " : "  ") + game + ($player1 === $server ? " *" : "  ")
		];
		$duration = 1.0 * 64.0;
		$step_things();
	};
	state_close = $State(@{
		$step_things();
	}, {
		xraft.Key.RETURN: @() $transit_back(),
		xraft.Key.ESCAPE: @() $transit_back()
	}, {
	});
	$transit_close = @{
		$state = state_close;
		$message = [
			($player0.game > $player1.game ? "P1" : "P2") + " WON!",
			"P1 " + $player0.game + " - " + $player1.game + " P2"
		];
		$sound_ace.play();
	};
	$transit_play = @{
		$state = $state_play;
		$message = [];
	};
	$transit_back = @() $main.screen__(MainMenu($main));
	$reset = @{
		$side = ($player0.point + $player1.point) % 2 == 0 ? 1.0 : -1.0;
		$ball.position = Vector3(2 * 12 * 0.0254 * $end * $side, 0.875, 39 * 12 * 0.0254 * $end);
		$ball.velocity = Vector3(0.0, 0.0, 0.0);
		$ball.reset($side);
		$mark.duration = 0.0;
		$server.reset($end, Player.state_serve_set);
		$receiver.placement.position = Vector3(-9 * 12 * 0.0254 * $end * $side, 0.0, -39 * 12 * 0.0254 * $end);
		$receiver.placement.valid = false;
		$receiver.reset(-$end, Player.state_default);
		$camera0.position = Vector3(0.0, 14.0, 0.0);
		$camera0.toward = Vector3(0.0, -12.0, -40.0 * $player0.end);
		$camera1.position = Vector3(0.0, 14.0, 0.0);
		$camera1.toward = Vector3(0.0, -12.0, -40.0 * $player1.end);
	};

	$__initialize = @(main, dual, controller0, controller1) {
		:$^__initialize[$](main, dual, controller0, controller1);
		$closed = false;
		$player0.game = $player1.game = 0;
		$new_game();
		$reset();
		$transit_ready();
	};
	$next = @{
		if (!$ball.done) return;
		if ($second || $ball.serving() && $ball.in && $ball.net) {
			$reset();
			$message = [];
			$step_things();
		} else if ($closed) {
			$transit_close();
		} else {
			$reset();
			$transit_ready();
		}
	};
};

TrainingMenu = null;

Training = Class(Stage) :: @{
	$ball_in = @{
		$mark.mark($ball);
		if ($ball.hitter !== $player0) return;
		$text_viewing = Matrix4().scale(0.25, 0.25, 1.0);
		$message = ["IN"];
	};
	$ball_ace = @{
		$duration = 0.5 * 64.0;
	};
	$ball_let = @{
		$mark.mark($ball);
		$text_viewing = Matrix4().scale(0.25, 0.25, 1.0);
		$message = ["LET"];
		$duration = 0.5 * 64.0;
	};
	$serve_miss = @{
		$text_viewing = Matrix4().scale(0.25, 0.25, 1.0);
		$message = ["FAULT"];
		$duration = 0.5 * 64.0;
		$sound_miss.play();
	};
	$ball_serve_air = $serve_miss;
	$miss = @(message) {
		$text_viewing = Matrix4().scale(0.25, 0.25, 1.0);
		$message = [message];
		$duration = 0.5 * 64.0;
		$sound_miss.play();
	};
	$step_things = @{
		:$^step_things[$]();
		$camera1.position = Vector3(($ball.position.x + $player0.placement.position.x) * 0.5, 4.0, ($ball.position.z + 40.0 * $player1.end) * 0.5);
		$camera1.toward = Vector3(0.0, -6.0, -40.0 * $player1.end);
	};
	$transit_back = @() $main.screen__(TrainingMenu($main));

	$__initialize = @(main, controller0) {
		:$^__initialize[$](main, true, controller0, @(controller, player) {
			super__step = controller.step[$];
			controller.step = @{
				if (player.state !== Player.state_swing) player.left = player.right = player.forward = player.backward = false;
				super__step();
			};
		});
		{
			xraft.Key.RETURN: @() {
				$side = -$side;
				$transit_ready();
			}
		}.each(@(key, value) {
			$state_ready.key_press[key] = value;
			$state_play.key_press[key] = value;
		}[$]);
		$side = 1.0;
		$transit_ready();
	};
	$next = @{
		if ($ball.done) $transit_ready();
	};
};

ServeTraining = Class(Training) :: @{
	$transit_ready = @{
		$ball.position = Vector3(2 * 12 * 0.0254 * $side, 0.875, 39 * 12 * 0.0254);
		$ball.velocity = Vector3(0.0, 0.0, 0.0);
		$ball.reset($side);
		$mark.duration = 0.0;
		$player0.reset(1.0, Player.state_serve_set);
		$player1.reset(-1.0, Player.state_default);
		$player1.placement.position = Vector3(-9 * 12 * 0.0254 * $side, 0.0, -39 * 12 * 0.0254);
		$player1.placement.valid = false;
		$camera0.position = Vector3(0.0, 14.0, 0.0);
		$camera0.toward = Vector3(0.0, -12.0, -40.0 * $player0.end);
		$state = $state_ready;
		$text_viewing = Matrix4().translate(0.0, -0.75, 0.0).scale(0.125, 0.125, 1.0);
		$message = [
			"POSITION: < + >",
			"    TOSS: SPACE",
			"  COURCE: < + >",
			"   SWING: SPACE"
		];
		$duration = 0.0 * 64.0;
		$step_things();
	};
	$transit_play = @{
		$state = $state_play;
	};
};

StrokeTraining = Class(Training) :: @{
	$transit_ready = @{
		$ball.position = Vector3(3 * 12 * 0.0254 * $side, 1.0, -39 * 12 * 0.0254);
		$ball.velocity = Vector3(0.0, 0.0, 0.0);
		$ball.reset($side);
		$mark.duration = 0.0;
		$player0.reset(1.0, Player.state_default);
		$player0.placement.position = Vector3((0.0 - 3.2 * $side) * 12 * 0.0254, 0.0, 39 * 12 * 0.0254);
		$player0.placement.valid = false;
		$player1.reset(-1.0, Player.state_default);
		$player1.placement.position = $ball.position - Posture().validate() * $player1.actions.swing.toss.spot * Vector3(0.0, 0.0, 0.0);
		$player1.placement.position.y = 0.0;
		$player1.placement.valid = false;
		$camera0.position = Vector3(0.0, 14.0, 0.0);
		$camera0.toward = Vector3(0.0, -12.0, -40.0 * $player0.end);
		$state = $state_ready;
		$text_viewing = Matrix4().translate(0.0, -0.875, 0.0).scale(0.125, 0.125, 1.0);
		$message = [
			"      POSITION: +        ",
			"COURCE & SWING: + & SPACE"
		];
		$duration = 0.5 * 64.0;
		$step_things();
	};
	$transit_play = @{
		$state = $state_play;
		$player1.placement.toward = $player1.shot_direction();
		$player1.placement.valid = false;
		$player1.motion = Player.Motion($player1.actions.swing.toss);
		$player1.transit(Player.state_swing);
	};
};

VolleyTraining = Class(Training) :: @{
	$transit_ready = @{
		$ball.position = Vector3(3 * 12 * 0.0254 * $side, 1.0, -39 * 12 * 0.0254);
		$ball.velocity = Vector3(0.0, 0.0, 0.0);
		$ball.reset($side);
		$mark.duration = 0.0;
		$player0.reset(1.0, Player.state_default);
		$player0.placement.position = Vector3((0.1 - 2.0 * $side) * 12 * 0.0254, 0.0, 13 * 12 * 0.0254);
		$player0.placement.valid = false;
		$player1.reset(-1.0, Player.state_default);
		$player1.placement.position = $ball.position - Posture().validate() * $player1.actions.swing.toss.spot * Vector3(0.0, 0.0, 0.0);
		$player1.placement.position.y = 0.0;
		$player1.placement.valid = false;
		$camera0.position = Vector3(0.0, 14.0, 0.0);
		$camera0.toward = Vector3(0.0, -12.0, -40.0 * $player0.end);
		$state = $state_ready;
		$text_viewing = Matrix4().translate(0.0, -0.875, 0.0).scale(0.125, 0.125, 1.0);
		$message = [
			"      POSITION: +        ",
			"COURCE & SWING: + & SPACE"
		];
		$duration = 0.5 * 64.0;
		$step_things();
	};
	$transit_play = @{
		$state = $state_play;
		$player1.placement.toward = $player1.shot_direction();
		$player1.placement.valid = false;
		$player1.motion = Player.Motion($player1.actions.swing.toss);
		$player1.transit(Player.state_swing);
	};
};

SmashTraining = Class(Training) :: @{
	$transit_ready = @{
		$ball.position = Vector3(3 * 12 * 0.0254 * $side, 1.0, -39 * 12 * 0.0254);
		$ball.velocity = Vector3(0.0, 0.0, 0.0);
		$ball.reset($side);
		$mark.duration = 0.0;
		$player0.reset(1.0, Player.state_default);
		$player0.placement.position = Vector3((0.8 - 0.8 * $side) * 12 * 0.0254, 0.0, 13 * 12 * 0.0254);
		$player0.placement.valid = false;
		$player1.reset(-1.0, Player.state_default);
		$player1.placement.position = $ball.position - Posture().validate() * $player1.actions.swing.toss_lob.spot * Vector3(0.0, 0.0, 0.0);
		$player1.placement.position.y = 0.0;
		$player1.placement.valid = false;
		$camera0.position = Vector3(0.0, 14.0, 0.0);
		$camera0.toward = Vector3(0.0, -12.0, -40.0 * $player0.end);
		$state = $state_ready;
		$text_viewing = Matrix4().translate(0.0, -0.875, 0.0).scale(0.125, 0.125, 1.0);
		$message = [
			"      POSITION: +        ",
			"COURCE & SWING: + & SPACE"
		];
		$duration = 0.5 * 64.0;
		$step_things();
	};
	$transit_play = @{
		$state = $state_play;
		$player1.placement.toward = $player1.shot_direction();
		$player1.placement.valid = false;
		$player1.motion = Player.Motion($player1.actions.swing.toss_lob);
		$player1.transit(Player.state_swing);
	};
};

controller0 = @(controller, player) {
	{
		xraft.Key.SPACE: @() player.do(),
		xraft.Key.D2: @() player.do(),
		xraft.Key.LEFT: @() player.left = true,
		xraft.Key.RIGHT: @() player.right = true,
		xraft.Key.UP: @() player.forward = true,
		xraft.Key.DOWN: @() player.backward = true,
		xraft.Key.S: @() player.left = true,
		xraft.Key.F: @() player.right = true,
		xraft.Key.E: @() player.forward = true,
		xraft.Key.C: @() player.backward = true
	}.each(@(key, value) controller.key_press[key] = value);
	{
		xraft.Key.LEFT: @() player.left = false,
		xraft.Key.RIGHT: @() player.right = false,
		xraft.Key.UP: @() player.forward = false,
		xraft.Key.DOWN: @() player.backward = false,
		xraft.Key.S: @() player.left = false,
		xraft.Key.F: @() player.right = false,
		xraft.Key.E: @() player.forward = false,
		xraft.Key.C: @() player.backward = false
	}.each(@(key, value) controller.key_release[key] = value);
};

controller1 = @(controller, player) {
	{
		xraft.Key.D0: @() player.do(),
		xraft.Key.J: @() player.left = true,
		xraft.Key.L: @() player.right = true,
		xraft.Key.I: @() player.forward = true,
		xraft.Key.M: @() player.backward = true
	}.each(@(key, value) controller.key_press[key] = value);
	{
		xraft.Key.J: @() player.left = false,
		xraft.Key.L: @() player.right = false,
		xraft.Key.I: @() player.forward = false,
		xraft.Key.M: @() player.backward = false
	}.each(@(key, value) controller.key_release[key] = value);
};

computer = @(controller, player) {
	ball = player.stage.ball;
	duration = 1.0 * 64.0;
	decided0 = decided1 = left = right = forward = backward = false;
	net = false;
	reset_decision = @() :decided0 = :decided1 = :left = :right = :forward = :backward = false;
	reset_move = @() player.left = player.right = player.forward = player.backward = false;
	super__step = controller.step[$];
	controller.step = @{
		if (ball.done) {
			reset_move();
			reset_decision();
			:net = false;
			return super__step();
		} else if (ball.hitter === null) {
			if (player === ball.stage.server) {
				if (player.state === Player.state_serve_set) {
					reset_move();
					if (duration <= 0.0) {
						player.do();
						:duration = 1.0 * 64.0;
					} else {
						:duration = duration - 1.0;
					}
				} else if (player.state === Player.state_serve_toss) {
					swing = player.actions.serve.swing;
					t = ball.projected_time_for_y(swing.spot[7], 1.0);
					if (t < (swing.impact - swing.start) * 50.0 + (ball.stage.second ? 0.0 : 1.0)) {
						i = Integer(24.0 * 60.0 * 60.0 * math.modf(time.now())[0]);
						:net = i % 10 > (ball.stage.second ? 7 : 3);
						player.do();
					} else if (t < (swing.impact - swing.start) * 50.0 + 8.0) {
						if (!player.left && !player.right) {
							i = Integer(24.0 * 60.0 * 60.0 * math.modf(time.now())[0]);
							if (i % 8 < (ball.stage.second ? 1 : 2))
								player.left = true;
							else if (i % 8 > (ball.stage.second ? 6 : 3))
								player.right = true;
						}
					}
				}
			} else {
				reset_move();
			}
		} else if (ball.hitter.end == player.end) {
			if (!decided0) {
				:decided0 = true;
				if (!net) {
					i = Integer(24.0 * 60.0 * 60.0 * math.modf(time.now())[0]);
					:net = i % 10 > 6;
				}
			}
			reset_move();
			point = Vector3(ball.position.x, 0.0, ball.position.z);
			side0 = Vector3(-(13 * 12 + 6) * 0.0254, 0.0, 21 * 12 * 0.0254 * player.end);
			side1 = Vector3((13 * 12 + 6) * 0.0254, 0.0, 21 * 12 * 0.0254 * player.end);
			v = (side0 - point).normalized() + (side1 - point).normalized();
			a = Vector3(-v.z, 0.0, v.x) * (player.placement.position - point);
			epsilon = 1.0 / 4.0;
			if (a < -epsilon)
				player.left = true;
			else if (a > epsilon)
				player.right = true;
			z = player.placement.position.z * player.end;
			zt = net ? 21 * 12 * 0.0254 : 39 * 12 * 0.0254;
			if (z < zt - epsilon)
				player.backward = true;
			else if (z > zt + epsilon)
				player.forward = true;
		} else {
			if (!decided1) {
				:decided1 = true;
				i = Integer(24.0 * 60.0 * 60.0 * math.modf(time.now())[0]);
				if (i % 3 == 1)
					:left = true;
				else if (i % 3 == 2)
					:right = true;
				if (i % 10 > 5)
					:forward = true;
				else if (i % 10 == 0)
					:backward = true;
			}
			position = ball.position;
			velocity = ball.velocity;
			v = player.direction();
			v.normalize();
			whichhand = player.whichhand(v);
			actions = player.actions.swing;
			t = projected_time_for_y(position.y, velocity.y, Player.smash_height, 1.0);
			if (t !== null) {
				d = (Vector3(position.x + velocity.x * t, 0.0, position.z + velocity.z * t) - player.placement.position).length();
				if (d < 0.5 || d / Player.speed <= t) {
					hand = whichhand > Player.smash_hand ? actions.forehand : actions.backhand;
					swing = hand.smash;
					ix = swing.spot[3];
					iz = swing.spot[11];
					t0 = 0.0;
					t = projected_time_for_y(position.y, velocity.y, swing.spot[7], 1.0);
					if (t === null) t = velocity.y / G;
					t = t - 4.0;
				}
			}
			if (swing === null) {
				hand = whichhand > 0.0 ? actions.forehand : actions.backhand;
				swing = net && !ball.in ? hand.volley : hand.stroke;
				ix = swing.spot[3];
				iz = swing.spot[11];
				if (net || ball.in) {
					t0 = 0.0;
				} else {
					t0 = math.ceil(ball.projected_time_for_y(Ball.radius, 1.0));
					position = Vector3(position.x + velocity.x * t0, Ball.radius, position.z + velocity.z * t0);
					velocity = ball.bounced_velocity(Vector3(velocity.x, velocity.y - G * t0, velocity.z));
				}
				if (net && !ball.in) {
					point = player.placement.position - Vector3(-v.z, 0.0, v.x) * ix + v * iz;
					t = reach_range(position, velocity, point, Player.speed, 0.0, -1.0) + 1.0;
				} else {
					t = projected_time_for_y(position.y, velocity.y, 1.25, -1.0);
					if (t === null) t = velocity.y / G;
				}
			}
			point = Vector3(position.x + velocity.x * t, 0.0, position.z + velocity.z * t);
			v = shot_direction(point, player.end, left, right, forward, backward);
			v.normalize();
			point = player.placement.position - Vector3(-v.z, 0.0, v.x) * ix + v * iz;
			tt = t0 + t;
			t1 = t0 + reach_range(position, velocity, point, Player.speed, t0, 1.0);
			if (t1 >= 0.0 && t1 < tt) tt = t1;
			if (tt < (swing.impact - swing.start) * 50.0 + 1.0) {
				reset_move();
				player.left = left;
				player.right = right;
				player.forward = forward;
				player.backward = backward;
				reset_decision();
				:net = player.placement.position.z * player.end < 21 * 12 * 0.0254;
				player.do();
			} else if (player.state !== Player.state_default) {
			} else {
				reset_move();
				point = Vector3(position.x + velocity.x * t, 0.0, position.z + velocity.z * t);
				v = shot_direction(point, player.end, left, right, forward, backward);
				v.normalize();
				target = point + Vector3(-v.z, 0.0, v.x) * ix - v * iz;
				epsilon = 1.0 / 32.0;
				if (player.placement.position.x * player.end < target.x * player.end - epsilon)
					player.right = true;
				else if (player.placement.position.x * player.end > target.x * player.end + epsilon)
					player.left = true;
				if (player.placement.position.z * player.end < target.z * player.end - epsilon)
					player.backward = true;
				else if (player.placement.position.z * player.end > target.z * player.end + epsilon)
					player.forward = true;
			}
		}
		super__step();
	};
};

Menu = Class() :: @{
	$Item = @(text, do) {
		o = Object();
		o.text = text;
		o.do = do;
		o;
	};

	$__initialize = @(main) {
		$main = main;
		$selected = 0;
		$duration = 0.0;
		$transit = null;
	};
	$step = @{
		if ($transit === null) return;
		if ($duration <= 0.0) return $transit[$]();
		$duration = $duration - 1.0;
	};
	$render = @{
		extent = $main.geometry();
		w = extent.width();
		h = extent.height();
		gl.viewport(0, 0, w, h);
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
		gl.disable(gl.DEPTH_TEST);
		a = w / (h * $background.unit);
		a = 2.0 * (a < 1.0 ? 1.0 : a);
		viewing = Matrix4().scale(a, a, 1.0).translate(-0.5 * $background.unit, -0.5, 0.0).bytes;
		$background($main.text_projection, viewing, String.from_code(0));
		viewing = Matrix4().translate(0.0, 0.375, 0.0).scale(0.5, 0.5, 1.0).translate($title.size() * -0.25, 0.0, 0.0).bytes;
		$main.text_renderer($main.text_projection, viewing, $title);
		viewing = Matrix4().translate(0.0, -0.125, 0.0).scale(0.25, 0.25, 1.0);
		i = 0;
		y = 1.0;
		$items.each(@(item) {
			text = item.text(i == $selected);
			viewing = Matrix4(:viewing).translate(text.size() * -0.25, y, 0.0).bytes;
			$main.text_renderer($main.text_projection, viewing, text);
			:y = y - 1.0;
			:i = i + 1.0;
		}[$]);
	};
	$up = @{
		if ($selected <= 0) return;
		$selected = $selected - 1;
		$main.sound_cursor.play();
	};
	$down = @{
		if ($selected >= $items.size() - 1) return;
		$selected = $selected + 1;
		$main.sound_cursor.play();
	};
	$select = @{
		$main.sound_select.play();
		$items[$selected].do[$]();
	};
	key_press = {
		xraft.Key.ESCAPE: @() $transit_back(),
		xraft.Key.SPACE: $select,
		xraft.Key.D2: $select,
		xraft.Key.UP: $up,
		xraft.Key.DOWN: $down,
		xraft.Key.E: $up,
		xraft.Key.C: $down
	};
	$key_press = @(modifier, key, ascii) {
		if ($transit === null && key_press.has(key)) key_press[key][$]();
	};
	$key_release = @(modifier, key, ascii) {
	};
};

TrainingMenu = Class(Menu) :: @{
	$title = "TRAINING";
	$items = '(
		$Item(@(selected) {
			selected ? "* SERVE   " : "  SERVE   ";
		}, @{
			$transit = @() $main.screen__(ServeTraining($main, controller0));
			$duration = 0.0 * 64.0;
		}),
		$Item(@(selected) {
			selected ? "* STROKE  " : "  STROKE  ";
		}, @{
			$transit = @() $main.screen__(StrokeTraining($main, controller0));
			$duration = 0.0 * 64.0;
		}),
		$Item(@(selected) {
			selected ? "* VOLLEY  " : "  VOLLEY  ";
		}, @{
			$transit = @() $main.screen__(VolleyTraining($main, controller0));
			$duration = 0.0 * 64.0;
		}),
		$Item(@(selected) {
			selected ? "* SMASH   " : "  SMASH   ";
		}, @{
			$transit = @() $main.screen__(SmashTraining($main, controller0));
			$duration = 0.0 * 64.0;
		}),
		$Item(@(selected) {
			selected ? "*  BACK   " : "   BACK   ";
		}, @{
			$transit = @() $main.screen__(MainMenu($main));
		})
	);

	$__initialize = @(main) {
		:$^__initialize[$](main);
		$background = glimage.Renderer.from_file((io.Path(system.script) / "../data/main-background.jpg").__string(), 1);
		$sound_background = main.load_sound("data/training-background.wav");
		$sound_background.setb(al.LOOPING, true);
		$sound_background.play();
	};
	$destroy = @{
		$background.destroy();
		$sound_background.delete();
	};
	$transit_back = @() $main.screen__(MainMenu($main));
};

MainMenu = Class(Menu) :: @{
	$title = "TENNIS";
	$items = '(
		$Item(@(selected) {
			selected ? "*  1P vs COM  " : "   1P vs COM  ";
		}, @{
			$transit = @() $main.screen__(Match($main, false, controller0, computer));
			$duration = 0.0 * 64.0;
		}),
		$Item(@(selected) {
			selected ? "*  1P vs 2P   " : "   1P vs 2P   ";
		}, @{
			$transit = @() $main.screen__(Match($main, true, controller0, controller1));
			$duration = 0.0 * 64.0;
		}),
		$Item(@(selected) {
			selected ? "* COM vs COM  " : "  COM vs COM  ";
		}, @{
			$transit = @() $main.screen__(Match($main, false, computer, computer));
			$duration = 0.0 * 64.0;
		}),
		$Item(@(selected) {
			selected ? "*  TRAINING   " : "   TRAINING   ";
		}, @{
			$transit = @() $main.screen__(TrainingMenu($main));
			$duration = 0.0 * 64.0;
		}),
		$Item(@(selected) {
			selected ? "*    EXIT     " : "     EXIT     ";
		}, @{
			xraft.application().exit();
		})
	);

	$__initialize = @(main) {
		:$^__initialize[$](main);
		$background = glimage.Renderer.from_file((io.Path(system.script) / "../data/main-background.jpg").__string(), 1);
		$sound_background = main.load_sound("data/main-background.wav");
		$sound_background.setb(al.LOOPING, true);
		$sound_background.play();
	};
	$destroy = @{
		$background.destroy();
		$sound_background.delete();
	};
	$transit_back = @() xraft.application().exit();
};

Game = Class(xraft.GLWidget) :: @{
	$on_create = @{
		$glcontext.make_current($);
		$shaders = glshaders();
		$text_renderer = glimage.Renderer.from_font();
		gl.enable(gl.CULL_FACE);
		$on_move();
		$screen = MainMenu($);
		$timer.start(20);
	};
	$on_move = @{
		extent = $geometry();
		w = Float(extent.width()) / extent.height();
		$text_projection = Matrix4().orthographic(-w, w, -1.0, 1.0, -1.0, 1.0).bytes;
	};
	$on_paint = @(g) {
t0 = time.now();
		$glcontext.make_current($);
		$screen.render();
if (print_time) print("render: " + (time.now() - t0));
t0 = time.now();
		$glcontext.flush();
if (print_time) print("flush: " + (time.now() - t0));
	};
	$on_key_press = @(modifier, key, ascii) {
		if (key == xraft.Key.Q) return xraft.application().exit();
		$glcontext.make_current($);
		$screen.key_press(modifier, key, ascii);
	};
	$on_key_release = @(modifier, key, ascii) {
		$glcontext.make_current($);
		$screen.key_release(modifier, key, ascii);
	};
	$load_sound = @(name) {
		path = (io.Path(system.script) / ".." / name).__string();
		source = $alcontext.create_source();
		source.set_buffer($alcontext.get_device().create_buffer_from_file(path));
		source;
	};
	$screen__ = @(screen) {
		$screen.destroy();
		$screen = screen;
	};
	$__initialize = @(format, alcontext) {
		:$^__initialize[$](format);
		$glcontext = xraft.GLContext(format);
		$alcontext = alcontext;
		$timer = xraft.Timer(@{
t0 = time.now();
			$screen.step();
			extent = $geometry();
			$invalidate(0, 0, extent.width(), extent.height());
if (print_time) print("step: " + (time.now() - t0));
		}[$]);
		$sound_cursor = $load_sound("data/cursor.wav");
		$sound_select = $load_sound("data/select.wav");
	};
};

Frame = Class(xraft.Frame) :: @{
	$on_move = @{
		extent = $geometry();
		$at(0).move(xraft.Rectangle(0, 0, extent.width(), extent.height()));
	};
	$on_focus_enter = @() xraft.application().focus__($at(0));
	$on_close = @() xraft.application().exit();
	$__initialize = @(alcontext) {
		:$^__initialize[$]();
		$add(Game(xraft.GLFormat(true, true, false, true), alcontext));
	};
};

xraft.main(system.arguments, @(application) {
	cairo.main(@{
		try {
		gl.main(@{
			al.main(@{
				device = al.Device(null);
				context = device.default_context();
				frame = Frame(context);
				frame.caption__("Tennis");
				frame.move(xraft.Rectangle(0, 0, 800, 600));
				application.add(frame);
				frame.show();
				application.run();
			});
		});
		} catch (Throwable e) {
			print(e);
			e.dump();
		}
	});
});
