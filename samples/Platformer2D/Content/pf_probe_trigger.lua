Script = {}
function Script:create(entity) end
function Script:update(entity, dt) end
function Script:onTriggerEnter(entity, other)
    error("PROBE trigger entered by entity " .. tostring(other:handlerID()))
end
function Script:destroy(entity) end
