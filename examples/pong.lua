local paddleSize = 22
local paddleSpeed = 80
local ballGraphics = nil
local gameState = "READY"
local t = 0.0
local font = nil
local snd = nil
local bounceSnd = nil
local stateTime = 0.0
local blinkText = false

local ball = {
	x = 0,
	y = 0,
	px = 0,
	py = 0,
	dx = 0,
	dy = 0,
	speed = 100
}
local paddle1 = {
	y = 0,
	score = 0
}
local paddle2 = {
	y = 0,
	score = 0
}

function math.choice(tbl)
	return tbl[math.random(1, #tbl)]
end

function create()
	Fx:title("Pong")
	Fx:resize(256, 164, 2)

	ballGraphics = Graphics.new(Fx:createImage(Fx.width, Fx.height))
	ballGraphics:clear(col)

	paddle1.y = math.floor(Fx.height / 2)
	paddle2.y = math.floor(Fx.height / 2)
	ball.x = math.floor(Fx.width / 2)
	ball.y = math.floor(Fx.height / 2)

	font = Font.new(Fx:loadImage("font.png"), "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-?!'\":;/\\)(,.$_+*=", 5, 16)
	font.spacing = 0

	snd = Fx:createSound()
	snd:set("C3F2E2B", "T", "3211", "S")
	snd.speed = 0.02

	bounceSnd = Fx:createSound()
	bounceSnd:set("B5C", "N", "3", "S")
	bounceSnd.speed = 0.02

	math.randomseed(os.time())
end

function collides(x1, y1, w1, h1, x2, y2, w2, h2)
	return x1+w1 >= x2 and
			x1 <= x2+w2 and
			y1+h1 >= y2 and
			y1 <= y2+h2
end

function update(dt)
	t = t + dt
	stateTime = stateTime + dt
	if math.floor(stateTime * 1000) % 100 == 0 then
		blinkText = not blinkText
	end
	if gameState == "READY" then
		if stateTime >= 3 then
			gameState = "GO"
			stateTime = 0
		end
	elseif gameState == "GO" then
		if stateTime >= 1 then
			gameState = "RESET"
			stateTime = 0
		end
	elseif gameState == "RESET" then
		stateTime = 0
		t = 0
		paddle1.y = math.floor(Fx.height / 2)
		paddle2.y = math.floor(Fx.height / 2)
		ball.x = math.floor(Fx.width / 2)
		ball.y = math.floor(Fx.height / 2)
		ball.px = ball.x
		ball.py = ball.y
		ball.dx = math.choice({ -1, 1 })
		ball.dy = math.choice({ -1, 1 })
		ball.speed = 80
		gameState = "PLAYING"
	elseif gameState == "PLAYING" then
		if Fx:buttonDown("up") then
			paddle1.y = paddle1.y - paddleSpeed * dt
		elseif Fx:buttonDown("down") then
			paddle1.y = paddle1.y + paddleSpeed * dt
		end

		if ball.x >= (Fx.width / 2) + (Fx.width / 4) then
			local dir = ball.y - paddle2.y
			if dir < 0 then
				paddle2.y = paddle2.y - paddleSpeed * dt
			else
				paddle2.y = paddle2.y + paddleSpeed * dt
			end
		end

		ball.x = ball.x + ball.dx * ball.speed * dt
		ball.y = ball.y + ball.dy * ball.speed * dt

		local hp = math.floor(paddleSize/2)
		if collides(ball.x - 4, ball.y - 4, 8, 8, 12, paddle1.y - hp, 1, paddleSize) then
			ball.dx = -ball.dx
			paddle1.score = paddle1.score + 1
			ball.x = ball.x + ball.dx * 4
			ball.y = ball.y + ball.dy * 4
			Fx:playSound(snd, false)
		end
		if collides(ball.x - 4, ball.y - 4, 8, 8, Fx.width - 13, paddle2.y - hp, 1, paddleSize) then
			ball.dx = -ball.dx
			paddle2.score = paddle2.score + 1
			ball.x = ball.x + ball.dx * 4
			ball.y = ball.y + ball.dy * 4
			Fx:playSound(snd, false)
		end

		if ball.y - 4 < 0 or ball.y + 4 > Fx.height then
			ball.dy = -ball.dy
			Fx:playSound(bounceSnd, false)
		end

		if ball.x - 4 < 0 or ball.x + 4 > Fx.width then
			gameState = "RESET"
		end
	end
end

local clrI = 0
local frame = 0
function clearBuf()
	local col = col
	for y = 0, Fx.height, 4 do
		for x = 0, Fx.width, 4 do
			local nx = x + (clrI % 4)
			local ny = y + math.floor(clrI / 4)
			if clrI == 0 then
				ballGraphics:pixel(x, y, col)
			elseif clrI == 1 then
				ballGraphics:pixel(x+2, y, col)
				ballGraphics:pixel(x+2, y+2, col)
				ballGraphics:pixel(x, y+2, col)
			elseif clrI == 2 then
				ballGraphics:pixel(x+1, y+1, col)
				ballGraphics:pixel(x+3, y+1, col)
				ballGraphics:pixel(x+3, y+3, col)
				ballGraphics:pixel(x+1, y+3, col)
			elseif clrI == 3 then
				ballGraphics:pixel(x+1, y, col)
				ballGraphics:pixel(x+3, y, col)
				ballGraphics:pixel(x+3, y+2, col)
				ballGraphics:pixel(x+1, y+2, col)
			elseif clrI == 4 then
				ballGraphics:pixel(x+2, y+1, col)
				ballGraphics:pixel(x, y+3, col)
			elseif clrI == 5 then
				ballGraphics:pixel(x, y+1, col)
				ballGraphics:pixel(x+2, y+3, col)
			end
		end
	end
	if frame >= 5 then
		clrI = clrI + 1
		clrI = clrI % 6
		frame = 0
	end
	frame = frame + 1
end

function draw(g)
	local centerX = math.floor(Fx.width / 2)
	local centerY = math.floor(Fx.height / 2)

	g:clear(Fx:color("darkgreen"))
	clearBuf()
	ballGraphics:circle(ball.x, ball.y, 4, Fx:color("white"), true)

	g:line(centerX, 0, centerX, Fx.height, Fx:color("white"))

	g:transparency(col)
	g:remap(Fx:color("white"), Fx:color("black"))
	g:sprite(ballGraphics.target, 0, 1)
	g:remap()
	g:sprite(ballGraphics.target, 0, 0)
	g:transparency()

	local hp = math.floor(paddleSize/2)
	g:rect(9, paddle1.y - hp + 1, 4, paddleSize, Fx:color("black"), true)
	g:rect(8, paddle1.y - hp, 4, paddleSize, Fx:color("yellow"), true)
	g:rect(Fx.width - 11, paddle2.y - hp + 1, 4, paddleSize, Fx:color("black"), true)
	g:rect(Fx.width - 12, paddle2.y - hp, 4, paddleSize, col, true)

	g:transparency(col)
	local h = math.floor(font.height / 2)
	if gameState == "READY" then
		local w = math.floor(font:stringWidth("Ready?") / 2)
		g:rect(0, 24 - h - 2, centerX*2, h*2 + 4, Fx:color("black"), true)
		g:text(font, "Ready?", centerX - w, 24 - h)
	elseif gameState == "GO" then
		local w = math.floor(font:stringWidth("Go!!!") / 2)
		g:rect(0, 24 - h - 2, centerX*2, h*2 + 4, Fx:color("black"), true)
		if blinkText then
			g:remap(Fx:color("white"), Fx:color("orange"))
			g:text(font, "Go!!!", centerX - w, 24 - h)
			g:remap()
		end
	elseif gameState == "PAUSED" then
		local w = math.floor(font:stringWidth("Paused") / 2)
		g:rect(0, 24 - h - 2, centerX*2, h*2 + 4, Fx:color("black"), true)
		g:text(font, "Paused?", centerX - w, 24 - h)
	elseif gameState == "LOST" then
		local w = math.floor(font:stringWidth("Game Over!!!") / 2)
		g:rect(0, 24 - h - 2, centerX*2, h*2 + 4, Fx:color("black"), true)
		g:text(font, "Game Over!!!", centerX - w, 24 - h)
	elseif gameState == "PLAYING" then
		local score1 = string.format("%03d", paddle1.score)
		local score2 = string.format("%03d", paddle2.score)
		local w1 = font:stringWidth(score1)
		local w2 = font:stringWidth(score2)
		g:text(font, score1, centerX - (w1 + 8), 24 - h)
		g:text(font, score2, centerX + 8, 24 - h)
	end
	g:transparency()

	Fx:flip()
end