local PLAYAREA_WIDTH = 10
local PLAYAREA_HEIGHT = 20
local PLAYAREA_X = 5
local PLAYAREA_Y = 5
local PLAYAREA = {}

-- Game
local currentPiece = nil
local nextPiece = nil
local score = 0
local speedMin = 0.1
local speedStart = 1.0
local speedDecrement = 0.025
local step = speedStart
local rows = 0
local time = 0
local blink = false
local blinkTime = 0
local gameState = "PLAYING"

--[[
	  8   4   2   1
	+---+---+---+---+
	|   | # |   |   |  0x4000
	+---+---+---+---+
	|   | # |   |   |  0x0400
	+---+---+---+---+
	|   | # | # |   |  0x0060
	+---+---+---+---+
	|   |   |   |   |  0x0000 = 0x4460
	+---+---+---+---+
]]--

local I = { blocks = { 0x4444, 0x0F00, 0x2222, 0x00F0 }, c0 = Fx:color("green"), c1 = Fx:color("darkgreen") }
local J = { blocks = { 0x44C0, 0x8E00, 0x6440, 0x0E20 }, c0 = Fx:color("orange"), c1 = Fx:color("darkbrown") }
local L = { blocks = { 0x4460, 0x0E80, 0xC440, 0x2E00 }, c0 = Fx:color("yellow"), c1 = Fx:color("darkbrown") }
local O = { blocks = { 0xCC00, 0xCC00, 0xCC00, 0xCC00 }, c0 = Fx:color("purple"), c1 = Fx:color("darkgray") }
local S = { blocks = { 0x06C0, 0x8C40, 0x6C00, 0x4620 }, c0 = Fx:color("blue"), c1 = Fx:color("darkblue") }
local T = { blocks = { 0x0E40, 0x4C40, 0x4E00, 0x4640 }, c0 = Fx:color("red"), c1 = Fx:color("darkbrown") }
local Z = { blocks = { 0x0C60, 0x4C80, 0xC600, 0x2640 }, c0 = Fx:color("brown"), c1 = Fx:color("darkbrown") }

local RES = {}
local Sounds = {}

function printPlayAreaLine(y)
	local line = ""
	for x = 1, PLAYAREA_WIDTH do
		if PLAYAREA[y][x] == nil then
			line = line .. "[ ]"
		else
			line = line .. "[X]"
		end
	end
	print(line)
end

function printPlayArea()
	for y = 1, PLAYAREA_HEIGHT do
		printPlayAreaLine(y)
	end
end

function dump(o)
	if type(o) == 'table' then
	   local s = '{ '
	   for k, v in pairs(o) do
		  if type(k) ~= 'number' then k = '"'..k..'"' end
		  s = s .. '['..k..'] = ' .. dump(v) .. ', '
	   end
	   return s .. '} '
	else
	   return tostring(o)
	end
 end

function table.clone(orig)
	local orig_type = type(orig)
	local copy
	if orig_type == 'table' then
		copy = {}
		for orig_key, orig_value in pairs(orig) do
			copy[orig_key] = orig_value
		end
	else -- number, string, boolean, etc
		copy = orig
	end
	return copy
end

pieces = {}
function randomPiece()
	if #pieces == 0 then
		pieces = { I, I, I, I, J, J, J, J, L, L, L, L, O, O, O, O, S, S, S, S, T, T, T, T, Z, Z, Z, Z }
	end
	local i = math.random(1, #pieces)
	ret = { type = table.clone(pieces[i]), dir = 1, x = 3, y = 1 }
	-- print(dump(ret))
	table.remove(pieces, i)
	return ret
end

function getBlock(x, y)
	return PLAYAREA[y][x]
end

function setBlock(x, y, type)
	PLAYAREA[y][x] = type
end

function setRows(n)
	rows = n
	print("ROWS: "..tostring(rows))
	step = math.max(speedMin, speedStart - (speedDecrement * rows))
	print("STEP: "..tostring(step))
end

function eachBlock(type, x, y, dir, func)
	local bit = 0x8000
	local row = 1
	local col = 1

	while true do
		if (type.blocks[dir] & bit) > 0 then
			func(x + col - 1, y + row - 1)
		end
		col = col + 1
		if col >= 5 then
			col = 1
			row = row + 1
		end
		bit = bit >> 1
		if bit <= 0 then break end
	end
end

function occupied(type, x, y, dir)
	local res = false
	eachBlock(type, x, y, dir,
		function(x, y)
			if not res and (x < 1 or y < 1 or x > PLAYAREA_WIDTH or y > PLAYAREA_HEIGHT or getBlock(x, y)) then
				res = true
			end
		end
	)
	return res
end

function move(dir)
	if currentPiece == nil then return end
	local x = currentPiece.x
	local y = currentPiece.y
	if dir == 0 then -- LEFT
		x = x - 1
	elseif dir == 1 then -- RIGHT
		x = x + 1
	elseif dir == 2 then -- DOWN
		y = y + 1
	end

	if not occupied(currentPiece.type, x, y, currentPiece.dir) then
		currentPiece.x = x
		currentPiece.y = y
		return true
	end
	return false
end

function rotate(dir)
	local newdir = currentPiece.dir + 1
	if newdir > 4 then
		newdir = 1
		currentPiece.dir = newdir
	end
	if not occupied(currentPiece.type, currentPiece.x, currentPiece.y, newdir) then
		currentPiece.dir = newdir
		return true
	end
	return false
end

function setCurrentPiece(piece)
	if piece == nil then
		currentPiece = randomPiece()
		return
	end
	currentPiece = piece
	currentPiece.x = 4
	currentPiece.y = 1
end

function setNextPiece(piece)
	if piece == nil then
		nextPiece = randomPiece()
		return
	end
	nextPiece = piece
end

function dropPiece()
	if currentPiece == nil then return end
	eachBlock(currentPiece.type, currentPiece.x, currentPiece.y, currentPiece.dir,
		function(x, y)
			setBlock(x, y, currentPiece.type)
		end
	)
end

function removeLine(n)
	-- print("Remove Line")
	for y = n, 1, -1 do
		for x = 1, PLAYAREA_WIDTH do
			setBlock(x, y, nil)
			if y > 1 then
				setBlock(x, y, getBlock(x, y-1))
			end
		end
	end
end

function removeLines()
	-- print("Remove Lines -=-=-=-=-=-=-")
	local n = 0
	local y = PLAYAREA_HEIGHT
	while y >= 1 do
		-- printPlayAreaLine(y)
		local complete = true
		for x = 1, PLAYAREA_WIDTH do
			if getBlock(x, y) == nil then
				complete = false
			end
		end
		if complete then
			removeLine(y)
			y = y + 1
			n = n + 1
		end
		y = y - 1
	end

	if n > 0 then
		Fx:playSound(Sounds["clearRow"], false)
		setRows(rows + n)
		score = score + (100 * math.pow(2, n - 1))
	end
end

function drop()
	if not move(2) then
		score = score + 10
		dropPiece()
		removeLines()
		setCurrentPiece(nextPiece)
		setNextPiece(randomPiece())
		nextPiece.x = 0
		nextPiece.y = 0
		if occupied(currentPiece.type, currentPiece.x, currentPiece.y, currentPiece.dir) then
			gameState = "LOST"
			Fx:playSound(Sounds["lose"], false)
		end
	end
end

function drawBlock(g, x, y, b)
	g:remap(2, b.c0)
	g:remap(1, b.c1)
	g:sprite(RES["block"], PLAYAREA_X + 3 + (x-1) * 8, PLAYAREA_Y + 3 + (y-1) * 8)
	g:remap()
end

function drawPiece(g, px, py, piece)
	if piece == nil then return end
	eachBlock(piece.type, piece.x, piece.y, piece.dir,
		function(x, y)
			drawBlock(g, x + px, y + py, piece.type)
		end
	)
end

function create()
	Fx:title("Tetris")
	Fx:resize(180, 175, 2)

	for y = 1, PLAYAREA_HEIGHT do
		PLAYAREA[y] = {}
		for x = 1, PLAYAREA_WIDTH do
			PLAYAREA[y][x] = nil
		end
	end

	RES["block"] = Fx:loadImage("block.png", 0)
	RES["playarea"] = Fx:loadImage("area.png", 0)
	RES["nextarea"] = Fx:loadImage("next.png", 0)
	RES["font"] = Fx.defaultFont
	RES["font"].spacing = 0

	setCurrentPiece(nil)
	setNextPiece(randomPiece())
	nextPiece.x = 0
	nextPiece.y = 0

	math.randomseed(os.time())

	Sounds["clearRow"] = Fx:createSound()
	Sounds["clearRow"]:set("F3A3C4D4 F3A3C4D4 F3A3C4D4 F3A3C4D4", "T", "5543 4432 3321 2211", "SV")
	Sounds["clearRow"].speed = 0.02

	Sounds["lose"] = Fx:createSound()
	Sounds["lose"]:set("F3C1 F3C1 F3C1 F3C1", "N", "53 42 31 21", "N")
	Sounds["lose"].speed = 0.05
end

function draw(g)
	local nextAreaX = PLAYAREA_X + RES["playarea"].width + 10
	local nextAreaY = PLAYAREA_Y + 8
	local scoreAreaY = nextAreaY + RES["nextarea"].height + 8

	g:clear(Fx:color("gray"))
	g:sprite(RES["playarea"], PLAYAREA_X, PLAYAREA_Y)
	g:sprite(RES["nextarea"], nextAreaX, nextAreaY)

	drawPiece(g, 14, 3, nextPiece)

	for y = 1, PLAYAREA_HEIGHT do
		for x = 0, PLAYAREA_WIDTH do
			local block = getBlock(x, y)
			if block ~= nil then
				drawBlock(g, x, y, block)
			end
		end
	end
	drawPiece(g, 0, 0, currentPiece)

	g:transparency(Fx:color("red"))
	g:text(RES["font"], "Next", nextAreaX, nextAreaY - 8)
	g:text(RES["font"], "Score\n"..string.format("%08d", math.floor(score)), nextAreaX, scoreAreaY)

	local centerX = math.floor(Fx.width / 2)
	local centerY = math.floor(Fx.height / 2)
	local h = math.floor(RES["font"].height / 2)
	if gameState == "PAUSED" then
		local w = math.floor(RES["font"]:stringWidth("Paused") / 2)
		g:rect(0, centerY - h - 2, centerX*2, h*2 + 4, Fx:color("black"), true)
		if blink then g:text(RES["font"], "Paused", centerX - w, centerY - h) end
	elseif gameState == "LOST" then
		local w = math.floor(RES["font"]:stringWidth("Game Over!!!") / 2)
		g:rect(0, centerY - h - 2, centerX*2, h*2 + 4, Fx:color("black"), true)
		if blink then g:text(RES["font"], "Game Over!!!", centerX - w, centerY - h) end
	end
	g:transparency()

	Fx:flip()
end

function update(dt)
	blinkTime = blinkTime + dt
	if blinkTime >= 0.25 then
		blinkTime = 0
		blink = not blink
	end

	if gameState == "PLAYING" then
		if Fx:buttonPressed("left") then
			move(0)
		elseif Fx:buttonPressed("right") then
			move(1)
		elseif Fx:buttonPressed("down") then
			move(2)
		elseif Fx:buttonPressed("up") then
			rotate()
		end

		if Fx:buttonPressed("select") then
			gameState = "PAUSED"
		end

		time = time + dt
		if time >= step then
			time = 0
			drop()
		end
	elseif gameState == "LOST" then
		if Fx:buttonPressed("x") then
			gameState = "PLAYING"
			score = 0
			time = 0
			setCurrentPiece(nil)
			setNextPiece(randomPiece())
			setRows(0)
			nextPiece.x = 0
			nextPiece.y = 0
		end
	elseif gameState == "PAUSED" then
		if Fx:buttonPressed("select") then
			gameState = "PLAYING"
		end
	end
end