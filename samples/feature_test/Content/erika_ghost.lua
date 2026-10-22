Script = {}

-- Idles while floating up and down and slowly turning
function Script:create(entity)
    self.transform = entity.Transform
    self.base = self.transform:getLocalPosition()
    self.time = 0.0
    entity.Animator:playAnimation("Idle", true)
end

function Script:update(entity, dt)
    self.time = self.time + dt
    local y = self.base.y + 0.4 + math.sin(self.time * 1.5) * 0.3
    self.transform:setLocalPosition(vec3.new(self.base.x, y, self.base.z))
    self.transform:setLocalRotation(self.time * 0.5, vec3.new(0, 1, 0))
end

function Script:destroy(entity)
end
