Script = {}

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

    local eventSystem = EventSystem.get()
    eventSystem:subscribe(EventType.KeyPressed, entity)
    eventSystem:subscribe(EventType.MouseMoved, entity)

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
    end

    if not self.isGrounded then
        self.velocityV = self.velocityV + self.gravity
    end

    -- Move player
    local disp = self.movementH + self.movementV + vec3.new(0, self.velocityV / 1000.0, 0)
    self.pc:move(disp)

    -- if on the ground
    if self.isGrounded then
        local hDir = self.movementH + self.movementV
        if math.abs(hDir.x) > 0.0 or math.abs(hDir.y) > 0.0 then
            local angle = -math.atan(hDir.z, hDir.x)
            self.modelTransform:setLocalRotation(angle + math.pi / 2, vec3.new(0, 1, 0))

            local animState = "Run"
            if self.animator:getCurrentAnimationName() ~= animState then
                self.animator:playAnimation(animState)
            end
        else
            local animState = "Idle"
            if self.animator:getCurrentAnimationName() ~= animState then
                self.animator:playAnimation(animState)
            end
        end
    else
        local animState = "Jump"
        if self.animator:getCurrentAnimationName() ~= animState then
            self.animator:playAnimation(animState)
        end
    end


    

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



    

    if e:type() == EventType.KeyPressed then
        if e.keysym == KeyCode.SCANCODE_SPACE then
            if self.isGrounded then
				self.velocityV = self.jumpForce;
            end
        end
    end
end

function Script:destroy(entity)
    local window = Window.get();
    window:unlockMouse()
end