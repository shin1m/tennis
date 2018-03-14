math = Module("math"
time = Module("time"
glmatrix = Module("glmatrix"
ball = Module("ball"
player = Module("player"

Vector3 = glmatrix.Vector3
G = ball.G
projected_time_for_y = ball.projected_time_for_y
Ball = ball.Ball
reach_range = player.reach_range
shot_direction = player.shot_direction
Player = player.Player

random = @ Integer(24.0 * 60.0 * 60.0 * math.modf(time.now())[0]

$__get_at = @(stage) @(controller, player)
	ball = stage.ball
	duration = 64
	decided0 = decided1 = left = right = forward = backward = false
	shot = 'flat
	net = false
	reset_decision = @
		:decided0 = :decided1 = :left = :right = :forward = :backward = false
		:shot = 'flat
	super__step = controller.step[stage]
	controller.step = @
		if ball.done
			player.reset(
			reset_decision(
			:net = false
		else if ball.hitter === null
			if player === stage.server
				if player.state === Player.state_serve_set
					player.reset(
					if duration <= 0
						player.do('flat
						:duration = 64
					else
						:duration = duration - 1
				else if player.state === Player.state_serve_toss
					if stage.second
						:shot = 'lob
					else
						i = random() % 10
						if i > 6
							:shot = 'topspin
						else if i > 4
							:shot = 'slice
						else
							:shot = 'flat
					swing = player.actions.serve.swing.(shot)
					t = ball.projected_time_for_y(swing.spot[13], 1.0
					dt = stage.second ? 0.0 : 1.0
					if random() % 2 == 0
						dt = dt + 1.0
					if t < (swing.impact - swing.start) * 60.0 + dt
						:net = random() % 10 > (stage.second ? 7 : 4)
						player.do(shot
					else if t < (swing.impact - swing.start) * 60.0 + 8.0
						if !player.left && !player.right
							i = random(
							if i % 8 < (stage.second ? 1 : 2)
								player.left = true
							else if i % 8 > (stage.second ? 5 : 4)
								player.right = true
			else
				player.reset(
		else if ball.hitter.end == player.end
			if !decided0
				:decided0 = true
				if !net
					:net = random() % 10 > 6
			player.reset(
			point = Vector3(ball.position.x, 0.0, ball.position.z
			side0 = Vector3(-(13 * 12 + 6) * 0.0254, 0.0, 21 * 12 * 0.0254 * player.end
			side1 = Vector3((13 * 12 + 6) * 0.0254, 0.0, 21 * 12 * 0.0254 * player.end
			v = (side0 - point).normalized() + (side1 - point).normalized()
			a = Vector3(-v.z, 0.0, v.x) * (player.placement.position - point)
			epsilon = 1.0 / 1.0
			if a < -epsilon
				player.left = true
			else if a > epsilon
				player.right = true
			z = player.placement.position.z * player.end
			zt = net ? 21 * 12 * 0.0254 : 39 * 12 * 0.0254
			epsilon = 1.0 / 2.0
			if z < zt - epsilon
				player.backward = true
			else if z > zt + epsilon
				player.forward = true
		else if player.state === Player.state_default
			if !decided1
				:decided1 = true
				i = random(
				if i % 3 == 1
					:left = true
				else if i % 3 == 2
					:right = true
				if i % 10 > 5
					:forward = true
				else if i % 10 == 0
					:backward = true
				i = random() % 10
				if i > 6
					:shot = 'topspin
				else if i > 4
					:shot = 'slice
				else if i > 3
					:shot = 'lob
				else
					:shot = 'flat
			position = ball.position
			velocity = ball.velocity
			if !ball.in
				bound_t = math.ceil(ball.projected_time_for_y(Ball.radius, 1.0
				bound_position = Vector3(position.x + velocity.x * bound_t, Ball.radius, position.z + velocity.z * bound_t
			v = player.direction(
			v.normalize(
			whichhand = player.whichhand(v
			actions = player.actions.swing
			swing = null
			t = projected_time_for_y(position.y, velocity.y, player.smash_height(), 1.0
			if t !== null
				hand = whichhand > player.smash_hand ? actions.forehand : actions.backhand
				smash = hand.smash
				d = (Vector3(position.x + velocity.x * t, 0.0, position.z + velocity.z * t) - player.placement.position).length(
				if d / player.speed + (smash.impact - smash.start) * 60.0 <= t
					swing = smash
					ix = swing.spot[12]
					iz = swing.spot[14]
					t0 = 0.0
					t = projected_time_for_y(position.y, velocity.y, swing.spot[13], 1.0
					if t === null
						t = velocity.y / G
			if swing === null
				hand = whichhand > 0.0 ? actions.forehand : actions.backhand
				swing = (net && !ball.in ? hand.volley.middle : hand.stroke).(shot)
				ix = swing.spot[12]
				iz = swing.spot[14]
				if net || ball.in
					t0 = 0.0
				else
					t0 = bound_t
					position = bound_position
					velocity = Vector3(velocity.x, velocity.y - G * bound_t, velocity.z
					ball.calculate_bounce(velocity, +ball.spin
				if net && !ball.in
					point = player.placement.position - Vector3(-v.z, 0.0, v.x) * ix + v * iz
					t = reach_range(position, velocity, point, player.speed, 0.0, -1.0) + 1.0
					tt = projected_time_for_y(position.y, velocity.y, swing.spot[13] + 1.0, 1.0
					if tt !== null && tt > t
						t = tt
				else
					t = projected_time_for_y(position.y, velocity.y, 1.25, -1.0
					if t === null
						t = velocity.y / G
			point = Vector3(position.x + velocity.x * t, 0.0, position.z + velocity.z * t
			v = shot_direction(point, player.end, left, right, forward, backward
			v.normalize(
			point = player.placement.position - Vector3(-v.z, 0.0, v.x) * ix + v * iz
			tt = t0 + t
			t1 = t0 + reach_range(position, velocity, point, player.speed, t0, 1.0)
			if t1 >= 0.0 && t1 < tt
				tt = t1
			if tt < -1.0
			else if (ball.in || bound_position.x > -(13 * 12 + 6) * 0.0254 - 0.125 && bound_position.x < (13 * 12 + 6) * 0.0254 + 0.125 && bound_position.z * player.end < 39 * 12 * 0.0254 + 0.5) && tt < (swing.impact - swing.start) * 60.0 + 1.0
				player.reset(
				player.left = left
				player.right = right
				player.forward = forward
				player.backward = backward
				player.do(shot
				reset_decision(
				:net = player.placement.position.z * player.end < 26 * 12 * 0.0254
			else
				player.reset(
				point = Vector3(position.x + velocity.x * t, 0.0, position.z + velocity.z * t
				v = shot_direction(point, player.end, left, right, forward, backward
				v.normalize(
				target = point + Vector3(-v.z, 0.0, v.x) * ix - v * iz
				epsilon = tt > 32.0 ? 1.0 / 8.0 : 1.0 / 32.0
				if player.placement.position.x * player.end < target.x * player.end - epsilon
					player.right = true
				else if player.placement.position.x * player.end > target.x * player.end + epsilon
					player.left = true
				epsilon = tt > 32.0 ? 1.0 : 1.0 / 4.0
				if player.placement.position.z * player.end < target.z * player.end - epsilon
					player.backward = true
				else if player.placement.position.z * player.end > target.z * player.end + epsilon
					player.forward = true
		super__step(
