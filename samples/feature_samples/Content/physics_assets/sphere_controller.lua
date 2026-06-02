-- Auto-generated Lua script

Script = {}

function Script:create(entity)
    self.velocityV      = 0.0
    self.gravity        = -10.0
    self.transform        = entity.Transform
    self.physics        = entity.Physics
end

function Script:update(entity, dt)
    local s = self

    local maxFall = math.abs(self.velocityV * dt)
    local rayLength = math.max(0.2, maxFall + 0.05) -- small buffer on top

    local hitResult = HitResult.new()
    self.isGrounded = raycast(
        self.transform:getWorldPosition(), 
        vec3.new(0, -1, 0), 
        rayLength, 
        hitResult, 
        LayerMask.Ground
    )

    local moveVector = vec3.new(0)
    if s.isGrounded and s.velocityV < 0 then
        s.velocityV = 0
        moveVector = vec3.new(0, -hitResult.distance, 0)
    else
        if not s.isGrounded then
            s.velocityV = s.velocityV + s.gravity * dt
        end
        moveVector = vec3.new(0, s.velocityV * dt, 0)
    end

    if s.moveDir then
        moveVector = moveVector + s.moveDir * s.speed
    end

    s.physics:move(moveVector)
end

function Script:destroy()
    -- destroy
end
