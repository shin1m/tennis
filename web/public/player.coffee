exports = module?.exports ? {}
window.player = exports if window?

G = ball.G

exports.reach_range = reach_range = (ball, velocity, player, speed, t0, sign) ->
  qp = ball.clone().sub(player).setY(0.0)
  v = new THREE.Vector3(velocity.x, 0.0, velocity.z)
  ss = speed * speed
  a = v.dot(v) - ss
  b = v.dot(qp) - ss * t0
  c = qp.dot(qp) - ss * t0 * t0
  d = b * b - a * c
  if d < 0.0 then -b / a else (-b + sign * Math.sqrt(d)) / a

exports.shot_direction = shot_direction = (ball, end, left, right, forward, backward) ->
  vx = -ball.x
  vx -= 12 * 12 * 0.0254 * end if left
  vx += 12 * 12 * 0.0254 * end if right
  vz = -24 * 12 * 0.0254 * end - ball.z
  vz -= 14 * 12 * 0.0254 * end if forward
  vz += 10 * 12 * 0.0254 * end if backward
  new THREE.Vector3(vx, 0.0, vz)

class Player
  State: State = (enter, step, perform) ->
    enter: enter
    step: step
    perform: perform
  class Action
    constructor: (skin, start, duration, use = (x) -> true) ->
      @start = start
      data = skin.geometry.animation
      @animation = new THREE.Animation skin,
        fps: data.fps
        hierarchy: bone for bone in data.hierarchy when use bone
        length: start + duration
        name: 'animation_' + start
      @animation.hierarchy = (bone for bone in @animation.hierarchy when use bone)
      @animation.loop = false
    play: ->
      @animation.play @start
      return if @start < @animation.data.length
      @animation.resetBlendWeights()
      @animation.update 0.0
      @animation.stop()
  class Swing extends Action
    bone: (time, skin, name) ->
      @animation.play time
      THREE.AnimationHandler.update 0.0
      @animation.stop()
      skin.updateMatrixWorld true
      for bone in skin.skeleton.bones
        return bone if bone.name == name
    constructor: (skin, start, duration, impact, speed, spin = new THREE.Vector3(0.0, 0.0, 0.0)) ->
      super skin, start, duration
      @impact = start + impact
      @speed = speed
      @spin = spin
      @spot = @bone(@impact, skin, 'Spot').matrixWorld.clone()
      root = @bone(start + duration, skin, 'Root').matrixWorld
      @end_position = new THREE.Vector3(root.elements[12], 0.0, root.elements[14])
      @end_toward = new THREE.Vector3(root.elements[8], 0.0, root.elements[10])
    merge: (player) ->
      node = player.node
      node.localToWorld node.position.copy(@end_position)
      at = @end_position.clone().add(@end_toward)
      node.lookAt node.localToWorld(at)
  class Run extends Action
    constructor: (skin, start, duration, use) ->
      super skin, start, duration, use
      animation = new THREE.Animation skin, skin.geometry.animation
      animation.loop = false
      animation.play start
      THREE.AnimationHandler.update 0.0
      animation.stop()
      skin.updateMatrixWorld true
      for bone in skin.skeleton.bones
        break if bone.name == 'Root'
      root = bone.matrixWorld
      @toward = new THREE.Vector3(root.elements[4], 0.0, root.elements[6])
  class Motion
    constructor: (@action) ->
    play: -> @action.play()
    time: -> @action.animation.currentTime
    playing: -> @action.animation.isPlaying
  exports.Motion = Motion
  class RunMotion extends Motion
    duration = 4.0 / 64.0

    constructor: (run, @toward, player) ->
      super run
      @node = player.node
      elements = @node.matrix.elements
      @toward0 = new THREE.Vector3(elements[8], 0.0, elements[10])
      player.root.matrix.copy player.root_transform
      player.root.applyMatrix new THREE.Matrix4
      t0 = @toward.clone().normalize()
      t1 = run.toward
      @toward1 = new THREE.Vector3(t0.x * t1.z + t0.z * t1.x, 0.0, t0.z * t1.z - t0.x * t1.x)
      @duration = if @toward0.dot(@toward1) < -0.75 then 0.0 else duration
    step: ->
      @duration -= 1.0 / 64.0 if @duration > 0.0
      t = @duration / duration
      t0 = @toward0.clone().multiplyScalar(t)
      t1 = @toward1.clone().multiplyScalar(1.0 - t)
      @node.lookAt t0.add(t1).add(@node.position)

  lowers =
    Center: true
    Leg0_R: true
    Leg1_R: true
    Foot_R: true
    Toe_R: true
    Leg0_L: true
    Leg1_L: true
    Foot_L: true
    Toe_L: true
  is_not_root = (x) -> x.name != 'Root'
  is_lower = (x) -> lowers[x.name]?
  is_upper = (x) -> is_not_root(x) && !is_lower(x)
  load = (skin, source, next) ->
    source_fps = null
    fps = 64.0
    read_action = (e, use = (x) -> true) ->
      start = parseFloat(e.getAttribute('start')) / source_fps
      duration = parseFloat(e.getAttribute('duration')) / source_fps
      new Action skin, start, duration, use
    read_swing = (e) ->
      start = parseFloat(e.getAttribute('start')) / source_fps
      duration = parseFloat(e.getAttribute('duration')) / source_fps
      impact = parseFloat(e.getAttribute('impact')) / source_fps
      speed = parseFloat(e.getAttribute('speed')) / fps
      spin = e.getAttribute('spin').split(/\s+/).map(parseFloat)
      new Swing skin, start, duration, impact, speed, new THREE.Vector3(spin[0], spin[1], spin[2])
    read_run = (e, use) ->
      start = parseFloat(e.getAttribute('start')) / source_fps
      duration = parseFloat(e.getAttribute('duration')) / source_fps
      new Run skin, start, duration, use
    read_shot = (e) ->
      flat: read_swing e.querySelector('flat')
      topspin: read_swing e.querySelector('topspin')
      lob: read_swing e.querySelector('lob')
      slice: read_swing e.querySelector('slice')
    read_ready_action = (e) ->
      stroke: read_run e.querySelector('stroke'), is_not_root
      volley: read_run e.querySelector('volley'), is_not_root
      smash: read_run e.querySelector('smash'), is_not_root
    read_run_actions = (e) ->
      lowers: ((e) ->
        [
          null
          read_run e.querySelector('left'), is_lower
          read_run e.querySelector('right'), is_lower
          null
          read_run e.querySelector('forward'), is_lower
          read_run e.querySelector('forward_left'), is_lower
          read_run e.querySelector('forward_right'), is_lower
          null
          read_run e.querySelector('backward'), is_lower
          read_run e.querySelector('backward_left'), is_lower
          read_run e.querySelector('backward_right'), is_lower
        ]
      ) e.querySelector('lowers')
      stroke: read_action e.querySelector('stroke'), is_upper
      volley: read_action e.querySelector('volley'), is_upper
      smash: read_action e.querySelector('smash'), is_upper
    read_swing_actions = (e) ->
      stroke: read_shot e.querySelector('stroke')
      volley: read_swing e.querySelector('volley')
      smash: read_swing e.querySelector('smash')
    request = new XMLHttpRequest
    request.addEventListener 'load', ->
      xml = @responseXML ? new DOMParser().parseFromString(@responseText, 'application/xml')
      root = xml.querySelector 'player'
      source_fps = parseFloat(root.getAttribute('fps'))
      e = root.querySelector ':scope > serve'
      serve =
        set: read_action e.querySelector('set')
        toss: read_action e.querySelector('toss')
        swing: read_shot e.querySelector('swing')
      e = root.querySelector ':scope > ready'
      ready =
        default: read_run e.querySelector('default'), is_not_root
        forehand: read_ready_action e.querySelector('forehand')
        backhand: read_ready_action e.querySelector('backhand')
      e = root.querySelector ':scope > run'
      run =
        speed: parseFloat(e.getAttribute('speed')) / fps
        lower: read_run e.querySelector('lower'), is_lower
        default: read_action e.querySelector('default'), is_upper
        forehand: read_run_actions e.querySelector('forehand')
        backhand: read_run_actions e.querySelector('backhand')
      e = root.querySelector ':scope > swing'
      swing =
        forehand: read_swing_actions e.querySelector('forehand')
        backhand: read_swing_actions e.querySelector('backhand')
        toss: read_swing e.querySelector('toss')
        toss_lob: read_swing e.querySelector('toss_lob')
      next
        serve: serve
        ready: ready
        run: run
        swing: swing
    request.open 'GET', source
    request.send()

  constructor: (@stage) -> @ball = @stage.ball
  initialize: (model, next) ->
    loader = new THREE.ColladaLoader
    loader.options.convertUpAxis = true
    loader.load model + '.dae', (collada) =>
      collada.scene.traverse (child) ->
        return unless child.material
        child.material.alphaTest = 0.5
        material.alphaTest = 0.5 for material in child.material.materials if child.material.materials?
      @node = collada.scene
      for bone in collada.skins[0].skeleton.bones
        continue unless bone.name == 'Root'
        @root = bone
        break
      @root_transform = @root.matrix.clone()
      load collada.skins[0], model + '.player', (actions) =>
        @actions = actions
        @speed = @actions.run.speed
        @actions.serve.set.play()
        console.log @root.position
        @lefty = @root.position.x < 0.0
        @smash_hand = if @lefty then 0.25 else -0.25
        @motion = null
        @blend_last = null
        @blend_duration = 0.0
        @blend_current = 0.0
        @reset 1.0, @state_default
        next()
  set_motion: (motion, blend = 0.0) ->
    @blend_last?.stop()
    @blend_last = @motion?.action.animation
    @blend_duration = blend
    @blend_current = blend
    @motion = motion
    @motion?.play()
  transit: (state) ->
    @state = state
    @state.enter.call @
  reset: (end, state) ->
    @left = @right = @forward = @backward = false
    @end = end
    @transit state
  setup: ->
  root_position: ->
    @node.updateMatrixWorld false
    @node.localToWorld @root.position.clone()
  direction: ->
    v = @ball.velocity
    e = (if v.z < 0.0 then 1.0 else -1.0) * @end
    v = new THREE.Vector3(v.x * e, 0.0, v.z * e)
    if v.length() > 0.01 / 64.0 then v else v.set(0.0, 0.0, -@end)
  whichhand: (v) -> new THREE.Vector3(-v.z, 0.0, v.x).dot(@ball.position.clone().sub(@node.position))
  relative_ball: (swing) ->
    p = @node.worldToLocal @ball.position.clone()
    x = p.x - swing.spot.elements[12]
    y = p.y - swing.spot.elements[13]
    z = p.z - swing.spot.elements[14]
    if swing.spot.elements[8] < 0.0 then new THREE.Vector3(-x, y, -z) else new THREE.Vector3(x, y, z)
  step: ->
    @state.step.call @
    if @blend_current > 0.0
      @blend_last?.weight = @blend_current / @blend_duration
      @motion?.action.animation.weight = (@blend_duration - @blend_current) / @blend_duration
      @blend_current -= 1.0 / 64.0
    else
      @blend_last?.stop()
      @blend_last = null
    @motion?.step?()
    position = @node.position
    if position.x < -30 * 12 * 0.0254
      position.x = -30 * 12 * 0.0254
    else if position.x > 30 * 12 * 0.0254
      position.x = 30 * 12 * 0.0254
    if position.z * @end < 1.0
      position.z = 1.0 * @end
    else if position.z * @end > 60 * 12 * 0.0254
      position.z = 60 * 12 * 0.0254 * @end
  perform: (shot) -> @state.perform.call @, shot
  shot_direction: ->
    if @ball.position.z * @end < 0.0
      new THREE.Vector3(0.0, 0.0, -@end)
    else
      shot_direction @ball.position, @end, @left, @right, @forward, @backward
  smash_height: 2.25
  state_default: State ->
    v = @ball.position.clone().setY(0.0)
    @node.lookAt v
    @node.updateMatrix()
    v.sub(@node.position).normalize()
    @set_motion new RunMotion(@actions.ready.default, v, @), 4.0 / 64.0
  , ->
    d = new THREE.Vector3(0.0, 0.0, 0.0)
    d.x = -@speed * @end if @left
    d.x = @speed * @end if @right
    d.z = -@speed * @end if @forward
    d.z = @speed * @end if @backward
    actions = if d.x == 0.0 && d.z == 0.0 then @actions.ready else @actions.run
    if @ball.done
      v = new THREE.Vector3(0.0, 0.0, -@end)
      action = actions.default
      run = @actions.run.lower if actions == @actions.run
    else if @ball.hitter == null || @ball.hitter.end == @end
      v = @ball.position.clone().sub(@node.position).setY(0.0).normalize()
      action = actions.default
      if actions == @actions.run
        if @forward
          run = @actions.run.lower
        else if @left
          run = actions.backhand.lowers[9]
        else if @right
          run = actions.forehand.lowers[10]
        else if @backward
          run = if v.x * @end > 0.0 then actions.forehand.lowers[9] else actions.backhand.lowers[10]
        else
          run = @actions.run.lower
    else
      v = @direction().normalize()
      whichhand = @whichhand(v)
      t = reach_range(@ball.position, @ball.velocity, @node.position, 0.0, 0.0, 1.0)
      y = @ball.position.y + (@ball.velocity.y - 0.5 * G * t) * t
      if y > @smash_height
        hand = if whichhand > @smash_hand then actions.forehand else actions.backhand
        action = hand.smash
      else
        hand = if whichhand > 0.0 then actions.forehand else actions.backhand
        action = if @ball.in || y < 0.0 then hand.stroke else hand.volley
      if actions == @actions.run
        run = hand.lowers[(if @left then 1 else if @right then 2 else 0) + (if @forward then 4 else if @backward then 8 else 0)]
    if actions == @actions.ready
      @set_motion new RunMotion(action, v, @), 4.0 / 64.0
    else
      @set_motion new RunMotion(run, d, @), 4.0 / 64.0 if @motion.action != run || !d.equals(@motion.toward)
      @motion.play() unless @motion.playing()
      action.play()
      @node.position.add(d)
  , (shot) ->
    @node.lookAt @shot_direction().add(@node.position)
    actions = @actions.swing
    whichhand = @whichhand @direction().normalize()
    t = @ball.projected_time_for_y @smash_height, 1.0
    if t
      swing = if whichhand > @smash_hand then actions.forehand.smash else actions.backhand.smash
      if t > (swing.impact - swing.start) * 60.0
        @set_motion new Motion swing
        return @transit @state_smash_swing
    t = if @ball.in then 0.0 else @ball.projected_time_for_y @ball.radius, 1.0
    hand = if whichhand > 0.0 then actions.forehand else actions.backhand
    if @ball.done
      @set_motion new Motion if @node.position.z * @end > 21 * 12 * 0.0254 then hand.stroke[shot] else hand.volley
    else
      @set_motion new Motion if t < (hand.volley.impact - hand.volley.start) * 60.0 then hand.stroke[shot] else hand.volley
    @transit @state_swing
  state_serve_set: State ->
    @set_motion new Motion @actions.serve.set
  , ->
    speed = 2.0 / 64.0
    @ball.position.x = @ball.position.x - speed * @end if @left
    @ball.position.x = @ball.position.x + speed * @end if @right
    @ball.position.y = 0.875
    @ball.velocity.set 0.0, 0.0, 0.0
    @ball.spin.set 0.0, 0.0, 0.0
    @node.position.set @ball.position.x, 0.0, @ball.position.z
    @node.lookAt new THREE.Vector3((6 * 12 + 9) * -0.0254 * @end * @stage.side + (if @lefty then -1 else 1) * 12 * 0.0254 * @end, 0.0, 21 * 12 * -0.0254 * @end)
  , (shot) ->
    @transit @state_serve_toss
  state_serve_toss: State ->
    @ball.position.y = 1.5
    @ball.velocity.set((if @lefty then -0.0075 else 0.0075), 0.085, 0.01).applyQuaternion(@node.getWorldQuaternion())
    @ball.spin.set 0.0, 0.0, 0.0
    @set_motion new Motion @actions.serve.toss
  , ->
    if @ball.position.y <= 1.5
      @ball.position.x = @node.position.x
      @ball.position.z = @node.position.z
      @ball.velocity.set 0.0, 0.0, 0.0
      @transit @state_serve_set
    @node.rotateOnAxis new THREE.Vector3(0.0, 1.0, 0.0), 1.0 / 64.0 if @left
    @node.rotateOnAxis new THREE.Vector3(0.0, 1.0, 0.0), -1.0 / 64.0 if @right
  , (shot) ->
    @set_motion new Motion @actions.serve.swing[shot]
    @transit @state_serve_swing
  state_serve_swing: State ->
    @stage.sound_swing.play()
  , ->
    if Math.abs(@motion.time() - @motion.action.impact) < 0.5 / 60.0
      ball = @relative_ball @motion.action
      if Math.abs(ball.y) < 0.3
        d = 58 * 12 * 0.0254 + ball.y * 10.0
        speed = @motion.action.speed + ball.y * 0.125
        toward = new THREE.Vector3(0.0, 0.0, 1.0).applyQuaternion(@node.getWorldQuaternion())
        @ball.impact toward.x, toward.z, speed, G * d / (2.0 * speed) - @ball.position.y * speed / d, @motion.action.spin
        @ball.hitter = @
        @stage.sound_hit.play()
    return if @motion.playing()
    @motion.action.merge @
    @transit @state_default
  , (shot) ->
  state_swing: State ->
    @stage.sound_swing.play()
  , ->
    v = @shot_direction()
    if @motion.time() <= @motion.action.impact
      @node.lookAt @node.position.clone().add(v)
    if Math.abs(@motion.time() - @motion.action.impact) < 0.5 / 60.0
      ball = @relative_ball @motion.action
      if Math.abs(ball.x) < 0.5 && Math.abs(ball.z) < 1.0
        d = v.length()
        n = -d * @ball.position.z / v.z
        b = @ball.position.y * (d - n) / d
        a = d / (60 * 12 * 0.0254)
        speed = @motion.action.speed * (if a > 1.25 then 1.25 else if a < 0.85 then 0.85 else a)
        spin = @motion.action.spin
        d *= 1.0 - spin.x * (2.0 / 64.0)
        if b < 42 * 0.0254
          vm = Math.sqrt(G * (d - n) * n * 0.5 / (42 * 0.0254 - b))
          speed = vm if vm < speed
        d -= ball.x * 2.0
        speed -= ball.x * 0.125
        dx = v.x + v.z * ball.z * 0.0625
        dz = v.z - v.x * ball.z * 0.0625
        @ball.impact dx, dz, speed, G * d / (2.0 * speed) - @ball.position.y * speed / d, spin
        @ball.hit @
        @stage.sound_hit.play()
    return if @motion.playing()
    @motion.action.merge @
    @transit @state_default
  , (shot) ->
  state_smash_swing: State ->
    @stage.sound_swing.play()
  , ->
    v = @shot_direction()
    if @motion.time() <= @motion.action.impact
      @node.lookAt @node.position.clone().add(v)
    if Math.abs(@motion.time() - @motion.action.impact) < 0.5 / 60.0
      ball = @relative_ball @motion.action
      if Math.abs(ball.x) < 0.5 && Math.abs(ball.y) < 0.5 && Math.abs(ball.z) < 1.0
        d = v.length() + (ball.y - ball.z) * 2.0
        speed = @motion.action.speed + ball.y * 0.125
        dx = v.x + v.z * ball.x * 0.0625
        dz = v.z - v.x * ball.x * 0.0625
        @ball.impact dx, dz, speed, G * d / (2.0 * speed) - @ball.position.y * speed / d, @motion.action.spin
        @ball.hit @
        @stage.sound_hit.play()
    return if @motion.playing()
    @motion.action.merge @
    @transit @state_default
  , (shot) ->
exports.Player = Player
