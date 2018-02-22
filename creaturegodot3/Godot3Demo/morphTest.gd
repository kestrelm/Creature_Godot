extends CreatureGodot

func _ready():
	set_process(true)
	self.set_morph_targets_active(true)
	pass

func _process(delta):
	var poly = get_node("../Polygon2D")	
	self.set_morph_targets_pt(poly.global_position, self.global_position - Vector2(-10, 200), 10.0)
		
	self.update_animation(delta)
	
func _input(event):
	var poly = get_node("../Polygon2D")	
	if(event is InputEventKey):
		if(event.scancode == KEY_A):
			poly.position.x -= 10
		elif(event.scancode == KEY_D):
			poly.position.x += 10
		elif(event.scancode == KEY_W):
			poly.position.y -= 10
		elif(event.scancode == KEY_S):
			poly.position.y += 10
			