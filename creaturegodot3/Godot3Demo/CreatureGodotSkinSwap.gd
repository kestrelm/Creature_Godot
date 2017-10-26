extends CreatureGodot

func _ready():
	set_process(true)
	self.blend_to_animation("cape", 0.1)
	self.set_skinswap_name("cape")
	self.enable_skinswap()
	pass

func _process(delta):
	self.update_animation(delta)

func _input(event):
	if(event is InputEventKey):
		if(event.scancode == KEY_A):
			self.set_skinswap_name("cape")
		elif(event.scancode == KEY_S):
			self.set_skinswap_name("dressSword")
		elif(event.scancode == KEY_D):
			self.set_skinswap_name("defaultGun")