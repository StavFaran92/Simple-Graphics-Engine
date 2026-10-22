Script = {}

-- Reports the animator's state through error(), since Lua errors reach the editor console
function Script:create(entity)
    self.animator = entity.Animator
    self.time = 0.0
    self.reports = 0
    self.animator:playAnimation("Run", true)
end

function Script:update(entity, dt)
    self.time = self.time + dt
    if self.reports < 3 and self.time > 0.5 * (self.reports + 1) then
        self.reports = self.reports + 1
        error("PROBE anim=" .. tostring(self.animator:getCurrentAnimationName()) .. " frame=" .. tostring(self.animator:getCurrentFrame()))
    end
end

function Script:destroy(entity)
end
