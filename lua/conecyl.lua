red = gr.material({0.8, 0.1, 0.1}, {0.5, 0.5, 0.5}, 25)
blue = gr.material({0.0, 0.0, 1.0}, {0.5, 0.5, 0.5}, 25)
rblue = gr.smaterial({0, 0, 0}, {1, 1, 1}, 50, 1, 1.65, 0)
blue = gr.material({0.0, 0.1, 1}, {0.5, 0.5, 0.5}, 25)
green = gr.material({0.0, 1.0, 0.0}, {0.5, 0.5, 0.5}, 25)
rgreen = gr.material({0.0, 0.1, 0.0}, {0.5, 0.5, 0.5}, 25)
rwhite = gr.material({1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 25)
fwhite = gr.material({1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 50, 1, 200, 10000)

root = gr.node('root')

-- floor
wall1 = gr.cube('wall1')
root:add_child(wall1)
wall1:set_material(fwhite)
wall1:translate(-15, -1, -15)
wall1:scale(30,1,30)

--roof
wall1 = gr.cube('wall1')
root:add_child(wall1)
wall1:set_material(rwhite)
wall1:translate(-15, 30, -15)
wall1:scale(30,1,30)

--Left wall
wall1 = gr.cube('wall1')
root:add_child(wall1)
wall1:set_material(red)
wall1:translate(-15, 0, -15)
wall1:rotate('z', 90)
wall1:scale(30,1,30)

-- right wall
wall1 = gr.cube('wall1')
root:add_child(wall1)
wall1:set_material(blue)
wall1:translate(15, 0, -15)
wall1:scale(1,30,30)

-- back wall
wall1 = gr.cube('wall1')
root:add_child(wall1)
wall1:set_material(rwhite)
wall1:translate(-15, 0, -15)
wall1:rotate('x', 90)
wall1:translate(0, 0, -30)
wall1:scale(30,1,30)

BALL = gr.cylinder('BALL')
BALL:translate(-4, 5, -7)
BALL:rotate('x', 90)
BALL:scale(4, 4, 5)
BALL:set_material(green)
root:add_child(BALL)

BALL = gr.cone('BALL')
BALL:translate(4, 15, 0)
BALL:rotate('x', 40)
BALL:rotate('y', 40)
BALL:scale(4, 4, 7)
BALL:set_material(red)
root:add_child(BALL)

white_light = gr.light(12000, {0.0, 29.0, 0.0}, {1, 1, 1}, {1, 0, 0})

camera = gr.pcamera({0, 15, 40}, {0, 0, -1}, {0, 1, 0}, 50, 0.1, 40)

gr.render(root, './img/conecyl.png', 1024, 1024,
	  camera,
	  {0.3, 0.3, 0.3}, {white_light}, {}, 0, 0, 1)