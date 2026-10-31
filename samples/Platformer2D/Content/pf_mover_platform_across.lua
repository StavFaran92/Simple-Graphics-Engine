Script = {}

-- Kinematic platform sliding along one axis around where it starts
local AXIS = vec3.new(1, 0, 0)
local RANGE = 2.5
local SPEED = 0.9

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
