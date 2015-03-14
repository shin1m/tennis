system = Module("system");
print = system.error.write_line;
io = Module("io");
time = Module("time");
gl = Module("gl");
glmatrix = Module("glmatrix");
xraft = Module("xraft");
collada = Module("collada");
placement = Module("placement");
ball = Module("ball");
player = Module("player");

print_time = false;

Matrix4 = glmatrix.Matrix4;
Vector3 = glmatrix.Vector3;
Placement = placement.Placement;
Ball = ball.Ball;
Mark = ball.Mark;
Player = player.Player;

$Stage = Class() :: @{
	$State = @(step, key_press, key_release) {
		o = Object();
		o.step = step;
		o.key_press = key_press;
		o.key_release = key_release;
		o;
	};

	$shader = @(name) {
		material = $scene.ids[name];
		material.build($scene.resolve);
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
	$ball_miss = @() $ball.serving() ? $serve_miss() : $miss("MISS");
	$ball_out = @{
		$mark.mark($ball);
		$ball.serving() ? $serve_miss() : $miss("OUT");
	};
	$step_things = @{
		$ball.step();
		$mark.step();
		$player0.step();
		$player1.step();
		target = $ball.position * 0.25;
		if ($fixed) {
			$camera0.position.x = target.x;
			$camera0.position.z = 48.0 + target.z;
			$camera1.position.x = target.x;
			$camera1.position.z = -48.0 + target.z;
		} else {
			$camera0.position.x = target.x + $player0.root_position().x * 0.5;
			$camera0.position.z = 48.0 * $player0.end + target.z;
			$camera1.position.x = target.x + $player1.root_position().x * 0.5;
			$camera1.position.z = 48.0 * $player1.end + target.z;
		}
	};

	$__initialize = @(main, dual, fixed, controller0, player0, controller1, player1) {
		$main = main;
		$dual = dual;
		$fixed = fixed;
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
		$player0 = Player($, player0);
		$scene.scene.instance_visual_scene._scene.nodes.push($player0.node);
		$player0.scene.scene.instance_visual_scene._scene._controllers.each($scene.scene.instance_visual_scene._scene._controllers.push);
		$player1 = Player($, player1);
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
		}, {});
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
	$key_press = @(modifier, key, ascii) $state.key_press.has(key) && $state.key_press[key][$]();
	$key_release = @(modifier, key, ascii) $state.key_release.has(key) && $state.key_release[key][$]();
};
