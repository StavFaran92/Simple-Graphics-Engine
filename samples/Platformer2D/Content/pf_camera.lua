Script = {}

-- Follows the player from the side, looking a little ahead of where it faces
local DISTANCE = 13
local HEIGHT = 2.5
local LOOK_AHEAD = 2.0
local FOLLOW = 5 -- higher follows tighter
local MIN_X = 2

function Script:create(entity)
    self.player = getActiveScene():getEntityByName("Player")
    self.lastX = nil
    self.ahead = LOOK_AHEAD
end

function Script:update(entity, dt)
    local p = self.player.Transform:getWorldPosition()
    if self.lastX then
        local dx = p.x - self.lastX
        if math.abs(dx) > 0.002 then self.ahead = dx > 0 and LOOK_AHEAD or -LOOK_AHEAD end
    end
    self.lastX = p.x

    local cam = entity.Transform:getWorldPosition()
    local t = 1 - math.exp(-FOLLOW * dt)
    local x = cam.x + (math.max(p.x + self.ahead, MIN_X) - cam.x) * t
    local y = cam.y + (math.max(p.y, 0) + HEIGHT - cam.y) * t
    entity.Transform:setWorldPosition(vec3.new(x, y, DISTANCE))
end

function Script:destroy(entity)
end
