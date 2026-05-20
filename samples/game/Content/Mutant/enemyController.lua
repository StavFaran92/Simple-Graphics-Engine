-- Auto-generated Lua script

Script = {}

local SEEK_RANGE   = 10.0
local ATTACK_RANGE = 1.5

local State = {
    IDLE        = "idle",
    SEEK_PLAYER = "seekPlayer",
    ATTACK      = "attack",
}

function Script:create(entity)
    self.player = getActiveScene():getEntityByName("player")
    self.physics = entity.Physics
    self.model = entity:getChildByName("model")
    self.modelTransform = entity:getChildByName("model").Transform
    self.speed = 0.02
    self.animator = self.model.Animator
    self.isGrounded = false
    self.velocityV = -10.0
    self.gravity = -10.0
    self.state = State.IDLE
end

function Script:update(entity, dt)
    local transform       = entity.Transform
    local playerTransform = self.player.Transform

    local toPlayer = playerTransform:getWorldPosition() - transform:getWorldPosition()
    local distToPlayer = length(toPlayer)

    if distToPlayer <= ATTACK_RANGE then
        self.state = State.ATTACK
    elseif distToPlayer <= SEEK_RANGE then
        self.state = State.SEEK_PLAYER
    else
        self.state = State.IDLE
    end

    local hitResult = HitResult.new()
    self.isGrounded = raycast(self.modelTransform:getWorldPosition(), vec3.new(0, -1, 0), .5, hitResult, LayerMask.LAYER_0);

    if self.isGrounded and self.velocityV < 0 then
        self.velocityV = 0
    end

    if not self.isGrounded then
        self.velocityV = self.velocityV + self.gravity
    end

    local moveVector = vec3.new(0, self.velocityV / 1000.0, 0)

    if self.state == State.ATTACK then
        local animState = "Attack"
        if self.animator:getCurrentAnimationName() ~= animState then
            self.animator:playAnimation(animState)
        end
    elseif self.state == State.SEEK_PLAYER then
        local dir = toPlayer / distToPlayer
        moveVector = moveVector + vec3.new(dir.x, 0, dir.z) * self.speed

        local angle = -math.atan(dir.z, dir.x)
        self.modelTransform:setLocalRotation(angle + math.pi / 2, vec3.new(0, 1, 0))

        local animState = "Walk"
        if self.animator:getCurrentAnimationName() ~= animState then
            self.animator:playAnimation(animState)
        end
    else
        local animState = "Idle"
        if self.animator:getCurrentAnimationName() ~= animState then
            self.animator:playAnimation(animState)
        end
    end

    self.physics:move(moveVector)
end

function Script:onTriggerEnter(entity, other)
    print("Enemy hit!")
end

function Script:destroy(entity)
end
