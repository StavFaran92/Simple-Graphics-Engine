-- Auto-generated Lua script

Script = {}

function Script:create(entity)
    self.player = getActiveScene():getEntityByName("player")
    self.physics = entity.Physics
    self.model = entity:getChildByName("model")
    self.modelTransform = entity:getChildByName("model").Transform
    self.speed = 20.0
    self.animator = self.model.Animator
end

function Script:update(entity, dt)
    local transform       = entity.Transform
    local playerTransform = self.player.Transform

    local dir = normalize(playerTransform:getWorldPosition() - transform:getWorldPosition())
    local movement = vec3.new(dir.x, 0, dir.z) * self.speed

    self.physics:setForce(movement)

    if math.abs(dir.x) > 0.0 or math.abs(dir.y) > 0.0 then
        local angle = -math.atan(dir.z, dir.x)
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

function Script:destroy(entity)
end
