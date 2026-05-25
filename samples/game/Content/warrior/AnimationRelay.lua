-- Auto-generated Lua script

Script = {}

Script.player_script = Ref(Entity)

function Script:create()
    -- initialization logic
end

function Script:update(dt)
    -- update logic
end

function Script:onAnimationTrigger(triggerName, frameID)
    if triggerName == "attack_start" then
        invoke(self.player_script, "onAnimationTrigger" , triggerName, frameID)

    end
end

function Script:destroy()
    -- destroy
end
