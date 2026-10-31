Script = {}
-- Teleports onto the platform, then stands still and reports x
function Script:create(entity)
    self.t = 0
    self.vy = 0
    self.reported = {}
end
function Script:update(entity, dt)
    self.t = self.t + dt
    local pc = entity.PlayerController
    if not self.placed then
        self.placed = true
        pc:teleport(vec3.new(-6, 2.2, 0))
    end
    if pc.isGrounded then self.vy = -1 else self.vy = self.vy - 20 * dt end
    pc:move(vec3.new(0, self.vy * dt, 0))
    for _, at in ipairs({ 1.0, 1.6, 2.2, 2.8 }) do
        if self.t >= at and not self.reported[at] then
            self.reported[at] = true
            local p = entity.Transform:getWorldPosition()
            error(string.format("PROBE ride t=%.2f x=%.2f y=%.2f grounded=%s", self.t, p.x, p.y, tostring(pc.isGrounded)))
        end
    end
end
function Script:destroy(entity) end
