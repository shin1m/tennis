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

Container = Class() :: @{
	$__initialize = @{
		$content = null;
		$transit_from = null;
	};
	$destroy = @{
		if ($transit_from !== null && $transit_from.?destroy) $transit_from.destroy();
		if ($content !== null && $content.?destroy) $content.destroy();
	};
	$step = @{
		if ($transit_from !== null) {
			if ($t < $duration) {
				$t = $t + 1.0;
			} else {
				if ($transit_from.?destroy) $transit_from.destroy();
				$transit_from = null;
			}
		}
		$content.step();
	};
	$render = @(viewing) {
		if ($transit_from !== null) {
			t = $t / $duration;
			$transit_from.render(Matrix4(viewing).translate($slide * t, 0.0, 0.0));
			viewing = Matrix4(viewing).translate($slide * (t - 1.0), 0.0, 0.0);
		}
		if ($content !== null) $content.render(viewing);
	};
	$key_press = @(key) $transit_from === null && $content !== null && $content.key_press(key);
	$transit = @(content, slide) {
		$transit_from = $content;
		$content = content;
		$slide = slide;
		$duration = 30.0;
		$t = 0.0;
	};
};

Menu = Class() :: @{
	$Item = @(label, do) {
		o = Object();
		o.label = label;
		o.do = do;
		o;
	};

	$__initialize = @(main, columns = 1) {
		$main = main;
		$columns = columns;
		$selected = 0;
	};
	$step = @{};
	$render = @(viewing) {
		n = ($items.size() + $columns - 1) / $columns;
		scale = 1.0 / (n < 5 ? 5 : n);
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
		xraft.Key.RETURN: $do,
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

	$__initialize = @(main, dual, fixed, controller0, player0, controller1, player1, back) {
		:$^__initialize[$](main, dual, fixed, controller0, player0, controller1, player1);
		$back = back;
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
};

Training = Class(stage.Stage) :: @{
	Item = @(label, do, ready, play) {
		o = Menu.Item(label, do);
		o.ready = ready;
		o.play = play;
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

	$__initialize = @(main, controller0, player0, player1, back) {
		:$^__initialize[$](main, true, false, controller0, player0, @(controller, player) {
			super__step = controller.step[$];
			controller.step = @{
				if (player.state !== Player.state_swing) player.left = player.right = player.forward = player.backward = false;
				super__step();
			};
		}, player1);
		{
			xraft.Key.RETURN: @{
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
		$menu.back = back;
		toss_message = [
			"  CHANGE SIDES: START",
			"      POSITION:   +  ",
			"COURCE & SWING: + & *",
			"",
			"          LOB        ",
			"   TOPSPIN * FLAT    ",
			"         SLICE       "
		];
		$menu.items = '(
			Item(" SERVE ", (@() $transit_ready())[$], @{
				$ball.reset($side, 2 * 12 * 0.0254 * $side, 0.875, 39 * 12 * 0.0254);
				$mark.duration = 0.0;
				$player0.reset(1.0, Player.state_serve_set);
				$player1.placement.position = Vector3(-9 * 12 * 0.0254 * $side, 0.0, -39 * 12 * 0.0254);
				$player1.placement.valid = false;
				$player1.reset(-1.0, Player.state_default);
				$step_things();
				$text_viewing = Matrix4().translate(0.0, -0.5, 0.0).scale(1.5 / 16.0, 1.5 / 16.0, 1.0);
				$message = [
					"CHANGE SIDES: START",
					"    POSITION: < + >",
					"        TOSS:   *  ",
					"      COURCE: < + >",
					"       SWING:   *  ",
					"",
					"       SECOND      ",
					"    SPIN * FLAT    ",
					"       SLICE       "
				];
				$duration = 0.0 * 64.0;
			}[$], @{}),
			Item(" STROKE", (@() $transit_ready())[$], @{
				$reset(3 * 12 * 0.0254 * $side, 1.0, -39 * 12 * 0.0254, Vector3((0.0 - 3.2 * $side) * 12 * 0.0254, 0.0, 39 * 12 * 0.0254), 'toss);
				$text_viewing = Matrix4().translate(0.0, -0.5, 0.0).scale(1.5 / 16.0, 1.5 / 16.0, 1.0);
				$message = toss_message;
				$duration = 0.5 * 64.0;
			}[$], (@() $toss('toss))[$]),
			Item(" VOLLEY", (@() $transit_ready())[$], @{
				$reset(3 * 12 * 0.0254 * $side, 1.0, -39 * 12 * 0.0254, Vector3((0.1 - 2.0 * $side) * 12 * 0.0254, 0.0, 13 * 12 * 0.0254), 'toss);
				$text_viewing = Matrix4().translate(0.0, -0.5, 0.0).scale(1.5 / 16.0, 1.5 / 16.0, 1.0);
				$message = toss_message;
				$duration = 0.5 * 64.0;
			}[$], (@() $toss('toss))[$]),
			Item(" SMASH ", (@() $transit_ready())[$], @{
				$reset(3 * 12 * 0.0254 * $side, 1.0, -39 * 12 * 0.0254, Vector3((0.4 - 0.4 * $side) * 12 * 0.0254, 0.0, 9 * 12 * 0.0254), 'toss_lob);
				$text_viewing = Matrix4().translate(0.0, -0.5, 0.0).scale(1.5 / 16.0, 1.5 / 16.0, 1.0);
				$message = toss_message;
				$duration = 0.5 * 64.0;
			}[$], (@() $toss('toss_lob))[$])
		);
		keys = {};
		Menu.keys.each(@(key, value) keys[key] = @() value[$menu]());
		$state_select = $State(@{}, keys, {}, @(width, height) $menu.render(Matrix4($main.text_scale).translate(0.0, 0.5, 0.0)));
		$transit_select();
	};
	$next = @() $ball.done && $transit_ready();
	$reset = @(x, y, z, position, shot) {
		$ball.reset($side, x, y, z, false);
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
	$back = @() $transit_select();
	$transit_play = @{
		$state = $state_play;
		$menu.items[$menu.selected].play();
	};
	$transit_select = @{
		$state = $state_select;
		$side = 1.0;
		$ball.reset($side, 2 * 12 * 0.0254, $ball.radius + 0.01, 2 * 12 * 0.0254);
		$mark.duration = 0.0;
		$player0.placement.position = Vector3((0.1 - 2.0 * $side) * 12 * 0.0254, 0.0, 13 * 12 * 0.0254);
		$player0.placement.valid = false;
		$player0.reset(1.0, Player.state_default);
		$player1.placement.position = Vector3((0.1 + 2.0 * $side) * 12 * 0.0254, 0.0, -13 * 12 * 0.0254);
		$player1.placement.valid = false;
		$player1.reset(-1.0, Player.state_default);
		$step_things();
		$message = [];
		$duration = 0.0 * 64.0;
	};
	$transit_ready = @{
		$state = $state_ready;
		selected = $menu.items[$menu.selected];
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

Background = Class(Container) :: @{
	$__initialize = @(main, image, sound) {
		:$^__initialize[$]();
		$main = main;
		$image = glimage.Image((io.Path(system.script) / ".." / image).__string());
		$sound = main.load_sound(sound);
		$sound.setb(al.LOOPING, true);
		$sound.play();
	};
	$destroy = @{
		$sound.delete();
		$image.destroy();
	};
	$render = @(viewing) {
		w = $main.aspect;
		a = w * $image.height / $image.width;
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
		:$^render[$](viewing);
	};
};

Titled = Class(Container) :: @{
	$__initialize = @(main, title) {
		:$^__initialize[$]();
		$main = main;
		$title = title;
	};
	$render = @(viewing) {
		:$^render[$](viewing);
		v = (viewing * $main.text_scale).translate(0.0, 0.5, 0.0).scale(1.125 / 4.0, 1.125 / 4.0, 1.0).translate($title.size() * -0.25, 0.0, 0.0).bytes;
		$main.font($main.projection, v, $title);
	};
};

StageMenu = Class(Titled) :: @{
	$__initialize = @(screen, title, done, back) {
		:$^__initialize[$](screen.main, title);
		$player0 = Menu(screen.main, 2);
		$player0.back = back;
		$player0.items = [];
		$player1 = Menu(screen.main, 2);
		$player1.back = (@() $transit($player0, 2.0 * $main.aspect))[$];
		$player1.items = [];
		screen.players.each(@(x) {
			$player0.items.push(Menu.Item(x.name, (@() $transit($player1, -2.0 * $main.aspect))[$]));
			$player1.items.push(Menu.Item(x.name, (@() $transit($ready, -2.0 * $main.aspect))[$]));
		}[$]);
		$ready = Menu(screen.main);
		$ready.back = (@() $transit($player1, 2.0 * $main.aspect))[$];
		$ready.items = '(Menu.Item("START", (@() done(screen.players[$player0.selected], screen.players[$player1.selected]))[$]));
		$content = $player0;
	};
	$render = @(viewing) {
		:$^render[$](viewing);
		message = $player0.items[$player0.selected].label;
		if ($content === $player0) message = message + "?";
		message = message + " vs " + $player1.items[$player1.selected].label;
		if ($content === $player1) message = message + "?";
		v = (viewing * $main.text_scale).translate(0.0, 0.25, 0.0).scale(1.0 / 4.0, 1.0 / 4.0, 1.0).translate(message.size() * -0.25, -0.5, 0.0).bytes;
		$main.font($main.projection, v, message);
	};
};

MainMenu = Class(Titled) :: @{
	$__initialize = @(screen, back) {
		:$^__initialize[$](screen.main, "TENNIS");
		back2this = @() screen.container.transit(MainMenu(screen, back), 2.0 * screen.main.aspect);
		$content = Menu(screen.main);
		$content.back = @() xraft.application().exit();
		$content.items = '(
			Menu.Item("  1P vs COM  ", @{
				screen.container.transit(StageMenu(screen, "1P vs COM", @(player0, player1) {
					$main.screen__(Match($main, false, false, controller0, player0.path, computer, player1.path, back));
				}[$], back2this), -2.0 * $main.aspect);
			}[$]),
			Menu.Item("  1P vs 2P   ", @{
				screen.container.transit(StageMenu(screen, "1P vs 2P", @(player0, player1) {
					$main.screen__(Match($main, true, false, controller0, player0.path, controller1, player1.path, back));
				}[$], back2this), -2.0 * $main.aspect);
			}[$]),
			Menu.Item(" COM vs COM  ", @{
				screen.container.transit(StageMenu(screen, "COM vs COM", @(player0, player1) {
					$main.screen__(Match($main, false, true, computer, player0.path, computer, player1.path, back));
				}[$], back2this), -2.0 * $main.aspect);
			}[$]),
			Menu.Item("  TRAINING   ", @{
				screen.container.transit(StageMenu(screen, "TRAINING", @(player0, player1) {
					$main.screen__(Training($main, controller0, player0.path, player1.path, back));
				}[$], back2this), -2.0 * $main.aspect);
			}[$])
		);
	};
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
		$container = Background(main, "data/main-background.jpg", "data/main-background.wav");
		$container.content = MainMenu($, @() main.screen__(MainScreen(main)));
	};
	$destroy = @() $container.destroy();
	$step = @() $container.step();
	$render = @(width, height) {
		gl.viewport(0, 0, width, height);
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
		gl.disable(gl.DEPTH_TEST);
		$container.render(Matrix4(1.0));
	};
	$key_press = @(modifier, key, ascii) $container.key_press(key);
	$key_release = @(modifier, key, ascii) {};
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
		$aspect = w;
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
