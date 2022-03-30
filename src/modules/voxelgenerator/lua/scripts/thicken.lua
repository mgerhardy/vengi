local vol = require "modules.volume"

function arguments()
	return {
		{ name = 'amount', desc = 'the amount of voxel to add', type = 'int', default = '1' },
		{ name = 'thickencolor', desc = 'a value of -1 means everything except air - otherwise it is taken as palette index', type = 'colorindex', default = '-1', min = '-1', max = '255' }
	}
end

function main(node, region, color, amount, thickencolor)
	local volume = node:volume()
	local activeLayer = scenegraph.get()
	local newName = activeLayer:name() .. "_thickened"
	local newLayer = scenegraph.new(newName, region)
	local newVolume = newLayer:volume()

	local visitor = function (volume, x, y, z)
		local voxel = volume:voxel(x, y, z)
		for cubeX = x, x + amount do
			for cubeZ = z, z + amount do
				for cubeY = y, y + amount do
					newVolume:setVoxel(cubeX, cubeY, cubeZ, voxel)
				end
			end
		end
	end

	local condition = function (volume, x, y, z)
		local voxel = volume:voxel(x, y, z)
		if thickencolor == -1 then
			if voxel ~= -1 then
				return true
			end
		else
			if voxel == thickencolor then
				return true
			end
		end
		return false
	end
	vol.conditionYXZ(volume, region, visitor, condition)
end
