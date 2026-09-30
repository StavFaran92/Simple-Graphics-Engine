Script = {}

local SEEK_RANGE   = 10.0
local ATTACK_RANGE = 1.5

local PROBE_UP  = 1.0   -- ground ray starts this far above the feet
local SNAP_DOWN = 0.3   -- ground this far below still counts (keeps it glued going downhill)

local KNOCKBACK_SPEED = 20.0 -- initial knockback speed when hit, units per second
local KNOCKBACK_DAMP  = 6.0 -- per second; total slide distance is about SPEED / DAMP

local CAPSULE_RADIUS   = 0.2 -- must match the prefab's collider
local CAPSULE_CENTER_Y = 0.7 -- collider offset above the feet
local SEPARATION_DIST  = 1.8 -- enemies closer than this (center to center) push apart
local SEPARATION_SPEED = 2 -- push speed at full overlap, units per second

function Script:create(entity)
    self.player         = getActiveScene():getEntityByName("player")
    self.physics        = entity.Physics
    self.model          = entity:getChildByName("model")
    self.modelTransform = self.model.Transform
    self.attack_collider = self.model:getChildByName("attack_collider").Physics
    self.speed          = 3.0 -- units per second
    self.animator       = self.model.Animator
    self.isGrounded     = false
    self.knockback      = vec3.new(0) -- horizontal velocity from being hit, decays over time
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
            s.animator:playAnimation("Run", true)
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

            -- Stay kinematic: knockback goes through moveWithGravity so it can't tunnel the terrain
            s.knockback = dir * KNOCKBACK_SPEED
        end,
        onUpdate = function(state, dt)
            if s.animator:isFinished() then s.sm:transitionTo("Idle") end
        end,
        onExit   = function(state)
            s.knockback = vec3.new(0)
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
    -- Runs in every state.
    local horizontal = self.knockback * dt
    if self.moveDir then
        horizontal = horizontal + self.moveDir * (self.speed * dt)
    end
    self.knockback = self.knockback * math.max(0, 1 - KNOCKBACK_DAMP * dt)

    horizontal = horizontal + self:separation(entity) * (SEPARATION_SPEED * dt)

    self:moveWithGravity(dt, horizontal)
end

-- Horizontal push away from nearby enemies, length 0..1. The overlap query only visits
-- nearby shapes, so this never loops over every enemy.
function Script:separation(entity)
    local pos = self.transform:getWorldPosition()

    -- The query sphere touches a neighbor's capsule when their centers are within SEPARATION_DIST
    local neighbors = overlapSphere(
        pos + vec3.new(0, CAPSULE_CENTER_Y, 0),
        SEPARATION_DIST - CAPSULE_RADIUS,
        LayerMask.Enemy
    )

    local push = vec3.new(0)
    for _, other in ipairs(neighbors) do
        if not other:equals(entity) then
            local offset = pos - other.Transform:getWorldPosition()
            offset = vec3.new(offset.x, 0, offset.z)
            local dist = length(offset)

            if dist < SEPARATION_DIST then
                local dir
                if dist > 0.0001 then
                    dir = offset / dist
                else
                    -- Exactly stacked: pick any direction so they can split
                    local angle = math.random() * 2 * math.pi
                    dir = vec3.new(math.cos(angle), 0, math.sin(angle))
                end
                push = push + dir * (1 - dist / SEPARATION_DIST)
            end
        end
    end

    -- Cap so a dense crowd doesn't shove harder than a single full overlap
    local pushLen = length(push)
    if pushLen > 1 then
        push = push / pushLen
    end

    return push
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

-- Called by the player's shot via invoke(), and by melee hits below
function Script:hurt()
    self.sm:transitionTo("Hurt")
end

function Script:onTriggerEnter(entity, other)
    if other.Tag:getTag() == "player_attack_collider" then
        self:hurt()
    end
end

function Script:destroy(entity)
end
