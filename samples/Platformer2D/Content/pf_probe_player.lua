Script = {}

-- Test: walk right for 2 s, report, then teleport up and report where it lands
function Script:create(entity)
    self.t = 0
    self.vy = 0
    self.reported = {}
end

function Script:update(entity, dt)
    self.t = self.t + dt
    local pc = entity.PlayerController
    if pc.isGrounded then self.vy = -1 else self.vy = self.vy - 20 * dt end
    local vx = (self.t > 1 and self.t < 3) and 3 or 0
    if self.t > 3.5 and not self.teleported then
        self.teleported = true
        pc:teleport(vec3.new(-2, 3, 0))
        self.vy = 0
    end
    pc:move(vec3.new(vx * dt, self.vy * dt, 0))
    for _, at in ipairs({ 0.9, 2.9, 3.55, 4.6 }) do
        if self.t >= at and not self.reported[at] then
            self.reported[at] = true
            local p = entity.Transform:getWorldPosition()
            error(string.format("PROBE t=%.2f pos=(%.2f, %.2f, %.2f) grounded=%s", self.t, p.x, p.y, p.z, tostring(pc.isGrounded)))
        end
    end
end

function Script:destroy(entity)
end
