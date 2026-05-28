Script = {}

local SEEK_RANGE   = 10.0
local ATTACK_RANGE = 1.5

function Script:create(entity)
    self.player         = getActiveScene():getEntityByName("player")
    self.physics        = entity.Physics
    self.model          = entity:getChildByName("model")
    self.modelTransform = self.model.Transform
    self.attack_collider = self.model:getChildByName("attack_collider").Physics
    self.speed          = 0.02
    self.animator       = self.model.Animator
    self.isGrounded     = false
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
        end,
        onUpdate = function(state, dt)
            if nearPlayer() then s.sm:transitionTo("Follow") end
        end,
        onExit   = function(state) end,
    }

    local FollowState = {
        lastY = 0,
        expectedPos = 0,
        onEnter  = function(state) end,
        onUpdate = function(state, dt)
            if canAttack() then
                s.sm:transitionTo("Attack")
            elseif not nearPlayer() then
                s.sm:transitionTo("Idle")
            else
                local dir = s.toPlayer / s.distToPlayer
                s.moveDir = vec3.new(dir.x, 0, dir.z)
                facePlayer()

                local maxFall = math.abs(s.velocityV * dt)
                local rayLength = math.max(0.2, maxFall + 0.05) -- small buffer on top

                local hitResult = HitResult.new()
                s.isGrounded = raycast(
                    s.modelTransform:getWorldPosition(), 
                    vec3.new(0, -1, 0), 
                    rayLength, 
                    hitResult, 
                    LayerMask.LAYER_0
                )

                local moveVector = vec3.new(0)
                if s.isGrounded and s.velocityV < 0 then
                    s.velocityV = 0
                    moveVector = vec3.new(0, -hitResult.distance, 0)
                else
                    if not s.isGrounded then
                        s.velocityV = s.velocityV + s.gravity * dt
                    end
                    moveVector = vec3.new(0, s.velocityV * dt, 0)
                end

                if s.moveDir then
                    moveVector = moveVector + s.moveDir * s.speed
                end

                s.physics:move(moveVector)
                
            end
        end,
        onExit   = function(state)
            s.moveDir = nil
        end,
    }

    local AttackState = {
        onEnter  = function(state)
            s.moveDir = nil
            s.animator:getGraph():setBool("attack", true)
        end,
        onUpdate = function(state, dt)
            facePlayer()
            if not canAttack() then s.sm:transitionTo("Follow") end
        end,
        onExit   = function(state)
            s.animator:getGraph():setBool("attack", false)
        end,
    }

    local HurtState = {
        onEnter  = function(state)
            s.moveDir = nil
            s.animator:getGraph():trigger("hit")

            local dir = s.toPlayer / s.distToPlayer
            dir = -vec3.new(dir.x, 0, dir.z)

            s.physics:turnToDynamic()
            s.physics:setForce(dir * 500)
        end,
        onUpdate = function(state, dt) end,
        onExit   = function(state) 
            s.physics:setForce(vec3.new(0))
            s.physics:turnToKinematic()
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

    -- gravity
    -- local hitResult = HitResult.new()
    -- self.isGrounded = raycast(self.modelTransform:getWorldPosition(), vec3.new(0, -1, 0), .5, hitResult, LayerMask.LAYER_0)

    -- if self.isGrounded and self.velocityV < 0 then
    --     self.velocityV = 0
    -- end
    -- if not self.isGrounded then
    --     self.velocityV = self.velocityV + self.gravity
    -- end

    local moveVector = vec3.new(0)
    if self.moveDir then
        moveVector = moveVector + self.moveDir * self.speed
    end
    
    --self.physics:move(moveVector)
    
    self.sm:update(dt)
    self.animator:getGraph():setFloat("speed", length(moveVector))
end

function Script:onAnimationTrigger(name, frame)
    if name == "attack_start" then self.attack_collider:activate()   end
    if name == "attack_end"   then self.attack_collider:deactivate() end
    if name == "hurt_end" then
        self.sm:transitionTo("Idle")
    end
end

function Script:onTriggerEnter(entity, other)
    if other.Tag:getTag() == "player_attack_collider" then
        self.sm:transitionTo("Hurt")
    end
end

function Script:destroy(entity)
end
