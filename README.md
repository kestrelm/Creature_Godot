# Creature Godot Engine Plugin

This repository contains the Creature Animation Tool's plugin for Godot Engine. 

Creature Animation Tool Website: [**https://creature.kestrelmoon.com/**](<https://creature.kestrelmoon.com/>)

Godot Engine Website: [**http://www.godotengine.org/**](<http://www.godotengine.org/>)


Sample UtahRaptor Artwork: Emily Willoughby (http://emilywilloughby.com) 

## Video Tutorials

![Alt text](https://github.com/kestrelm/Creature_Godot/blob/master/splash.png)

### Getting Started

An introductory video tutorial about using the plugin is [here](https://www.youtube.com/watch?v=Z2oeDl_2ZU8)


### Animation Player / Animation Properties

Watch this tutorial to learn how to manipulate/playback a Creature Godot object with Godot's AnimationPlayer node [here](https://youtu.be/afSObHcj9O8)

## Godot 3 + Future Roadmap

This documentation is updated for the new [**Godot 3.0**](<https://www.youtube.com/watch?v=XptlVErsL-o>) engine. Make sure you grab the latest engine sources from the [Godot Engine Github](https://github.com/godotengine/godot). The source for the new **Creature Godot 3 Runtimes** is located [here](<https://github.com/kestrelm/Creature_Godot/tree/master/creaturegodot3>). Moving forwards, **all new features will be added to the Godot 3 Creature Runtimes**. 

#### New features of this runtime:

- Smooth Animation Transitions or Blending + Playback
- Point Caching for super fast Playback Performance
- Supports the new Animation Gap Step Compression from Creature Game Export
- Skin/Item Swapping
- Animated Layer Ordering
- Creature Events with Godot Signals
- Godot 3 Visual Scripting Support
- Lots of bug fixes/enhancements
- Ready for Godot 3

### Godot 3 Sample Project/Demo

There is a sample **Demo Project** for **Godot 3** located [here](<https://github.com/kestrelm/Creature_Godot/tree/master/creaturegodot3/Godot3Demo>) In it, there are a couple of simple scripts in both **GDScript** and Godot **VisualScript** format showing you how to drive the **Creature** characters using the runtimes. They demonstrate basic animation blending/switching as well as Skin/Item swapping.


## Compilation & Installation

### 1. Grab the Godot Engine Source

First, download the Godot Engine source from [here](https://github.com/godotengine/godot)

Make sure you can compile Godot Engine by following their instructions for your platform [here](http://docs.godotengine.org/en/latest/reference/_compiling.html)

### 2. Drop in creaturegodot into modules 
The Creature Godot plugin must be compiled with Godot Engine in order to run. Compilation is very easy and simply involves dropping the **creaturegodot** directory into Godot Engine's **modules** directory.

For example take a look at the image below:

![Alt text](https://github.com/kestrelm/Creature_Godot/blob/master/godot-docs1.png)

The **creaturegodot** directory has been put into the **modules** directory of Godot Engine.

### 3. Build Godot Engine

Go ahead and build Godot Engine from the terminal with 
scons. It will build the Creature Plugin with it. You are done!

For example, to build in Windows, you go into the **Godot source directory** and type:

    scons platform=windows

Once again, please look at the documentation from the official [**Godot Website**](http://docs.godotengine.org/en/latest/reference/_compiling.html) for compilation instructions.

## Usage

### 1. Add CreatureGodot Object

Create a new node in Godot Engine, and type/search for **creature**:

![Alt text](https://github.com/kestrelm/Creature_Godot/blob/master/godot-docs2.png)

You should see **CreatureGodot** show up which means the plugin is now active in Godot Engine. Add **CreatureGodot** into your scene.

### 2. Configure CreatureGodot

First, set the **Asset Filename** property. This points to the exported Creature JSON
filename living within your project directory. 

The **Asset Filename** has to be in the format: **res://myfilename.json**.

If all goes well, you should see your character load up:

![Alt text](https://github.com/kestrelm/Creature_Godot/blob/master/godot-docs3.png)

### 3. Activating it with a Script

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


### 4. Skin/Item Swapping

First, make sure that you have setup **Skin Swapping** in Creature. For more information on how to set this up, please head over [here](<https://www.kestrelmoon.com/creaturedocs/Animation/Skin_Swapping.html>)

**Setup:**

- Copy the **MetaData File** ( .mdata ) from your exported **Creature Character Game Export Folder** into your **Godot Game Project Folder**

- In your **CreatureGodotNode**, there is a property called **metadata_filename**. Point this to your **MetaData File**, something like this: **res://myCharMetaData.mdata** . 

**Code:**

Now let's take a look at how to activate and run **Skin Swapping**. As you can see it is rather simple:

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
          
In the above example, we have a character with 3 **Skin Swaps**: **cape, dressSword, defaultGun**. The character swaps between 3 different **Skin Swap Sets** based on user input keys ( **A, S, D** ).

To activate **Skin Swapping**, you do this:

    self.enable_skinswap()
      
Then to swap in a new **Skin Swap Set**, you run:

    self.set_skinswap_name("mySwap")
         
You can also add your own **Custom Skin Swap** at runtime by calling:

    self.add_skinswap("NewSwap", swapNameList)
    
This adds a new custom **Skin Swap** with the specified name and item list into the system. Similarly, you can remove a **Skin Swap** by calling **remove_skinswap(name)**. To disable **Skin Swap** and return back to normal playback mode, call:

    self.disable_skinswap()

### 5. Morph Targets

Real-time Morph Targets enable you to achieve some sophistcated effecs, including the ability to have user-controllable front facing characters that will turn their Heads/eyes towards a desired target point. Please read up on [Morph Targets](https://www.kestrelmoon.com/creaturedocs/Animation/Morph_Targets_For_GameEngines.html) to learn how to setup **Morph Targets** in the Creature Animation Tool first before continuing.

**Morph Targets** data are stored in the **MetaData** file so make sure you have connected/setup the **MetaData** slot of the **CreatureGodot** node before you start.

To activate **Morph Targets**, do the following:

    self.set_morph_targets_active(true)

Then at each update step, make sure you point your morph targets to a target point with a source base point and radius:

	  self.set_morph_targets_pt(target_pos, src_pos, radius)    

The full demo sample ( **An Owl Head turning character, Artwork by David Revoy, CC-BY-SA**) has an Owl Character that will look at a target tracker polygon. The full simple example looks like this:

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

Here, we have a polygon node object that is driven by the W, A, S, D keys. The owl character which is of **CreatureGodot** node type uses **Morph Targets** to perform a head-turning action tracking the position of the polygon. Play around with the provided sample to learn more about this powerful functionality.    
         
### Animation Functions
**set_mirror_y(flag_in)** - Sets a boolean to flip the character along the Y axis

**blend_to_animation(name, blend_factor)** - Switches to another animation smoothly based off the blend_factor. blend_factor is a value > 0 and <= 1.0. The higher the value, the faster the blending to the target animation.

**set_should_loop(flag_in)** - Sets whether the animation should be looping or not.

**set_anim_speed(speed)** - A multiplier on how quickly the animation plays back.

### Item Swapping
**set_active_item_swap(region, id)** - Sets up a region for item swapping with the given region name and internal swap id. Read the Creature Tool Docs to see how to set this up.

**remove_active_item_swap(region)** - Removes the currently swapped out region/item.

### Anchor Points
**set_anchor_points_active(flag)** - Activation of Anchor points setup for the character

### Speeding up playback with Point Caching
**make_point_cache(animation_name, gap_step)** - Creates a point cache for the animation for super fast playback. gap_step determines the accuracy of the point cache, the higher the step the less accurate but faster the cache generation.

### Grabbing the position along a bone for attachments
**get_bone_pos( bone_name, slide_factor)** - Returns a Vector2 that denotes the position along the 2 end points of a given bone in world space. The slide_factor determins how far along the bone you are. So if the value is 0.5, you are retrieving the mid point of the bone. A value of 0 gives you the starting point, a value of 1 gives you the end point.

