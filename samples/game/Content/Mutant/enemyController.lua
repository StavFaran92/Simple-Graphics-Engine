Script = {}

local SEEK_RANGE   = 10.0
local ATTACK_RANGE = 1.5

local PROBE_UP  = 1.0   -- ground ray starts this far above the feet
local SNAP_DOWN = 0.3   -- ground this far below still counts (keeps it glued going downhill)

function Script:create(entity)
    self.player         = getActiveScene():getEntityByName("player")
    self.physics        = entity.Physics
    self.model          = entity:getChildByName("model")
    self.modelTransform = self.model.Transform
    self.attack_collider = self.model:getChildByName("attack_collider").Physics
    self.speed          = 1.2 -- units per second
    self.animator       = self.model.Animator
    self.isGrounded     = false
    self.isKnockedBack  = false -- body is dynamic while hurt, PhysX moves it
    self.velocityV      = 0.0
    self.gravity        = -10.0
    self.moveDir        = nil
    self.toPlayer       = vec3.new(0, 0, 0)
    self.distToPlayer   = 0
    self.transform      = entity.Transform

    local s = self  -- capture for state closures

    local function nearPlayer()  return s.distToPlayer <= SEEK_RANGE   end
    local function canAttack()   return s.distToPlayer <= ATTACK_RANGE  end

    local function facePlayer()
        if s.distToPlayer < 0.001 then return end
        local dir = s.toPlayer / s.distToPlayer
        s.modelTransform:setLocalRotation(-math.atan(dir.z, dir.x) + math.pi / 2, vec3.new(0, 1, 0))
    end

    local IdleState = {
        onEnter  = function(state)
            s.moveDir = nil
            s.animator:playAnimation("Idle", true)
        end,
        onUpdate = function(state, dt)
            if nearPlayer() then s.sm:transitionTo("Follow") end
        end,
        onExit   = function(state) end,
    }

    local FollowState = {
        onEnter  = function(state)
            s.animator:playAnimation("Walk", true)
        end,
        onUpdate = function(state, dt)
            if canAttack() then
                s.sm:transitionTo("Attack")
            elseif not nearPlayer() then
                s.sm:transitionTo("Idle")
            else
                local dir = s.toPlayer / s.distToPlayer
                s.moveDir = vec3.new(dir.x, 0, dir.z)
                facePlayer()
            end
        end,
        onExit   = function(state)
            s.moveDir = nil
        end,
    }

    local AttackState = {
        onEnter  = function(state)
            s.moveDir = nil
            s.animator:playAnimation("Attack", false)
        end,
        onUpdate = function(state, dt)
            facePlayer()
            -- A swing always plays to the end
            if s.animator:isFinished() then
                s.sm:transitionTo(canAttack() and "Attack" or "Follow")
            end
        end,
        onExit   = function(state)
            -- The swing may be cut short before attack_end fires
            s.attack_collider:deactivate()
        end,
    }

    local HurtState = {
        onEnter  = function(state)
            s.moveDir = nil
            s.animator:playAnimation("Hit", false)

            local dir = s.toPlayer / s.distToPlayer
            dir = -vec3.new(dir.x, 0, dir.z)

            s.isKnockedBack = true
            s.physics:turnToDynamic()
            s.physics:setForce(dir * 150)
        end,
        onUpdate = function(state, dt)
            if s.animator:isFinished() then s.sm:transitionTo("Idle") end
        end,
        onExit   = function(state) 
            s.physics:setForce(vec3.new(0))
            s.physics:turnToKinematic()
            s.isKnockedBack = false
            --s.velocityV = 0
        end,
    }

    self.sm = StateMachine.new()
    self.sm:addState("Idle",   IdleState)
    self.sm:addState("Follow", FollowState)
    self.sm:addState("Attack", AttackState)
    self.sm:addState("Hurt", HurtState)
    self.sm:transitionTo("Idle")
end

function Script:update(entity, dt)
    local playerTransform = self.player.Transform
    self.toPlayer     = playerTransform:getWorldPosition() - entity.Transform:getWorldPosition()
    self.distToPlayer = length(self.toPlayer)

    self.sm:update(dt)

    -- The body is kinematic, so terrain never stops it - this is the only thing keeping it grounded.
    -- Runs in every state; skipped while hurt since PhysX drives the dynamic body then.
    if not self.isKnockedBack then
        local horizontal = vec3.new(0)
        if self.moveDir then
            horizontal = self.moveDir * (self.speed * dt)
        end
        self:moveWithGravity(dt, horizontal)
    end
end

-- Must be the only physics:move call per frame - move() overwrites, it does not accumulate.
function Script:moveWithGravity(dt, horizontal)
    local feet = self.transform:getWorldPosition()

    self.velocityV = self.velocityV + self.gravity * dt
    local fall = math.max(0, -self.velocityV * dt) -- this frame's full drop, gravity included

    -- Start above the feet so the ray never begins inside the terrain
    local hitResult = HitResult.new()
    local grounded = raycast(
        feet + vec3.new(0, PROBE_UP, 0),
        vec3.new(0, -1, 0),
        PROBE_UP + fall + SNAP_DOWN,
        hitResult,
        LayerMask.Ground
    )

    local dy
    if grounded then
        dy = hitResult.position.y - feet.y -- > 0 pushes out of a hill, < 0 snaps down
        self.velocityV = 0
    else
        dy = self.velocityV * dt
    end
    self.isGrounded = grounded

    self.physics:move(horizontal + vec3.new(0, dy, 0))
end

function Script:onAnimationTrigger(name, frame)
    if name == "attack_start" then self.attack_collider:activate()   end
    if name == "attack_end"   then self.attack_collider:deactivate() end
end

function Script:onTriggerEnter(entity, other)
    if other.Tag:getTag() == "player_attack_collider" then
        self.sm:transitionTo("Hurt")
    end
end

function Script:destroy(entity)
end
