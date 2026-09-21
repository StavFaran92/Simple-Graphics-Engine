-- Auto-generated Lua script

Script = {}

function Script:create(entity)
    local prefab = Prefab.new("zombie")

    local gridSize = 50
    local spacing = 1

    -- for i = 0, gridSize - 1 do
    --     for j = 0, gridSize - 1 do
    --         prefab:instansiate(vec3.new(i * spacing, 0, j * spacing))
    --     end
    -- end

    prefab:instansiate(vec3.new(0, 0, 0))
    -- initialization logic
end

function Script:update(dt)
    -- update logic
end

function Script:destroy()
    -- destroy
end
