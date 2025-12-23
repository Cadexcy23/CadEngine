-- onCreate runs once on creation and deletes itself so there is minimal performance hit by doing so with a Lua func
-- BUG: runs during object creation so dta is not fully initialized meaning read data may be initialized and writing data may be overwritten
function onCreate(obj)
    --set to mouse pos
	
	--randomize velocity
	engine.printObjectCount()
end

local deltaSeconds, x, y, xVel, yVel, rot, spin, SB

function steer(obj)
	--control the movement of the object with keyboard
	if engine.keyState("W") > 0 then
		yVel = yVel - 100 * deltaSeconds
		end
	if engine.keyState("S") > 0 then
		yVel = yVel + 100 * deltaSeconds
		end
	if engine.keyState("A") > 0 then
		xVel = xVel - 100 * deltaSeconds
		end
	if engine.keyState("D") > 0 then
		xVel = xVel + 100 * deltaSeconds
		end
	if engine.keyState("Space") == 1 then
		xVel = xVel * 2
		yVel = yVel * 2
		end
	if engine.keyState("Q") > 0 then
		spin = spin - 100 * deltaSeconds
		end
	if engine.keyState("E") > 0 then
		spin = spin + 100 * deltaSeconds
		end
end

function perpetuate(obj)
	-- keep the objects moving with their velocity and spin
	x = x + xVel * deltaSeconds
	y = y + yVel * deltaSeconds

	rot = rot + spin * deltaSeconds
end

function keepIn(obj)
	--keep object in the screen
	if x > SB.w then
		x = SB.w
		obj:flipVelX()
	elseif x < SB.x then
		x = SB.x
		obj:flipVelX()
	elseif y > SB.h then
		y = SB.h
		obj:flipVelY()
	elseif y < SB.y then
		y = SB.y
		obj:flipVelY()
	end
end

--this CAN be enabled but it is ill advised as even just having an empty update function DRASTICALLY increases memory usage and lowers performance
--function update(obj)
	--deltaSeconds = engine:getDeltaSeconds()
	--x = obj.hull.x
	--y = obj.hull.y
	--xVel = obj.vel.x
	--yVel = obj.vel.y
	--rot = obj.rotation
	--spin = obj.spin
	--SB = engine.screenBounds

	--steer(obj)
	--perpetuate(obj)
	--keepIn(obj)

	--obj.hull.x = x
	--obj.hull.y = y
	--obj.vel.x = xVel
	--obj.vel.y = yVel
	--obj.rotation = rot
	--obj.spin = spin

--end

