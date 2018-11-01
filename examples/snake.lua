-- Data --------------------
local SnakeFollowSpeed = 1
local MinSegmentDist = 4
local SnakeSpeed = 40.0
local FruitCount = 10
local FruitSpawnTime = 0.2
local SnakeImage = nil
local GameState = "PAUSE"
local OutlineBuf = nil
local SpawnTime = 0.0

function collides(x1, y1, w1, h1, x2, y2, w2, h2)
	return x1+w1 >= x2 and
			x1 <= x2+w2 and
			y1+h1 >= y2 and
			y1 <= y2+h2
end

local SnakeSegment = {}
SnakeSegment.__index = SnakeSegment
function SnakeSegment.new(pos, parent)
	return setmetatable({
		position = pos,
		parent = parent
	}, SnakeSegment)
end

function SnakeSegment:update()
	local vec = (self.parent.position - self.position)
	local dist = vec.length

	if dist > MinSegmentDist then
		local nvec = vec:normalized()
		self.position = self.position + nvec * SnakeFollowSpeed
	end
end

local Snake = {}
Snake.__index = Snake
function Snake.new()
	return setmetatable({
		segment = SnakeSegment.new(Vector.new(0), nil),
		body = {},
		rot = 0.0,
		score = 0
	}, Snake)
end

function Snake:init(x, y, length)
	self.segment.position.x = x
	self.segment.position.y = y

	local parent = self.segment
	for i = 1, length do
		local np = Vector.new(parent.position.x, parent.position.y)
		local part = SnakeSegment.new(np, parent)
		table.insert(self.body, part)
		parent = part
	end
end

function Snake:update(dt)
	local x = self.segment.position.x
	local y = self.segment.position.y

	local cl = collides(0, 16, 8, Fx.height-8, x-4, y-4, 8, 8)
	local cr = collides(Fx.width-8, 16, 8, Fx.height-8, x-4, y-4, 8, 8)
	local ct = collides(0, 16, Fx.width-8, 8, x-4, y-4, 8, 8)
	local cb = collides(0, Fx.height-8, Fx.width-8, 8, x-4, y-4, 8, 8)

	local dir = Vector.new(math.cos(self.rot), math.sin(self.rot))
	if cr or cl or ct or cb then
		GameState = "LOST"
	end
	self.segment.position.x = self.segment.position.x + dir.x * SnakeSpeed * dt
	self.segment.position.y = self.segment.position.y + dir.y * SnakeSpeed * dt

	for i = 1, #self.body do
		local part = self.body[i]
		part:update()
	end
end

function Snake:grow()
	local parent = self.body[#self.body]
	local np = Vector.new(parent.position.x, parent.position.y)
	local part = SnakeSegment.new(np, parent)
	table.insert(self.body, part)
end

function Snake:draw(g, sillouette)
	local nangle = self.rot / (math.pi*2)
	local idx = math.floor(nangle * 8) + 1
	g:transparency(Fx:color("purple"))
	if sillouette then
		g:remap(Fx:color("black"))
	end

	for i = #self.body, 1, -1 do
		local part = self.body[i]
		g:tile(SnakeImage, 0, part.position.x-4, part.position.y-4, 8, 8)
	end
	g:tile(SnakeImage, idx, self.segment.position.x-4, self.segment.position.y-4, 8, 8)

	g:transparency()
	g:remap()
end

----------------------------

local Snek = nil
local Fruits = {}

function create()
	Fx:title("Snek")
	Fx:resize(320, 240, 1)

	Snek = Snake.new()
	Snek:init(160, 120, 4)
	SnakeImage = Fx:loadImage("snek.png")

	OutlineBuf = Graphics.new(Fx:createImage(Fx.width, Fx.height))

	math.randomseed(os.time())
end

function update(dt)
	if GameState == "PLAYING" then
		SpawnTime = SpawnTime + dt
		if SpawnTime > FruitSpawnTime and #Fruits < FruitCount then
			local f = {
				x = 8 + math.floor(math.random() * (Fx.width-16)),
				y = 16 + math.floor(math.random() * (Fx.height-32)),
				id = math.random(0, 2)
			}

			while true do
				local intersects = false
				for i = 1, #Fruits do
					local fr = Fruits[i]
					if collides(fr.x-4, fr.y-4, 8, 8, f.x-4, f.y-4, 8, 8) then
						intersects = true
						break
					end
				end

				if intersects then
					f.x = 8 + math.floor(math.random() * (Fx.width-16))
					f.y = 16 + math.floor(math.random() * (Fx.height-32))
				else
					break
				end
			end

			table.insert(Fruits, f)
			SpawnTime = 0
		end

		if Fx:buttonDown("left") then
			Snek.rot = Snek.rot - dt * 4.0
		elseif Fx:buttonDown("right") then
			Snek.rot = Snek.rot + dt * 4.0
		end

		if Snek.rot < 0 then
			Snek.rot = Snek.rot + math.pi * 2
		elseif Snek.rot >= math.pi * 2 then
			Snek.rot = Snek.rot - math.pi * 2
		end
		Snek:update(dt)

		local x = Snek.segment.position.x
		local y = Snek.segment.position.y
		local rem = 0
		for i = 1, #Fruits do
			local f = Fruits[i]
			if collides(x-4, y-4, 8, 8, f.x-4, f.y-4, 8, 8) then
				rem = i
				break
			end
		end

		if rem > 0 then
			table.remove(Fruits, rem)
			Snek.score = Snek.score + 100
			Snek:grow()
		end

	elseif GameState == "PAUSE" then
		if Fx:buttonPressed("select") then
			GameState = "PLAYING"
		end
	end

end

function drawFruits(g, sillouette)
	g:transparency(Fx:color("purple"))
	if sillouette then
		g:remap(Fx:color("black"))
	end

	for i = 1, #Fruits do
		local fruit = Fruits[i]
		g:tile(SnakeImage, 9 + fruit.id, fruit.x-4, fruit.y-4, 8, 8)
	end

	g:transparency()
	g:remap()
end

function drawField(g, sillouette)
	g:transparency(Fx:color("purple"))
	if sillouette then
		g:remap(Fx:color("black"))
	end

	g:tile(SnakeImage, 14, 0, 16, 8, 8)
	g:tile(SnakeImage, 16, Fx.width-8, 16, 8, 8)
	g:tile(SnakeImage, 18, Fx.width-8, Fx.height-8, 8, 8)
	g:tile(SnakeImage, 17, 0, Fx.height-8, 8, 8)

	for i = 1, (Fx.width/8)-2 do
		g:tile(SnakeImage, 15, i * 8, 16, 8, 8)
		g:tile(SnakeImage, 15, i * 8, Fx.height-8, 8, 8)
	end

	for i = 3, (Fx.height/8)-2 do
		g:tile(SnakeImage, 13, 0, i * 8, 8, 8)
		g:tile(SnakeImage, 13, Fx.width-8, i * 8, 8, 8)
	end

	g:transparency()
	g:remap()
end

function draw(g)
	g:clear(Fx:color("blue"))

	OutlineBuf:clear(Fx:color("white"))
	drawFruits(OutlineBuf, true)
	Snek:draw(OutlineBuf, true)
	drawField(OutlineBuf, true)

	g:transparency(Fx:color("white"))
	g:sprite(OutlineBuf.target, -1, 0)
	g:sprite(OutlineBuf.target, 1, 0)
	g:sprite(OutlineBuf.target, 0, -1)
	g:sprite(OutlineBuf.target, 0, 1)
	g:transparency()

	drawFruits(g, false)
	Snek:draw(g, false)
	drawField(g, false)

	g:transparency(Fx:color("red"))
	g:text(Fx.defaultFont, string.format("%08d", Snek.score), 4, 4)
	g:transparency()

	Fx:flip()
end