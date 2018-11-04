local playerX = 0
local playerY = 0
local playerSpeed = 80

-- Handling input in the TGF is very straight forward:
-- The API provides a bunch of useful functions for that.
function update(dt)
	if Fx:buttonDown("left") then
		playerX = playerX - dt * playerSpeed
	elseif Fx:buttonDown("right") then
		playerX = playerX + dt * playerSpeed
	end

	if Fx:buttonDown("up") then
		playerY = playerY - dt * playerSpeed
	elseif Fx:buttonDown("down") then
		playerY = playerY + dt * playerSpeed
	end

	-- Check the API doc for more!
end

function create()
	Fx:resize(160, 120, 2)
	Fx:title("Input Handling")
end

function draw(g)
	g:clear(Fx:color("darkblue"))

	g:circle(playerX, playerY, 8, Fx:color("orange"), true)

	Fx:flip()
end