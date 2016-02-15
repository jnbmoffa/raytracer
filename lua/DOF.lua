red = gr.material({0.8, 0.1, 0.1}, {0.5, 0.5, 0.5}, 25)
green = gr.material({0.1, 0.8, 0.1}, {0.5, 0.5, 0.5}, 25)
blue = gr.material({0.1, 0.1, 0.8}, {0.5, 0.5, 0.5}, 25)

root = gr.node('root')

BALL = gr.sphere('BALL1')
BALL:translate(500, 0, -1000)
BALL:scale(100, 100, 100)
BALL:set_material(green)
root:add_child(BALL)

BALL = gr.sphere('BALL1')
BALL:translate(0, 0, 0)
BALL:scale(100, 100, 100)
BALL:set_material(red)
root:add_child(BALL)

BALL = gr.sphere('BALL1')
BALL:translate(-175, 0, 700)
BALL:scale(100, 100, 100)
BALL:set_material(blue)
root:add_child(BALL)

white_light = gr.light(1, {700.0, 0.0, 700.0}, {1, 1, 1}, {1, 0, 0})

camera = gr.lcamera({0, 0, 1000}, {0, 0, -1}, {0, 1, 0}, 50, 5, 2000, 100)

gr.render(root, './img/DOF.png', 1024, 1024,
	  camera,
	  {0.3, 0.3, 0.3}, {white_light}, {}, 0, 0, 1)