Script = {}

-- The engine crashes on a trigger error that isn't a string, so errors are re-raised as text
local function guarded(fn, ...)
    local ok, err = pcall(fn, ...)
    if not ok then error(tostring(err), 0) end
end


-- Passing it moves the player's respawn point here; the light on top turns on
function Script:create(entity)
    self.lamp = entity:getChildByName("Checkpoint Lamp")
    self.lamp.MeshRenderer.isActive = false
end
function Script:update(entity, dt) end
local function onEnter(self, entity, other)
    self.lamp.MeshRenderer.isActive = true
    invoke(other, "checkpoint")
end
function Script:onTriggerEnter(entity, other)
    guarded(onEnter, self, entity, other)
end

function Script:destroy(entity) end
