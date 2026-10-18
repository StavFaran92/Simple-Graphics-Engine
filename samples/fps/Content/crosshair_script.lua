-- Auto-generated Lua script

Script = {}

function Script:create(entity)
    -- UI space is window pixels with a top-left origin; position is the image's top-left corner
    local window = Window.get()
    local image = entity.Image
    image.position = vec2.new(
        (window:width() - image.size.x) / 2,
        (window:height() - image.size.y) / 2)
end

function Script:update(dt)
    -- update logic
end

function Script:destroy()
    -- destroy
end
