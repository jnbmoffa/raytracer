red = gr.material({0.8, 0.1, 0.1}, {0.5, 0.5, 0.5}, 25)

root = gr.node('root')

BALL = gr.sphere('BALL1')
BALL:translate(512, 512, 512)
BALL:scale(10, 10, 10)
BALL:set_material(red)
root:add_child(BALL)

BALL = gr.sphere('BALL1')
BALL:translate(-512, 512, 512)
BALL:scale(10, 10, 10)
BALL:set_material(red)
root:add_child(BALL)

BALL = gr.sphere('BALL1')
BALL:translate(512, -512, 512)
BALL:scale(10, 10, 10)
BALL:set_material(red)
root:add_child(BALL)

BALL = gr.sphere('BALL1')
BALL:translate(-512, -512, 512)
BALL:scale(10, 10, 10)
BALL:set_material(red)
root:add_child(BALL)

BALL = gr.sphere('BALL2')
BALL:translate(-512, -512, -512)
BALL:scale(10, 10, 10)
BALL:set_material(red)
root:add_child(BALL)

BALL = gr.sphere('BALL3')
BALL:translate(512, -512, -512)
BALL:scale(10, 10, 10)
BALL:set_material(red)
root:add_child(BALL)

BALL = gr.sphere('BALL4')
BALL:translate(512, 512, -512)
BALL:scale(10, 10, 10)
BALL:set_material(red)
root:add_child(BALL)

BALL = gr.sphere('BALL5')
BALL:translate(-512, 512, -512)
BALL:scale(10, 10, 10)
BALL:set_material(red)
root:add_child(BALL)

BALL = gr.sphere('BALL200000')
BALL:translate(-3072, -3072, -3072)
BALL:scale(10, 10, 10)
BALL:set_material(red)
root:add_child(BALL)

BALL = gr.sphere('BALL200000')
BALL:translate(3072, -3072, -3072)
BALL:scale(10, 10, 10)
BALL:set_material(red)
root:add_child(BALL)

BALL = gr.sphere('BALL200000')
BALL:translate(-3072, 3072, -3072)
BALL:scale(10, 10, 10)
BALL:set_material(red)
root:add_child(BALL)

BALL = gr.sphere('BALL200000')
BALL:translate(3072, 3072, -3072)
BALL:scale(10, 10, 10)
BALL:set_material(red)
root:add_child(BALL)

white_light = gr.light({0.0, 0.0, 50.0}, {1, 1, 1}, {1, 0, 0})

gr.render(root, 'octree.png', 1024, 1024,
	  {0, 0, 5000}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.3, 0.3, 0.3}, {white_light})