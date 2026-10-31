Script = {}

-- Walks back and forth. Landing on it from above defeats it and bounces the player; touching it from the side hurts
local RANGE = 2.5
local SPEED = 2
local HALF = 0.4      -- half its size
local PLAYER_RADIUS = 0.3
local PLAYER_HEIGHT = 1.7

function Script:create(entity)
    self.start = entity.Transform:getLocalPosition()
    self.t = 0
    self.alive = true
    self.player = getActiveScene():getEntityByName("Player")
end

function Script:update(entity, dt)
    if not self.alive then return end
    self.t = self.t + dt
    local x = self.start.x + math.sin(self.t * SPEED / RANGE) * RANGE
    entity.Transform:setLocalPosition(vec3.new(x, self.start.y, self.start.z))

    local e = entity.Transform:getWorldPosition()
    local p = self.player.Transform:getWorldPosition()
    local overlaps = math.abs(p.x - e.x) < HALF + PLAYER_RADIUS and p.y < e.y + HALF + 0.05 and p.y + PLAYER_HEIGHT > e.y - HALF
    if not overlaps then return end

    if p.y > e.y then
        self.alive = false
        entity.Transform:setLocalScale(vec3.new(0.8, 0.15, 0.8)) -- squashed
        entity.Transform:setLocalPosition(vec3.new(x, self.start.y - HALF + 0.08, self.start.z))
        invoke(self.player, "bounce")
    else
        invoke(self.player, "hurt")
    end
end

function Script:destroy(entity)
end
