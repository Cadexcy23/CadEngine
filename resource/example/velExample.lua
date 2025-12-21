-- Lua script example

-- obj is automatically of its derived type
    -- Access properties directly
    --engine.log("Initial velocity: " .. tostring(obj.velocity))
    --engine.log("Initial spin: " .. obj.spin)
    
    -- Call methods
    --obj:flipVel()  -- Note the colon (:) for methods!
    
    -- You can also modify properties directly
    --obj.velocity = {x = 100, y = 50}
    --obj.spin = 180

-- Access properties
    --local vel = obj.velocity
    --local spin = obj.spin
    
    -- Call methods with colon syntax
    --obj:flipVelX()
    --obj:flipVelY()
    
    -- Or modify properties
    --obj.vel.x = 0.99 * obj.vel.x -- Apply friction
	--obj.vel.y = 0.99 * obj.vel.y
    --obj.spin = spin * 0.95  -- Slow down spin

function onCreate(obj)
    --set to mouse pos
	
	--randomize velocity
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
	if x > SB.w then -- try this next for optimization
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

function update(obj)
	deltaSeconds = engine:getDeltaSeconds()
	x = obj.hull.x
	y = obj.hull.y
	xVel = obj.vel.x
	yVel = obj.vel.y
	rot = obj.rotation
	spin = obj.spin
	SB = engine.screenBounds

	steer(obj)
	perpetuate(obj)
	keepIn(obj)

	obj.hull.x = x
	obj.hull.y = y
	obj.vel.x = xVel
	obj.vel.y = yVel
	obj.rotation = rot
	obj.spin = spin

end

