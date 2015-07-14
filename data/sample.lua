red = gr.material({0.8, 0.1, 0.1}, {0.5, 0.5, 0.5}, 25)
blue = gr.material({0.0, 0.0, 1.0}, {0.5, 0.5, 0.5}, 25)
rblue = gr.smaterial({0, 0, 0}, {1, 1, 1}, 50, 1, 1.65, 0)
blue = gr.material({0.0, 0.1, 1}, {0.5, 0.5, 0.5}, 25)
green = gr.material({0.0, 1.0, 0.0}, {0.5, 0.5, 0.5}, 25)
rgreen = gr.material({0.0, 0.1, 0.0}, {0.5, 0.5, 0.5}, 25)
rwhite = gr.material({1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 25)
fwhite = gr.smaterial({1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 25, 1, 200, 5000)

root = gr.node('root')

-- Many puppets
-- P1 = gr.node('P1')
-- root:add_child(P1)
-- P1:add_child(puppet)
-- P1:translate(-10, 0, 0)
-- P1:rotate('y', 90)

-- P2 = gr.node('P2')
-- root:add_child(P2)
-- P2:add_child(puppet)
-- P2:translate(10, 0, 0)
-- P2:rotate('y', -90)

flooroffset = -14
-- poly1 = gr.node('poly1')
-- poly1:translate(0, flooroffset+2, 0)
-- poly1:scale(2, 2, 2)
-- poly1:add_child(poly)
-- root:add_child(poly1)

-- floor
wall1 = gr.cube('wall1')
root:add_child(wall1)
wall1:set_material(fwhite)
wall1:translate(-15, 0, -15)
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
wall1:translate(-15, 0, -15)
wall1:rotate('z', 90)
wall1:translate(0, -30, 0)
wall1:scale(30,1,30)

-- back wall
wall1 = gr.cube('wall1')
root:add_child(wall1)
wall1:set_material(rwhite)
wall1:translate(-15, 0, -15)
wall1:rotate('x', 90)
wall1:translate(0, 0, -30)
wall1:scale(30,1,30)

BALL = gr.sphere('BALL')
BALL:translate(8, 7, 0)
BALL:scale(5, 5, 5)
BALL:set_material(rblue)
root:add_child(BALL)

BALL = gr.sphere('BALL')
BALL:translate(-7, 7, -4)
BALL:scale(5, 5, 5)
BALL:set_material(rblue)
root:add_child(BALL)

-- BALL = gr.cylinder('BALL')
-- BALL:translate(-4, 4, 7)
-- BALL:scale(6, 8, 6)
-- BALL:rotate('x', 90)
-- BALL:set_material(rblue)
-- root:add_child(BALL)

white_light = gr.light({0.0, 29.0, 0.0}, {1, 1, 1}, {1, 0, 0})

gr.render(root, 'sample.png', 512, 512,
	  {0, 15, 40}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.3, 0.3, 0.3}, {white_light})