function update(obj)
 -- Rotate slowly
    obj.rotation = obj.rotation + obj.spin * engine.getDeltaSeconds()
	--obj.spin = obj.spin * .99
	--engine.log(tostring(obj.spin))
end