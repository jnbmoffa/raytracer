--
-- CS488 -- Introduction to Computer Graphics
-- 
-- a3mark.lua
--
-- A very simple scene creating a trivial puppet.  The TAs will be
-- using this scene as part of marking your assignment.  You'll
-- probably want to make sure you get reasonable results with it!

rootnode = gr.node('root')
rootnode:rotate('y', 150)

torsoH = 4
torsoW = 1.75
shbase = gr.node('shbase')
rootnode:add_child(shbase)
shbase:translate(0, torsoH, 0)

-- Arms/Shoudlers
-- left arm
shH = 1
shW = 3.5
lShJoint = gr.joint('lShJoint', {-90, 0, 90}, {0, 0, 0})
shbase:add_child(lShJoint)
lShJoint:translate(-shW, 0, 0)

UpArmH = 2
UpArmW = 1
lUpArmBase = gr.node('lUpArmBase')
lShJoint:add_child(lUpArmBase)
lUpArmBase:translate(0, -UpArmH, 0)

lLowArmJoint = gr.joint('lLowArmJoint', {-90, 0, 0}, {0, 0, 0})
lUpArmBase:add_child(lLowArmJoint)
lLowArmJoint:translate(0, -UpArmH, 0)

LowArmH = 1.75
LowArmW = 0.75
lLowArmBase = gr.node('lLowArmBase')
lLowArmJoint:add_child(lLowArmBase)
lLowArmBase:translate(0, -LowArmH, 0)

lHandJoint = gr.joint('lHandJoint', {-90, 0, 90}, {0, 0, 0})
lLowArmBase:add_child(lHandJoint)
lHandJoint:translate(0, -LowArmH, 0)

HandH = 0.5
HandW = 0.5
lHandBase = gr.node('lHandBase')
lHandJoint:add_child(lHandBase)
lHandBase:translate(0, -HandH, 0)

-- right arm
rShJoint = gr.joint('rShJoint', {-90, 0, 90}, {0, 0, 0})
shbase:add_child(rShJoint)
rShJoint:translate(shW, 0, 0)
rShJoint:rotate('y', 20)

rUpArmBase = gr.node('rUpArmBase')
rShJoint:add_child(rUpArmBase)
rUpArmBase:translate(0, -UpArmH, 0)

rLowArmJoint = gr.joint('rLowArmJoint', {-90, 0, 0}, {0, 0, 0})
rUpArmBase:add_child(rLowArmJoint)
rLowArmJoint:translate(0, -UpArmH, 0)
rLowArmJoint:rotate('x', -75)

rLowArmBase = gr.node('rLowArmBase')
rLowArmJoint:add_child(rLowArmBase)
rLowArmBase:translate(0, -LowArmH, 0)

rHandJoint = gr.joint('rHandJoint', {-90, 0, 90}, {0, 0, 0})
rLowArmBase:add_child(rHandJoint)
rHandJoint:translate(0, -LowArmH, 0)

rHandBase = gr.node('rHandBase')
rHandJoint:add_child(rHandBase)
rHandBase:translate(0, -HandH, 0)

-- Neck
LowNeckJoint = gr.joint('LowNeckJoint', {-10, 0, 45}, {0, 0, 0})
shbase:add_child(LowNeckJoint)
LowNeckJoint:translate(0, shH, 0)

NeckH = 0.6
NeckW = 0.7
NeckBase = gr.node('NeckBase')
LowNeckJoint:add_child(NeckBase)
NeckBase:translate(0, NeckH, 0)

HighNeckJoint = gr.joint('HighNeckJoint', {-10, 0, 45}, {-90, 0, 90})
NeckBase:add_child(HighNeckJoint)
HighNeckJoint:translate(0, NeckH, 0)

-- head
HeadH = 1.5
HeadW = 1.5
HeadBase = gr.node('HeadBase')
HighNeckJoint:add_child(HeadBase)
HeadBase:translate(0, HeadH, 0)

NoseH = 0.25
NoseW = 0.25
NoseBase = gr.node('NoseBase')
HeadBase:add_child(NoseBase)
NoseBase:translate(0, NoseH, 1)

-- Hips
HipH = 0.75
HipW = 2
hipbase = gr.node('hipbase')
rootnode:add_child(hipbase)
hipbase:translate(0, -torsoH, 0)

--legs
lHipJoint = gr.joint('lHipJoint', {-90, 0, 90}, {0, 0, 0})
hipbase:add_child(lHipJoint)
lHipJoint:translate(-HipW, 0, 0)

UpLegH = 2.25
UpLegW = 1
lUpLegBase = gr.node('lUpLegBase')
lHipJoint:add_child(lUpLegBase)
lUpLegBase:translate(0, -UpLegH, 0)

lLowLegJoint = gr.joint('lLowLegJoint', {0, 0, 90}, {0, 0, 0})
lUpLegBase:add_child(lLowLegJoint)
lLowLegJoint:translate(0, -UpLegH, 0)

LowLegH = 2
LowLegW = 1
lLowLegBase = gr.node('lLowLegBase')
lLowLegJoint:add_child(lLowLegBase)
lLowLegBase:translate(0, -LowLegH, 0)

lFootJoint = gr.joint('lFootJoint', {-90, 0, 90}, {0, 0, 0})
lLowLegBase:add_child(lFootJoint)
lFootJoint:translate(0, -LowLegH, 0)

FootH = 0.75
FootW = 0.75
lFootBase = gr.node('lFootBase')
lFootJoint:add_child(lFootBase)
lFootBase:translate(0, -FootH, 0)

rHipJoint = gr.joint('rHipJoint', {-90, 0, 90}, {0, 0, 0})
hipbase:add_child(rHipJoint)
rHipJoint:translate(HipW, 0, 0)

rUpLegBase = gr.node('rUpLegBase')
rHipJoint:add_child(rUpLegBase)
rUpLegBase:translate(0, -UpLegH, 0)

rLowLegJoint = gr.joint('rLowLegJoint', {0, 0, 90}, {0, 0, 0})
rUpLegBase:add_child(rLowLegJoint)
rLowLegJoint:translate(0, -UpLegH, 0)

rLowLegBase = gr.node('rLowLegBase')
rLowLegJoint:add_child(rLowLegBase)
rLowLegBase:translate(0, -LowLegH, 0)

rFootJoint = gr.joint('rFootJoint', {-90, 0, 90}, {0, 0, 0})
rLowLegBase:add_child(rFootJoint)
rFootJoint:translate(0, -LowLegH, 0)

rFootBase = gr.node('rFootBase')
rFootJoint:add_child(rFootBase)
rFootBase:translate(0, -FootH, 0)




red = gr.material({0.8, 0.1, 0.1}, {0.5, 0.5, 0.5}, 25)
blue = gr.material({0.0, 0.0, 1.0}, {0.5, 0.5, 0.5}, 25)
green = gr.smaterial({0.0, 1.0, 0.0}, {0.5, 0.5, 0.5}, 25, 1)
white = gr.material({1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 25)

-- Spheres
sh = gr.sphere('shoulders')
shbase:add_child(sh)
sh:scale(shW, shH, 1)
sh:set_material(blue)

torso = gr.sphere('torso')
rootnode:add_child(torso)
torso:scale(torsoW, torsoH, 1)
torso:set_material(green)

-- left arm
lUpArm = gr.sphere('lUpArm')
lUpArmBase:add_child(lUpArm)
lUpArm:scale(UpArmW, UpArmH, 1)
lUpArm:set_material(red)

lLowArm = gr.sphere('lLowArm')
lLowArmBase:add_child(lLowArm)
lLowArm:scale(LowArmW, LowArmH, 1)
lLowArm:set_material(red)

lHand = gr.sphere('lHand')
lHandBase:add_child(lHand)
lHand:scale(HandW, HandH, 1)
lHand:set_material(white)

-- right arm
rUpArm = gr.sphere('rUpArm')
rUpArmBase:add_child(rUpArm)
rUpArm:scale(UpArmW, UpArmH, 1)
rUpArm:set_material(red)

rLowArm = gr.sphere('rLowArm')
rLowArmBase:add_child(rLowArm)
rLowArm:scale(LowArmW, LowArmH, 1)
rLowArm:set_material(red)

rHand = gr.sphere('rHand')
rHandBase:add_child(rHand)
rHand:scale(HandW, HandH, 1)
rHand:set_material(white)

-- neck
Neck = gr.sphere('Neck')
NeckBase:add_child(Neck)
Neck:scale(NeckW, NeckH, 1)
Neck:set_material(red)

-- head
Head = gr.sphere('Head')
HeadBase:add_child(Head)
Head:scale(HeadW, HeadH, 1)
Head:set_material(red)

Nose = gr.sphere('Nose')
NoseBase:add_child(Nose)
Nose:scale(NoseW, NoseH, 1)
Nose:set_material(white)

-- hips
Hips = gr.sphere('Hips')
hipbase:add_child(Hips)
Hips:scale(HipW, HipH, 1)
Hips:set_material(blue)

-- left leg
lUpLeg = gr.sphere('lUpLeg')
lUpLegBase:add_child(lUpLeg)
lUpLeg:scale(UpLegW, UpLegH, 1)
lUpLeg:set_material(red)

lLowLeg = gr.sphere('lLowLeg')
lLowLegBase:add_child(lLowLeg)
lLowLeg:scale(LowLegW, LowLegH, 1)
lLowLeg:set_material(red)

lFoot = gr.sphere('lFoot')
lFootBase:add_child(lFoot)
lFoot:scale(FootW, FootH, 1)
lFoot:set_material(white)

rUpLeg = gr.sphere('rUpLeg')
rUpLegBase:add_child(rUpLeg)
rUpLeg:scale(UpLegW, UpLegH, 1)
rUpLeg:set_material(red)

rLowLeg = gr.sphere('rLowLeg')
rLowLegBase:add_child(rLowLeg)
rLowLeg:scale(LowLegW, LowLegH, 1)
rLowLeg:set_material(red)

rFoot = gr.sphere('rFoot')
rFootBase:add_child(rFoot)
rFoot:scale(FootW, FootH, 1)
rFoot:set_material(white)

white_light = gr.light({-10.0, 15.0, 40.0}, {0.9, 0.9, 0.9}, {1, 0, 0})
orange_light = gr.light({40.0, 10.0, 5.0}, {0.7, 0.0, 0.7}, {1, 0, 0})

gr.render(rootnode, 'puppet.png', 512, 512,
	  {0, 0, 25}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.3, 0.3, 0.3}, {white_light, orange_light})