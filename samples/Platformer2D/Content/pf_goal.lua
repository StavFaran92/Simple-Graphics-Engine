Script = {}

-- The engine crashes on a trigger error that isn't a string, so errors are re-raised as text
local function guarded(fn, ...)
    local ok, err = pcall(fn, ...)
    if not ok then error(tostring(err), 0) end
end


-- The flag waves; touching it wins the level
function Script:create(entity)
    self.t = 0
    self.flag = entity:getChildByName("Goal Flag")
end
function Script:update(entity, dt)
    self.t = self.t + dt
    self.flag.Transform:setLocalRotation(math.sin(self.t * 4) * 0.25, vec3.new(0, 1, 0))
end
local function onEnter(self, entity, other)
    invoke(other, "win")
end
function Script:onTriggerEnter(entity, other)
    guarded(onEnter, self, entity, other)
end

function Script:destroy(entity) end
