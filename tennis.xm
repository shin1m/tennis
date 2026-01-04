system = Module("system"
print = system.error.write_line
os = Module("os"
time = Module("time"
gl = Module("gl"
glmatrix = Module("glmatrix"
glshaders = Module("glshaders"
glimage = Module("glimage"
al = Module("al"
skia = Module("skia"
suisha = Module("suisha"
xade = Module("xade"
collada = Module("collada"
placement = Module("placement"
ball = Module("ball"
player = Module("player"
stage = Module("stage"
computer = Module("computer").main

VOLUMEUP = 0x1008FF13
VOLUMEDOWN = 0x1008FF11

print_time = false

Matrix4 = glmatrix.Matrix4
Vector3 = glmatrix.Vector3
Posture = placement.Posture
Player = player.Player

Container = Object + @
	$content
	$transit_from
	$slide
	$duration
	$t
	$destroy = @
		$transit_from && $transit_from.?destroy && $transit_from.destroy(
		$content && $content.?destroy && $content.destroy(
	$step = @
		if $transit_from
			if $t < $duration
				$t = $t + 1
			else
				$transit_from.?destroy && $transit_from.destroy(
				$transit_from = null
		$content.step(
	$render = @(viewing)
		if $transit_from
			t = Float($t) / $duration
			$transit_from.render(Matrix4(viewing).translate($slide * t, 0.0, 0.0
			viewing = Matrix4(viewing).translate($slide * (t - 1.0), 0.0, 0.0
		$content && $content.render(viewing
	$key_press = @(key) $transit_from || $content && $content.key_press(key
	$transit = @(content, slide)
		$transit_from = $content
		$content = content
		$slide = slide
		$duration = 30
		$t = 0

Menu = Object + @
	$Item = Object + @
		$label
		$do
		$__initialize = @(label, do)
			$label = label
			$do = do

	$main
	$columns
	$selected
	$back
	$items
	$__initialize = @(main, columns = 1)
		$main = main
		$columns = columns
		$selected = 0
	$step = @
	$render = @(viewing)
		n = ($items.size() + $columns - 1) / $columns
		scale = 1.0 / (n < 5 ? 5 : n)
		viewing = Matrix4(viewing).scale(scale, scale, 1.0
		dx = 4.0 * 2.0 / $columns
		x = dx * 0.5 - 4.0
		y = 0.0
		i = 0
		$items.each((@(item)
			text = (i == $selected ? "*" : " ") + item.label
			:y = y - 1.0
			viewing = Matrix4(:viewing).translate(x - text.size() * 0.25, y, 0.0).bytes
			$main.font($main.projection, viewing, text
			:i = i + 1
			i % n == 0 || return
			:x = x + dx
			:y = 0.0
		)[$]
	$select = @(i)
		$selected = i
		$main.sound_cursor.play(
	$up = @ $selected > 0 && $select($selected - 1
	$down = @ $selected < $items.size() - 1 && $select($selected + 1
	$left = @
		n = ($items.size() + $columns - 1) / $columns
		$selected >= n && $select($selected - n
	$right = @
		n = ($items.size() + $columns - 1) / $columns
		$selected < $items.size() - n && $select($selected + n
	$do = @
		$main.sound_select.play(
		$items[$selected].do(
	$keys = keys = {
		xade.Key.ESCAPE: @ $back(
		xade.Key.RETURN: @ $do(
		xade.Key.SPACE: @ $do(
		xade.Key.KP_2: @ $do(
		xade.Key.LEFT: @ $left(
		xade.Key.RIGHT: @ $right(
		xade.Key.UP: @ $up(
		xade.Key.DOWN: @ $down(
		xade.Key.s: @ $left(
		xade.Key.f: @ $right(
		xade.Key.e: @ $up(
		xade.Key.c: @ $down(
	$key_press = @(key) keys.has(key) && keys[key][$](

Match = stage.Stage + @
	$back
	$records
	$closed
	$second
	$end
	$server
	$receiver
	$side
	$state
	$message
	$duration
	$new_game = @
		$player0.point = $player1.point = 0
		$second = false
		games = $player0.game + $player1.game
		$end = games % 4 < 2 ? 1.0 : -1.0
		$server = games % 2 == 0 ? $player0 : $player1
		$receiver = games % 2 == 0 ? $player1 : $player0
	$point = @(player)
		player.point = player.point + 1
		(player.point < 4 || player.point - player.opponent.point < 2) && return
		player.game = player.game + 1
		if player.game < 6 || player.game - player.opponent.game < 2
			$new_game(
		else
			$closed = true
	$ball_ace = @
		$second = false
		$point($ball.hitter
		$transit_replay(
		$sound_ace.play(
	$ball_let = @
		$mark.mark($ball
		$message = ["LET"
		$duration = 2 * 64
	$serve_miss = @
		if $second
			$message = ["DOUBLE FAULT"
			$second = false
			$point($receiver
		else
			$message = ["FAULT"
			$second = true
		$duration = 2 * 64
		$sound_miss.play(
	$ball_serve_air = @ $serve_miss(
	$miss = @(message)
		$message = [message
		$second = false
		$point($ball.hitter.opponent
		$duration = 2 * 64
		$sound_miss.play(
	points0 = [" 0", "15", "30", "40"
	points1 = ["0 ", "15", "30", "40"
	$transit_ready = @
		$reset(
		$state = $state_ready
		if $player0.point + $player1.point < 6
			game = points0[$player0.point] + " - " + points1[$player1.point]
		else if $player0.point > $player1.point
			game = " A -   "
		else if $player0.point < $player1.point
			game = "   - A "
		else
			game = " DEUCE "
		$message = [
			"P1 " + $player0.game + " - " + $player1.game + " P2"
			($player0 === $server ? "* " : "  ") + game + ($player1 === $server ? " *" : "  ")
		$duration = 64
		$step_things(
	state_close = stage.Stage.State(@
		$step_things(
	, {
		xade.Key.RETURN: @ $new_set(
		xade.Key.ESCAPE: @ $back(
	}, {}, @(width, height)
	$transit_close = @
		$player0.reset(
		$player1.reset(
		$reset_cameras(
		$set_cameras(
		$state = state_close
		$message = [
			($player0.game > $player1.game ? "P1" : "P2") + " WON!"
			"P1 " + $player0.game + " - " + $player1.game + " P2"
			"PRESS START"
			"TO PLAY AGAIN"
		$sound_ace.play(
	$transit_play = @
		$state = $state_play
		$message = [
	state_replay = stage.Stage.State(@
		$duration > 0 || return $next(
		$duration = $duration - 1
		$duration % 2 == 0 && return
		record = $records.shift(
		$ball.replay(record.ball
		$mark.replay(record.mark
		$player0.replay(record.player0
		$player1.replay(record.player1
		$records.push(record
		$camera0.position = Vector3(($ball.position.x + $ball.hitter.root_position().x) * 0.5, 4.0, ($ball.position.z + 40.0 * $ball.hitter.opponent.end) * 0.5
		$camera0.toward = Vector3(0.0, -6.0, -40.0 * $ball.hitter.opponent.end
		$camera1.position = Vector3(($ball.position.x + $ball.hitter.opponent.root_position().x) * 0.5, 4.0, ($ball.position.z + 40.0 * $ball.hitter.end) * 0.5
		$camera1.toward = Vector3(0.0, -6.0, -40.0 * $ball.hitter.end
	, {
		xade.Key.RETURN: @ $next(
		xade.Key.ESCAPE: @ $back(
	}, {}, @(width, height)
	$transit_replay = @
		$state = state_replay
		$duration = $records.size() * 2
	$reset_cameras = @
		$camera0.position = Vector3(0.0, 14.0, 0.0
		$camera0.toward = Vector3(0.0, -12.0, -40.0 * ($fixed ? 1.0 : $player0.end)
		$camera1.position = Vector3(0.0, 14.0, 0.0
		$camera1.toward = Vector3(0.0, -12.0, -40.0 * ($fixed ? -1.0 : $player1.end)
	$reset = @
		$side = ($player0.point + $player1.point) % 2 == 0 ? 1.0 : -1.0
		$ball.reset($side, 2 * 12 * 0.0254 * $end * $side, 0.875, 39 * 12 * 0.0254 * $end
		$mark.duration = 0
		$server.reset($end, Player.state_serve_set
		$receiver.placement.position = Vector3(-9 * 12 * 0.0254 * $end * $side, 0.0, -39 * 12 * 0.0254 * $end
		$receiver.placement.valid = false
		$receiver.reset(-$end, Player.state_default
		$reset_cameras(
	$step_things = @
		stage.Stage.step_things[$](
		record = $records.shift(
		$ball.record(record.ball
		$mark.record(record.mark
		$player0.record(record.player0
		$player1.record(record.player1
		$records.push(record

	Record = Object + @
		$ball
		$mark
		$player0
		$player1
	$__initialize = @(main, dual, fixed, controller0, player0, controller1, player1, back)
		stage.Stage.__initialize[$](main, dual, fixed, controller0, player0, controller1, player1
		$back = back
		$records = [
		for i = 0; i < 150; i = i + 1
			record = Record(
			record.ball = $ball.create_record(
			record.mark = $mark.create_record(
			record.player0 = $player0.create_record(
			record.player1 = $player1.create_record(
			$records.push(record
		$new_set(
	$new_set = @
		$closed = false
		$player0.game = $player1.game = 0
		$new_game(
		$transit_ready(
	$next = @
		$ball.done || return
		if $second || $ball.serving() && $ball.in && $ball.net
			$reset(
			$message = [
			$step_things(
		else if $closed
			$transit_close(
		else
			$transit_ready(

Training = stage.Stage + @
	Item = Menu.Item + @
		$ready
		$play
		$__initialize = @(label, do, ready, play)
			Menu.Item.__initialize[$](label, do
			$ready = ready
			$play = play

	$menu
	$state_select
	$state
	$side
	$message
	$duration
	$ball_ace = @ $duration = 32
	$ball_let = @
		$mark.mark($ball
		$text_viewing = Matrix4().scale(0.25, 0.25, 1.0
		$message = ["LET"
		$duration = 32
	$serve_miss = @
		$text_viewing = Matrix4().scale(0.25, 0.25, 1.0
		$message = ["FAULT"
		$duration = 32
		$sound_miss.play(
	$ball_serve_air = @ $serve_miss(
	$miss = @(message)
		$text_viewing = Matrix4().scale(0.25, 0.25, 1.0
		$message = [message
		$duration = 32
		$sound_miss.play(
	$ball_in = @
		$mark.mark($ball
		$ball.hitter === $player0 || return
		$text_viewing = Matrix4().scale(0.25, 0.25, 1.0
		$message = ["IN"
	$step_things = @
		stage.Stage.step_things[$](
		$camera1.position = Vector3(($ball.position.x + $player0.root_position().x) * 0.5, 4.0, ($ball.position.z + 40.0 * $player1.end) * 0.5
		$camera1.toward = Vector3(0.0, -6.0, -40.0 * $player1.end

	$__initialize = @(main, controller0, player0, player1, back)
		stage.Stage.__initialize[$](main, true, false, controller0, player0, @(controller, player)
			super__step = controller.step[$]
			controller.step = @
				if player.state !== Player.state_swing
					player.left = player.right = player.forward = player.backward = false
				super__step(
		, player1
		{
			xade.Key.RETURN: @
				$side = -$side
				$transit_ready(
		}.each((@(key, value)
			$state_ready.key_press[key] = value
			$state_play.key_press[key] = value
		)[$]
		$camera0.position = Vector3(0.0, 14.0, 0.0
		$camera0.toward = Vector3(0.0, -12.0, -40.0 * $player0.end
		$menu = Menu(main
		$menu.back = back
		toss_message = [
			"     CHANGE SIDES: START"
			"         POSITION:   +  "
			"PLACEMENT & SWING: + & *"
			""
			"            LOB         "
			"     TOPSPIN * FLAT     "
			"           SLICE        "
		$menu.items = '(
			Item(" SERVE ", (@ $transit_ready())[$], (@
				$ball.reset($side, 2 * 12 * 0.0254 * $side, 0.875, 39 * 12 * 0.0254
				$mark.duration = 0
				$player0.reset(1.0, Player.state_serve_set
				$player1.placement.position = Vector3(-9 * 12 * 0.0254 * $side, 0.0, -39 * 12 * 0.0254
				$player1.placement.valid = false
				$player1.reset(-1.0, Player.state_default
				$step_things(
				$text_viewing = Matrix4().translate(0.0, -0.5, 0.0).scale(1.5 / 16.0, 1.5 / 16.0, 1.0
				$message = [
					"CHANGE SIDES: START"
					"    POSITION: < + >"
					"        TOSS:   *  "
					"   DIRECTION: < + >"
					"       SWING:   *  "
					""
					"       SECOND      "
					"    SPIN * FLAT    "
					"       SLICE       "
				$duration = 0
			)[$], @
			Item(" STROKE", (@ $transit_ready())[$], (@
				$reset(3 * 12 * 0.0254 * $side, 1.0, -39 * 12 * 0.0254, Vector3((0.0 - 3.2 * $side) * 12 * 0.0254, 0.0, 39 * 12 * 0.0254), 'toss
				$text_viewing = Matrix4().translate(0.0, -0.5, 0.0).scale(1.5 / 16.0, 1.5 / 16.0, 1.0
				$message = toss_message
				$duration = 32
			)[$], (@ $toss('toss))[$]
			Item(" VOLLEY", (@ $transit_ready())[$], (@
				$reset(3 * 12 * 0.0254 * $side, 1.0, -39 * 12 * 0.0254, Vector3((0.1 - 2.0 * $side) * 12 * 0.0254, 0.0, 13 * 12 * 0.0254), 'toss
				$text_viewing = Matrix4().translate(0.0, -0.5, 0.0).scale(1.5 / 16.0, 1.5 / 16.0, 1.0
				$message = toss_message
				$duration = 32
			)[$], (@ $toss('toss))[$]
			Item(" SMASH ", (@ $transit_ready())[$], (@
				$reset(3 * 12 * 0.0254 * $side, 1.0, -39 * 12 * 0.0254, Vector3((0.4 - 0.4 * $side) * 12 * 0.0254, 0.0, 9 * 12 * 0.0254), 'toss_lob
				$text_viewing = Matrix4().translate(0.0, -0.5, 0.0).scale(1.5 / 16.0, 1.5 / 16.0, 1.0
				$message = toss_message
				$duration = 32
			)[$], (@ $toss('toss_lob))[$]
		keys = {
		Menu.keys.each(@(key, value) keys[key] = @ value[$menu](
		$state_select = $State(@{}, keys, {}, @(width, height) $menu.render(Matrix4($main.text_scale).translate(0.0, 0.5, 0.0
		$transit_select(
	$next = @ $ball.done && $transit_ready(
	$reset = @(x, y, z, position, shot)
		$ball.reset($side, x, y, z, false
		$mark.duration = 0
		$player0.placement.position = position
		$player0.placement.valid = false
		$player0.reset(1.0, Player.state_default
		$player1.placement.position = $ball.position - Posture().validate() * $player1.actions.swing.(shot).spot * Vector3(0.0, 0.0, 0.0)
		$player1.placement.position.y = 0.0
		$player1.placement.valid = false
		$player1.reset(-1.0, Player.state_default
		$step_things(
	$toss = @(shot)
		$player1.placement.toward = $player1.shot_direction(
		$player1.placement.valid = false
		$player1.motion = Player.Motion($player1.actions.swing.(shot)
		$player1.transit(Player.state_swing
	$back = @ $transit_select(
	$transit_play = @
		$state = $state_play
		$menu.items[$menu.selected].play(
	$transit_select = @
		$state = $state_select
		$side = 1.0
		$ball.reset($side, 2 * 12 * 0.0254, $ball.radius + 0.01, 2 * 12 * 0.0254
		$mark.duration = 0
		$player0.placement.position = Vector3((0.1 - 2.0 * $side) * 12 * 0.0254, 0.0, 13 * 12 * 0.0254
		$player0.placement.valid = false
		$player0.reset(1.0, Player.state_default
		$player1.placement.position = Vector3((0.1 + 2.0 * $side) * 12 * 0.0254, 0.0, -13 * 12 * 0.0254
		$player1.placement.valid = false
		$player1.reset(-1.0, Player.state_default
		$step_things(
		$message = [
		$duration = 0
	$transit_ready = @
		$state = $state_ready
		selected = $menu.items[$menu.selected]
		selected.ready(

controller0 = @(controller, player)
	{
		xade.Key.KP_1: @ player.do('topspin
		xade.Key.KP_2: @ player.do('flat
		VOLUMEUP: @ player.do('lob
		VOLUMEDOWN: @ player.do('slice
		xade.Key.LEFT: @ player.left = true
		xade.Key.RIGHT: @ player.right = true
		xade.Key.UP: @ player.forward = true
		xade.Key.DOWN: @ player.backward = true
		xade.Key.SPACE: @ player.do('flat
		xade.Key.j: @ player.do('topspin
		xade.Key.l: @ player.do('flat
		xade.Key.i: @ player.do('lob
		xade.Key.m: @ player.do('slice
		xade.Key.s: @ player.left = true
		xade.Key.f: @ player.right = true
		xade.Key.e: @ player.forward = true
		xade.Key.c: @ player.backward = true
	}.each(@(key, value) controller.key_press[key] = value
	{
		xade.Key.LEFT: @ player.left = false
		xade.Key.RIGHT: @ player.right = false
		xade.Key.UP: @ player.forward = false
		xade.Key.DOWN: @ player.backward = false
		xade.Key.s: @ player.left = false
		xade.Key.f: @ player.right = false
		xade.Key.e: @ player.forward = false
		xade.Key.c: @ player.backward = false
	}.each(@(key, value) controller.key_release[key] = value

controller1 = @(controller, player)
	{
		xade.Key.KP_7: @ player.do('topspin
		xade.Key.KP_9: @ player.do('flat
		xade.Key.COMMA: @ player.do('lob
		xade.Key.PERIOD: @ player.do('slice
		xade.Key.KP_4: @ player.left = true
		xade.Key.KP_6: @ player.right = true
		xade.Key.KP_8: @ player.forward = true
		xade.Key.KP_5: @ player.backward = true
	}.each(@(key, value) controller.key_press[key] = value
	{
		xade.Key.KP_4: @ player.left = false
		xade.Key.KP_6: @ player.right = false
		xade.Key.KP_8: @ player.forward = false
		xade.Key.KP_5: @ player.backward = false
	}.each(@(key, value) controller.key_release[key] = value

Background = Container + @
	$main
	$image
	$sound
	$__initialize = @(main, image, sound)
		Container.__initialize[$](
		$main = main
		$image = glimage.Image((os.Path(system.script) / ".." / image).__string(
		$sound = main.load_sound(sound
		$sound.looping__(true
		$sound.play(
	$destroy = @
		$sound.delete(
		$image.destroy(
	$render = @(viewing)
		w = $main.aspect
		a = w * $image.height / $image.width
		s = a < 1.0 ? a : 1.0
		t = a < 1.0 ? 1.0 : 1.0 / a
		bytes = Bytes(20 * gl.Float32Array.BYTES_PER_ELEMENT
		array = gl.Float32Array(bytes
		array[0] = -w
		array[1] = -1.0
		array[2] = 0.0
		array[3] = (1.0 - s) * 0.5
		array[4] = (1.0 + t) * 0.5
		array[5] = w
		array[6] = -1.0
		array[7] = 0.0
		array[8] = (1.0 + s) * 0.5
		array[9] = (1.0 + t) * 0.5
		array[10] = -w
		array[11] = 1.0
		array[12] = 0.0
		array[13] = (1.0 - s) * 0.5
		array[14] = (1.0 - t) * 0.5
		array[15] = w
		array[16] = 1.0
		array[17] = 0.0
		array[18] = (1.0 + s) * 0.5
		array[19] = (1.0 - t) * 0.5
		$image($main.projection, viewing.bytes, bytes
		Container.render[$](viewing

Titled = Container + @
	$main
	$title
	$__initialize = @(main, title)
		Container.__initialize[$](
		$main = main
		$title = title
	$render = @(viewing)
		Container.render[$](viewing
		v = (viewing * $main.text_scale).translate(0.0, 0.5, 0.0).scale(1.125 / 4.0, 1.125 / 4.0, 1.0).translate($title.size() * -0.25, 0.0, 0.0).bytes
		$main.font($main.projection, v, $title

StageMenu = Titled + @
	$player0
	$player1
	$ready
	$__initialize = @(screen, title, done, back)
		Titled.__initialize[$](screen.main, title
		$player0 = Menu(screen.main, 2
		$player0.back = back
		$player0.items = [
		$player1 = Menu(screen.main, 2
		$player1.back = (@ $transit($player0, 2.0 * $main.aspect))[$]
		$player1.items = [
		screen.players.each((@(x)
			$player0.items.push(Menu.Item(x.name, (@ $transit($player1, -2.0 * $main.aspect))[$]
			$player1.items.push(Menu.Item(x.name, (@ $transit($ready, -2.0 * $main.aspect))[$]
		)[$]
		$ready = Menu(screen.main
		$ready.back = (@ $transit($player1, 2.0 * $main.aspect))[$]
		$ready.items = '(Menu.Item("START", (@ done(screen.players[$player0.selected], screen.players[$player1.selected]))[$]
		$content = $player0
	$render = @(viewing)
		Titled.render[$](viewing
		message = $player0.items[$player0.selected].label
		if $content === $player0
			message = message + "?"
		message = message + " vs " + $player1.items[$player1.selected].label
		if $content === $player1
			message = message + "?"
		v = (viewing * $main.text_scale).translate(0.0, 0.25, 0.0).scale(1.0 / 4.0, 1.0 / 4.0, 1.0).translate(message.size() * -0.25, -0.5, 0.0).bytes
		$main.font($main.projection, v, message

MainMenu = Titled + @
	$__initialize = @(screen, back)
		Titled.__initialize[$](screen.main, "TENNIS"
		back2this = @ screen.container.transit(MainMenu(screen, back), 2.0 * screen.main.aspect
		$content = Menu(screen.main
		$content.back = @ suisha.loop().exit(
		$content.items = '(
			Menu.Item("  1P vs COM  ", (@
				screen.container.transit(StageMenu(screen, "1P vs COM", (@(player0, player1)
					$main.screen__(Match($main, false, false, controller0, player0.path, computer, player1.path, back
				)[$], back2this), -2.0 * $main.aspect
			)[$]
			Menu.Item("  1P vs 2P   ", (@
				screen.container.transit(StageMenu(screen, "1P vs 2P", (@(player0, player1)
					$main.screen__(Match($main, true, false, controller0, player0.path, controller1, player1.path, back
				)[$], back2this), -2.0 * $main.aspect
			)[$]
			Menu.Item(" COM vs COM  ", (@
				screen.container.transit(StageMenu(screen, "COM vs COM", (@(player0, player1)
					$main.screen__(Match($main, false, true, computer, player0.path, computer, player1.path, back
				)[$], back2this), -2.0 * $main.aspect
			)[$]
			Menu.Item("  TRAINING   ", (@
				screen.container.transit(StageMenu(screen, "TRAINING", (@(player0, player1)
					$main.screen__(Training($main, controller0, player0.path, player1.path, back
				)[$], back2this), -2.0 * $main.aspect
			)[$]

MainScreen = Object + @
	Player = Object + @
		$name
		$path
	load = @(source)
		reader = collada.Reader(source
		try
			reader.read_next(
			reader.move_to_tag(
			reader.check_start_element("players"
			reader.read_next(
			players = [
			while reader.is_start_element("player")
				player = Player(
				player.name = reader.get_attribute("name"
				player.path = (os.Path(source) / ".." / reader.get_attribute("path")).__string(
				reader.read_element_text(
				players.push(player
			reader.end_element(
			players
		finally
			reader.free(

	$main
	$players
	$container
	$__initialize = @(main)
		$main = main
		$players = load((os.Path(system.script) / "../data/players").__string(
		$container = Background(main, "data/main-background.jpg", "data/main-background.wav"
		$container.content = MainMenu($, @ main.screen__(MainScreen(main
	$destroy = @ $container.destroy(
	$step = @ $container.step(
	$render = @(width, height)
		gl.viewport(0, 0, width, height
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT
		gl.disable(gl.DEPTH_TEST
		$container.render(Matrix4(1.0
	$key_press = @(key) $container.key_press(key
	$key_release = @(key)

Game = Object + @
	$alcontext
	$sound_cursor
	$sound_select
	$shaders
	$font
	$screen
	$aspect
	$projection
	$text_scale
	Sound = Object + @
		$buffer
		$source
		$delete = @
			$source.delete(
			$buffer.delete(
		$play = @ $source.play(
		$looping__ = @(value) $source.setb(al.LOOPING, value
	$load_sound = @(name)
		path = (os.Path(system.script) / ".." / name).__string(
		buffer = $alcontext.get_device().create_buffer_from_file(path
		source = $alcontext.create_source(
		source.set_buffer(buffer
		sound = Sound(
		sound.buffer = buffer
		sound.source = source
		sound
	$screen__ = @(screen)
		$screen.destroy(
		$screen = screen
	$__initialize = @(alcontext, typeface)
		$alcontext = alcontext
		$sound_cursor = $load_sound("data/cursor.wav"
		$sound_select = $load_sound("data/select.wav"
		$shaders = glshaders.shaders(
		$font = glimage.Font(typeface
		gl.enable(gl.CULL_FACE
		$screen = MainScreen($
	$resize = @(width, height)
		w = Float(width) / height
		$aspect = w
		$projection = Matrix4().orthographic(-w, w, -1.0, 1.0, -1.0, 1.0).bytes
		$text_scale = width < height ? Matrix4().scale(w, w, 1.0) : Matrix4().scale(1.0, 1.0, 1.0)
	$step = @ $screen.step(
	$render = @(width, height) $screen.render(width, height
	$key_press = @(key) $screen.key_press(key
	$key_release = @(key) $screen.key_release(key

suisha.main(@ xade.main(@ skia.main(@ gl.main(@ al.main(@
	device = al.Device(null
	context = device.default_context(
	fm = skia.FontManager.make_default(
	typeface = fm.match_family_style(null, skia.FontStyle.Normal(
	cursor_move = xade.Cursor("move"
	cursor_resize = xade.Cursor("se-resize"
	frame = xade.Frame(true
	frame.make_current(
	game = Game(context, typeface
	#frame.caption__("Tennis"
	frame.on_measure = @(width, height) '(
		width > 0 ? width : 800
		height > 0 ? height : 600
	frame.on_map = @(width, height)
		frame.make_current(
		:size = '(width, height
		game.resize(width, height
	frame.on_frame = @(_)
		t0 = time.now(
		frame.request_frame(
		game.render(size[0], size[1
		print_time && print("render: " + (time.now() - t0
		t0 = time.now(
		frame.swap_buffers(
		print_time && print("flush: " + (time.now() - t0
	loop = suisha.loop(
	frame.on_close = loop.exit
	client = xade.client(
	frame.on_pointer_enter = frame.on_pointer_move = @
		w = size[0
		h = size[1
		s = (w < h ? w : h) * 0.5
		p = client.pointer(
		if p[0] + p[1] > w + h - s
			client.cursor__(cursor_resize
			frame.on_button_press = @(button) frame.resize(xade.FrameResizeEdge.BOTTOM_RIGHT
		else
			client.cursor__(cursor_move
			frame.on_button_press = @(button) frame.move(
	frame.on_key_press = @(sym, c)
		sym == xade.Key.F && return (frame.is(xade.FrameState.FULLSCREEN) ? frame.unset_fullscreen : frame.set_fullscreen)(
		sym == xade.Key.q && return loop.exit(
		game.key_press(sym
	frame.on_key_release = @(sym, c) game.key_release(sym
	last = time.now(
	loop.timer(@
		print_time && print("since last: " + (time.now() - last
		:last = t0 = time.now(
		game.step(
		print_time && print("step: " + (time.now() - t0
	, 1000 / 60
	loop.run(
