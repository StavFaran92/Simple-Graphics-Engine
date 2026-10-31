Script = {}

-- The engine crashes on a trigger error that isn't a string, so errors are re-raised as text
local function guarded(fn, ...)
    local ok, err = pcall(fn, ...)
    if not ok then error(tostring(err), 0) end
end


-- Touching it costs a life and sends the player back to the checkpoint
function Script:create(entity) end
function Script:update(entity, dt) end
local function onEnter(self, entity, other)
    invoke(other, "hurt")
end
function Script:onTriggerEnter(entity, other)
    guarded(onEnter, self, entity, other)
end

function Script:destroy(entity) end
