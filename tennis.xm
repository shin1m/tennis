system = Module("system");
print = system.error.write_line;
io = Module("io");
time = Module("time");
gl = Module("gl");
glmatrix = Module("glmatrix");
glshaders = Module("glshaders");
glimage = Module("glimage");
al = Module("al");
cairo = Module("cairo");
xraft = Module("xraft");
collada = Module("collada");
placement = Module("placement");
player = Module("player");
stage = Module("stage");
computer = Module("computer");

xraft.Key.VOLUMEUP = 0x1008FF13;
xraft.Key.VOLUMEDOWN = 0x1008FF11;

print_time = false;

Matrix4 = glmatrix.Matrix4;
Vector3 = glmatrix.Vector3;
Posture = placement.Posture;
Player = player.Player;

Menu = Class() :: @{
	$Item = @(label, select, do) {
		o = Object();
		o.label = label;
		o.select = select;
		o.do = do;
		o;
	};

	$__initialize = @(main, columns = 1) {
		$main = main;
		$columns = columns;
		$selected = 0;
	};
	$render = @(viewing) {
		n = ($items.size() + $columns - 1) / $columns;
		scale = 1.0 / (n < 4 ? 4 : n);
		viewing = Matrix4(viewing).scale(scale, scale, 1.0);
		dx = 4.0 * 2.0 / $columns;
		x = dx * 0.5 - 4.0;
		y = 0.0;
		i = 0;
		$items.each(@(item) {
			text = (i == $selected ? "*" : " ") + item.label;
			:y = y - 1.0;
			viewing = Matrix4(:viewing).translate(x - text.size() * 0.25, y, 0.0).bytes;
			$main.font($main.projection, viewing, text);
			:i = i + 1;
			if (i % n != 0) return;
			:x = x + dx;
			:y = 0.0;
		}[$]);
	};
	$select = @(i) {
		$selected = i;
		$main.sound_cursor.play();
		$items[$selected].select();
	};
	$up = @() $selected > 0 && $select($selected - 1);
	$down = @() $selected < $items.size() - 1 && $select($selected + 1);
	$left = @{
		n = ($items.size() + $columns - 1) / $columns;
		if ($selected >= n) $select($selected - n);
	};
	$right = @{
		n = ($items.size() + $columns - 1) / $columns;
		if ($selected < $items.size() - n) $select($selected + n);
	};
	$do = @{
		$main.sound_select.play();
		$items[$selected].do();
	};
	$keys = keys = {
		xraft.Key.ESCAPE: @() $back(),
		xraft.Key.SPACE: $do,
		xraft.Key.D2: $do,
		xraft.Key.LEFT: $left,
		xraft.Key.RIGHT: $right,
		xraft.Key.UP: $up,
		xraft.Key.DOWN: $down,
		xraft.Key.S: $left,
		xraft.Key.F: $right,
		xraft.Key.E: $up,
		xraft.Key.C: $down
	};
	$key_press = @(key) keys.has(key) && keys[key][$]();
};

MainScreen = null;

Match = Class(stage.Stage) :: @{
	$new_game = @{
		$player0.point = $player1.point = 0;
		$second = false;
		games = $player0.game + $player1.game;
		$end = games % 4 < 2 ? 1.0 : -1.0;
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
		xraft.Key.RETURN: @() $new_set(),
		xraft.Key.ESCAPE: @() $back()
	}, {}, @(width, height) {});
	$transit_close = @{
		$state = state_close;
		$message = [
			($player0.game > $player1.game ? "P1" : "P2") + " WON!",
			"P1 " + $player0.game + " - " + $player1.game + " P2",
			"PRESS START",
			"TO PLAY AGAIN"
		];
		$sound_ace.play();
	};
	$transit_play = @{
		$state = $state_play;
		$message = [];
	};
	$reset = @{
		$side = ($player0.point + $player1.point) % 2 == 0 ? 1.0 : -1.0;
		$ball.reset($side, 2 * 12 * 0.0254 * $end * $side, 0.875, 39 * 12 * 0.0254 * $end);
		$mark.duration = 0.0;
		$server.reset($end, Player.state_serve_set);
		$receiver.placement.position = Vector3(-9 * 12 * 0.0254 * $end * $side, 0.0, -39 * 12 * 0.0254 * $end);
		$receiver.placement.valid = false;
		$receiver.reset(-$end, Player.state_default);
		$camera0.position = Vector3(0.0, 14.0, 0.0);
		$camera0.toward = Vector3(0.0, -12.0, -40.0 * ($fixed ? 1.0 : $player0.end));
		$camera1.position = Vector3(0.0, 14.0, 0.0);
		$camera1.toward = Vector3(0.0, -12.0, -40.0 * ($fixed ? -1.0 : $player1.end));
	};

	$__initialize = @(main, dual, fixed, controller0, player0, controller1, player1) {
		:$^__initialize[$](main, dual, fixed, controller0, player0, controller1, player1);
		$new_set();
	};
	$new_set = @{
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
	$back = @() $main.screen__(MainScreen($main));
};

Training = Class(stage.Stage) :: @{
	Item = @(label, select, do, ready, play, back) {
		o = Menu.Item(label, select, do);
		o.ready = ready;
		o.play = play;
		o.back = back;
		o;
	};

	$ball_ace = @() $duration = 0.5 * 64.0;
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
	$ball_in = @{
		$mark.mark($ball);
		if ($ball.hitter !== $player0) return;
		$text_viewing = Matrix4().scale(0.25, 0.25, 1.0);
		$message = ["IN"];
	};
	$step_things = @{
		:$^step_things[$]();
		$camera1.position = Vector3(($ball.position.x + $player0.root_position().x) * 0.5, 4.0, ($ball.position.z + 40.0 * $player1.end) * 0.5);
		$camera1.toward = Vector3(0.0, -6.0, -40.0 * $player1.end);
	};

	$__initialize = @(main, controller0, player0, player1) {
		:$^__initialize[$](main, true, false, controller0, player0, @(controller, player) {
			super__step = controller.step[$];
			controller.step = @{
				if (player.state !== Player.state_swing) player.left = player.right = player.forward = player.backward = false;
				super__step();
			};
		}, player1);
		{
			xraft.Key.RETURN: @() {
				$side = -$side;
				$transit_ready();
			}
		}.each(@(key, value) {
			$state_ready.key_press[key] = value;
			$state_play.key_press[key] = value;
		}[$]);
		$camera0.position = Vector3(0.0, 14.0, 0.0);
		$camera0.toward = Vector3(0.0, -12.0, -40.0 * $player0.end);
		$menu = Menu(main);
		$menu.back = $exit;
		toss_message = [
			"  CHANGE SIDES: START",
			"      POSITION: +    ",
			"COURCE & SWING: + & *"
		];
		$menu.items = '(
			Item(" SERVE ", @{
				$ball.reset($side, 2 * 12 * 0.0254 * $side, 0.875, 39 * 12 * 0.0254);
				$mark.duration = 0.0;
				$player0.reset(1.0, Player.state_serve_set);
				$player1.placement.position = Vector3(-9 * 12 * 0.0254 * $side, 0.0, -39 * 12 * 0.0254);
				$player1.placement.valid = false;
				$player1.reset(-1.0, Player.state_default);
				$step_things();
			}[$], (@() $transit_ready())[$], @{
				$text_viewing = Matrix4().translate(0.0, -0.625, 0.0).scale(0.125, 0.125, 1.0);
				$message = [
					"CHANGE SIDES: START",
					"    POSITION: < + >",
					"        TOSS:   *  ",
					"      COURCE: < + >",
					"       SWING:   *  "
				];
				$duration = 0.0 * 64.0;
			}[$], @{}, (@() $transit_select())[$]),
			Item(" STROKE", @{
				$reset(3 * 12 * 0.0254 * $side, 1.0, -39 * 12 * 0.0254, Vector3((0.0 - 3.2 * $side) * 12 * 0.0254, 0.0, 39 * 12 * 0.0254), 'toss);
			}[$], (@() $transit_ready())[$], @{
				$text_viewing = Matrix4().translate(0.0, -0.75, 0.0).scale(0.125, 0.125, 1.0);
				$message = toss_message;
				$duration = 0.5 * 64.0;
			}[$], (@() $toss('toss))[$], (@() $transit_select())[$]),
			Item(" VOLLEY", @{
				$reset(3 * 12 * 0.0254 * $side, 1.0, -39 * 12 * 0.0254, Vector3((0.1 - 2.0 * $side) * 12 * 0.0254, 0.0, 13 * 12 * 0.0254), 'toss);
			}[$], (@() $transit_ready())[$], @{
				$text_viewing = Matrix4().translate(0.0, -0.75, 0.0).scale(0.125, 0.125, 1.0);
				$message = toss_message;
				$duration = 0.5 * 64.0;
			}[$], (@() $toss('toss))[$], (@() $transit_select())[$]),
			Item(" SMASH ", @{
				$reset(3 * 12 * 0.0254 * $side, 1.0, -39 * 12 * 0.0254, Vector3((0.4 - 0.4 * $side) * 12 * 0.0254, 0.0, 9 * 12 * 0.0254), 'toss_lob);
			}[$], (@() $transit_ready())[$], @{
				$text_viewing = Matrix4().translate(0.0, -0.75, 0.0).scale(0.125, 0.125, 1.0);
				$message = toss_message;
				$duration = 0.5 * 64.0;
			}[$], (@() $toss('toss_lob))[$], (@() $transit_select())[$]),
			Item(" BACK  ", @{
				$ball.reset($side, 2 * 12 * 0.0254, $ball.radius + 0.01, 2 * 12 * 0.0254);
				$mark.duration = 0.0;
				$player0.placement.position = Vector3((0.1 - 2.0 * $side) * 12 * 0.0254, 0.0, 13 * 12 * 0.0254);
				$player0.placement.valid = false;
				$player0.reset(1.0, Player.state_default);
				$player1.placement.position = Vector3((0.1 + 2.0 * $side) * 12 * 0.0254, 0.0, -13 * 12 * 0.0254);
				$player1.placement.valid = false;
				$player1.reset(-1.0, Player.state_default);
				$step_things();
			}[$], (@() $back())[$], @{}, @{}, (@() $exit())[$])
		);
		keys = {};
		Menu.keys.each(@(key, value) keys[key] = @() value[$menu]());
		$state_select = $State(@{}, keys, {}, @(width, height) $menu.render(Matrix4($main.text_scale).translate(0.0, 0.5, 0.0)));
		$transit_select();
	};
	$next = @() $ball.done && $transit_ready();
	$reset = @(x, y, z, position, shot) {
		$ball.reset($side, x, y, z);
		$mark.duration = 0.0;
		$player0.placement.position = position;
		$player0.placement.valid = false;
		$player0.reset(1.0, Player.state_default);
		$player1.placement.position = $ball.position - Posture().validate() * $player1.actions.swing.(shot).spot * Vector3(0.0, 0.0, 0.0);
		$player1.placement.position.y = 0.0;
		$player1.placement.valid = false;
		$player1.reset(-1.0, Player.state_default);
		$step_things();
	};
	$toss = @(shot) {
		$player1.placement.toward = $player1.shot_direction();
		$player1.placement.valid = false;
		$player1.motion = Player.Motion($player1.actions.swing.(shot));
		$player1.transit(Player.state_swing);
	};
	$back = @() $menu.items[$menu.selected].back();
	$exit = @() $main.screen__(MainScreen($main));
	$transit_play = @{
		$state = $state_play;
		$menu.items[$menu.selected].play();
	};
	$transit_select = @{
		$state = $state_select;
		$side = 1.0;
		$menu.items[$menu.selected].select();
		$message = [];
		$duration = 0.0 * 64.0;
	};
	$transit_ready = @{
		$state = $state_ready;
		selected = $menu.items[$menu.selected];
		selected.select();
		selected.ready();
	};
};

controller0 = @(controller, player) {
	{
		xraft.Key.D1: @() player.do('topspin),
		xraft.Key.D2: @() player.do('flat),
		xraft.Key.VOLUMEUP: @() player.do('lob),
		xraft.Key.VOLUMEDOWN: @() player.do('slice),
		xraft.Key.LEFT: @() player.left = true,
		xraft.Key.RIGHT: @() player.right = true,
		xraft.Key.UP: @() player.forward = true,
		xraft.Key.DOWN: @() player.backward = true,
		xraft.Key.SPACE: @() player.do('flat),
		xraft.Key.J: @() player.do('topspin),
		xraft.Key.L: @() player.do('flat),
		xraft.Key.I: @() player.do('lob),
		xraft.Key.M: @() player.do('slice),
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
		xraft.Key.D7: @() player.do('topspin),
		xraft.Key.D9: @() player.do('flat),
		xraft.Key.COMMA: @() player.do('lob),
		xraft.Key.PERIOD: @() player.do('slice),
		xraft.Key.D4: @() player.left = true,
		xraft.Key.D6: @() player.right = true,
		xraft.Key.D8: @() player.forward = true,
		xraft.Key.D5: @() player.backward = true
	}.each(@(key, value) controller.key_press[key] = value);
	{
		xraft.Key.D4: @() player.left = false,
		xraft.Key.D6: @() player.right = false,
		xraft.Key.D8: @() player.forward = false,
		xraft.Key.D5: @() player.backward = false
	}.each(@(key, value) controller.key_release[key] = value);
};

Dialog = Class() :: @{
	$__initialize = @(main, image, sound, title) {
		$main = main;
		$image = glimage.Image((io.Path(system.script) / ".." / image).__string());
		$sound = main.load_sound(sound);
		$sound.setb(al.LOOPING, true);
		$sound.play();
		$title = title;
	};
	$destroy = @{
		$sound.delete();
		$image.destroy();
	};
	$render = @(width, height, viewing) {
		w = Float(width) / height;
		a = width * $image.height / (height * $image.width);
		s = a < 1.0 ? a : 1.0;
		t = a < 1.0 ? 1.0 : 1.0 / a;
		bytes = Bytes(20 * gl.Float32Array.BYTES_PER_ELEMENT);
		array = gl.Float32Array(bytes);
		array[0] = -w;
		array[1] = -1.0;
		array[2] = 0.0;
		array[3] = (1.0 - s) * 0.5;
		array[4] = (1.0 + t) * 0.5;
		array[5] = w;
		array[6] = -1.0;
		array[7] = 0.0;
		array[8] = (1.0 + s) * 0.5;
		array[9] = (1.0 + t) * 0.5;
		array[10] = -w;
		array[11] = 1.0;
		array[12] = 0.0;
		array[13] = (1.0 - s) * 0.5;
		array[14] = (1.0 - t) * 0.5;
		array[15] = w;
		array[16] = 1.0;
		array[17] = 0.0;
		array[18] = (1.0 + s) * 0.5;
		array[19] = (1.0 - t) * 0.5;
		$image($main.projection, viewing.bytes, bytes);
		v = (viewing * $main.text_scale).translate(0.0, 0.5, 0.0).scale(1.5 / 4.0, 1.5 / 4.0, 1.0).translate($title.size() * -0.25, 0.0, 0.0).bytes;
		$main.font($main.projection, v, $title);
	};
};

MainMenu = null;

StageMenu = Class(Dialog) :: @{
	$__initialize = @(screen, image, sound, title, done) {
		:$^__initialize[$](screen.main, image, sound, title);
		$player0 = Menu(screen.main, 2);
		$player0.back = @{
			$sound.stop();
			screen.transit(MainMenu(screen), 1.0);
		}[$];
		$player0.items = [];
		$player1 = Menu(screen.main, 2);
		$player1.back = (@() $transit($player0, 1.0))[$];
		$player1.items = [];
		screen.players.each(@(x) {
			$player0.items.push(Menu.Item(x.name, @{}, (@() $transit($player1, -1.0))[$]));
			$player1.items.push(Menu.Item(x.name, @{}, (@() $transit($ready, -1.0))[$]));
		}[$]);
		$ready = Menu(screen.main);
		$ready.back = (@() $transit($player1, 1.0))[$];
		$ready.items = '(Menu.Item("START", @{}, (@() done(screen.players[$player0.selected], screen.players[$player1.selected]))[$]));
		$menu = $player0;
		$transit_from = null;
	};
	$step = @{
		if ($transit_from === null) return;
		if ($t < $duration)
			$t = $t + 1.0;
		else
			$transit_from = null;
	};
	$render = @(width, height, viewing) {
		:$^render[$](width, height, viewing);
		message = $player0.items[$player0.selected].label;
		if ($menu === $player0) message = message + "?";
		message = message + " vs " + $player1.items[$player1.selected].label;
		if ($menu === $player1) message = message + "?";
		v = (viewing * $main.text_scale).translate(0.0, 0.25, 0.0).scale(1.5 / 8.0, 1.5 / 8.0, 1.0).translate(message.size() * -0.25, -0.5, 0.0).bytes;
		$main.font($main.projection, v, message);
		if ($transit_from !== null) {
			a = 2.0 * $direction * width / height;
			t = $t / $duration;
			$transit_from.render(Matrix4(viewing).translate(a * t, 0.0, 0.0) * $main.text_scale);
			viewing = Matrix4(viewing).translate(a * (t - 1.0), 0.0, 0.0);
		}
		$menu.render(viewing * $main.text_scale);
	};
	$key_press = @(key) $transit_from === null && $menu.key_press(key);
	$transit = @(menu, direction) {
		$transit_from = $menu;
		$menu = menu;
		$direction = direction;
		$duration = 30.0;
		$t = 0.0;
	};
};

MainMenu = Class(Dialog) :: @{
	$__initialize = @(screen) {
		:$^__initialize[$](screen.main, "data/main-background.jpg", "data/main-background.wav", "TENNIS");
		$menu = Menu(screen.main);
		$menu.back = @() xraft.application().exit();
		$menu.items = '(
			Menu.Item("  1P vs COM  ", @{}, @{
				$sound.stop();
				screen.transit(StageMenu(screen, "data/main-background.jpg", "data/main-background.wav", "1P vs COM", @(player0, player1) {
					$main.screen__(Match($main, false, false, controller0, player0.path, computer, player1.path));
				}[$]), -1.0);
			}[$]),
			Menu.Item("  1P vs 2P   ", @{}, @{
				$sound.stop();
				screen.transit(StageMenu(screen, "data/main-background.jpg", "data/main-background.wav", "1P vs 2P", @(player0, player1) {
					$main.screen__(Match($main, true, false, controller0, player0.path, controller1, player1.path));
				}[$]), -1.0);
			}[$]),
			Menu.Item(" COM vs COM  ", @{}, @{
				$sound.stop();
				screen.transit(StageMenu(screen, "data/main-background.jpg", "data/main-background.wav", "COM vs COM", @(player0, player1) {
					$main.screen__(Match($main, false, true, computer, player0.path, computer, player1.path));
				}[$]), -1.0);
			}[$]),
			Menu.Item("  TRAINING   ", @{}, @{
				$sound.stop();
				screen.transit(StageMenu(screen, "data/main-background.jpg", "data/training-background.wav", "TRAINING", @(player0, player1) {
					$main.screen__(Training($main, controller0, player0.path, player1.path));
				}[$]), -1.0);
			}[$]),
			Menu.Item("    EXIT     ", @{}, $menu.back)
		);
	};
	$step = @{};
	$render = @(width, height, viewing) {
		:$^render[$](width, height, viewing);
		$menu.render((viewing * $main.text_scale).translate(0.0, 0.125, 0.0));
	};
	$key_press = @(key) $menu.key_press(key);
};

MainScreen = Class() :: @{
	load = @(source) {
		reader = collada.Reader(source);
		try {
			reader.read_next();
			reader.move_to_tag();
			reader.check_start_element("players");
			reader.read_next();
			players = [];
			while (reader.is_start_element("player")) {
				player = Object();
				player.name = reader.get_attribute("name");
				player.path = (io.Path(source) / ".." / reader.get_attribute("path")).__string();
				reader.read_element_text();
				players.push(player);
			}
			reader.end_element();
			players;
		} finally {
			reader.free();
		}
	};

	$__initialize = @(main) {
		$main = main;
		$players = load((io.Path(system.script) / "../data/players").__string());
		$dialog = MainMenu($);
		$transit_from = null;
	};
	$destroy = @{
		if ($transit_from !== null) $transit_from.destroy();
		$dialog.destroy();
	};
	$step = @{
		if ($transit_from !== null) {
			if ($t < $duration) {
				$t = $t + 1.0;
			} else {
				$transit_from.destroy();
				$transit_from = null;
			}
		}
		$dialog.step();
	};
	$render = @(width, height) {
		gl.viewport(0, 0, width, height);
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
		gl.disable(gl.DEPTH_TEST);
		viewing = Matrix4(1.0);
		if ($transit_from !== null) {
			a = 2.0 * $direction * width / height;
			t = $t / $duration;
			$transit_from.render(width, height, Matrix4(viewing).translate(a * t, 0.0, 0.0));
			viewing = Matrix4(viewing).translate(a * (t - 1.0), 0.0, 0.0);
		}
		$dialog.render(width, height, viewing);
	};
	$key_press = @(modifier, key, ascii) $transit_from === null && $dialog.key_press(key);
	$key_release = @(modifier, key, ascii) {};
	$transit = @(dialog, direction) {
		$transit_from = $dialog;
		$dialog = dialog;
		$direction = direction;
		$duration = 30.0;
		$t = 0.0;
	};
};

Game = Class(xraft.GLWidget) :: @{
	$on_create = @{
		$glcontext.make_current($);
		$shaders = glshaders();
		$font = glimage.Font();
		gl.enable(gl.CULL_FACE);
		$on_move();
		$screen = MainScreen($);
		$timer.start(1000 / 60);
	};
	$on_move = @{
		extent = $geometry();
		width = extent.width();
		height = extent.height();
		w = Float(width) / height;
		$projection = Matrix4().orthographic(-w, w, -1.0, 1.0, -1.0, 1.0).bytes;
		$text_scale = width < height ? Matrix4().scale(w, w, 1.0) : Matrix4().scale(1.0, 1.0, 1.0);
	};
	$on_paint = @(g) {
t0 = time.now();
		$glcontext.make_current($);
		extent = $geometry();
		$screen.render(extent.width(), extent.height());
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
		buffer = $alcontext.get_device().create_buffer_from_file(path);
		source = $alcontext.create_source();
		source.set_buffer(buffer);
		source__delete = source.delete;
		source.delete = @{
			source__delete();
			buffer.delete();
		};
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
$last = time.now();
		$timer = xraft.Timer(@{
if (print_time) print("since last: " + (time.now() - $last));
$last = t0 = time.now();
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
	});
});
