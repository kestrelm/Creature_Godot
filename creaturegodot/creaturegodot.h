/* sumator.h */
#ifndef CREATURE_GODOT_H
#define CREATURE_GODOT_H

#include "reference.h"
#include "scene/2d/node_2d.h"
#include "CreatureModule.h"
#include <memory>

class CreatureGodot :  public Node2D {

	OBJ_TYPE(CreatureGodot,Node2D);

    Vector<Vector2> points;
	Vector<Vector2> uvs;
    Vector<int> indices;
	Color color;
	Ref<Texture> texture;
    String asset_filename;
    float anim_speed;
    bool mirror_y;
    float anim_frame;
    String anim_name;
    
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
    
    bool blend_to_animation(String animation_name, float blend_delta);
    
    void set_should_loop(bool flag_in);
    
    void set_asset_filename(const String& filename_in);
    String get_asset_filename() const;
    
    void set_anim_speed(float value_in);
    float get_anim_speed() const;

	void set_color(const Color& p_color);
	Color get_color() const;

	void set_texture(const Ref<Texture>& p_texture);
	Ref<Texture> get_texture() const;
    
    void set_offset(const Vector2& p_offset);
	Vector2 get_offset() const;
    
    void set_mirror_y(bool flag_in);
    bool get_mirror_y() const;
    
    void set_active_item_swap(const String& region_name, int swap_idx);
	void remove_active_item_swap(const String& region_name);
    
    void set_anchor_points_active(bool flag_in);
    
    void make_point_cache(const String& animation_name_in, int gap_step);
    
    Vector2 get_bone_pos(const String& bone_name, float slide_factor);
    
    void set_anim_frame(float frame_in);
    float get_anim_frame() const;
    
    void set_anim_name(const String& name_in);
    String get_anim_name() const;

	//editor stuff

	virtual void edit_set_pivot(const Point2& p_pivot);
	virtual Point2 edit_get_pivot() const;
	virtual bool edit_has_pivot() const;

	virtual Rect2 get_item_rect() const;


    CreatureGodot();
};

#endif