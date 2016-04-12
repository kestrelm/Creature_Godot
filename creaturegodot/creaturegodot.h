/* sumator.h */
#ifndef CREATURE_GODOT_H
#define CREATURE_GODOT_H

#include "reference.h"
#include "scene/2d/node_2d.h"
#include "CreatureModule.h"
#include <memory>

class CreatureGodot :  public Node2D {

	OBJ_TYPE(CreatureGodot,Node2D);

	DVector<Vector2> uv;
    Vector<Vector2> points;
	Vector<Vector2> uvs;
    Vector<int> indices;
	Color color;
	Ref<Texture> texture;
    
	Vector2 offset;
	mutable bool rect_cache_dirty;
	mutable Rect2 item_rect;
    std::shared_ptr<CreatureModule::CreatureManager> manager;

	void _set_texture_rotationd(float p_rot);
	float _get_texture_rotationd() const;

protected:

	void _notification(int p_what);
	static void _bind_methods();
public:

    void load_json(const String& filename_in);
    
    void update_animation(float delta);
    
    void blend_to_animation(String animation_name, float blend_delta);
    
    void set_should_loop(bool flag_in);

	void set_color(const Color& p_color);
	Color get_color() const;

	void set_texture(const Ref<Texture>& p_texture);
	Ref<Texture> get_texture() const;
    
    void set_offset(const Vector2& p_offset);
	Vector2 get_offset() const;

	//editor stuff

	virtual void edit_set_pivot(const Point2& p_pivot);
	virtual Point2 edit_get_pivot() const;
	virtual bool edit_has_pivot() const;

	virtual Rect2 get_item_rect() const;


    CreatureGodot();
};

#endif