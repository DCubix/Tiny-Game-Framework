local image = nil
local anim = nil

function create()
	Fx:resize(160, 120, 2)
	Fx:title("Sprites")

	-- Images in TGF, can be loaded from simple PNG files.
	-- The colors will be converted to fit TGF's palette,
	-- and dithering can also be applied.
	image = Fx:loadImage("image.png", 2)

	-- Images can also be animated:
	image:animate(4, 4)
	image:add("anim", {})
	image:play("anim", 24, true)

	-- Check the API doc for more!
end

function update(dt)
end

function draw(g)
	g:clear(Fx:color("darkblue"))

	-- Now you can just draw the image
	g:sprite(image, 20, 20)

	Fx:flip()
end