-- A simple scene with some miscellaneous geometry.
-- This file is very similar to nonhier.lua, but interposes
-- an additional transformation on the root node.  
-- The translation moves the scene, and the position of the camera
-- changes accordingly.

mat1 = gr.material({0.7, 1.0, 0.7}, {0.5, 0.7, 0.5}, 25)
mat2 = gr.material({0.5, 0.5, 0.5}, {0.5, 0.7, 0.5}, 25)
mat3 = gr.material({1.0, 0.6, 0.1}, {0.5, 0.7, 0.5}, 25)
mat4 = gr.material({0.7, 0.6, 1.0}, {0.5, 0.4, 0.8}, 25)

scene = gr.node( 'scene' )
--scene:translate(0, 0, -800)

s1 = gr.nh_sphere('s1', {0, 0, -400}, 100)
scene:add_child(s1)
s1:set_material(mat1)

s2 = gr.nh_sphere('s2', {200, 50, -100}, 150)
scene:add_child(s2)
s2:set_material(mat1)

s3 = gr.nh_sphere('s3', {0, -1200, -500}, 1000)
scene:add_child(s3)
s3:set_material(mat2)

b1 = gr.nh_box('b1', {-200, -125, 0}, 100)
scene:add_child(b1)
b1:set_material(mat4)

s4 = gr.nh_sphere('s4', {-100, 25, -300}, 50)
scene:add_child(s4)
s4:set_material(mat3)

s5 = gr.nh_sphere('s5', {0, 100, -250}, 25)
scene:add_child(s5)
s5:set_material(mat1)

-- A small stellated dodecahedron.

-- A small stellated dodecahedron.  I'm not sure where this is located.
-- Further, some of its faces may be facing the wrong way.
steldodec = gr.mesh('smstellateddodecahedron', {
	{ -158.333350, 225.647350, -64.699450},
	{ -207.117500, 241.498300, -64.699450},
	{ -176.967250, 200.000000, -64.699450},
	{ -176.967250, 200.000000, -64.699450},
	{ -207.117500, 158.501700, -64.699450},
	{ -158.333350, 174.352650, -64.699450},
	{ -158.333350, 174.352650, -64.699450},
	{ -128.183050, 132.854500, -64.699450},
	{ -128.183050, 184.149050, -64.699450},
	{ -128.183050, 184.149050, -64.699450},
	{ -79.399000, 200.000000, -64.699450},
	{ -128.183050, 215.850950, -64.699450},
	{ -128.183050, 215.850950, -64.699450},
	{ -128.183050, 267.145500, -64.699450},
	{ -158.333350, 225.647350, -64.699450},
	{ -128.183050, 215.850950, -64.699450},
	{ -79.399000, 200.000000, -64.699450},
	{ -114.699450, 225.647350, -91.666650},
	{ -114.699450, 225.647350, -91.666650},
	{ -92.882500, 241.498300, -135.300550},
	{ -136.516400, 241.498300, -108.333350},
	{ -136.516400, 241.498300, -108.333350},
	{ -171.816950, 267.145500, -135.300550},
	{ -163.483600, 241.498300, -91.666650},
	{ -163.483600, 241.498300, -91.666650},
	{ -207.117500, 241.498300, -64.699450},
	{ -158.333350, 225.647350, -64.699450},
	{ -158.333350, 225.647350, -64.699450},
	{ -150.000000, 200.000000, -21.065500},
	{ -128.183050, 215.850950, -64.699450},
	{ -171.816950, 184.149050, -135.300550},
	{ -171.816950, 132.854500, -135.300550},
	{ -185.300550, 174.352650, -108.333350},
	{ -185.300550, 174.352650, -108.333350},
	{ -207.117500, 158.501700, -64.699450},
	{ -193.633900, 200.000000, -91.666650},
	{ -193.633900, 200.000000, -91.666650},
	{ -207.117500, 241.498300, -64.699450},
	{ -185.300550, 225.647350, -108.333350},
	{ -185.300550, 225.647350, -108.333350},
	{ -171.816950, 267.145500, -135.300550},
	{ -171.816950, 215.850950, -135.300550},
	{ -171.816950, 215.850950, -135.300550},
	{ -150.000000, 200.000000, -178.934500},
	{ -171.816950, 184.149050, -135.300550},
	{ -158.333350, 225.647350, -64.699450},
	{ -128.183050, 267.145500, -64.699450},
	{ -163.483600, 241.498300, -91.666650},
	{ -163.483600, 241.498300, -91.666650},
	{ -171.816950, 267.145500, -135.300550},
	{ -185.300550, 225.647350, -108.333350},
	{ -185.300550, 225.647350, -108.333350},
	{ -220.601000, 200.000000, -135.300550},
	{ -193.633900, 200.000000, -91.666650},
	{ -193.633900, 200.000000, -91.666650},
	{ -207.117500, 158.501700, -64.699450},
	{ -176.967250, 200.000000, -64.699450},
	{ -176.967250, 200.000000, -64.699450},
	{ -150.000000, 200.000000, -21.065500},
	{ -158.333350, 225.647350, -64.699450},
	{ -141.666650, 174.352650, -135.300550},
	{ -92.882500, 158.501700, -135.300550},
	{ -136.516400, 158.501700, -108.333350},
	{ -136.516400, 158.501700, -108.333350},
	{ -128.183050, 132.854500, -64.699450},
	{ -163.483600, 158.501700, -91.666650},
	{ -163.483600, 158.501700, -91.666650},
	{ -207.117500, 158.501700, -64.699450},
	{ -185.300550, 174.352650, -108.333350},
	{ -185.300550, 174.352650, -108.333350},
	{ -220.601000, 200.000000, -135.300550},
	{ -171.816950, 184.149050, -135.300550},
	{ -171.816950, 184.149050, -135.300550},
	{ -150.000000, 200.000000, -178.934500},
	{ -141.666650, 174.352650, -135.300550},
	{ -176.967250, 200.000000, -64.699450},
	{ -207.117500, 241.498300, -64.699450},
	{ -193.633900, 200.000000, -91.666650},
	{ -193.633900, 200.000000, -91.666650},
	{ -220.601000, 200.000000, -135.300550},
	{ -185.300550, 174.352650, -108.333350},
	{ -185.300550, 174.352650, -108.333350},
	{ -171.816950, 132.854500, -135.300550},
	{ -163.483600, 158.501700, -91.666650},
	{ -163.483600, 158.501700, -91.666650},
	{ -128.183050, 132.854500, -64.699450},
	{ -158.333350, 174.352650, -64.699450},
	{ -158.333350, 174.352650, -64.699450},
	{ -150.000000, 200.000000, -21.065500},
	{ -176.967250, 200.000000, -64.699450},
	{ -123.032750, 200.000000, -135.300550},
	{ -92.882500, 241.498300, -135.300550},
	{ -106.366100, 200.000000, -108.333350},
	{ -106.366100, 200.000000, -108.333350},
	{ -79.399000, 200.000000, -64.699450},
	{ -114.699450, 174.352650, -91.666650},
	{ -114.699450, 174.352650, -91.666650},
	{ -128.183050, 132.854500, -64.699450},
	{ -136.516400, 158.501700, -108.333350},
	{ -136.516400, 158.501700, -108.333350},
	{ -171.816950, 132.854500, -135.300550},
	{ -141.666650, 174.352650, -135.300550},
	{ -141.666650, 174.352650, -135.300550},
	{ -150.000000, 200.000000, -178.934500},
	{ -123.032750, 200.000000, -135.300550},
	{ -158.333350, 174.352650, -64.699450},
	{ -207.117500, 158.501700, -64.699450},
	{ -163.483600, 158.501700, -91.666650},
	{ -163.483600, 158.501700, -91.666650},
	{ -171.816950, 132.854500, -135.300550},
	{ -136.516400, 158.501700, -108.333350},
	{ -136.516400, 158.501700, -108.333350},
	{ -92.882500, 158.501700, -135.300550},
	{ -114.699450, 174.352650, -91.666650},
	{ -114.699450, 174.352650, -91.666650},
	{ -79.399000, 200.000000, -64.699450},
	{ -128.183050, 184.149050, -64.699450},
	{ -128.183050, 184.149050, -64.699450},
	{ -150.000000, 200.000000, -21.065500},
	{ -158.333350, 174.352650, -64.699450},
	{ -141.666650, 225.647350, -135.300550},
	{ -171.816950, 267.145500, -135.300550},
	{ -136.516400, 241.498300, -108.333350},
	{ -136.516400, 241.498300, -108.333350},
	{ -128.183050, 267.145500, -64.699450},
	{ -114.699450, 225.647350, -91.666650},
	{ -114.699450, 225.647350, -91.666650},
	{ -79.399000, 200.000000, -64.699450},
	{ -106.366100, 200.000000, -108.333350},
	{ -106.366100, 200.000000, -108.333350},
	{ -92.882500, 158.501700, -135.300550},
	{ -123.032750, 200.000000, -135.300550},
	{ -123.032750, 200.000000, -135.300550},
	{ -150.000000, 200.000000, -178.934500},
	{ -141.666650, 225.647350, -135.300550},
	{ -128.183050, 184.149050, -64.699450},
	{ -128.183050, 132.854500, -64.699450},
	{ -114.699450, 174.352650, -91.666650},
	{ -114.699450, 174.352650, -91.666650},
	{ -92.882500, 158.501700, -135.300550},
	{ -106.366100, 200.000000, -108.333350},
	{ -106.366100, 200.000000, -108.333350},
	{ -92.882500, 241.498300, -135.300550},
	{ -114.699450, 225.647350, -91.666650},
	{ -114.699450, 225.647350, -91.666650},
	{ -128.183050, 267.145500, -64.699450},
	{ -128.183050, 215.850950, -64.699450},
	{ -128.183050, 215.850950, -64.699450},
	{ -150.000000, 200.000000, -21.065500},
	{ -128.183050, 184.149050, -64.699450},
	{ -171.816950, 215.850950, -135.300550},
	{ -220.601000, 200.000000, -135.300550},
	{ -185.300550, 225.647350, -108.333350},
	{ -185.300550, 225.647350, -108.333350},
	{ -207.117500, 241.498300, -64.699450},
	{ -163.483600, 241.498300, -91.666650},
	{ -163.483600, 241.498300, -91.666650},
	{ -128.183050, 267.145500, -64.699450},
	{ -136.516400, 241.498300, -108.333350},
	{ -136.516400, 241.498300, -108.333350},
	{ -92.882500, 241.498300, -135.300550},
	{ -141.666650, 225.647350, -135.300550},
	{ -141.666650, 225.647350, -135.300550},
	{ -150.000000, 200.000000, -178.934500},
	{ -171.816950, 215.850950, -135.300550},
	{ -123.032750, 200.000000, -135.300550},
	{ -92.882500, 158.501700, -135.300550},
	{ -141.666650, 174.352650, -135.300550},
	{ -141.666650, 174.352650, -135.300550},
	{ -171.816950, 132.854500, -135.300550},
	{ -171.816950, 184.149050, -135.300550},
	{ -171.816950, 184.149050, -135.300550},
	{ -220.601000, 200.000000, -135.300550},
	{ -171.816950, 215.850950, -135.300550},
	{ -171.816950, 215.850950, -135.300550},
	{ -171.816950, 267.145500, -135.300550},
	{ -141.666650, 225.647350, -135.300550},
	{ -141.666650, 225.647350, -135.300550},
	{ -92.882500, 241.498300, -135.300550},
	{ -123.032750, 200.000000, -135.300550},
     }, {
    { 0, 1, 2},
    { 3, 4, 5},
    { 6, 7, 8},
    { 9, 10, 11},
    { 12, 13, 14},
    { 15, 16, 17},
    { 18, 19, 20},
    { 21, 22, 23},
    { 24, 25, 26},
    { 27, 28, 29},
    { 30, 31, 32},
    { 33, 34, 35},
    { 36, 37, 38},
    { 39, 40, 41},
    { 42, 43, 44},
    { 45, 46, 47},
    { 48, 49, 50},
    { 51, 52, 53},
    { 54, 55, 56},
    { 57, 58, 59},
    { 60, 61, 62},
    { 63, 64, 65},
    { 66, 67, 68},
    { 69, 70, 71},
    { 72, 73, 74},
    { 75, 76, 77},
    { 78, 79, 80},
    { 81, 82, 83},
    { 84, 85, 86},
    { 87, 88, 89},
    { 90, 91, 92},
    { 93, 94, 95},
    { 96, 97, 98},
    { 99, 100, 101},
    { 102, 103, 104},
    { 105, 106, 107},
    { 108, 109, 110},
    { 111, 112, 113},
    { 114, 115, 116},
    { 117, 118, 119},
    { 120, 121, 122},
    { 123, 124, 125},
    { 126, 127, 128},
    { 129, 130, 131},
    { 132, 133, 134},
    { 135, 136, 137},
    { 138, 139, 140},
    { 141, 142, 143},
    { 144, 145, 146},
    { 147, 148, 149},
    { 150, 151, 152},
    { 153, 154, 155},
    { 156, 157, 158},
    { 159, 160, 161},
    { 162, 163, 164},
    { 165, 166, 167},
    { 168, 169, 170},
    { 171, 172, 173},
    { 174, 175, 176},
    { 177, 178, 179}
 } )

steldodec:set_material(mat3)
scene:add_child(steldodec)

white_light = gr.light({-100.0, 150.0, 400.0}, {0.9, 0.9, 0.9}, {1, 0, 0})
orange_light = gr.light({400.0, 100.0, 150.0}, {0.7, 0.0, 0.7}, {1, 0, 0})

gr.render(scene, 'nonhier.png', 512, 512,
	  {0, 0, 800}, {0, 0, -800}, {0, 1, 0}, 50,
	  {0.3, 0.3, 0.3}, {white_light, orange_light})
