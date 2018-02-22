extends CreatureGodot

func _ready():
	set_process(true)
	self.blend_to_animation("run", 0.1)
	pass

func _process(delta):
	self.update_animation(delta)