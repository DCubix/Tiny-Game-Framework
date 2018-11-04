local mySound = nil

function create()
	Fx:resize(160, 120, 2)
	Fx:title("Sound")

	-- Sound effects can be created using strings:
	-- You can only make 8bit sounds for now.

	-- First you create a new Sound
	mySound = Fx:createSound()
	-- Then you set the sound parameters such as notes, volumes, waveforms, etc...
	-- From the API: Sound:set(notes, waveForms, volumes, effects)
	-- (The parameters are all strings!)
	mySound:set("C2D2E2F2G2A2B2C3", "T", "5", "F")
	-- You can also set some options...
	mySound.speed = 0.08

	-- Check the API doc for more!
end

function update(dt)
	if Fx:buttonPressed("select") then
		-- Now we can just play the sound we just created:
		Fx:playSound(mySound, false)
	end
	-- Check the API doc for more!
end

function draw(g)
	g:clear(Fx:color("darkblue"))
	Fx:flip()
end