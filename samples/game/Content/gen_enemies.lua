-- Auto-generated Lua script

Script = {}

function Script:create(entity)
    local prefab = Prefab.new("zombie")

    local gridSize = 10
    local spacing = 5

    for i = 0, gridSize - 1 do
        for j = 0, gridSize - 1 do
            prefab:instansiate(vec3.new(i * spacing, 0, j * spacing))
        end
    end
    -- initialization logic
end

function Script:update(dt)
    -- update logic
end

function Script:destroy()
    -- destroy
end
