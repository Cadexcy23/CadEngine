-- Lua script for object movement
--local velocity = {x = 50, y = 30}
--local timeAccumulator = 0

function onCreate(obj)
    -- obj is automatically a "VelObject" userdata in Lua
    -- Access properties directly
    --engine.log("Initial velocity: " .. tostring(obj.velocity))
    --engine.log("Initial spin: " .. obj.spin)
    
    -- Call methods
    --obj:flipVel()  -- Note the colon (:) for methods!
    
    -- You can also modify properties directly
    --obj.velocity = {x = 100, y = 50}
    --obj.spin = 180
	
	--engine.log(tostring(engine.isKeyPressed("V")))
	
	engine.log("Spawned!")
	engine.log(tostring(obj.spin)) -- this should NOT be 0, fix it Caden!
	engine.log(obj)
end
	


function update(obj)
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
	
end

