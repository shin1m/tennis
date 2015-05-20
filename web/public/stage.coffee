exports = module?.exports ? {}
window.stage = exports if window?

Ball = ball.Ball
Mark = ball.Mark
Player = player.Player

class Stage
  State: State = (step, key_press, key_release) ->
    step: step
    key_press: key_press
    key_release: key_release

  ball_bounce: -> @sound_bounce.play()
  ball_in: -> @mark.mark @ball
  ball_net: -> @sound_net.play()
  ball_chip: -> @sound_chip.play()
  ball_miss: -> if @ball.serving() then @serve_miss() else @miss('MISS')
  ball_out: ->
    @mark.mark @ball
    if @ball.serving() then @serve_miss() else @miss('OUT')
  step_things: ->
    @ball.step()
    @mark.step()
    @player0.step()
    @player1.step()
    tx = @ball.position.x * 0.25
    tz = @ball.position.z * 0.25
    if @fixed
      @camera0.position.x = tx
      @camera0.position.z = 48.0 + tz
      @camera1.position.x = tx
      @camera1.position.z = -48.0 + tz
    else
      @camera0.position.x = tx + @player0.root_position().x * 0.5
      @camera0.position.z = 48.0 * @player0.end + tz
      @camera1.position.x = tx + @player1.root_position().x * 0.5
      @camera1.position.z = 48.0 * @player1.end + tz
  load_sound: (url, field, next) ->
    @audio.load url, (source) =>
      @[field] = source
      next()
  load_sounds: (sounds, next) ->
    n = 0
    for sound in sounds
      @load_sound sound[1], sound[0], -> next() if ++n >= sounds.length

  constructor: (@audio, @dual, @fixed) ->
  initialize: (controller0, player0, controller1, player1, next) ->
    @load_sounds [
      ['sound_bounce', 'data/bounce.wav']
      ['sound_net', 'data/net.wav']
      ['sound_chip', 'data/chip.wav']
      ['sound_hit', 'data/hit.wav']
      ['sound_swing', 'data/swing.wav']
      ['sound_ace', 'data/ace.wav']
      ['sound_miss', 'data/miss.wav']
    ], =>
      loader = new THREE.ColladaLoader
      loader.options.convertUpAxis = true
      loader.load 'data/court.dae', (collada) =>
        collada.scene.traverse (child) -> child.material?.alphaTest = 0.5
        @scene = new THREE.Scene
        @scene.add collada.scene
        @camera0 = new THREE.PerspectiveCamera 12.0, 1.0, 10.0, 200.0
        @camera1 = new THREE.PerspectiveCamera 12.0, 1.0, 10.0, 200.0
        @ball = new Ball @
        @mark = new Mark
        @player0 = new Player @
        @player0.initialize player0, =>
          @player1 = new Player @
          @player1.initialize player1, =>
            @player0.opponent = @player1
            @player1.opponent = @player0
            @scene.add @ball.node, @mark.node, @player0.node, @player1.node
            @state_ready = State(->
              return @transit_play() if @duration <= 0.0
              @duration -= 1.0
            ,
              13: -> @transit_play()
              27: -> @back()
            , {})
            @state_play = State(->
              @step_things()
              return unless @ball.done
              return @next() if @duration <= 0.0
              @duration -= 1.0
            ,
              13: -> @next()
              27: -> @back()
            , {})
            controller0.call @, @state_play, @player0
            controller1.call @, @state_play, @player1
            next()
  step: -> @state.step.call @
  render: (renderer, width, height) ->
    @ball.setup()
    @mark.setup()
    @player0.setup()
    @player1.setup()
    renderer.clear()
    if @dual
      @camera0.aspect = width * 0.5 / height
      @camera0.setViewOffset width * 0.5, height, 0, 0, width * 0.5, height
      renderer.setViewport 0, 0, width * 0.5, height
    else
      @camera0.aspect = width / height
      @camera0.setViewOffset width, height, 0, 0, width, height
      renderer.setViewport 0, 0, width, height
    renderer.render @scene, @camera0
    if @dual
      @camera1.aspect = width * 0.5 / height
      @camera1.setViewOffset width * 0.5, height, 0, 0, width * 0.5, height
      renderer.setViewport width * 0.5, 0, width * 0.5, height
      renderer.render @scene, @camera1
  key_press: (key) ->
    handler = @state.key_press[key]
    handler?.call @
    handler?
  key_release: (key) ->
    handler = @state.key_release[key]
    handler?.call @
    handler?
exports.Stage = Stage
