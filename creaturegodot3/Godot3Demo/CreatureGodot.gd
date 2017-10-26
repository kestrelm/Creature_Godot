extends CreatureGodot

var cnter = 0
func _ready():
	set_process(true)
	self.blend_to_animation("run", 0.1)
	pass

func _process(delta):
	self.update_animation(delta)
	cnter += 1
	
	if(cnter % 200 == 0):
		self.blend_to_animation("default", 0.05)
	elif(cnter % 420 == 0):
		self.blend_to_animation("run", 0.05)
		
	
