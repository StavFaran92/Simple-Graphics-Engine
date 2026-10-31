Script = {}

-- The engine crashes on a trigger error that isn't a string, so errors are re-raised as text
local function guarded(fn, ...)
    local ok, err = pcall(fn, ...)
    if not ok then error(tostring(err), 0) end
end


-- Spins and bobs; touching it gives the player a coin once
function Script:create(entity)
    self.t = 0
    self.base = entity.Transform:getLocalPosition()
    self.disc = entity:getChildByName("Coin Disc")
    self.taken = false
end

function Script:update(entity, dt)
    if self.taken then return end
    self.t = self.t + dt
    entity.Transform:setLocalRotation(self.t * 3, vec3.new(0, 1, 0))
    entity.Transform:setLocalPosition(vec3.new(self.base.x, self.base.y + math.sin(self.t * 3) * 0.1, self.base.z))
end

local function onEnter(self, entity, other)
    if self.taken then return end
    self.taken = true
    self.disc.MeshRenderer.isActive = false
    invoke(other, "coin")
end

function Script:onTriggerEnter(entity, other)
    guarded(onEnter, self, entity, other)
end

function Script:destroy(entity)
end
