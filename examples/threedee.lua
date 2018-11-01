local tex = nil
local mod = nil
local rot = 0

function create()
	Fx:resize(240, 160, 2)

	tex = Fx:loadImage("tex.png")
	mod = Fx:loadModel("tgf.tmd")
	mod:addAnimation("stand", 1, 50)
	mod:play("stand", 24, true)
end

function update(dt)
	rot = rot - dt * 0.9
end

function draw(g)
	g:clip()
	g:clear(Fx:color("black"))

	g:matrixMode("projection")
	g:identity()
	g:perspective(math.radians(45), Fx.width/Fx.height, 0.01, 100.0)

	g:matrixMode("modelview")
	g:pushMatrix()
		g:translate(0, 0, -3)
		g:rotate(-math.pi/6, 1, 0, 0)
		g:rotate(math.pi+rot, 0, 1, 0)

		g:lightDirection(-1, 1, 1)
		g:lighting(true)
		g:color(4)
		g:bind(tex)
		g:begin3D()
		g:model(mod)
		g:end3D()
	g:popMatrix()

	Fx:flip()
end