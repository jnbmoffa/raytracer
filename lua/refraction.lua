red = gr.material({0.8, 0.1, 0.1}, {0.5, 0.5, 0.5}, 25)
blue = gr.material({0.0, 0.0, 1.0}, {0.5, 0.5, 0.5}, 25)
rblue = gr.smaterial({0, 0, 0}, {1, 1, 1}, 50, 1, 1.65, 0)
blue = gr.material({0.0, 0.1, 1}, {0.5, 0.5, 0.5}, 25)
green = gr.material({0.0, 1.0, 0.0}, {0.5, 0.5, 0.5}, 25)
rgreen = gr.material({0.0, 0.1, 0.0}, {0.5, 0.5, 0.5}, 25)
rwhite = gr.material({1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 25)
fwhite = gr.material({1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 50, 1, 200, 10000)
chrome = gr.smaterial({1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 50, 1, 2000, 0)

root = gr.node('root')

-- floor
wall1 = gr.cube('wall1')
root:add_child(wall1)
wall1:set_material(fwhite)
wall1:translate(-15.5, 0, -15)
wall1:scale(30,1,30)

--roof
wall1 = gr.cube('wall1')
root:add_child(wall1)
wall1:set_material(rwhite)
wall1:translate(-15.5, 30, -15)
wall1:scale(30,1,30)

--Left wall
wall1 = gr.cube('wall1')
root:add_child(wall1)
wall1:set_material(red)
wall1:translate(-15.5, 0.5, -15)
wall1:rotate('z', 90)
wall1:scale(30,1,30)

-- right wall
wall1 = gr.cube('wall1')
root:add_child(wall1)
wall1:set_material(blue)
wall1:translate(-15, 0.5, -15)
wall1:rotate('z', 90)
wall1:translate(0, -30.5, 0)
wall1:scale(30,1,30)

-- back wall
wall1 = gr.cube('wall1')
root:add_child(wall1)
wall1:set_material(rwhite)
wall1:translate(-15.5, 0, -15)
wall1:rotate('x', 90)
wall1:translate(0, 0, -30)
wall1:scale(30,1,30)

BALL = gr.sphere('BALL')
BALL:translate(0, 15, 30)
BALL:scale(6, 6, 6)
BALL:set_material(rblue)
root:add_child(BALL)

-- BALL = gr.cylinder('BALL')
-- BALL:translate(0, 15, 15)
-- BALL:rotate('y', 90)
-- BALL:scale(6, 6, 9)
-- BALL:set_material(rblue)
-- root:add_child(BALL)

white_light = gr.light(12000, {0.0, 25.0, 0.0}, {1, 1, 1}, {1, 0, 0}, 4, 4, 4)

camera = gr.pcamera({0, 15, 55}, {0, 0, -1}, {0, 1, 0}, 50) -- Point camera

gr.render(root, -- Scene root node
          './img/refraction.png',  -- Save file
          1024, 1024,   -- Aspect ratio
    	  camera,
    	  {0.3, 0.3, 0.3} --[[ ambient colour ]],
          {white_light} --[[ lights ]], {} --[[ area lights ]],
          1000 --[[ photons ]], 0 --[[ time duration ]], 1 --[[ time steps ]])