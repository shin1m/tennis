math = Module("math");
time = Module("time");
glmatrix = Module("glmatrix");
ball = Module("ball");
player = Module("player");

Vector3 = glmatrix.Vector3;
G = ball.G;
projected_time_for_y = ball.projected_time_for_y;
Ball = ball.Ball;
reach_range = player.reach_range;
shot_direction = player.shot_direction;
Player = player.Player;

$__get_at = @(stage) @(controller, player) {
	ball = stage.ball;
	duration = 1.0 * 64.0;
	decided0 = decided1 = left = right = forward = backward = false;
	shot = 'flat;
	net = false;
	reset_decision = @{
		:decided0 = :decided1 = :left = :right = :forward = :backward = false;
		:shot = 'flat;
	};
	reset_move = @() player.left = player.right = player.forward = player.backward = false;
	super__step = controller.step[stage];
	controller.step = @{
		if (ball.done) {
			reset_move();
			reset_decision();
			:net = false;
			return super__step();
		} else if (ball.hitter === null) {
			if (player === stage.server) {
				if (player.state === Player.state_serve_set) {
					reset_move();
					if (duration <= 0.0) {
						player.do('flat);
						:duration = 1.0 * 64.0;
					} else {
						:duration = duration - 1.0;
					}
				} else if (player.state === Player.state_serve_toss) {
					if (stage.second) {
						:shot = 'lob;
					} else {
						i = Integer(24.0 * 60.0 * 60.0 * math.modf(time.now())[0]) % 10;
						if (i > 6)
							:shot = 'topspin;
						else if (i > 4)
							:shot = 'slice;
						else
							:shot = 'flat;
					}
					swing = player.actions.serve.swing.(shot);
					t = ball.projected_time_for_y(swing.spot[7], 1.0);
					if (t < (swing.impact - swing.start) * 50.0 + (stage.second ? 0.0 : 1.0)) {
						i = Integer(24.0 * 60.0 * 60.0 * math.modf(time.now())[0]);
						:net = i % 10 > (stage.second ? 7 : 3);
						player.do(shot);
					} else if (t < (swing.impact - swing.start) * 50.0 + 8.0) {
						if (!player.left && !player.right) {
							i = Integer(24.0 * 60.0 * 60.0 * math.modf(time.now())[0]);
							if (i % 8 < (stage.second ? 1 : 2))
								player.left = true;
							else if (i % 8 > (stage.second ? 6 : 5))
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
			epsilon = 1.0 / 1.0;
			if (a < -epsilon)
				player.left = true;
			else if (a > epsilon)
				player.right = true;
			z = player.placement.position.z * player.end;
			zt = net ? 21 * 12 * 0.0254 : 39 * 12 * 0.0254;
			epsilon = 1.0 / 2.0;
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
				i = Integer(24.0 * 60.0 * 60.0 * math.modf(time.now())[0]) % 10;
				if (i > 6)
					:shot = 'topspin;
				else if (i > 4)
					:shot = 'slice;
				else if (i > 3)
					:shot = 'lob;
				else
					:shot = 'flat;
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
				if (d < 0.5 || d / player.speed <= t) {
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
				swing = net && !ball.in ? hand.volley : hand.stroke.(shot);
				ix = swing.spot[3];
				iz = swing.spot[11];
				if (net || ball.in) {
					t0 = 0.0;
				} else {
					t0 = math.ceil(ball.projected_time_for_y(Ball.radius, 1.0));
					position = Vector3(position.x + velocity.x * t0, Ball.radius, position.z + velocity.z * t0);
					velocity = Vector3(velocity.x, velocity.y - G * t0, velocity.z);
					ball.calculate_bounce(velocity, ball.spin * 1.0);
				}
				if (net && !ball.in) {
					point = player.placement.position - Vector3(-v.z, 0.0, v.x) * ix + v * iz;
					t = reach_range(position, velocity, point, player.speed, 0.0, -1.0) + 1.0;
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
			t1 = t0 + reach_range(position, velocity, point, player.speed, t0, 1.0);
			if (t1 >= 0.0 && t1 < tt) tt = t1;
			if (tt < (swing.impact - swing.start) * 50.0 + 1.0) {
				reset_move();
				player.left = left;
				player.right = right;
				player.forward = forward;
				player.backward = backward;
				player.do(shot);
				reset_decision();
				:net = player.placement.position.z * player.end < 21 * 12 * 0.0254;
			} else if (player.state !== Player.state_default) {
			} else {
				reset_move();
				point = Vector3(position.x + velocity.x * t, 0.0, position.z + velocity.z * t);
				v = shot_direction(point, player.end, left, right, forward, backward);
				v.normalize();
				target = point + Vector3(-v.z, 0.0, v.x) * ix - v * iz;
				epsilon = tt > 32.0 ? 1.0 / 8.0 : 1.0 / 32.0;
				if (player.placement.position.x * player.end < target.x * player.end - epsilon)
					player.right = true;
				else if (player.placement.position.x * player.end > target.x * player.end + epsilon)
					player.left = true;
				epsilon = tt > 32.0 ? 1.0 : 1.0 / 4.0;
				if (player.placement.position.z * player.end < target.z * player.end - epsilon)
					player.backward = true;
				else if (player.placement.position.z * player.end > target.z * player.end + epsilon)
					player.forward = true;
			}
		}
		super__step();
	};
};
