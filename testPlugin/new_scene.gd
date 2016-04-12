
extends CreatureGodot

# member variables here, example:
# var a=2
# var b="textvar"

func _ready():
	# Called every time the node is added to the scene.
	# Initialization here
	set_process(true)
	
	pass

func _process(delta):
	self.update_animation(delta)

