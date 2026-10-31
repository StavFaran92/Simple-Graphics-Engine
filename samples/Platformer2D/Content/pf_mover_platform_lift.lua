Script = {}

-- Kinematic platform sliding along one axis around where it starts
local AXIS = vec3.new(0, 1, 0)
local RANGE = 2.2
local SPEED = 0.8

function Script:create(entity)
    self.t = 0
    self.offset = 0
end

function Script:update(entity, dt)
    self.t = self.t + dt
    local offset = math.sin(self.t * SPEED) * RANGE
    entity.Physics:move(AXIS * (offset - self.offset))
    self.offset = offset
end

function Script:destroy(entity)
end
