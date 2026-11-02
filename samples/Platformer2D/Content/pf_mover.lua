Script = {}

-- Kinematic platform gliding back and forth between where it starts and its target marker
Script.target = Ref(Entity)
local SPEED = 0.85

function Script:create(entity)
    self.t = 0
    self.start = entity.Transform:getWorldPosition()
    self.span = self.target.Transform:getWorldPosition() - self.start
    self.offset = vec3.new(0, 0, 0)
end

function Script:update(entity, dt)
    self.t = self.t + dt
    local offset = self.span * ((1 - math.cos(self.t * SPEED)) * 0.5)
    entity.Physics:move(offset - self.offset)
    self.offset = offset
end

function Script:destroy(entity)
end
