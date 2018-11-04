--[[
	Most of the functionality you're going to be using
	lives inside the "Fx" global variable.
	Each script may include 3 functions:
		- create
		- update(dt) and
		- draw(g)
]]--

local t = 0.0

-- The "create" funtion gets called after the game starts:
-- That means it will run only once.
function create()
	-- We can set some stuff in here
	-- like the window size, the window title and
	-- load and create resources.
	Fx:resize(160, 120, 2)
	Fx:title("API Demo")
end

-- The "update" function gets called every frame:
-- This is where you should do your game logic.
function update(dt)
	t = t + dt
end

-- The "draw" function also gets called every frame:
-- This is where you will perform the game rendering.
function draw(g)
	-- Let's clear the screen
	g:clear(Fx:color("darkblue"))

	-- then draw some stuff...
	g:circle(Fx.width/2 + math.cos(t) * 24, Fx.height/2 + math.sin(t) * 24, 16, Fx:color("orange"), true)
	g:line(20, 20, 120, 100, Fx:color("white"))
	g:circle(64, 64, 20, Fx:color("red"))
	g:rect(72, 32, 50, 32, Fx:color("yellow"))

	-- And flip the backbuffer, this will cause the
	-- modifications we have just made to the
	-- main screen buffer to be presented to the screen.
	Fx:flip()

	-- Check the API doc for more!
end