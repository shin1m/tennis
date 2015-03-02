exports = module?.exports ? {}
window.computer = exports if window?

G = ball.G
projected_time_for_y = ball.projected_time_for_y
Ball = ball.Ball
reach_range = player.reach_range
shot_direction = player.shot_direction
Player = player.Player

random = -> Math.round(24.0 * 60.0 * 60.0 * (Date.now() / 1000.0 % 1))

exports.controller = (controller, player) ->
  ball = @ball
  duration = 1.0 * 64.0
  decided0 = decided1 = left = right = forward = backward = false
  shot = 'flat'
  net = false
  reset_decision = ->
    decided0 = decided1 = left = right = forward = backward = false
    shot = 'flat'
  reset_move = -> player.left = player.right = player.forward = player.backward = false
  super__step = controller.step
  controller.step = =>
    if ball.done
      reset_move()
      reset_decision()
      net = false
      return super__step.call @
    else if ball.hitter == null
      if player == @server
        if player.state == Player::state_serve_set
          reset_move()
          if duration <= 0.0
            player.perform 'flat'
            duration = 1.0 * 64.0
          else
            duration -= 1.0
        else if player.state == Player::state_serve_toss
          if @second
            shot = 'lob'
          else
            i = random() % 10
            if i > 6
              shot = 'topspin'
            else if i > 4
              shot = 'slice'
            else
              shot = 'flat'
          swing = player.actions.serve.swing[shot]
          t = ball.projected_time_for_y(swing.spot.elements[13], 1.0)
          dt = if @second then 0.0 else 1.0
          dt += 1.0 if random() % 2 == 0
          if t < (swing.impact - swing.start) * 60.0 + dt
            net = random() % 10 > (if @second then 7 else 3)
            player.perform shot
          else if t < (swing.impact - swing.start) * 60.0 + 8.0
            if !player.left && !player.right
              i = random()
              if i % 8 < (if @second then 1 else 2)
                player.left = true
              else if i % 8 > (if @second then 6 else 5)
                player.right = true
      else
        reset_move()
    else if ball.hitter.end == player.end
      unless decided0
        decided0 = true
        net = random() % 10 > 6 unless net
      reset_move()
      point = new THREE.Vector3(ball.position.x, 0.0, ball.position.z)
      side0 = new THREE.Vector3(-(13 * 12 + 6) * 0.0254, 0.0, 21 * 12 * 0.0254 * player.end).sub(point).normalize()
      side1 = new THREE.Vector3((13 * 12 + 6) * 0.0254, 0.0, 21 * 12 * 0.0254 * player.end).sub(point).normalize()
      v = side0.add(side1)
      a = new THREE.Vector3(-v.z, 0.0, v.x).dot(point.negate().add(player.node.position))
      epsilon = 1.0 / 1.0
      if a < -epsilon
        player.left = true
      else if a > epsilon
        player.right = true
      z = player.node.position.z * player.end
      zt = if net then 21 * 12 * 0.0254 else 39 * 12 * 0.0254
      epsilon = 1.0 / 2.0
      if z < zt - epsilon
        player.backward = true
      else if z > zt + epsilon
        player.forward = true
    else if player.state == Player::state_default
      unless decided1
        decided1 = true
        i = random()
        if i % 3 == 1
          left = true
        else if i % 3 == 2
          right = true
        if i % 10 > 5
          forward = true
        else if i % 10 == 0
          backward = true
        i = random() % 10
        if i > 6
          shot = 'topspin'
        else if i > 4
          shot = 'slice'
        else if i > 3
          shot = 'lob'
        else
          shot = 'flat'
      position = ball.position
      velocity = ball.velocity
      v = player.direction().normalize()
      whichhand = player.whichhand(v)
      actions = player.actions.swing
      t = projected_time_for_y(position.y, velocity.y, Player::smash_height, 1.0)
      if t != null
        d = new THREE.Vector3(position.x + velocity.x * t, 0.0, position.z + velocity.z * t).sub(player.node.position).length()
        if d < 0.5 || d / player.speed <= t
          hand = if whichhand > Player::smash_hand then actions.forehand else actions.backhand
          swing = hand.smash
          ix = swing.spot.elements[12]
          iz = swing.spot.elements[14]
          t0 = 0.0
          t = projected_time_for_y(position.y, velocity.y, swing.spot.elements[13], 1.0)
          t = velocity.y / G if t == null
          t -= 2.0
      unless swing
        hand = if whichhand > 0.0 then actions.forehand else actions.backhand
        swing = if net && !ball.in then hand.volley else hand.stroke[shot]
        ix = swing.spot.elements[12]
        iz = swing.spot.elements[14]
        if net || ball.in
          t0 = 0.0
        else
          t0 = Math.ceil(ball.projected_time_for_y(Ball::radius, 1.0))
          position = new THREE.Vector3(position.x + velocity.x * t0, Ball::radius, position.z + velocity.z * t0)
          velocity = new THREE.Vector3(velocity.x, velocity.y - G * t0, velocity.z)
          ball.calculate_bounce(velocity, ball.spin.clone())
        if net && !ball.in
          point = new THREE.Vector3(v.z, 0.0, -v.x).multiplyScalar(ix).add(v.multiplyScalar(iz)).add(player.node.position)
          t = reach_range(position, velocity, point, player.speed, 0.0, -1.0) + 1.0
        else
          t = projected_time_for_y(position.y, velocity.y, 1.25, -1.0)
          t = velocity.y / G if t == null
      point = new THREE.Vector3(position.x + velocity.x * t, 0.0, position.z + velocity.z * t)
      v = shot_direction(point, player.end, left, right, forward, backward).normalize()
      point = new THREE.Vector3(v.z, 0.0, -v.x).multiplyScalar(ix).add(v.multiplyScalar(iz)).add(player.node.position)
      tt = t0 + t
      t1 = t0 + reach_range(position, velocity, point, player.speed, t0, 1.0)
      tt = t1 if t1 >= 0.0 && t1 < tt
      if tt < (swing.impact - swing.start) * 60.0 + 1.0
        reset_move()
        player.left = left
        player.right = right
        player.forward = forward
        player.backward = backward
        player.perform shot
        reset_decision()
        net = player.node.position.z * player.end < 22 * 12 * 0.0254
      else
        reset_move()
        point = new THREE.Vector3(position.x + velocity.x * t, 0.0, position.z + velocity.z * t)
        v = shot_direction(point, player.end, left, right, forward, backward).normalize()
        target = new THREE.Vector3(-v.z, 0.0, v.x).multiplyScalar(ix).sub(v.multiplyScalar(iz)).add(point)
        epsilon = if tt > 32.0 then 1.0 / 8.0 else 1.0 / 32.0
        if player.node.position.x * player.end < target.x * player.end - epsilon
          player.right = true
        else if player.node.position.x * player.end > target.x * player.end + epsilon
          player.left = true
        epsilon = if tt > 32.0 then 1.0 else 1.0 / 4.0
        if player.node.position.z * player.end < target.z * player.end - epsilon
          player.backward = true
        else if player.node.position.z * player.end > target.z * player.end + epsilon
          player.forward = true
    super__step.call @
