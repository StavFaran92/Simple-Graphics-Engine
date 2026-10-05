Script = {}

local HIT_GRACE = 1.5 -- seconds of invulnerability after being hit
local SHOOT_RANGE = 100.0 -- max distance a shot travels
local FIRE_INTERVAL = 0.1 -- seconds between shots while the button is held
local RECOIL_KICK = 1.2 -- degrees the view kicks up per shot
local RECOIL_SIDE = 0.3 -- max random sideways kick per shot, degrees
local RECOIL_RECOVER = 10.0 -- per second; how fast the view settles back after a kick

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
    self.velocityV = 0.0 -- units per second
    self.gravity = -36.0 -- units per second^2
    self.groundStick = -1.0 -- units per second; keeps pushing into the ground so the CCT keeps reporting it
    self.yaw = 0;
    self.pitch = 0;
    self.turnSpeed = 10.0;
    self.jumpForce = 12.0 -- initial upward speed, units per second
    self.isGrounded = false
    self.isJumping = false
    self.graceTimer = 0 -- > 0 while invulnerable
    self.fireCooldown = 0 -- <= 0 when the next shot may fire
    self.recoilPitch = 0 -- current kick in degrees, added on top of the aim and decays back to 0
    self.recoilYaw = 0
    self.hDir = vec3.new(0) -- this frame's horizontal input, already scaled by speed * dt
    self.canMove = false    -- set by Run/Jump each frame
    self.wantJump = false -- input request, consumed by the states

    local s = self  -- capture for state closures

    local function isMoving() return length(s.hDir) > 0 end

    -- Shared by Idle and Run: start a jump if requested
    local function handleActions()
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

    -- Hold to fire. Adding the interval (rather than resetting to it) keeps the rate frame-rate
    -- independent; clamping on release stops idle time from banking a burst of shots.
    self.fireCooldown = self.fireCooldown - dt
    if Mouse.get():getButtonPressed(MouseButton.Left) then
        if self.fireCooldown <= 0 then
            self:shoot()
            self.fireCooldown = self.fireCooldown + FIRE_INTERVAL
        end
    else
        self.fireCooldown = math.max(self.fireCooldown, 0)
    end

    -- Ease the recoil back toward the aim, then refresh the camera even if the mouse didn't move
    local recover = math.max(0, 1 - RECOIL_RECOVER * dt)
    self.recoilPitch = self.recoilPitch * recover
    self.recoilYaw = self.recoilYaw * recover
    self:applyCameraRotation()

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

    -- Ground contact as reported by the character controller's last move
    self.isGrounded = self.pc.isGrounded

    if self.isGrounded and self.velocityV < 0 then
        self.velocityV = self.groundStick
        self.isJumping = false
    end

    if not self.isGrounded then
        self.velocityV = self.velocityV + self.gravity * dt
    end

    self.hDir = self.movementH + self.movementV

    -- States decide whether we can move this frame and consume the input requests
    self.canMove = false
    self.sm:update(dt)
    self.wantJump = false

    -- Move player
    local disp = vec3.new(0, self.velocityV * dt, 0)

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

        self:applyCameraRotation()
    end



    

    if e:type() == EventType.KeyPressed then
        if e.keysym == KeyCode.SCANCODE_SPACE then
            self.wantJump = true
        end
    end
end

-- Hitscan from the camera along its view direction. Ground is in the mask so walls block shots;
-- only enemy scripts define hurt(), so invoke() is a no-op on anything else.
function Script:shoot()
    local origin = self.mainCamera.Transform:getWorldPosition()
    local hitResult = HitResult.new()
    if raycast(origin, self.camComponent.front, SHOOT_RANGE, hitResult, LayerMask.Ground | LayerMask.Enemy) then
        invoke(hitResult.entity, "hurt")
    end

    -- Kick after the trace so this shot lands where the player aimed
    self.recoilPitch = self.recoilPitch + RECOIL_KICK
    self.recoilYaw = self.recoilYaw + (math.random() * 2 - 1) * RECOIL_SIDE
end

-- Camera pivot rotation = aim (yaw/pitch from the mouse) + current recoil offset
function Script:applyCameraRotation()
    local pitch = math.max(-70.0, math.min(70.0, self.pitch + self.recoilPitch))
    local yaw = self.yaw + self.recoilYaw

    local pitchQuat = angleAxis(math.rad(pitch), vec3.new(1, 0, 0))
    local yawQuat = angleAxis(math.rad(yaw), vec3.new(0, 1, 0))
    self.cameraPivot.Transform:setWorldRotation(yawQuat * pitchQuat)
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