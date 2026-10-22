Script = {}

-- Runs in a circle around where the entity starts, facing the way it moves
function Script:create(entity)
    self.transform = entity.Transform
    self.center = self.transform:getLocalPosition()
    self.radius = 5.0
    self.turnRate = 1.1 -- radians per second around the circle
    self.angle = 0.0
    entity.Animator:playAnimation("Run", true)
end

function Script:update(entity, dt)
    self.angle = self.angle + self.turnRate * dt
    local x = self.center.x + math.sin(self.angle) * self.radius
    local z = self.center.z + math.cos(self.angle) * self.radius
    self.transform:setLocalPosition(vec3.new(x, self.center.y, z))
    -- The tangent of the circle; the model faces +Z
    self.transform:setLocalRotation(self.angle + math.pi / 2, vec3.new(0, 1, 0))
end

function Script:destroy(entity)
end
