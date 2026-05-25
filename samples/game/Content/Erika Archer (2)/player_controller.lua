Script = {}

local PlayerState = {
    Idle   = "Idle",
    Run    = "Run",
    Jump   = "Jump",
    Attack = "Attack",
}

Script.target = Ref(Entity)
Script.spawnPoint = Ref(Entity)

--Player
----camera pivot
------Main Camera
----model

function Script:create(entity)
    self.movementSpeed = 5.0
    self.movementH = vec3.new(0)
    self.movementV = vec3.new(0)
    self.camComponent = getActiveScene():getEntityByName("Main Camera").Camera
    self.transform = entity.Transform
    self.cameraPivot = entity:getChildByName("cameraPivot")
    self.model = entity:getChildByName("model")
    self.attack_collider = self.model:getChildByName("attack_collider").Physics
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
    self.isAttacking = false
    self.state = PlayerState.Idle

    local eventSystem = EventSystem.get()
    eventSystem:subscribe(EventType.KeyPressed, entity)
    eventSystem:subscribe(EventType.MouseMoved, entity)
    eventSystem:subscribe(EventType.MouseButtonPressed, entity)

    local window = Window.get();
    window:lockMouse()
end

function Script:update(entity, dt)

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
    self.isGrounded = raycast(self.modelTransform:getWorldPosition(), vec3.new(0, -1, 0), 0.01, hitResult, LayerMask.LAYER_0);  

    if self.isGrounded and self.velocityV < 0 then
        self.velocityV = 0
        self.isJumping = false
    end

    if not self.isGrounded then
        self.velocityV = self.velocityV + self.gravity
    end

    -- Move player
    local disp = vec3.new(0, self.velocityV / 1000.0, 0)

    if self.animator:getGraph():getCurrentStateID() == "Run" or 
    self.animator:getGraph():getCurrentStateID() == "Jump" then
        disp = disp + self.movementH + self.movementV
    end
    self.pc:move(disp)

    -- Update state
    local hDir = self.movementH + self.movementV
    local isMoving = math.abs(hDir.x) > 0.0 or math.abs(hDir.z) > 0.0

    -- Rotate model toward movement direction
    if isMoving and not self.isJumping then
        local angle = -math.atan(hDir.z, hDir.x)
        self.modelTransform:setLocalRotation(angle + math.pi / 2, vec3.new(0, 1, 0))
    end

    local speed = length(hDir)
    self.animator:getGraph():setFloat("speed", speed)
    self.animator:getGraph():setBool("isJumping", self.isJumping)
    self.animator:getGraph():setBool("isGrounded", self.isGrounded)
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
        if self.pitch < 0.0 then
            self.pitch = 0.0
        end

        local pitchQuat = angleAxis(math.rad(self.pitch), vec3.new(-1, 0, 0))
        local yawQuat = angleAxis(math.rad(self.yaw), vec3.new(0, 1, 0))
        local combinedQuat = yawQuat * pitchQuat

        local transform = self.cameraPivot.Transform
        transform:setWorldRotation(combinedQuat)
    end



    

    if e:type() == EventType.MouseButtonPressed then
        if e.button == MouseButton.Left then
            self.animator:getGraph():trigger("attack")
        end
    end

    if e:type() == EventType.KeyPressed then
        if e.keysym == KeyCode.SCANCODE_SPACE then
            if self.isGrounded then
				self.velocityV = self.jumpForce;
                self.isJumping = true
                self.animator:getGraph():trigger("jump")
            end
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

function Script:destroy(entity)
    local window = Window.get();
    window:unlockMouse()
end