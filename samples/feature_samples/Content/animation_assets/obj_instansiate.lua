-- Auto-generated Lua script

Script = {}

function Script:create(entity)
    local prefab = Prefab.new(59)

    for i = -5, 5, 1 do
        for j =-5, 5, 1 do
            prefab:instansiate(vec3.new(i * 10, 0, j * 10))
        end
    end 
    -- initialization logic
end

function Script:update(entity, dt)
    -- update logic
end

function Script:destroy()
    -- destroy
end
