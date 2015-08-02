Player = player.Player
Stage = stage.Stage

class Match extends Stage
  new_game: ->
    @player0.point = @player1.point = 0
    @second = false
    games = @player0.game + @player1.game
    @end = if games % 4 < 2 then 1.0 else -1.0
    @server = if games % 2 == 0 then @player0 else @player1
    @receiver = if games % 2 == 0 then @player1 else @player0
  point: (player) ->
    ++player.point
    return if player.point < 4 || player.point - player.opponent.point < 2
    ++player.game
    if player.game < 6 || player.game - player.opponent.game < 2
      @new_game()
    else
      @closed = true
  ball_ace: ->
    @second = false
    @point @ball.hitter
    @duration = 2.0 * 64.0
    @sound_ace.play()
  ball_let: ->
    @mark.mark @ball
    @set_message ['LET']
    @duration = 2.0 * 64.0
  serve_miss: ->
    if @second
      @set_message ['DOUBLE FAULT']
      @second = false
      @point @receiver
    else
      @set_message ['FAULT']
      @second = true
    @duration = 2.0 * 64.0
    @sound_miss.play()
  ball_serve_air: -> @serve_miss()
  miss: (message) ->
    @set_message [message]
    @second = false
    @point @ball.hitter.opponent
    @duration = 2.0 * 64.0
    @sound_miss.play()
  points0 = [' 0', '15', '30', '40']
  points1 = ['0 ', '15', '30', '40']
  transit_ready: ->
    @state = @state_ready
    if @player0.point + @player1.point < 6
      game = points0[@player0.point] + ' - ' + points1[@player1.point]
    else if @player0.point > @player1.point
      game = ' A -   '
    else if @player0.point < @player1.point
      game = '   - A '
    else
      game = ' DEUCE '
    @set_message [
      'P1 ' + @player0.game + ' - ' + @player1.game + ' P2'
      (if @player0 == @server then '* ' else '  ') + game + (if @player1 == @server then ' *' else '  ')
    ]
    @duration = 1.0 * 64.0
    @step_things()
  state_close = Stage::State ->
    @step_things()
  ,
    13: -> @new_set()
    27: -> @back()
  , {}
  transit_close: ->
    @state = state_close
    @set_message [
      (if @player0.game > @player1.game then 'P1' else 'P2') + ' WON!'
      'P1 ' + @player0.game + ' - ' + @player1.game + ' P2'
      'PRESS START'
      'TO PLAY AGAIN'
    ]
    @sound_ace.play()
  transit_play: ->
    @state = @state_play
    @set_message []
  reset: ->
    @side = if (@player0.point + @player1.point) % 2 == 0 then 1.0 else -1.0
    @ball.reset @side, 2 * 12 * 0.0254 * @end * @side, 0.875, 39 * 12 * 0.0254 * @end
    @mark.duration = 0.0
    @server.reset @end, Player::state_serve_set
    @receiver.node.position.set -9 * 12 * 0.0254 * @end * @side, 0.0, -39 * 12 * 0.0254 * @end
    @receiver.reset -@end, Player::state_default
    @camera0.position.set 0.0, 14.0, 0.0
    @camera0.lookAt new THREE.Vector3(0.0, -12.0, -40.0 * (if @fixed then 1.0 else @player0.end)).add(@camera0.position)
    @camera1.position.set 0.0, 14.0, 0.0
    @camera1.lookAt new THREE.Vector3(0.0, -12.0, -40.0 * (if @fixed then -1.0 else @player1.end)).add(@camera1.position)

  initialize: (controller0, player0, controller1, player1, container, next) ->
    super controller0, player0, controller1, player1, =>
      @exited = false
      @container = container
      @message = container.querySelector '.message'
      @onclick = (event) =>
        return unless event.target.tagName == 'BUTTON'
        @audio.sound_select.play()
        if event.target.value == ''
          @back()
        else
          @next()
      @container.addEventListener 'click', @onclick
      @new_set()
      next()
  new_set: ->
    @closed = false
    @player0.game = @player1.game = 0
    @new_game()
    @reset()
    @transit_ready()
  next: ->
    return unless @ball.done
    if @second || @ball.serving() && @ball.in && @ball.net
      @reset()
      @set_message []
      @step_things()
    else if @closed
      @transit_close()
    else
      @reset()
      @transit_ready()
  back: ->
    @container.removeEventListener 'click', @onclick
    @exited = true
  set_message: (message) ->
    @message.innerText = message.join '\n'

class Training extends Stage
  ball_in: ->
    @mark.mark @ball
    return if @ball.hitter != @player0
    @set_message ['IN']
  ball_ace: -> @duration = 0.5 * 64.0
  ball_let: ->
    @mark.mark @ball
    @set_message ['LET']
    @duration = 0.5 * 64.0
  serve_miss: ->
    @set_message ['FAULT']
    @duration = 0.5 * 64.0
    @sound_miss.play()
  ball_serve_air: -> @serve_miss()
  miss: (message) ->
    @set_message [message]
    @duration = 0.5 * 64.0
    @sound_miss.play()
  step_things: ->
    super()
    @camera1.position.set (@ball.position.x + @player0.root_position().x) * 0.5, 4.0, (@ball.position.z + 40.0 * @player1.end) * 0.5
    @camera1.lookAt new THREE.Vector3(0.0, -6.0, -40.0 * @player1.end).add(@camera1.position)

  constructor: (audio) -> super audio, true, false
  initialize: (controller0, player0, player1, container, next) ->
    super controller0, player0, (controller, player) =>
      super__step = controller.step
      controller.step = ->
        player.left = player.right = player.forward = player.backward = false if player.state != Player::state_swing
        super__step.call @
    , player1, =>
      for key, value of {
        13: ->
          @side = -@side
          @transit_ready()
      }
        @state_ready.key_press[key] = value
        @state_play.key_press[key] = value
      @camera0.position.set 0.0, 14.0, 0.0
      @camera0.lookAt new THREE.Vector3(0.0, -12.0, -40.0 * @player0.end).add(@camera0.position)
      @exited = false
      @container = container
      @message = container.querySelector '.message'
      @onclick = (event) =>
        return unless event.target.tagName == 'BUTTON'
        @audio.sound_select.play()
        if event.target.value == ''
          if @current
            @transit_select()
          else
            @exit()
        else
          slide @container.querySelector('#trainings'), 'left'
          @current = trainings[event.target.value]
          @transit_ready()
      @container.addEventListener 'click', @onclick
      @transit_select()
      next()
  next: -> @ball.done && @transit_ready()
  reset: (x, y, z, position, shot) ->
    @ball.reset @side, x, y, z
    @mark.duration = 0.0
    @player0.node.position.copy position
    @player0.reset 1.0, Player::state_default
    @player1.node.position.copy(@ball.position).sub(new THREE.Vector3(0.0, 0.0, 0.0).applyMatrix4(@player1.actions.swing[shot].spot))
    @player1.node.position.setY 0.0
    @player1.reset -1.0, Player::state_default
  toss: (shot) ->
    @player1.node.lookAt @player1.shot_direction().add(@player1.node.position)
    @player1.set_motion new player.Motion @player1.actions.swing[shot]
    @player1.transit Player::state_swing
  toss_message = [
    '  CHANGE SIDES: START'
    '      POSITION: +    '
    'COURCE & SWING: + & *'
  ]
  trainings =
    serve:
      reset: ->
        @ball.reset @side, 2 * 12 * 0.0254 * @side, 0.875, 39 * 12 * 0.0254
        @mark.duration = 0.0
        @player0.reset 1.0, Player::state_serve_set
        @player1.node.position.set -9 * 12 * 0.0254 * @side, 0.0, -39 * 12 * 0.0254
        @player1.reset -1.0, Player::state_default
        @set_instruction [
          'CHANGE SIDES: START'
          '    POSITION: < + >'
          '        TOSS:   *  '
          '      COURCE: < + >'
          '       SWING:   *  '
        ]
        @duration = 0.0 * 64.0
      play: ->
    stroke:
      reset: ->
        @reset 3 * 12 * 0.0254 * @side, 1.0, -39 * 12 * 0.0254, new THREE.Vector3((0.0 - 3.2 * @side) * 12 * 0.0254, 0.0, 39 * 12 * 0.0254), 'toss'
        @set_instruction toss_message
        @duration = 0.5 * 64.0
      play: -> @toss 'toss'
    volley:
      reset: ->
        @reset 3 * 12 * 0.0254 * @side, 1.0, -39 * 12 * 0.0254, new THREE.Vector3((0.1 - 2.0 * @side) * 12 * 0.0254, 0.0, 13 * 12 * 0.0254), 'toss'
        @set_instruction toss_message
        @duration = 0.5 * 64.0
      play: -> @toss 'toss'
    smash:
      reset: ->
        @reset 3 * 12 * 0.0254 * @side, 1.0, -39 * 12 * 0.0254, new THREE.Vector3((0.4 - 0.4 * @side) * 12 * 0.0254, 0.0, 9 * 12 * 0.0254), 'toss_lob'
        @set_instruction toss_message
        @duration = 0.5 * 64.0
      play: -> @toss 'toss_lob'
  back: -> @transit_select()
  exit: ->
    @container.removeEventListener 'click', @onclick
    @exited = true
  state_select = Stage::State (->),
    27: -> @exit()
  , {}
  transit_select: ->
    slide @container.querySelector('#trainings'), 'center'
    @state = state_select
    @current = null
    @side = 1.0
    @ball.reset @side, 2 * 12 * 0.0254, @ball.radius + 0.01, 2 * 12 * 0.0254
    @mark.duration = 0.0
    @player0.node.position.set (0.1 - 2.0 * @side) * 12 * 0.0254, 0.0, 13 * 12 * 0.0254
    @player0.reset 1.0, Player::state_default
    @player1.node.position.set (0.1 + 2.0 * @side) * 12 * 0.0254, 0.0, -13 * 12 * 0.0254
    @player1.reset -1.0, Player::state_default
    @set_message []
    @step_things()
  transit_ready: ->
    @state = @state_ready
    @current.reset.call @
    @step_things()
  transit_play: ->
    @state = @state_play
    @current.play.call @
  set_message: (message) ->
    @message.style.verticalAlign = 'middle'
    @message.style.fontSize = '2em'
    @message.innerText = message.join '\n'
  set_instruction: (message) ->
    @message.style.verticalAlign = 'bottom'
    @message.style.fontSize = '0.5em'
    @message.innerText = message.join '\n'

controller0 = (controller, player) ->
  controller.key_press[key] = value for key, value of {
    49: -> player.perform 'topspin'
    50: -> player.perform 'flat'
    VOLUMEUP: -> player.perform 'lob'
    VOLUMEDOWN: -> player.perform 'slice'
    37: -> player.left = true
    39: -> player.right = true
    38: -> player.forward = true
    40: -> player.backward = true
    #32: -> player.perform 'flat'
    74: -> player.perform 'topspin'
    76: -> player.perform 'flat'
    73: -> player.perform 'lob'
    77: -> player.perform 'slice'
    83: -> player.left = true
    70: -> player.right = true
    69: -> player.forward = true
    67: -> player.backward = true
  }
  controller.key_release[key] = value for key, value of {
    37: -> player.left = false
    39: -> player.right = false
    38: -> player.forward = false
    40: -> player.backward = false
    83: -> player.left = false
    70: -> player.right = false
    69: -> player.forward = false
    67: -> player.backward = false
  }

controller1 = (controller, player) ->
  controller.key_press[key] = value for key, value of {
    55: -> player.perform 'topspin'
    57: -> player.perform 'flat'
    60: -> player.perform 'lob'
    62: -> player.perform 'slice'
    52: -> player.left = true
    54: -> player.right = true
    56: -> player.forward = true
    53: -> player.backward = true
  }
  controller.key_release[key] = value for key, value of {
    52: -> player.left = false
    54: -> player.right = false
    56: -> player.forward = false
    53: -> player.backward = false
  }

class Audio
  class Source
    constructor: (@audio, @buffer) ->
    play: ->
      source = @audio.createBufferSource()
      source.buffer = @buffer
      source.connect @audio.destination
      source.start 0

  constructor: -> @audio = new AudioContext
  load: (url, next) ->
    request = new XMLHttpRequest
    request.open 'GET', url
    request.responseType = 'arraybuffer'
    request.onload = =>
      @audio.decodeAudioData request.response, (buffer) =>
        next new Source(@audio, buffer)
    request.send()

Detector.addGetWebGLMessage() unless Detector.webgl

setup = (screen, container, onexit) ->
  screen.scene.add new THREE.AmbientLight(0x333333)
  directionalLight = new THREE.DirectionalLight 0x777777
  directionalLight.position.set(1.0, 1.0, -1.0).normalize()
  screen.scene.add directionalLight
  particleLight = new THREE.Mesh new THREE.SphereGeometry(4, 8, 8), new THREE.MeshBasicMaterial({color: 0xffffff})
  particleLight.position.set 1000, 4000, 3000
  screen.scene.add particleLight
  pointLight = new THREE.PointLight 0x333333, 4
  particleLight.add pointLight

  back = container.querySelector '.back'
  overlay = container.querySelector 'div'
  renderer = new THREE.WebGLRenderer
  renderer.autoClear = false
  renderer.setPixelRatio window.devicePixelRatio
  renderer.setSize window.innerWidth, window.innerHeight
  container.insertBefore renderer.domElement, overlay
  stats = new Stats
  stats.domElement.style.position = 'absolute'
  stats.domElement.style.top = '3em'
  stats.domElement.style.visibility = back.style.visibility
  container.appendChild stats.domElement

  onresize = -> renderer.setSize window.innerWidth, window.innerHeight
  onkeydown = (event) ->
    event.preventDefault() if screen.key_press event.keyCode
  onkeyup = (event) ->
    event.preventDefault() if screen.key_release event.keyCode
  window.addEventListener 'resize', onresize
  window.addEventListener 'keydown', onkeydown
  window.addEventListener 'keyup', onkeyup
  onclick = ->
    return unless event.target.classList.contains 'content'
    value = if back.style.visibility == 'hidden' then 'visible' else 'hidden'
    back.style.visibility = value
    stats.domElement.style.visibility = value
  window.addEventListener 'click', onclick

  render = ->
    if screen.exited
      window.removeEventListener 'resize', onresize
      window.removeEventListener 'keydown', onkeydown
      window.removeEventListener 'keyup', onkeyup
      window.removeEventListener 'click', onclick
      container.removeChild renderer.domElement
      container.removeChild stats.domElement
      return onexit()
    requestAnimationFrame render
    THREE.AnimationHandler.update 1.0 / 60.0
    screen.step()
    screen.render renderer, window.innerWidth, window.innerHeight
    stats.update()
  render()

audio = new Audio

slide = (element, where) ->
  element.disabled = where != 'center'
  element.classList.remove 'slide-left'
  element.classList.remove 'slide-center'
  element.classList.remove 'slide-right'
  element.classList.add 'slide-' + where

chooseItem = (items, next) ->
  onclick = (event) ->
    return unless event.target.tagName == 'BUTTON'
    audio.sound_select.play()
    items.removeEventListener 'click', onclick
    next event.target
  items.addEventListener 'click', onclick

showMenu = ->
  screen.style.display = 'none' for screen in document.querySelectorAll '.screen'
  slide slider, 'right' for slider in document.querySelectorAll '.slider'
  slide document.getElementById('players0'), 'center'
  document.getElementById('players-screen').style.display = 'table'
  document.getElementById('background').style.display = 'block'
  menu = document.getElementById 'menu-screen'
  slide menu, 'center'
  menu.style.display = 'table'
  chooseItem menu, showPlayers0

showPlayers0 = (stage) ->
  slide document.getElementById('menu-screen'), 'left'
  slide document.getElementById('players1'), 'right'
  slide document.getElementById('players0'), 'center'
  players = document.getElementById 'players-screen'
  players.querySelector('.title').innerText = stage.innerText + '\n???? vs ____'
  slide players, 'center'
  chooseItem players, (player0) ->
    return showMenu() if player0.value == ''
    showPlayers1 stage, player0

showPlayers1 = (stage, player0) ->
  slide document.getElementById('players0'), 'left'
  slide document.getElementById('ready'), 'right'
  players = document.getElementById 'players-screen'
  players.querySelector('.title').innerText = stage.innerText + '\n' + player0.innerText + ' vs ????'
  slide document.getElementById('players1'), 'center'
  chooseItem players, (player1) ->
    return showPlayers0 stage if player1.value == ''
    showReady stage, player0, player1

showReady = (stage, player0, player1) ->
  slide document.getElementById('players1'), 'left'
  players = document.getElementById 'players-screen'
  players.querySelector('.title').innerText = stage.innerText + '\n' + player0.innerText + ' vs ' + player1.innerText
  slide document.getElementById('ready'), 'center'
  chooseItem players, (start) ->
    return showPlayers1 stage, player0 if start.value == ''
    players.style.display = 'none'
    document.getElementById('loading-screen').style.display = 'table'
    container = document.getElementById if stage.value == 'training' then 'training-screen' else 'match-screen'
    onready = (screen) -> ->
      document.getElementById('background').style.display = 'none'
      container.style.display = 'block'
      setup screen, container, showMenu
    switch stage.value
      when '1pvscom'
        screen = new Match audio, false, false
        screen.initialize controller0, player0.value, computer.controller, player1.value, container, onready(screen)
      when 'comvscom'
        screen = new Match audio, false, true
        screen.initialize computer.controller, player0.value, computer.controller, player1.value, container, onready(screen)
      when 'training'
        screen = new Training audio
        screen.initialize controller0, player0.value, player1.value, container, onready(screen)

audio.load 'data/select.wav', (sound) ->
  audio.sound_select = sound
  showMenu()
