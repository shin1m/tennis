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

MainMenu = null;

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
		xraft.Key.ESCAPE: @() $transit_back()
	}, {});
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
	$transit_back = @() $main.screen__(MainMenu($main));
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
};

Training = Class(stage.Stage) :: @{
	$ball_in = @{
		$mark.mark($ball);
		if ($ball.hitter !== $player0) return;
		$text_viewing = Matrix4().scale(0.25, 0.25, 1.0);
		$message = ["IN"];
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
		$selected = 0;
		$camera0.position = Vector3(0.0, 14.0, 0.0);
		$camera0.toward = Vector3(0.0, -12.0, -40.0 * $player0.end);
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
	};
	$toss = @(shot) {
		$player1.placement.toward = $player1.shot_direction();
		$player1.placement.valid = false;
		$player1.motion = Player.Motion($player1.actions.swing.(shot));
		$player1.transit(Player.state_swing);
	};
	toss_message = [
		"  CHANGE SIDES: START",
		"      POSITION: +    ",
		"COURCE & SWING: + & *"
	];
	Item = @(label, reset, do, ready, play, back) {
		o = Object();
		o.label = label;
		o.reset = reset;
		o.do = do;
		o.ready = ready;
		o.play = play;
		o.back = back;
		o;
	};
	items = '(
		Item(" SERVE   ", @{
			$ball.reset($side, 2 * 12 * 0.0254 * $side, 0.875, 39 * 12 * 0.0254);
			$mark.duration = 0.0;
			$player0.reset(1.0, Player.state_serve_set);
			$player1.placement.position = Vector3(-9 * 12 * 0.0254 * $side, 0.0, -39 * 12 * 0.0254);
			$player1.placement.valid = false;
			$player1.reset(-1.0, Player.state_default);
		}, @() $transit_ready(), @{
			$text_viewing = Matrix4().translate(0.0, -0.625, 0.0).scale(0.125, 0.125, 1.0);
			$message = [
				"CHANGE SIDES: START",
				"    POSITION: < + >",
				"        TOSS:   *  ",
				"      COURCE: < + >",
				"       SWING:   *  "
			];
			$duration = 0.0 * 64.0;
		}, @{}, @() $transit_select()),
		Item(" STROKE  ", @{
			$reset(3 * 12 * 0.0254 * $side, 1.0, -39 * 12 * 0.0254, Vector3((0.0 - 3.2 * $side) * 12 * 0.0254, 0.0, 39 * 12 * 0.0254), 'toss);
		}, @() $transit_ready(), @{
			$text_viewing = Matrix4().translate(0.0, -0.75, 0.0).scale(0.125, 0.125, 1.0);
			$message = toss_message;
			$duration = 0.5 * 64.0;
		}, @() $toss('toss), @() $transit_select()),
		Item(" VOLLEY  ", @{
			$reset(3 * 12 * 0.0254 * $side, 1.0, -39 * 12 * 0.0254, Vector3((0.1 - 2.0 * $side) * 12 * 0.0254, 0.0, 13 * 12 * 0.0254), 'toss);
		}, @() $transit_ready(), @{
			$text_viewing = Matrix4().translate(0.0, -0.75, 0.0).scale(0.125, 0.125, 1.0);
			$message = toss_message;
			$duration = 0.5 * 64.0;
		}, @() $toss('toss), @() $transit_select()),
		Item(" SMASH   ", @{
			$reset(3 * 12 * 0.0254 * $side, 1.0, -39 * 12 * 0.0254, Vector3((0.4 - 0.4 * $side) * 12 * 0.0254, 0.0, 11.5 * 12 * 0.0254), 'toss_lob);
		}, @() $transit_ready(), @{
			$text_viewing = Matrix4().translate(0.0, -0.75, 0.0).scale(0.125, 0.125, 1.0);
			$message = toss_message;
			$duration = 0.5 * 64.0;
		}, @() $toss('toss_lob), @() $transit_select()),
		Item("  BACK   ", @{
			$ball.reset($side, 2 * 12 * 0.0254, $ball.radius + 0.01, 2 * 12 * 0.0254);
			$mark.duration = 0.0;
			$player0.placement.position = Vector3((0.1 - 2.0 * $side) * 12 * 0.0254, 0.0, 13 * 12 * 0.0254);
			$player0.placement.valid = false;
			$player0.reset(1.0, Player.state_default);
			$player1.placement.position = Vector3((0.1 + 2.0 * $side) * 12 * 0.0254, 0.0, -13 * 12 * 0.0254);
			$player1.placement.valid = false;
			$player1.reset(-1.0, Player.state_default);
		}, @() $transit_back(), @{}, @{}, @() $back())
	);
	$update_select = @{
		$text_viewing = Matrix4().translate(0.0, 0.0, 0.0).scale(0.25, 0.25, 1.0);
		$message = [];
		items.each(@(item) {
			$message.push(($message.size() == $selected ? "*" : " ") + item.label);
		}[$]);
		$step_things();
	};
	$back = @() $main.screen__(MainMenu($main));
	$up = @{
		if ($selected <= 0) return;
		$selected = $selected - 1;
		$main.sound_cursor.play();
		items[$selected].reset[$]();
		$update_select();
	};
	$down = @{
		if ($selected >= items.size() - 1) return;
		$selected = $selected + 1;
		$main.sound_cursor.play();
		items[$selected].reset[$]();
		$update_select();
	};
	$select = @{
		$main.sound_select.play();
		items[$selected].do[$]();
	};
	state_select = $State(@{}, {
		xraft.Key.ESCAPE: $back,
		xraft.Key.SPACE: $select,
		xraft.Key.D2: $select,
		xraft.Key.UP: $up,
		xraft.Key.DOWN: $down,
		xraft.Key.E: $up,
		xraft.Key.C: $down
	}, {});
	$transit_back = @() items[$selected].back[$]();
	$transit_select = @{
		$state = state_select;
		$side = 1.0;
		items[$selected].reset[$]();
		$duration = 0.0 * 64.0;
		$update_select();
	};
	$transit_ready = @{
		$state = $state_ready;
		items[$selected].reset[$]();
		items[$selected].ready[$]();
		$step_things();
	};
	$transit_play = @{
		$state = $state_play;
		items[$selected].play[$]();
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
	$Item = @(label, do) {
		o = Object();
		o.label = label;
		o.do = do;
		o;
	};

	$__initialize = @(main, background, sound) {
		$main = main;
		$background = glimage.Renderer.from_file((io.Path(system.script) / ".." / background).__string(), 1);
		$sound_background = main.load_sound(sound);
		$sound_background.setb(al.LOOPING, true);
		$sound_background.play();
		$action = null;
		$duration = 0.0;
	};
	$destroy = @{
		$background.destroy();
		$sound_background.delete();
	};
	$flush = @{
		$action[$]();
		$action = null;
	};
	$step = @{
		if ($action === null) return;
		if ($duration <= 0.0) return $flush();
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
		viewing = Matrix4().translate(0.0, 0.5, 0.0).scale(1.5 / 4.0, 1.5 / 4.0, 1.0).translate($title.size() * -0.25, 0.0, 0.0).bytes;
		$main.text_renderer($main.text_projection, viewing, $title);
	};
	$transit = @(action, duration) {
		if ($action !== null) $flush();
		$action = action;
		$duration = duration;
	};
	$key_press = @(modifier, key, ascii) $action === null && $keys.has(key) && $keys[key][$]();
	$key_release = @(modifier, key, ascii) {};
};

PlayersSelector = Class(Dialog) :: @{
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

	Item = $Item;
	StateMain = Class() :: @{
		$__initialize = @(parent, player0, player1) {
			$main = parent.main;
			$parent = parent;
			$items = '(
				Item(player0, @() $transit(@{
					$state = $state_player0;
				}, 0.0 * 64.0)),
				Item(player1, @() $transit(@{
					$state = $state_player1;
				}, 0.0 * 64.0)),
				Item("NEXT", @() $transit(@{
					$next($players[$state_player0.selected], $players[$state_player1.selected]);
				}, 0.0 * 64.0)),
				Item("BACK", @() $transit(@{
					$main.screen__(MainMenu($main));
				}, 0.0 * 64.0))
			);
			$selected = 0;
		};
		$left = @{
			if ($selected <= 0) return;
			$selected = $selected - 1;
			$main.sound_cursor.play();
		};
		$right = @{
			if ($selected >= $items.size() - 1) return;
			$selected = $selected + 1;
			$main.sound_cursor.play();
		};
		$up = $left;
		$down = $right;
		$select = @{
			$main.sound_select.play();
			$items[$selected].do[$parent]();
		};
	};
	StatePlayer = Class() :: @{
		$__initialize = @(parent, selected) {
			$main = parent.main;
			$parent = parent;
			$selected = selected;
		};
		$left = @{
			if ($selected <= 0) return;
			$selected = $selected - 1;
			$main.sound_cursor.play();
		};
		$right = @{
			if ($selected >= $parent.players.size() - 1) return;
			$selected = $selected + 1;
			$main.sound_cursor.play();
		};
		$up = @{
			if ($selected < 4) return;
			$selected = $selected - 4;
			$main.sound_cursor.play();
		};
		$down = @{
			if ($selected >= $parent.players.size() - 4) return;
			$selected = $selected + 4;
			$main.sound_cursor.play();
		};
		$select = @{
			$main.sound_select.play();
			$parent.state = $parent.state_main;
		};
	};

	$__initialize = @(main, title, background, sound, next) {
		:$^__initialize[$](main, background, sound);
		$title = title;
		$next = next;
		$players = load((io.Path(system.script) / "../data/players").__string());
		$state_main = StateMain($, "1:", "2:");
		$state_player0 = StatePlayer($, 0);
		$state_player1 = StatePlayer($, 1);
		$state = $state_main;
	};
	current = @(text) " " + text + "?";
	selected = @(text) "[" + text + "]";
	normal = @(text) " " + text + " ";
	$render = @{
		:$^render[$]();
		player0 = ($state === $state_player0 ? current : $state_main.selected == 0 ? selected : normal)($state_main.items[0].label + $players[$state_player0.selected].name);
		player1 = ($state === $state_player1 ? current : $state_main.selected == 1 ? selected : normal)($state_main.items[1].label + $players[$state_player1.selected].name);
		text = player0 + player1 + ($state_main.selected == 2 ? selected : normal)($state_main.items[2].label) + ($state_main.selected == 3 ? selected : normal)($state_main.items[3].label);
		viewing = Matrix4().translate(0.0, 0.125, 0.0).scale(2.5 / 16.0, 2.5 / 16.0, 1.0).translate(text.size() * -0.25, 0.0, 0.0).bytes;
		$main.text_renderer($main.text_projection, viewing, text);
		viewing = Matrix4().translate(0.0, -0.25, 0.0);
		i = 0;
		x = -3.0 / 4.0;
		y = 0.0;
		$players.each(@(player) {
			if ($state === $state_player0 && i == $state_player0.selected || $state === $state_player1 && i == $state_player1.selected) {
				text = selected(player.name);
			} else {
				c0 = i == $state_player0.selected ? ">" : " ";
				c1 = i == $state_player1.selected ? "<" : " ";
				text = c0 + player.name + c1;
			}
			viewing = Matrix4(:viewing).translate(x, y, 0.0).scale(2.5 / 16.0, 2.5 / 16.0, 1.0).translate(text.size() * -0.25, 0.0, 0.0).bytes;
			$main.text_renderer($main.text_projection, viewing, text);
			:i = i + 1;
			if ((i % 4) == 0) {
				:x = -3.0 / 4.0;
				:y = y - 1.0 / 4.0;
			} else {
				:x = x + 2.0 / 4.0;
			}
		}[$]);
	};
	$transit_back = @() $main.screen__(MainMenu($main));
	$keys = {
		xraft.Key.ESCAPE: @() $transit_back(),
		xraft.Key.SPACE: @() $state.select(),
		xraft.Key.D2: @() $state.select(),
		xraft.Key.LEFT: @() $state.left(),
		xraft.Key.RIGHT: @() $state.right(),
		xraft.Key.UP: @() $state.up(),
		xraft.Key.DOWN: @() $state.down(),
		xraft.Key.S: @() $state.left(),
		xraft.Key.F: @() $state.right(),
		xraft.Key.E: @() $state.up(),
		xraft.Key.C: @() $state.down()
	};
};

Menu = Class(Dialog) :: @{
	$__initialize = @(main, background, sound) {
		:$^__initialize[$](main, background, sound);
		$selected = 0;
	};
	$render = @{
		:$^render[$]();
		viewing = Matrix4().translate(0.0, -0.125, 0.0).scale(3.0 / 16.0, 3.0 / 16.0, 1.0);
		i = 0;
		y = 1.0;
		$items.each(@(item) {
			text = (i == $selected ? "*" : " ") + item.label;
			viewing = Matrix4(:viewing).translate(text.size() * -0.25, y, 0.0).bytes;
			$main.text_renderer($main.text_projection, viewing, text);
			:y = y - 1.0;
			:i = i + 1;
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
	$keys = {
		xraft.Key.ESCAPE: @() $transit_back(),
		xraft.Key.SPACE: $select,
		xraft.Key.D2: $select,
		xraft.Key.UP: $up,
		xraft.Key.DOWN: $down,
		xraft.Key.E: $up,
		xraft.Key.C: $down
	};
};

MainMenu = Class(Menu) :: @{
	$title = "TENNIS";
	$items = '(
		$Item("  1P vs COM  ", @() $transit(@{
			$main.screen__(PlayersSelector($main, "1P vs COM", "data/main-background.jpg", "data/main-background.wav", @(player0, player1) {
				$main.screen__(Match($main, false, false, controller0, player0.path, computer, player1.path));
			}[$]));
		}, 0.0 * 64.0)),
		$Item("  1P vs 2P   ", @() $transit(@{
			$main.screen__(PlayersSelector($main, "1P vs 2P", "data/main-background.jpg", "data/main-background.wav", @(player0, player1) {
				$main.screen__(Match($main, true, false, controller0, player0.path, controller1, player1.path));
			}[$]));
		}, 0.0 * 64.0)),
		$Item(" COM vs COM  ", @() $transit(@{
			$main.screen__(PlayersSelector($main, "COM vs COM", "data/main-background.jpg", "data/main-background.wav", @(player0, player1) {
				$main.screen__(Match($main, false, true, computer, player0.path, computer, player1.path));
			}[$]));
		}, 0.0 * 64.0)),
		$Item("  TRAINING   ", @() $transit(@{
			$main.screen__(PlayersSelector($main, "TRAINING", "data/main-background.jpg", "data/training-background.wav", @(player0, player1) {
				$main.screen__(Training($main, controller0, player0.path, player1.path));
			}[$]));
		}, 0.0 * 64.0)),
		$Item("    EXIT     ", @() xraft.application().exit())
	);

	$__initialize = @(main) :$^__initialize[$](main, "data/main-background.jpg", "data/main-background.wav");
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
		$timer.start(1000 / 60);
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
