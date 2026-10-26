Script = {}

-- Tetris on a 10x20 board in the XY plane (cell x,y sits at world x,y).
-- Every block is spawned from the Block_<kind> prefab and removed when its line clears.
local W, H = 10, 20
local AUTOPLAY = false -- plays by itself, for testing without a keyboard
local NEXT_X, NEXT_Y = W + 2, H - 5
local CUBE = 0.92

local SHAPES = {
    I = { n = 4, cells = { {0,2}, {1,2}, {2,2}, {3,2} } },
    O = { n = 2, cells = { {0,0}, {1,0}, {0,1}, {1,1} } },
    T = { n = 3, cells = { {0,1}, {1,1}, {2,1}, {1,2} } },
    S = { n = 3, cells = { {0,1}, {1,1}, {1,2}, {2,2} } },
    Z = { n = 3, cells = { {1,1}, {2,1}, {0,2}, {1,2} } },
    J = { n = 3, cells = { {0,1}, {1,1}, {2,1}, {0,2} } },
    L = { n = 3, cells = { {0,1}, {1,1}, {2,1}, {2,2} } },
}
local KINDS = { "I", "O", "T", "S", "Z", "J", "L" }

local function cellsOf(kind, rot)
    local s = SHAPES[kind]
    local out = {}
    for i, c in ipairs(s.cells) do
        local x, y = c[1], c[2]
        for _ = 1, rot do x, y = y, s.n - 1 - x end -- clockwise quarter turn
        out[i] = { x, y }
    end
    return out
end

local function hide(e)
    e.MeshRenderer.isActive = false
end

local function show(e, x, y)
    e.MeshRenderer.isActive = true
    e.Transform:setLocalScale(vec3.new(CUBE, CUBE, CUBE))
    e.Transform:setLocalPosition(vec3.new(x, y, 0))
end

function Script:create(entity)
    self.keyboard = Keyboard.get()
    self.prevKeys = {}
    self.repeatTimer = 0
    self.prefabs = {}
    for _, k in ipairs(KINDS) do self.prefabs[k] = Prefab.new("Block_" .. k) end
    local ghost = Prefab.new("Block_Ghost")
    self.ghost = {}
    for i = 1, 4 do
        self.ghost[i] = ghost:instansiate(vec3.new())
        hide(self.ghost[i])
    end
    -- Entities, not their Text: spawning blocks can move component storage
    local scene = getActiveScene()
    self.hud = {}
    for _, name in ipairs({ "Score", "Lines", "Level", "Message" }) do
        self.hud[name] = scene:getEntityByName("HUD " .. name)
    end
    self:reset()
end

function Script:updateHud()
    self.hud.Score.Text.text = "SCORE  " .. self.score
    self.hud.Lines.Text.text = "LINES  " .. self.lines
    self.hud.Level.Text.text = "LEVEL  " .. math.floor(self.lines / 10)
end

function Script:gameOver()
    self.over = true
    self.overTimer = 0
    self.hud.Message.Text.isActive = true
end

function Script:reset()
    if self.board then
        for _, cell in pairs(self.board) do self:release(cell.kind, cell.e) end
    end
    if self.piece then
        for _, e in ipairs(self.piece.cubes) do self:release(self.piece.kind, e) end
    end
    if self.next then
        for _, e in ipairs(self.next.cubes) do self:release(self.next.kind, e) end
    end
    self.board = {}
    self.bag = {}
    self.lines = 0
    self.score = 0
    self.over = false
    self.hud.Message.Text.isActive = false
    self:updateHud()
    self.fallTimer = 0
    self.piece = nil
    self.next = self:take(self:fromBag())
    self:spawn()
end

function Script:release(kind, e)
    getActiveScene():removeEntity(e)
end

-- 7-bag randomizer: every kind once per seven pieces
function Script:fromBag()
    if #self.bag == 0 then
        for _, k in ipairs(KINDS) do table.insert(self.bag, k) end
        for i = #self.bag, 2, -1 do
            local j = math.random(i)
            self.bag[i], self.bag[j] = self.bag[j], self.bag[i]
        end
    end
    return table.remove(self.bag)
end

function Script:take(kind)
    local piece = { kind = kind, rot = 0, cubes = {} }
    for i, c in ipairs(cellsOf(kind, 0)) do
        piece.cubes[i] = self.prefabs[kind]:instansiate(vec3.new())
        show(piece.cubes[i], NEXT_X + c[1], NEXT_Y + c[2])
    end
    return piece
end

function Script:spawn()
    local piece = self.next
    self.next = self:take(self:fromBag())

    local top = 0
    for _, c in ipairs(cellsOf(piece.kind, 0)) do top = math.max(top, c[2]) end
    piece.x = math.floor((W - SHAPES[piece.kind].n) / 2)
    piece.y = H - 1 - top
    self.piece = piece
    self:draw()
    if not self:fits(piece.kind, piece.rot, piece.x, piece.y) then return self:gameOver() end
end

function Script:fits(kind, rot, px, py)
    for _, c in ipairs(cellsOf(kind, rot)) do
        local x, y = px + c[1], py + c[2]
        if x < 0 or x >= W or y < 0 then return false end
        if y < H and self.board[y * W + x] then return false end
    end
    return true
end

function Script:dropY(p)
    local y = p.y
    while self:fits(p.kind, p.rot, p.x, y - 1) do y = y - 1 end
    return y
end

function Script:draw()
    local p = self.piece
    local cells = cellsOf(p.kind, p.rot)
    local gy = self:dropY(p)
    for i, c in ipairs(cells) do
        local x, y = p.x + c[1], p.y + c[2]
        if y < H then show(p.cubes[i], x, y) else hide(p.cubes[i]) end
        if self.ghost[i] then
            if gy ~= p.y then show(self.ghost[i], p.x + c[1], gy + c[2]) else hide(self.ghost[i]) end
        end
    end
end

function Script:move(dx, dy, drot)
    local p = self.piece
    local rot = (p.rot + drot) % 4
    -- Rotation tries small sideways kicks so pieces can turn next to a wall
    local kicks = drot ~= 0 and { 0, -1, 1, -2, 2 } or { 0 }
    for _, k in ipairs(kicks) do
        if self:fits(p.kind, rot, p.x + dx + k, p.y + dy) then
            p.x, p.y, p.rot = p.x + dx + k, p.y + dy, rot
            self:draw()
            return true
        end
    end
    return false
end

function Script:lock()
    local p = self.piece
    local cells = cellsOf(p.kind, p.rot)
    for _, c in ipairs(cells) do
        if p.y + c[2] >= H then return self:gameOver() end -- locked above the top
    end
    for i, c in ipairs(cells) do
        self.board[(p.y + c[2]) * W + p.x + c[1]] = { kind = p.kind, e = p.cubes[i] }
    end
    self.piece = nil

    -- Classic scoring: more lines at once are worth more, and every level multiplies it
    local cleared = self:clearLines()
    local points = { 100, 300, 500, 800 }
    if cleared > 0 then self.score = self.score + points[cleared] * (math.floor(self.lines / 10) + 1) end
    self:updateHud()
    self:spawn()
end

function Script:clearLines()
    local newBoard, row, cleared = {}, 0, 0
    for y = 0, H - 1 do
        local full = true
        for x = 0, W - 1 do
            if not self.board[y * W + x] then full = false break end
        end
        for x = 0, W - 1 do
            local cell = self.board[y * W + x]
            if cell then
                if full then
                    self:release(cell.kind, cell.e)
                else
                    newBoard[row * W + x] = cell
                    show(cell.e, x, row)
                end
            end
        end
        if full then cleared = cleared + 1 else row = row + 1 end
    end
    self.board = newBoard
    self.lines = self.lines + cleared
    return cleared
end

function Script:pressed(key)
    local v = self.keyboard:getKeyState(key)
    return v == true or (type(v) == "number" and v ~= 0)
end

-- True on the frame a key goes down, and again while held after a short delay
function Script:held(key, dt)
    local now, was = self:pressed(key), self.prevKeys[key]
    self.prevKeys[key] = now
    if now and not was then
        self.repeatTimer = 0.18
        return true
    end
    if now then
        self.repeatTimer = self.repeatTimer - dt
        if self.repeatTimer <= 0 then
            self.repeatTimer = 0.05
            return true
        end
    end
    return false
end

function Script:tapped(key)
    local now, was = self:pressed(key), self.prevKeys[key]
    self.prevKeys[key] = now
    return now and not was
end

-- Picks the rotation and column that leave the lowest, flattest board with the fewest holes
function Script:bestMove()
    local p = self.piece
    local best, bestScore = nil, -math.huge
    for rot = 0, 3 do
        for x = -3, W do
            if self:fits(p.kind, rot, x, p.y) then
                local y = p.y
                while self:fits(p.kind, rot, x, y - 1) do y = y - 1 end
                local filled = {}
                for _, c in ipairs(cellsOf(p.kind, rot)) do filled[(y + c[2]) * W + x + c[1]] = true end
                local function at(cx, cy) return self.board[cy * W + cx] or filled[cy * W + cx] end
                local lines, holes, heights = 0, 0, {}
                for cy = 0, H - 1 do
                    local full = true
                    for cx = 0, W - 1 do if not at(cx, cy) then full = false break end end
                    if full then lines = lines + 1 end
                end
                for cx = 0, W - 1 do
                    heights[cx] = 0
                    for cy = H - 1, 0, -1 do
                        if at(cx, cy) then
                            if heights[cx] == 0 then heights[cx] = cy + 1 end
                        elseif heights[cx] > 0 then
                            holes = holes + 1
                        end
                    end
                end
                local total, bump = 0, 0
                for cx = 0, W - 1 do
                    total = total + heights[cx]
                    if cx > 0 then bump = bump + math.abs(heights[cx] - heights[cx - 1]) end
                end
                local score = -0.51 * total + 0.76 * lines - 0.36 * holes - 0.18 * bump
                if score > bestScore then bestScore, best = score, { rot = rot, x = x } end
            end
        end
    end
    return best
end

function Script:update(entity, dt)
    if self.over then
        self.overTimer = self.overTimer + dt
        if self:tapped(KeyCode.SCANCODE_SPACE) or (AUTOPLAY and self.overTimer > 2) then self:reset() end
        return
    end

    if not self.piece then return end
    local p = self.piece

    if AUTOPLAY then
        self.fallTimer = self.fallTimer + dt
        if self.fallTimer < 0.15 then return end
        self.fallTimer = 0
        local m = self:bestMove()
        if m then p.rot, p.x = m.rot, m.x end
        p.y = self:dropY(p)
        self:draw()
        return self:lock()
    end

    if self:tapped(KeyCode.SCANCODE_UP) then self:move(0, 0, 1) end
    if self:held(KeyCode.SCANCODE_LEFT, dt) then self:move(-1, 0, 0) end
    if self:held(KeyCode.SCANCODE_RIGHT, dt) then self:move(1, 0, 0) end
    if self:tapped(KeyCode.SCANCODE_SPACE) then
        local landing = self:dropY(p)
        self.score = self.score + 2 * (p.y - landing)
        p.y = landing
        self:draw()
        return self:lock()
    end

    local level = math.floor(self.lines / 10)
    local interval = math.max(0.08, 0.8 - 0.07 * level)
    if self:pressed(KeyCode.SCANCODE_DOWN) then interval = 0.04 end
    self.fallTimer = self.fallTimer + dt
    if self.fallTimer >= interval then
        self.fallTimer = 0
        if not self:move(0, -1, 0) then self:lock() end
    end
end

function Script:destroy(entity)
end
