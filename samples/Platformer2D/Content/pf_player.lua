Script = {}

-- Side-scroller player: runs and jumps on the z = 0 plane through a PlayerController.
-- Coins, spikes, enemies, checkpoints and the goal call coin / hurt / bounce / checkpoint / win on it with invoke().
local AUTOPLAY = false -- runs right and jumps over gaps and walls by itself, for testing without a keyboard
local TOTAL_COINS = 12

local RUN_SPEED = 6.5
local GROUND_ACCEL = 45
local AIR_ACCEL = 25
local GRAVITY = 32
local JUMP_SPEED = 12.5
local MAX_FALL = 22
local COYOTE_TIME = 0.1   -- a jump still works this long after running off a ledge
local JUMP_BUFFER = 0.12  -- a jump pressed this long before landing still happens
local BOUNCE_SPEED = 10
local DEATH_Y = -8
local LIVES = 3
local INVULNERABLE = 1.5

local function sign(v) return v > 0 and 1 or (v < 0 and -1 or 0) end

local function approach(v, target, step)
    if v < target then return math.min(v + step, target) end
    return math.max(v - step, target)
end

function Script:create(entity)
    -- The entity passed to create() / update() is only valid during that call; a looked-up copy stays valid
    self.entity = entity
    self.model = entity:getChildByName("Player Model")
    self.keyboard = Keyboard.get()
    self.prevKeys = {}
    self.hit = HitResult.new()

    local scene = getActiveScene()
    self.hud = {
        coins = scene:getEntityByName("HUD Coins"),
        lives = scene:getEntityByName("HUD Lives"),
        message = scene:getEntityByName("HUD Message"),
    }

    local p = entity.Transform:getWorldPosition()
    self.spawn = vec3.new(p.x, p.y, 0)
    self.respawnPoint = self.spawn
    self.lives = LIVES
    self.coins = 0
    self:respawn()
    self:updateHud()
end

function Script:respawn()
    self.entity.PlayerController:teleport(self.respawnPoint)
    self.vx, self.vy = 0, 0
    self.facing = 1
    self.coyote, self.buffer = 0, 0
    self.invulnerable = INVULNERABLE
    self.rideId, self.ridePos = nil, nil
    self.anim = nil
    self.state = "playing"
end

-- ---- Called by other scripts through invoke() ----

function Script:coin()
    self.coins = self.coins + 1
    self:updateHud()
end

function Script:hurt(fell)
    if self.state ~= "playing" or self.invulnerable > 0 then return end
    if AUTOPLAY and not fell then return end -- the test run can't see enemies or spikes, only falls cost it
    self.lives = self.lives - 1
    self:updateHud()
    if self.lives <= 0 then
        self.state = "dead"
        self:message("GAME OVER - SPACE TO RETRY")
        return
    end
    self:respawn()
end

function Script:bounce()
    self.vy = BOUNCE_SPEED
end

function Script:checkpoint()
    if self.state ~= "playing" then return end
    local p = self.entity.Transform:getWorldPosition()
    self.respawnPoint = vec3.new(p.x, p.y + 0.5, 0)
end

function Script:win()
    if self.state ~= "playing" then return end
    self.state = "won"
    self:message("LEVEL COMPLETE!  COINS " .. self.coins .. " / " .. TOTAL_COINS)
end

-- ---- HUD ----

function Script:updateHud()
    self.hud.coins.Text.text = "COINS  " .. self.coins .. " / " .. TOTAL_COINS
    self.hud.lives.Text.text = "LIVES  " .. self.lives
end

function Script:message(text)
    local msg = self.hud.message.Text
    msg.text = text
    msg.isActive = text ~= ""
end

-- ---- Input ----

function Script:down(key)
    local v = self.keyboard:getKeyState(key)
    return v == true or (type(v) == "number" and v ~= 0)
end

function Script:tapped(name, isDown)
    local was = self.prevKeys[name]
    self.prevKeys[name] = isDown
    return isDown and not was
end

function Script:readInput()
    local left = self:down(KeyCode.SCANCODE_LEFT) or self:down(KeyCode.SCANCODE_A)
    local right = self:down(KeyCode.SCANCODE_RIGHT) or self:down(KeyCode.SCANCODE_D)
    local jump = self:down(KeyCode.SCANCODE_SPACE) or self:down(KeyCode.SCANCODE_UP) or self:down(KeyCode.SCANCODE_W)
    if AUTOPLAY then return self:autoInput() end
    return (right and 1 or 0) - (left and 1 or 0), self:tapped("jump", jump), jump
end

-- Runs right. At an edge it jumps if there is something to land on within reach, otherwise waits there
-- (for a moving platform to come by); it also jumps over anything blocking the way
function Script:autoInput()
    local p = self.entity.Transform:getWorldPosition()
    local function standable(dx)
        return raycast(vec3.new(p.x + dx, p.y + 0.3, 0), vec3.new(0, -1, 0), 6.0, self.hit, 0) and self.hit.position.y > p.y - 2.5
    end

    local input, jump = 1, false
    if standable(0.9) then
        jump = raycast(vec3.new(p.x + 0.4, p.y + 0.4, 0), vec3.new(1, 0, 0), 0.8, self.hit, 0)
    elseif standable(2.5) or standable(3.5) then
        jump = true
    else
        input = 0
    end
    return input, self:tapped("jump", jump), jump
end

-- ---- Movement ----

-- The controller isn't carried by moving platforms, so add the platform's own movement while standing on it
function Script:platformDelta(p)
    if not raycast(vec3.new(p.x, p.y + 0.05, 0), vec3.new(0, -1, 0), 0.3, self.hit, 0) then
        self.rideId = nil
        return 0, 0
    end
    local id = self.hit.entity:handlerID()
    local at = self.hit.entity.Transform:getWorldPosition()
    local dx, dy = 0, 0
    if id == self.rideId and self.ridePos then
        dx, dy = at.x - self.ridePos.x, at.y - self.ridePos.y
    end
    self.rideId, self.ridePos = id, at
    return dx, dy
end

function Script:playAnim(name, loop)
    if self.anim == name then return end
    self.anim = name
    self.model.Animator:playAnimation(name, loop)
end

function Script:update(entity, dt)
    dt = math.min(dt, 1 / 30)

    if self.state ~= "playing" then
        if self.state == "dead" and self:tapped("retry", self:down(KeyCode.SCANCODE_SPACE)) then
            self.lives = LIVES
            self.respawnPoint = self.spawn
            self:message("")
            self:updateHud()
            self:respawn()
        end
        entity.PlayerController:move(vec3.new(0, -2 * dt, 0))
        self:playAnim("Idle", true)
        return
    end

    local pc = entity.PlayerController
    local p = entity.Transform:getWorldPosition()
    local input, jumpPressed, jumpHeld = self:readInput()

    if p.y < DEATH_Y then
        self.invulnerable = 0
        return self:hurt(true)
    end

    -- Horizontal: accelerate toward the input speed, slower in the air
    local accel = pc.isGrounded and GROUND_ACCEL or AIR_ACCEL
    self.vx = approach(self.vx, input * RUN_SPEED, accel * dt)
    if input ~= 0 then self.facing = sign(input) end

    -- Vertical: gravity, coyote time and a buffered jump
    if pc.isGrounded then
        self.coyote = COYOTE_TIME
        if self.vy < 0 then self.vy = -2 end
    else
        self.coyote = self.coyote - dt
        local g = GRAVITY
        if self.vy > 0 and not jumpHeld then g = GRAVITY * 2.2 end -- short hop when the jump is let go early
        self.vy = math.max(self.vy - g * dt, -MAX_FALL)
    end
    self.buffer = jumpPressed and JUMP_BUFFER or self.buffer - dt
    if self.buffer > 0 and self.coyote > 0 then
        self.vy = JUMP_SPEED
        self.buffer, self.coyote = 0, 0
    end

    -- A head bump stops the rise
    if self.vy > 0 and self.lastY and p.y <= self.lastY and not pc.isGrounded then self.vy = 0 end
    self.lastY = p.y

    local rideX, rideY = 0, 0
    if pc.isGrounded then rideX, rideY = self:platformDelta(p) else self.rideId = nil end
    pc:move(vec3.new(self.vx * dt + rideX, self.vy * dt + rideY, 0))

    -- Model: face the run direction, blink while invulnerable, pick the animation
    self.model.Transform:setLocalRotation(self.facing * math.pi / 2, vec3.new(0, 1, 0))
    if self.invulnerable > 0 then
        self.invulnerable = self.invulnerable - dt
        self.model.MeshRenderer.isActive = self.invulnerable <= 0 or math.floor(self.invulnerable * 10) % 2 == 0
    end
    if not pc.isGrounded and self.coyote <= 0 then
        self:playAnim("Jump", false)
    elseif math.abs(self.vx) > 0.5 then
        self:playAnim("Run", true)
    else
        self:playAnim("Idle", true)
    end
end

function Script:destroy(entity)
end
