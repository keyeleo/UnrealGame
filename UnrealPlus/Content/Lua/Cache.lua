
Sensor={
	id = 0,
	sensorId = 0,
	moduleId = 0,
	kks = "",
	name = "",
	url = "",
	unit = "℃",
	type = "",
	min = null,
	max = null,
	alarm = "real > 85.0",
	exception = 0,
	values = {},
}

Data={
	id = 0,
	value = 0.0,
	alarm = 0,
	status = 0,
	url = "",
	imageUrl = "",
	timestamp = 0
}

Cache={}
function Cache:instance()
	if self._instance == nil then
		self._instance = {}
	end
	return self._instance
end
