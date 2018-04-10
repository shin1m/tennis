system = Module("system"
print = system.error.write_line
math = Module("math"
gl = Module("gl"
glmatrix = Module("glmatrix"
collada = Module("collada"
placement = Module("placement"
ball = Module("ball"

Matrix4 = glmatrix.Matrix4
Vector3 = glmatrix.Vector3
Posture = placement.Posture
Placement = placement.Placement
G = ball.G

$reach_range = reach_range = @(ball, velocity, player, speed, t0, sign)
	qp = ball - player
	qp.y = 0.0
	v = Vector3(velocity.x, 0.0, velocity.z
	ss = speed * speed
	a = v * v - ss
	b = v * qp - ss * t0
	c = qp * qp - ss * t0 * t0
	d = b * b - a * c
	d < 0.0 ? -b / a : (-b + sign * math.sqrt(d)) / a

$shot_direction = shot_direction = @(ball, end, left, right, forward, backward)
	vx = -ball.x
	if left
		vx = vx - 12 * 12 * 0.0254 * end
	if right
		vx = vx + 12 * 12 * 0.0254 * end
	vz = -24 * 12 * 0.0254 * end - ball.z
	if forward
		vz = vz - 16 * 12 * 0.0254 * end
	if backward
		vz = vz + 10 * 12 * 0.0254 * end
	Vector3(vx, 0.0, vz

$Player = Class() :: @
	State = @(enter, step, do)
		o = Object(
		o.enter = enter
		o.step = step
		o.do = do
		o
	Action = Class() :: @
		$__initialize = @(scene, start, duration, use = @(x) true)
			$start = start
			$end = start + duration
			$iterators = scene.iterators(use
			$iterators.each(@(key, value)
				value.rewind(start
				value.index = value.i
		$rewind = @
			time = $start
			$iterators.each(@(key, value)
				value.i = value.index
				value.forward(time
		$forward = @(time) $iterators.each(@(key, value) value.forward(time
	Swing = Class(Action) :: @
		$__initialize = @(scene, skeleton, start, duration, impact, speed, spin = Vector3(0.0, 0.0, 0.0))
			:$^__initialize[$](scene, start, duration
			$impact = start + impact
			$speed = speed
			$spin = spin
			iterators = scene.iterators(
			iterators.each((@(key, value) value.rewind($impact))[$]
			spot = Object(
			skeleton.render(null, Matrix4(), [{"Spot": spot}]
			$spot = spot.vertex
			iterators.each((@(key, value) value.rewind($end))[$]
			root = Object(
			skeleton.render(null, Matrix4(), [{"Root": root}]
			$end_position = Vector3(root.vertex.v[12], 0.0, root.vertex.v[14]
			$end_toward = Vector3(root.vertex.v[8], 0.0, root.vertex.v[10]
		$merge = @(player)
			placement = player.placement.validate(
			placement.position = placement * $end_position
			placement.toward = placement * $end_toward
			placement.valid = false
	Run = Class(Action) :: @
		$__initialize = @(scene, skeleton, start, duration, use)
			:$^__initialize[$](scene, start, duration, use
			iterators = scene.iterators(@(x) x._node.id == "Armature_Root"
			iterators.each((@(key, value) value.rewind($start))[$]
			root = Object(
			skeleton.render(null, Matrix4(), [{"Root": root}]
			$toward = Vector3(root.vertex.v[8], 0.0, root.vertex.v[10]
	Motion = Class() :: @
		$__initialize = @(action)
			$action = action
			$end = action.end
			$rewind(
		$rewind = @
			$time = $action.start
			$action.rewind(
		$__call = @
			$action.forward($time
			if $time < $end
				$time = $time + 1.0 / 60.0
	$Motion = Motion
	RunMotion = Class(Motion) :: @
		duration = 4.0 / 64.0

		$__initialize = @(run, toward, player)
			:$^__initialize[$](run
			$toward = toward
			$placement = player.placement
			$toward0 = player.placement.toward
			player.root_transform.copy(0, player.root_transform.size(), player.root.transforms[1].bytes, 0
			t0 = toward.normalized(
			t1 = run.toward
			$toward1 = Vector3(t0.x * t1.z + t0.z * t1.x, 0.0, t0.z * t1.z - t0.x * t1.x
			$duration = $toward0 * $toward1 < -0.75 ? 0.0 : duration
		$__call = @
			:$^__call[$](
			if $duration > 0.0
				$duration = $duration - 1.0 / 64.0
			t = $duration / duration
			$placement.toward = $toward0 * t + $toward1 * (1.0 - t)

	lowers = {
		"Armature_Center": true
		"Armature_Leg0_R": true
		"Armature_Leg1_R": true
		"Armature_Foot_R": true
		"Armature_Toe_R": true
		"Armature_Leg0_L": true
		"Armature_Leg1_L": true
		"Armature_Foot_L": true
		"Armature_Toe_L": true
	is_not_root = @(x) x._node.id != "Armature_Root"
	is_lower = @(x) lowers.has(x._node.id
	is_upper = @(x) is_not_root(x) && !is_lower(x)
	load = @(scene, skeleton, source)
		source_fps = null
		fps = 64.0
		reader = collada.Reader(source
		read_action = @(use = @(x) true)
			start = Float(reader.get_attribute("start")) / source_fps
			duration = Float(reader.get_attribute("duration")) / source_fps
			reader.read_element_text(
			Action(scene, start, duration, use
		read_swing = @
			start = Float(reader.get_attribute("start")) / source_fps
			duration = Float(reader.get_attribute("duration")) / source_fps
			impact = Float(reader.get_attribute("impact")) / source_fps
			speed = Float(reader.get_attribute("speed")) / fps
			spin = collada.parse_array(gl.Float32Array, Float, 3, reader.get_attribute("spin")
			reader.read_element_text(
			Swing(scene, skeleton, start, duration, impact, speed, Vector3(spin[0], spin[1], spin[2])
		read_run = @(use)
			start = Float(reader.get_attribute("start")) / source_fps
			duration = Float(reader.get_attribute("duration")) / source_fps
			reader.read_element_text(
			Run(scene, skeleton, start, duration, use
		shot_swing_elements = {
			"flat": @(x) x.flat = read_swing(
			"topspin": @(x) x.topspin = read_swing(
			"lob": @(x) x.lob = read_swing(
			"slice": @(x) x.slice = read_swing(
			"reach": @(x) x.reach = read_swing(
		serve_elements = {
			"set": @(x) x.set = read_action(
			"toss": @(x) x.toss = read_action(
			"swing": @(x)
				x.swing = Object(
				reader.parse_elements(shot_swing_elements, x.swing
		ready_action_elements = {
			"stroke": @(x) x.stroke = read_run(is_not_root
			"volley": @(x) x.volley = read_run(is_not_root
			"smash": @(x) x.smash = read_run(is_not_root
		ready_elements = {
			"default": @(x) x.default = read_run(is_not_root
			"forehand": @(x)
				x.forehand = Object(
				reader.parse_elements(ready_action_elements, x.forehand
			"backhand": @(x)
				x.backhand = Object(
				reader.parse_elements(ready_action_elements, x.backhand
		run_lowers_elements = {
			"left": @(x) x[1] = read_run(is_lower
			"right": @(x) x[2] = read_run(is_lower
			"forward": @(x) x[4] = read_run(is_lower
			"forward_left": @(x) x[5] = read_run(is_lower
			"forward_right": @(x) x[6] = read_run(is_lower
			"backward": @(x) x[8] = read_run(is_lower
			"backward_left": @(x) x[9] = read_run(is_lower
			"backward_right": @(x) x[10] = read_run(is_lower
		run_action_elements = {
			"lowers": @(x)
				x.lowers = [null, null, null, null, null, null, null, null, null, null, null
				reader.parse_elements(run_lowers_elements, x.lowers
			"stroke": @(x) x.stroke = read_action(is_upper
			"volley": @(x) x.volley = read_action(is_upper
			"smash": @(x) x.smash = read_action(is_upper
		run_elements = {
			"lower": @(x) x.lower = read_run(is_lower
			"default": @(x) x.default = read_action(is_upper
			"forehand": @(x)
				x.forehand = Object(
				reader.parse_elements(run_action_elements, x.forehand
			"backhand": @(x)
				x.backhand = Object(
				reader.parse_elements(run_action_elements, x.backhand
		swing_volley_elements = {
			"middle": @(x)
				x.middle = Object(
				reader.parse_elements(shot_swing_elements, x.middle
			"high": @(x)
				x.high = Object(
				reader.parse_elements(shot_swing_elements, x.high
			"low": @(x)
				x.low = Object(
				reader.parse_elements(shot_swing_elements, x.low
		swing_action_elements = {
			"stroke": @(x)
				x.stroke = Object(
				reader.parse_elements(shot_swing_elements, x.stroke
			"volley": @(x)
				x.volley = Object(
				reader.parse_elements(swing_volley_elements, x.volley
			"smash": @(x) x.smash = read_swing(
		swing_elements = {
			"forehand": @(x)
				x.forehand = Object(
				reader.parse_elements(swing_action_elements, x.forehand
			"backhand": @(x)
				x.backhand = Object(
				reader.parse_elements(swing_action_elements, x.backhand
			"toss": @(x) x.toss = read_swing(
			"toss_lob": @(x) x.toss_lob = read_swing(
		root_elements = {
			"serve": @(x)
				x.serve = Object(
				reader.parse_elements(serve_elements, x.serve
			"ready": @(x)
				x.ready = Object(
				reader.parse_elements(ready_elements, x.ready
			"run": @(x)
				x.run = Object(
				x.run.speed = Float(reader.get_attribute("speed")) / fps
				reader.parse_elements(run_elements, x.run
			"swing": @(x)
				x.swing = Object(
				reader.parse_elements(swing_elements, x.swing
		try
			reader.read_next(
			reader.move_to_tag(
			reader.check_start_element("player"
			x = Object(
			source_fps = Float(reader.get_attribute("fps"
			reader.parse_elements(root_elements, x
			x
		finally
			reader.free(

	$__initialize = @(stage, model, scene)
		$stage = stage
		$ball = stage.ball
		$scene = scene
		$scene.build(stage.main.shaders
		$node = $scene.ids["Armature"]
		$node.transforms.clear(
		$placement = Placement(
		$node.transforms.push($placement
		zup = Posture(
		zup.toward = Vector3(0.0, 1.0, 0.0
		zup.upward = Vector3(0.0, 0.0, -1.0
		$root = $scene.ids["Armature_Root"]
		$root_transform = Matrix4($root.transforms[0]).bytes
		$root.transforms.unshift(zup.validate(
		$actions = load($scene, $root, model + ".player"
		$speed = $actions.run.speed
		$actions.serve.set.rewind(
		$lefty = $root.transforms[1].v[12] < 0.0 ? -1.0 : 1.0
		$smash_hand = -0.25 * $lefty
		$motion = RunMotion($actions.ready.default, $placement.toward, $
		$ready = null
		$reset(1.0, $state_default
		scene = stage.scene.scene.instance_visual_scene._scene
		scene.nodes.push($node
		$scene.scene.instance_visual_scene._scene._controllers.each(scene._controllers.push
	$transit = @(state)
		$state = state
		$state.enter[$](
	$reset = @(end = null, state = null)
		$left = $right = $forward = $backward = false
		if end !== null
			$end = end
		state !== null && $transit(state
	$setup = @ $placement.validate(
	$root_position = @
		v = $root.transforms[1].v
		$placement.validate() * ($root.transforms[0] * Vector3(v[12], v[13], v[14]))
	$direction = @
		v = $ball.velocity
		e = (v.z < 0.0 ? 1.0 : -1.0) * $end
		v = Vector3(v.x * e, 0.0, v.z * e
		v.length() > 0.01 / 64.0 ? v : Vector3(0.0, 0.0, -$end)
	$whichhand = @(v) Vector3(-v.z, 0.0, v.x) * ($ball.position - $placement.position)
	$relative_ball = @(swing, ball = null)
		if ball === null
			ball = $ball.position
		$placement.validate(
		p = ball - $placement.position
		v = $placement.toward
		x = v.z * p.x - v.x * p.z - swing.spot[12]
		y = p.y - swing.spot[13]
		z = v.x * p.x + v.z * p.z - swing.spot[14]
		swing.spot[4] > 0.0 ? Vector3(-x, y, -z) : Vector3(x, y, z)
	$step = @
		$state.step[$](
		position = $placement.position
		if position.x < -30 * 12 * 0.0254
			position.x = -30 * 12 * 0.0254
		else if position.x > 30 * 12 * 0.0254
			position.x = 30 * 12 * 0.0254
		if position.z * $end < 1.0
			position.z = 1.0 * $end
		else if position.z * $end > 60 * 12 * 0.0254
			position.z = 60 * 12 * 0.0254 * $end
	$do = @(shot) $state.do[$](shot
	$shot_direction = @ $ball.position.z * $end < 0.0 ? Vector3(0.0, 0.0, -$end) : shot_direction($ball.position, $end, $left, $right, $forward, $backward)
	$volley_height = @ $actions.swing.forehand.volley.middle.flat.spot[13]
	$smash_height = @ $actions.swing.forehand.smash.spot[13] - 0.25
	$create_record = @
		record = Object(
		record.placement = Placement(
		record.root = Matrix4(
		$record(record
		record
	$record = @(to)
		$placement.copy(to.placement
		bytes = $root.transforms[1].bytes
		bytes.copy(0, bytes.size(), to.root.bytes, 0
		to.action = $motion.action
		to.time = $motion.time
		to.ready = $ready
	$replay = @(from)
		from.placement.copy($placement
		bytes = from.root.bytes
		bytes.copy(0, bytes.size(), $root.transforms[1].bytes, 0
		from.ready !== null && from.ready.rewind(
		from.action.rewind(
		from.action.forward(from.time
	$state_default = State(@
		v = $ball.position - $placement.position
		v.y = 0.0
		v.normalize(
		$placement.toward = v
		$motion = RunMotion($actions.ready.default, v, $
	, @
		d = Vector3(0.0, 0.0, 0.0
		if $left
			d.x = -$speed * $end
		if $right
			d.x = $speed * $end
		if $forward
			d.z = -$speed * $end
		if $backward
			d.z = $speed * $end
		actions = d.x == 0.0 && d.z == 0.0 ? $actions.ready : $actions.run
		if $ball.done
			v = Vector3(0.0, 0.0, -$end
			action = actions.default
			if actions === $actions.run
				run = $actions.run.lower
		else if $ball.hitter === null || $ball.hitter.end == $end
			v = $ball.position - $placement.position
			v.y = 0.0
			v.normalize(
			action = actions.default
			if actions === $actions.run
				if $forward
					run = $actions.run.lower
				else if $left
					run = actions.backhand.lowers[9]
				else if $right
					run = actions.forehand.lowers[10]
				else if $backward
					run = v.x * $end > 0.0 ? actions.forehand.lowers[9] : actions.backhand.lowers[10]
				else
					run = $actions.run.lower
		else
			v = $direction(
			v.normalize(
			whichhand = $whichhand(v
			t = reach_range($ball.position, $ball.velocity, $placement.position, 0.0, 0.0, 1.0
			y = $ball.position.y + ($ball.velocity.y - 0.5 * G * t) * t
			if y > $smash_height()
				hand = whichhand > $smash_hand ? actions.forehand : actions.backhand
				action = hand.smash
			else
				hand = whichhand > 0.0 ? actions.forehand : actions.backhand
				action = $ball.in || y < 0.0 ? hand.stroke : hand.volley
			if actions === $actions.run
				run = hand.lowers[($left ? 1 : $right ? 2 : 0) + ($forward ? 4 : $backward ? 8 : 0)]
		if actions === $actions.ready
			$motion = RunMotion(action, v, $
			$ready = null
		else
			if $motion.action !== run || d != $motion.toward
				$motion = RunMotion(run, d, $
			$motion.time >= run.end && $motion.rewind(
			$ready = action
			action.rewind(
			$placement.position = $placement.position + d
		$motion(
		$placement.valid = false
	, @(shot)
		$ready = null
		$placement.toward = $shot_direction(
		$placement.valid = false
		actions = $actions.swing
		whichhand = $whichhand($direction().normalized(
		t = $ball.projected_time_for_y($smash_height(), 1.0
		if t !== null
			swing = whichhand > $smash_hand ? actions.forehand.smash : actions.backhand.smash
			impact = (swing.impact - swing.start) * 60.0
			if t > impact
				ball = $relative_ball(swing, $ball.position + $ball.velocity * impact
				if math.fabs(ball.x) < 0.5
					$motion = Motion(swing
					return $transit($state_smash_swing
		hand = whichhand > 0.0 ? actions.forehand : actions.backhand
		if $ball.done
			$motion = Motion(($placement.position.z * $end > 21 * 12 * 0.0254 ? hand.stroke : hand.volley.middle).(shot)
		else
			swing = hand.volley.middle.(shot)
			y = $ball.in ? -1.0 : $ball.projected_y_in((swing.impact - swing.start) * 60.0)
			volley_height = $volley_height()
			if y > volley_height + 0.375
				shots = hand.volley.high
			else if y > volley_height - 0.375
				shots = hand.volley.middle
			else if y > 0.0
				shots = hand.volley.low
			else
				shots = hand.stroke
			swing = shots.(shot)
			impact = (swing.impact - swing.start) * 60.0
			ball = $relative_ball(swing, $ball.position + $ball.velocity * impact
			if ball.x < -0.5 || (whichhand > 0.0 ? ball.z > 1.0 : ball.z < -1.0)
				$motion = Motion(shots.reach
				return $transit($state_reach_swing
			$motion = Motion(swing
		$transit($state_swing
	$state_serve_set = State(@
		$motion = Motion($actions.serve.set
		$ready = null
	, @
		speed = 2.0 / 64.0
		if $left
			$ball.position.x = $ball.position.x - speed * $end
		if $right
			$ball.position.x = $ball.position.x + speed * $end
		es = $end * $stage.side
		xes = $ball.position.x * es
		center = 12 * 0.0254
		wide = 14 * 12 * 0.0254
		if xes < center
			$ball.position.x = center * es
		if xes > wide
			$ball.position.x = wide * es
		$ball.position.y = 0.875
		$ball.velocity = Vector3(0.0, 0.0, 0.0
		$ball.spin = Vector3(0.0, 0.0, 0.0
		$placement.position = Vector3($ball.position.x, 0.0, $ball.position.z
		$placement.toward = Vector3((6 * 12 + 9) * -0.0254 * es + 2 * 12 * 0.0254 * $lefty * $end, 0.0, 21 * 12 * -0.0254 * $end) - $placement.position
		$placement.valid = false
		$motion(
	, @(shot)
		$transit($state_serve_toss
	$state_serve_toss = State(@
		$ball.position.y = 1.5
		$placement.validate(
		toward = $placement.toward
		left = Vector3(toward.z, 0.0, -toward.x
		$ball.velocity = left * 0.0075 * $lefty + toward * 0.01
		$ball.velocity.y = 0.085
		$ball.spin = Vector3(0.0, 0.0, 0.0
		$motion = Motion($actions.serve.toss
	, @
		if $ball.position.y <= 1.5
			$ball.position.x = $placement.position.x
			$ball.position.z = $placement.position.z
			$ball.velocity = Vector3(0.0, 0.0, 0.0
			$transit($state_serve_set
		if $left
			$placement.toward.x = $placement.toward.x - 1.0 / 64.0 * $end
		if $right
			$placement.toward.x = $placement.toward.x + 1.0 / 64.0 * $end
		$placement.valid = false
		$motion(
	, @(shot)
		$motion = Motion($actions.serve.swing.(shot)
		$transit($state_serve_swing
	$state_serve_swing = State(@
		$stage.sound_swing.play(
	, @
		if math.fabs($motion.time - $motion.action.impact) < 0.5 / 60.0
			ball = $relative_ball($motion.action
			if math.fabs(ball.y) < 0.3
				d = 58 * 12 * 0.0254 + ball.y * 10.0
				spin = $motion.action.spin
				d = d * math.pow(2.0, -spin.x * (4.0 / 64.0))
				speed = $motion.action.speed + ball.y * 0.125
				$ball.impact($placement.toward.x, $placement.toward.z, speed, G * d / (2.0 * speed) - $ball.position.y * speed / d, spin
				$ball.hitter = $
				$stage.sound_hit.play(
		$motion(
		$motion.time < $motion.end && return
		!$ball.done && $ball.hitter === null && $ball.emit_serve_air(
		$motion.action.merge($
		$transit($state_default
	, @(shot)
	$swing_impact = @(v)
		if math.fabs($motion.time - $motion.action.impact) < 0.5 / 60.0
			ball = $relative_ball($motion.action
			#print("x: " + ball.x + ", y: " + ball.y + ", z: " + ball.z
			if math.fabs(ball.x) < 0.5 && ball.y < 1.0 && math.fabs(ball.z) < 1.0
				d = v.length(
				n = -d * $ball.position.z / v.z
				b = $ball.position.y * (d - n) / d
				a = d / (60 * 12 * 0.0254)
				speed = $motion.action.speed * (a > 1.25 ? 1.25 : a < 0.85 ? 0.85 : a)
				spin = $motion.action.spin
				dd = $ball.position.z * $end
				d = dd + (d - dd) * math.pow(2.0, -spin.x * ((spin.x > 0.0 ? 12.0 : 8.0) / 64.0))
				nh = (36 + 42) * 0.5 * 0.0254 + $ball.radius
				if b < nh
					vm = math.sqrt(G * (d - n) * n * 0.5 / (nh - b)
					if vm < speed
						speed = vm
				d = d - ball.x * 2.0
				speed = speed - ball.x * 0.125
				dx = v.x + v.z * ball.z * 0.0625
				dz = v.z - v.x * ball.z * 0.0625
				$ball.impact(dx, dz, speed, G * d / (2.0 * speed) - $ball.position.y * speed / d, spin
				$ball.hit($
				$stage.sound_hit.play(
		$motion(
		$motion.time < $motion.end && return
		$motion.action.merge($
		$transit($state_default
	$state_swing = State(@
		$stage.sound_swing.play(
	, @
		v = $shot_direction(
		if $motion.time <= $motion.action.impact
			$placement.toward = +v
			$placement.valid = false
		$swing_impact(v
	, @(shot)
	$state_smash_swing = State(@
		$stage.sound_swing.play(
	, @
		v = $shot_direction(
		if $motion.time <= $motion.action.impact
			$placement.toward = +v
			$placement.valid = false
		if math.fabs($motion.time - $motion.action.impact) < 0.5 / 60.0
			ball = $relative_ball($motion.action
			#print("x: " + ball.x + ", y: " + ball.y + ", z: " + ball.z
			if math.fabs(ball.x) < 0.5 && math.fabs(ball.y) < 1.0 && math.fabs(ball.z) < 1.0
				d = v.length() + (ball.y - ball.z) * 2.0
				speed = $motion.action.speed + ball.y * 0.125
				dx = v.x + v.z * ball.x * 0.0625
				dz = v.z - v.x * ball.x * 0.0625
				$ball.impact(dx, dz, speed, G * d / (2.0 * speed) - $ball.position.y * speed / d, $motion.action.spin
				$ball.hit($
				$stage.sound_hit.play(
		$motion(
		$motion.time < $motion.end && return
		$motion.action.merge($
		$transit($state_default
	, @(shot)
	$state_reach_swing = State(@
		impact = ($motion.action.impact - $motion.time) * 60.0
		vx = $ball.position.x + $ball.velocity.x * impact - $placement.position.x
		vz = $ball.position.z + $ball.velocity.z * impact - $placement.position.z
		$placement.toward = Vector3(vx, 0.0, vz
		$placement.valid = false
		$stage.sound_swing.play(
	, @
		$swing_impact($shot_direction(
	, @(shot)
