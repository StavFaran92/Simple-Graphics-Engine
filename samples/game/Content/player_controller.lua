Script = {}

--Player
----PlayerController
------Main Camera
------Model

function Script:create(entity)
    self.movementSpeed = 5.0
    self.movementH = vec3.new(0)
    self.movementV = vec3.new(0)
    self.camComponent = getActiveScene():getEntityByName("Main Camera").Camera
    self.transform = entity.Transform
    self.controller = entity:getChildByName("controller")
    self.pc = self.controller.PlayerController
    self.model = self.controller:getChildByName("model")
    self.modelTransform = self.model.Transform
    self.animator = self.model.Animator
    self.velocity = -10.0
    self.gravity = -10.0

    local eventSystem = EventSystem.get()
    eventSystem:subscribe(EventType.KeyPressed, entity)

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

    -- Combine horizontal, vertical, and vertical-velocity displacement
    local disp = self.movementH + self.movementV + vec3.new(0, self.velocity / 1000.0, 0)
    self.velocity = self.velocity + self.gravity
    
    -- Move player
    self.pc:move(disp)

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

    
end

function Script:onEvent(e)
    print("test")
end

function Script:destroy(entity)
end