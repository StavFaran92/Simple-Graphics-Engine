Script = {}

local HIT_GRACE = 1.5 -- seconds of invulnerability after being hit

Script.cameraPivot = Ref(Entity)
Script.mainCamera = Ref(Entity)
Script.model = Ref(Entity)

function Script:create(entity)
    self.movementSpeed = 5.0
    self.movementH = vec3.new(0)
    self.movementV = vec3.new(0)
    self.camComponent = self.mainCamera.Camera
    self.transform = entity.Transform
    self.pc = entity.PlayerController
    self.modelTransform = self.model.Transform
    self.animator = self.model.Animator
    self.velocity = -10.0
    self.velocityV = -10.0
    self.gravity = -10.0
    self.yaw = 0;
    self.pitch = 0;
    self.turnSpeed = 10.0;
    self.jumpForce = 200;
    self.isGrounded = false
    self.isJumping = false
    self.graceTimer = 0 -- > 0 while invulnerable
    self.hDir = vec3.new(0) -- this frame's horizontal input, already scaled by speed * dt
    self.canMove = false    -- set by Run/Jump each frame
    self.wantAttack = false -- input requests, consumed by the states
    self.wantJump = false

    local s = self  -- capture for state closures

    local function isMoving() return length(s.hDir) > 0 end

    -- Shared by Idle and Run: start an attack or a jump if requested
    local function handleActions()
        if s.wantAttack then
            s.sm:transitionTo("Attack")
            return true
        end
        if s.wantJump and s.isGrounded then
            s.velocityV = s.jumpForce
            s.isJumping = true
            s.sm:transitionTo("Jump")
            return true
        end
        return false
    end

    local IdleState = {
        onEnter  = function(state) s.animator:playAnimation("Idle", true) end,
        onUpdate = function(state, dt)
            if handleActions() then return end
            if isMoving() then s.sm:transitionTo("Run") end
        end,
        onExit   = function(state) end,
    }

    local RunState = {
        onEnter  = function(state) s.animator:playAnimation("Run", true) end,
        onUpdate = function(state, dt)
            if handleActions() then return end
            if not isMoving() then
                s.sm:transitionTo("Idle")
            else
                s.canMove = true
            end
        end,
        onExit   = function(state) end,
    }

    local JumpState = {
        onEnter  = function(state) s.animator:playAnimation("Jump", false) end,
        onUpdate = function(state, dt)
            if not s.isJumping then
                s.sm:transitionTo(isMoving() and "Run" or "Idle")
            else
                s.canMove = true
            end
        end,
        onExit   = function(state) end,
    }

    local AttackState = {
        onEnter  = function(state) s.animator:playAnimation("Attack", false) end,
        onUpdate = function(state, dt)
            if s.animator:isFinished() then
                s.sm:transitionTo(isMoving() and "Run" or "Idle")
            end
        end,
        onExit   = function(state)
            -- The swing may be cut short (e.g. by a hit) before attack_end fires
            s.attack_collider:deactivate()
        end,
    }

    local HurtState = {
        onEnter  = function(state) s.animator:playAnimation("GetHurt", false) end,
        onUpdate = function(state, dt)
            if s.animator:isFinished() then s.sm:transitionTo("Idle") end
        end,
        onExit   = function(state) end,
    }

    self.sm = StateMachine.new()
    self.sm:addState("Idle",   IdleState)
    self.sm:addState("Run",    RunState)
    self.sm:addState("Jump",   JumpState)
    self.sm:addState("Attack", AttackState)
    self.sm:addState("Hurt",   HurtState)
    self.sm:transitionTo("Idle")

    local eventSystem = EventSystem.get()
    eventSystem:subscribe(EventType.KeyPressed, entity)
    eventSystem:subscribe(EventType.MouseMoved, entity)
    eventSystem:subscribe(EventType.MouseButtonPressed, entity)

    local window = Window.get();
    window:lockMouse()
end

function Script:update(entity, dt)
    self.graceTimer = math.max(0, self.graceTimer - dt)

    local velocity = self.movementSpeed * dt
    local keyboard = Keyboard.get()

        
   
    -- Horizontal movement (forward/back)
    if keyboard:getKeyState(KeyCode.SCANCODE_W) > 0 then
        self.movementH = vec3.new(self.camComponent.front.x, 0, self.camComponent.front.z) * velocity
    elseif keyboard:getKeyState(KeyCode.SCANCODE_S) > 0 then
        self.movementH = -vec3.new(self.camComponent.front.x, 0, self.camComponent.front.z) * velocity
    else
        self.movementH = vec3.new(0)
    end

    -- Vertical movement (left/right)
    if keyboard:getKeyState(KeyCode.SCANCODE_A) > 0 then
        self.movementV = -vec3.new(self.camComponent.right.x, 0, self.camComponent.right.z) * velocity
    elseif keyboard:getKeyState(KeyCode.SCANCODE_D) > 0 then
        self.movementV = vec3.new(self.camComponent.right.x, 0, self.camComponent.right.z) * velocity
    else
        self.movementV = vec3.new(0)
    end

    local hitResult = HitResult.new()
    self.isGrounded = raycast(self.modelTransform:getWorldPosition(), 
        vec3.new(0, -1, 0), 
        0.01, 
        hitResult, 
        LayerMask.Ground);  

    if self.isGrounded and self.velocityV < 0 then
        self.velocityV = 0
        self.isJumping = false
    end

    if not self.isGrounded then
        self.velocityV = self.velocityV + self.gravity
    end

    self.hDir = self.movementH + self.movementV

    -- States decide whether we can move this frame and consume the input requests
    self.canMove = false
    self.sm:update(dt)
    self.wantAttack = false
    self.wantJump = false

    -- Move player
    local disp = vec3.new(0, self.velocityV / 1000.0, 0)

    if self.canMove and length(self.hDir) > 0 then
        disp = disp + self.hDir

        -- Rotate model toward movement direction
        -- local angle = -math.atan(self.hDir.z, self.hDir.x)
        -- self.modelTransform:setLocalRotation(angle + math.pi / 2, vec3.new(0, 1, 0))
    end
    self.pc:move(disp)
end

function Script:onEvent(e)
    if e:type() == EventType.MouseMoved then
        
        local system = System.get()
        local xChange = e.xrel
        local yChange = e.yrel
        xChange = xChange * self.turnSpeed * system:getDeltaTime()
        yChange = yChange * self.turnSpeed * system:getDeltaTime()
        self.yaw = self.yaw - xChange
        self.pitch = self.pitch - yChange

        if self.pitch > 70.0 then
            self.pitch = 70.0
        end
        if self.pitch < -70.0 then
            self.pitch = -70.0
        end

        local pitchQuat = angleAxis(math.rad(self.pitch), vec3.new(1, 0, 0))
        local yawQuat = angleAxis(math.rad(self.yaw), vec3.new(0, 1, 0))
        local combinedQuat = yawQuat * pitchQuat

        local transform = self.cameraPivot.Transform
        transform:setWorldRotation(combinedQuat)
    end



    

    if e:type() == EventType.MouseButtonPressed then
        if e.button == MouseButton.Left then
            self.wantAttack = true
        end
    end

    if e:type() == EventType.KeyPressed then
        if e.keysym == KeyCode.SCANCODE_SPACE then
            self.wantJump = true
        end
    end
end

function Script:onAnimationTrigger(name, frame)
    if name == "attack_start" then
        self.attack_collider:activate()
    end
    if name == "attack_end" then
        self.attack_collider:deactivate()
    end
end

function Script:onTriggerEnter(entity, other)
    local function faceEnemy()
        local dirToEnemy = other.Transform:getWorldPosition() - entity.Transform:getWorldPosition()
        dirToEnemy = normalize(dirToEnemy)
        self.modelTransform:setLocalRotation(-math.atan(dirToEnemy.z, dirToEnemy.x) + math.pi / 2, vec3.new(0, 1, 0))
    end

    if other.Tag:getTag() == "enemy_attack_collider" then
        -- Still in grace from the last hit: ignore so the player has time to escape
        if self.graceTimer > 0 then return end

        self.graceTimer = HIT_GRACE
        --faceEnemy()
        self.sm:transitionTo("Hurt")
    end
end

function Script:destroy(entity)
    local window = Window.get();
    window:unlockMouse()
end