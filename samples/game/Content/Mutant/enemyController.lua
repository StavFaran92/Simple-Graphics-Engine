-- Auto-generated Lua script

Script = {}

function Script:create(entity)
    self.player = getActiveScene():getEntityByName("player")
    self.physics = entity.Physics
    self.speed = 10.0
end

function Script:update(entity, dt)
    local transform       = entity.Transform
    local playerTransform = self.player.Transform

    local dir = normalize(playerTransform:getWorldPosition() - transform:getWorldPosition())
    local movement = vec3.new(dir.x, 0, dir.z) * self.speed

    self.physics:setForce(movement)
end

function Script:destroy(entity)
end
