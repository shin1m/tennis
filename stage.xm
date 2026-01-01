system = Module("system"
print = system.error.write_line
os = Module("os"
time = Module("time"
gl = Module("gl"
glmatrix = Module("glmatrix"
xade = Module("xade"
collada = Module("collada"
placement = Module("placement"
ball = Module("ball"
player = Module("player"

print_time = false

Matrix4 = glmatrix.Matrix4
Vector3 = glmatrix.Vector3
Placement = placement.Placement
Ball = ball.Ball
Mark = ball.Mark
Player = player.Player

$Stage = Object + @
	$State = Object + @
		$step
		$key_press
		$key_release
		$render
		$__initialize = @(step, key_press, key_release, render)
			$step = step
			$key_press = key_press
			$key_release = key_release
			$render = render

	$ball_bounce = @ $sound_bounce.play(
	$ball_in = @ $mark.mark($ball
	$ball_net = @ $sound_net.play(
	$ball_chip = @ $sound_chip.play(
	$ball_miss = @ $ball.serving() ? $serve_miss() : $miss("MISS")
	$ball_out = @
		$mark.mark($ball
		$ball.serving() ? $serve_miss() : $miss("OUT")
	$set_cameras = @
		target = $ball.position * 0.25
		if $fixed
			$camera0.position.x = target.x
			$camera0.position.z = 48.0 + target.z
			$camera1.position.x = target.x
			$camera1.position.z = -48.0 + target.z
		else
			$camera0.position.x = target.x + $player0.root_position().x * 0.5
			$camera0.position.z = 48.0 * $player0.end + target.z
			$camera1.position.x = target.x + $player1.root_position().x * 0.5
			$camera1.position.z = 48.0 * $player1.end + target.z
	$step_things = @
		$ball.step(
		$mark.step(
		$player0.step(
		$player1.step(
		$set_cameras(

	$main
	$dual
	$fixed
	$sound_bounce
	$sound_net
	$sound_chip
	$sound_hit
	$sound_swing
	$sound_ace
	$sound_miss
	$text_viewing
	$scene
	$camera0
	$camera1
	$ball
	$mark
	$player0
	$player1
	$state_ready
	$state_play
	$__initialize = @(main, dual, fixed, controller0, player0, controller1, player1)
		$main = main
		$dual = dual
		$fixed = fixed
		t0 = Thread(@
			:scene = collada.load((os.Path(system.script) / "../data/court.dae").__string(
			scene.share(
		t1 = Thread(@
			:p0 = collada.load(player0 + ".dae"
			p0.share(
		t2 = Thread(@
			:p1 = collada.load(player1 + ".dae"
			p1.share(
		$sound_bounce = main.load_sound("data/bounce.wav"
		$sound_net = main.load_sound("data/net.wav"
		$sound_chip = main.load_sound("data/chip.wav"
		$sound_hit = main.load_sound("data/hit.wav"
		$sound_swing = main.load_sound("data/swing.wav"
		$sound_ace = main.load_sound("data/ace.wav"
		$sound_miss = main.load_sound("data/miss.wav"
		$text_viewing = Matrix4().scale(0.25, 0.25, 1.0
		t0.join(
		scene.own(
		$scene = scene
		$scene.build(main.shaders
		$camera0 = Placement(
		$camera1 = Placement(
		$ball = Ball($, main.shaders, "#Material-Shadow", "#Material-Ball"
		$mark = Mark($, main.shaders, "#Material-Shadow"
		t1.join(
		p0.own(
		$player0 = Player($, player0, p0
		t2.join(
		p1.own(
		$player1 = Player($, player1, p1
		$player0.opponent = $player1
		$player1.opponent = $player0
		$state_ready = $State(@
			$duration > 0 || return $transit_play(
			$duration = $duration - 1
		, {
			xade.Key.RETURN: @ $transit_play(
			xade.Key.ESCAPE: @ $back(
		}, {}, @(width, height)
		$state_play = $State(@
			$step_things(
			$ball.done || return
			$duration > 0 || return $next(
			$duration = $duration - 1
		, {
			xade.Key.RETURN: @ $next(
			xade.Key.ESCAPE: @ $back(
		}, {}, @(width, height)
		controller0[$]($state_play, $player0
		controller1[$]($state_play, $player1
	$destroy = @
		$sound_bounce.delete(
		$sound_net.delete(
		$sound_chip.delete(
		$sound_hit.delete(
		$sound_swing.delete(
		$sound_ace.delete(
		$sound_miss.delete(
		$scene.destroy(
		$player0.scene.destroy(
		$player1.scene.destroy(
	$step = @ $state.step[$](
	$render = @(width, height)
		gl.viewport(0, 0, width, height
		t0 = time.now()
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT
		print_time && print("\tclear: " + (time.now() - t0)
		$ball.setup(
		$mark.setup(
		$player0.setup(
		$player1.setup(
		gl.enable(gl.DEPTH_TEST
		$dual && gl.viewport(0, 0, width / 2, height
		pw = width * ($dual ? 0.5 : 1.0) / height
		ph = 1.0
		if pw < 0.75
			ph = 0.75 / pw
			pw = 0.75
		projection = Matrix4().frustum(-pw, pw, -ph, ph, 10.0, 200.0).bytes
		$scene.render(projection, $camera0.viewing()
		if $dual
			gl.viewport(width / 2, 0, width - width / 2, height
			$scene.render(projection, $camera1.viewing()
			gl.viewport(0, 0, width, height
		t0 = time.now(
		gl.disable(gl.DEPTH_TEST
		viewing = $main.text_scale * $text_viewing
		y = $message.size() * 0.5 - 1.0
		$message.each((@(line)
			v = Matrix4(viewing).translate(line.size() * -0.25, y, 0.0).bytes
			$main.font($main.projection, v, line
			:y = y - 1.0
		)[$]
		$state.render[$](width, height
		print_time && print("\ttext: " + (time.now() - t0)
	$key_press = @(key) $state.key_press.has(key) && $state.key_press[key][$]()
	$key_release = @(key) $state.key_release.has(key) && $state.key_release[key][$]()
