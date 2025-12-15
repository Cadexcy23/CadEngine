function update(obj)
 -- Rotate slowly
    obj.rotation = obj.rotation + obj.spin * 0.016
	--obj.spin = obj.spin * .99
	--engine.log(tostring(obj.spin))
end