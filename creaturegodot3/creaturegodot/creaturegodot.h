/* sumator.h */
#ifndef CREATURE_GODOT_H
#define CREATURE_GODOT_H

#include "scene/2d/node_2d.h"
#include "CreatureModule.h"
#include "CreatureMetaData.h"
#include <memory>

class CreatureGodot :  public Node2D {

    GDCLASS(CreatureGodot, Node2D);

    Vector<Vector2> points;
    Vector<Vector2> uvs;
    Vector<int> indices, meta_indices, real_meta_indices;
    Color color;
    Vector<Color> fill_colors;
    Ref<Texture> texture;
    String asset_filename, metadata_filename;
    float anim_speed;
    bool mirror_y;
    float anim_frame;
    String anim_name;
    String skinswap_name;
    bool reload_data;
    bool run_morph_targets;

    const int INDICES_MODE_NONE = 0;
    const int INDICES_MODE_ORDER = 1;
    const int INDICES_MODE_SKINSWAP = 2;
    int indices_process_mode;
    
    Vector2 offset;
    mutable bool rect_cache_dirty;
    mutable Rect2 item_rect;
    std::unique_ptr<CreatureModule::CreatureManager> manager;
    std::unique_ptr<CreatureModule::CreatureMetaData> metadata;

    void _set_texture_rotationd(float p_rot);
    float _get_texture_rotationd() const;

protected:

    void _notification(int p_what);
    
    static void _bind_methods();

    void process_skinswap();

    void process_layerorder(int time_in);

    void update_colors();
    
public:
    CreatureGodot();
    
    bool load_json(String filename_in);
    
    void update_animation(float delta);
    
    bool blend_to_animation(String animation_name, float blend_delta);

    void set_skinswap_name(String name_in);

    void add_skinswap(String name_in, Vector<String> custom_swap);

    void remove_skinswap(String name_in);

    void enable_skinswap();

    void enable_layerorder();

    void disable_skinswap_or_order();
    
    void set_should_loop(bool flag_in);
    
    void set_asset_filename(String filename_in);
    String get_asset_filename() const;

    void set_metadata_filename(String filename_in);
    String get_metadata_filename() const;

    void set_anim_speed(float value_in);
    float get_anim_speed() const;

    void set_color(Color p_color);
    Color get_color() const;

    void set_texture(const Ref<Texture>& p_texture);
    Ref<Texture> get_texture() const;
    
    void set_offset(const Vector2& p_offset);
    Vector2 get_offset() const;
    
    void set_mirror_y(bool flag_in);
    bool get_mirror_y() const;
    
    void set_active_item_swap(String region_name, int swap_idx);
    void remove_active_item_swap(String region_name);
    
    void set_anchor_points_active(bool flag_in);
    
    void make_point_cache(String animation_name_in, int gap_step);
    
    Vector2 get_bone_pos(String bone_name, float slide_factor);
    
    void set_anim_frame(float frame_in);
    float get_anim_frame() const;
    
    void set_anim_name(String name_in);
    String get_anim_name() const;

    Vector<String> get_anim_clips() const;

    void set_morph_targets_active(bool flag_in);
    bool get_morph_targets_active() const;

    void set_morph_targets_pt(const Vector2& pt_in, const Vector2& base_pt, float radius);

    //editor stuff

    virtual void edit_set_pivot(const Point2& p_pivot);
    virtual Point2 edit_get_pivot() const;
    virtual bool edit_has_pivot() const;

    virtual Rect2 _edit_get_rect() const;
};

#endif