#Creature Godot Engine Plugin

This repository contains the Creature Animation Tool's plugin for Godot Engine. 

Creature Animation Tool Website: https://http://creature.kestrelmoon.com/

Godot Engine Website:
http://www.godotengine.org/

##Compilation & Installation

###1. Grab the Godot Engine Source
First, download the Godot Engine source from:
https://github.com/godotengine/godot

Make sure you can compile Godot Engine by following their instructions for your platform:
http://docs.godotengine.org/en/latest/reference/_compiling.html

###2. Drop in creaturegodot into modules 
The Creature Godot plugin must be compiled with Godot Engine in order to run. Compilation is very easy and simply involves dropping the **creaturegodot** directory into Godot Engine's **modules** directory.

For example take a look at the image below:

![Alt text](https://github.com/kestrelm/Creature_Godot/blob/master/godot-docs1.png)

The **creaturegodot** directory has been put into the **modules** directory of Godot Engine.

###3. Build Godot Engine
Go ahead and build Godot Engine from the terminal with 
scons. It will build the Creature Plugin with it. You are done!

##Usage

###1. Add CreatureGodot Object
Create a new node in Godot Engine, and type/search for **creature**:

![Alt text](https://github.com/kestrelm/Creature_Godot/blob/master/godot-docs2.png)

You should see **CreatureGodot** show up which means the plugin is now active in Godot Engine. Add **CreatureGodot** into your scene.

###2. Configure CreatureGodot
First, set the **Asset Filename** property. This points to the exported Creature JSON filename living within your project directory. You can also provide a **zipped version** ( has to end with a .zip extension ) of the file to save disk space.

The **Asset Filename** has to be in the format: **res://myfilename.json** or **res://myfilename.zip**.

If all goes well, you should see your character load up:

![Alt text](https://github.com/kestrelm/Creature_Godot/blob/master/godot-docs3.png)

###3. Activating it with a Script
Now it is time to load up scripting to make the character animate.
First, assign and create a new script to the character using the typical way you would in Godot.

Open up the script and type:

	extends CreatureGodot

	func _ready():
		set_process(true)
		pass

	func _process(delta):
		self.update_animation(delta)


**func _ready()** calls **set_process(true)** to make the the update tick **_process(delta)** call active.

In **_process(delta)** itself, we play forward the animation by a delta timestep via the **update_animation** call.

Play the game and you should see your character running!

###Switching Animations & Other functions
**set_mirror_y(flag_in)** - Sets a boolean to flip the character along the Y axis

**blend_to_animation(name, blend_factor)** - Switches to another animation smoothly based off the blend_factor. blend_factor is a value > 0 and <= 1.0. The higher the value, the faster the blending to the target animation.

**set_should_loop(flag_in)** - Sets whether the animation should be looping or not.

**set_anim_speed(speed)** - A multiplier on how quickly the animation plays back.


