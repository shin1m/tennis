exports = module?.exports ? {}
window.ball = exports if window?

exports.G = G = 9.8 / (64.0 * 64.0)

exports.projected_time_for_y = projected_time_for_y = (py, vy, y, sign) ->
  a = vy * vy + 2.0 * G * (py - y)
  if a < 0.0 then null else (vy + sign * Math.sqrt(a)) / G

radius = 0.0625

class Ball
  radius: radius
  constructor: (@stage) ->
    @position = new THREE.Vector3(0.0, 0.0, 0.0)
    @velocity = new THREE.Vector3(0.0, 0.0, 0.0)
    @spin = new THREE.Vector3(0.0, 0.0, 0.0)
    @node = new THREE.Object3D
    shadow = new THREE.Mesh new THREE.CircleGeometry(radius), new THREE.MeshBasicMaterial({color: 0x000000})
    shadow.position.y = 1.0 / 64.0
    shadow.rotateOnAxis new THREE.Vector3(1.0, 0.0, 0.0), Math.PI * -0.5
    @body = new THREE.Mesh new THREE.SphereGeometry(radius), new THREE.MeshLambertMaterial({color: 0xffff00})
    @node.add shadow, @body
  setup: ->
    @node.position.x = @position.x
    @body.position.y = @position.y
    @node.position.z = @position.z
  netin_part: (x0, y0, x1, y1) ->
    ex = x1 - x0
    ey = y1 - y0
    l = Math.sqrt(ex * ex + ey * ey)
    ex /= l
    ey /= l
    dx = @position.x - x0
    dy = @position.y - y0
    y = -ey * dx + ex * dy
    return if y > radius
    if y < 0.0
      @position.z = if @velocity.z < 0.0 then radius else -radius
      @velocity.z = @velocity.z * -1.0
      @velocity.multiplyScalar 0.125
      @stage.ball_net()
    else
      x = ex * dx + ey * dy
      @position.x = x0 + ex * x - ey * radius
      @position.y = y0 + ey * x + ex * radius
      if @velocity.z < 0.0
        @position.z = 0.0 if @position.z < 0.0
      else
        @position.z = 0.0 if @position.z > 0.0
      v = new THREE.Vector3(ex, ey, 0.0)
      p = new THREE.Vector3(dx, dy, @position.z)
      n = p.clone().cross(v).normalize()
      m = v.clone().cross(n).normalize()
      vv = v.dot(@velocity) * 0.375
      vn = n.dot(@velocity) * 0.375
      vm = m.dot(@velocity) * 0.0
      @velocity = v.multiplyScalar(vv).add(n.multiplyScalar(vn)).add(m.multiplyScalar(vm))
      @stage.ball_chip()
    @net = true
  netin: ->
    return if @position.x < -21 * 12 * 0.0254
    if @position.x < -0.0254
      @netin_part -21 * 12 * 0.0254, (3 * 12 + 6) * 0.0254, -0.0254, 3 * 12 * 0.0254
    else if @position.x < 0.0254
      @netin_part -0.0254, 3 * 12 * 0.0254, 0.0254, 3 * 12 * 0.0254
    else if @position.x < 21 * 12 * 0.0254
      @netin_part 0.0254, 3 * 12 * 0.0254, 21 * 12 * 0.0254, (3 * 12 + 6) * 0.0254
  emit_ace: ->
    @done = true
    @stage.ball_ace()
  emit_miss: ->
    @done = true
    @stage.ball_miss()
  emit_out: ->
    @done = true
    @stage.ball_out()
  emit_serve_air: ->
    @done = true
    @stage.ball_serve_air()
  emit_bounce: -> @stage.ball_bounce()
  wall: ->
    return if @done
    if @in
      @emit_ace()
    else
      @emit_out()
  rally = [-(13 * 12 + 6) * 0.0254, (13 * 12 + 6) * 0.0254, -39 * 12 * 0.0254]
  step: ->
    last = @position.clone()
    @position.add @velocity
    @position.y = @position.y - 0.5 * G
    @velocity.y = @velocity.y - G
    @velocity.add(@spin.clone().cross(@velocity).multiplyScalar(1.0 / 1500.0)).multiplyScalar(0.999)
    @spin.multiplyScalar 0.99
    if @position.y - radius <= 0.0
      @position.y = radius
      @bounce()
      unless @done
        if @in
          @emit_ace()
        else if @hitter == null
          @emit_serve_air()
        else
          x = @hitter.end * @position.x
          z = @hitter.end * @position.z
          if z > 0.0
            @emit_miss()
          else if x < @target[0] || x > @target[1] || z < @target[2]
            @emit_out()
          else
            @in = true
            if @serving() && @net
              @done = true
              @stage.ball_let()
            else
              @stage.ball_in()
              @target = rally
      @emit_bounce() if @velocity.y > 1.0 / 64.0
    if @position.x - radius <= -30 * 12 * 0.0254
      @position.x = radius - 30 * 12 * 0.0254
      @velocity.x = @velocity.x * -0.5
      @wall()
      @emit_bounce() if Math.abs(@velocity.x) > 1.0 / 64.0
    else if @position.x + radius >= 30 * 12 * 0.0254
      @position.x = 30 * 12 * 0.0254 - radius
      @velocity.x = @velocity.x * -0.5
      @wall()
      @emit_bounce() if Math.abs(@velocity.x) > 1.0 / 64.0
    if @position.z - radius <= -60 * 12 * 0.0254
      @position.z = radius - 60 * 12 * 0.0254
      @velocity.z = @velocity.z * -0.5
      @wall()
      @emit_bounce() if Math.abs(@velocity.z) > 1.0 / 64.0
    else if @position.z + radius >= 60 * 12 * 0.0254
      @position.z = 60 * 12 * 0.0254 - radius
      @velocity.z = @velocity.z * -0.5
      @wall()
      @emit_bounce() if Math.abs(@velocity.z) > 1.0 / 64.0
    if @velocity.z < 0.0
      @netin() if last.z > radius && @position.z <= radius
    else
      @netin() if last.z < -radius && @position.z >= -radius
  set: (hitter) ->
    @hitter = hitter
    @in = @net = false
  reset: (side, x, y, z, serving = true) ->
    @position.set x, y, z
    @velocity.set 0.0, 0.0, 0.0
    @spin.set 0.0, 0.0, 0.0
    @done = false
    x0 = 1 * 0.0254 * side
    x1 = -(13 * 12 + 6) * 0.0254 * side
    @target = if serving then [
      if x0 < x1 then x0 else x1
      if x0 < x1 then x1 else x0
      -21 * 12 * 0.0254
    ] else rally
    @set null
  hit: (hitter) ->
    return if @done
    if @target == rally
      @set hitter
    else
      @target = rally
      @hitter = hitter
      @emit_miss()
  serving: -> @target != rally
  impact: (dx, dz, speed, vy, spin) ->
    dl = 1.0 / Math.sqrt(dx * dx + dz * dz)
    dx *= dl
    dz *= dl
    @velocity.set dx * speed, vy, dz * speed
    @spin.set -dz * spin.x + dx * spin.z, spin.y, dx * spin.x + dz * spin.z
  calculate_bounce: (velocity, spin) ->
    f = 0.0
    v0 = velocity
    w0 = spin
    v1x = v0.x + radius * w0.z
    v1z = v0.z - radius * w0.x
    e = 1.25 + v0.y * 10.0
    e = 0.0 if e < 0.25
    e = 1.0 if e > 1.0
    b = (e + 2.0 / 3.0) * radius
    w1x = (1.0 - e) * v0.z + b * w0.x + f * v1z
    w1z = (e - 1.0) * v0.x + b * w0.z - f * v1x
    velocity.x = e * v1x - 3.0 / 5.0 * w1z
    velocity.y = v0.y * -0.75
    velocity.z = e * v1z + 3.0 / 5.0 * w1x
    d = 3.0 / 5.0 / radius
    spin.x = w1x * d
    spin.z = w1z * d
  bounce: -> @calculate_bounce @velocity, @spin
  projected_time_for_y: (y, sign) -> projected_time_for_y @position.y, @velocity.y, y, sign
  create_record: ->
    record = {}
    @record record
    record
  record: (to) ->
    to.position = @position.clone()
    to.velocity = @velocity.clone()
    to.spin = @spin.clone()
  replay: (from) ->
    @position.copy from.position
    @velocity.copy from.velocity
    @spin.copy from.spin
exports.Ball = Ball

class Mark
  constructor: ->
    @duration = 0
    @stretch = 1.0
    @node = new THREE.Mesh new THREE.CircleGeometry(radius), new THREE.MeshBasicMaterial({color: 0x000000})
    @node.position.y = 1.0 / 64.0
    node__render = @node.render
  setup: ->
    @node.visible = @duration > 0
    @node.scale.y = @stretch
  step: -> --@duration if @duration > 0
  mark: (ball) ->
    @duration = 2 * 64
    @node.position.x = ball.position.x
    @node.position.z = ball.position.z
    v = @node.up.set(ball.velocity.x, 0.0, ball.velocity.z)
    @stretch = 1.0 + v.length() * 8.0
    @node.lookAt new THREE.Vector3(0.0, 1.0, 0.0).add(@node.position)
  create_record: ->
    record = {}
    @record record
    record
  record: (to) ->
    to.duration = @duration
    to.stretch = @stretch
    to.position = @node.position.clone()
    to.quaternion = @node.quaternion.clone()
  replay: (from) ->
    @duration = from.duration
    @stretch = from.stretch
    @node.position.copy from.position
    @node.quaternion.copy from.quaternion
exports.Mark = Mark
