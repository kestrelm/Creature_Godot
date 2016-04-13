#include "creaturegodot.h"
#include "globals.h"
#include <string>
#include <iostream>
#include <fstream>

static std::map<std::string, std::shared_ptr<CreatureModule::CreatureAnimation> > global_animations;
static std::map<std::string, std::shared_ptr<CreatureModule::CreatureLoadDataPacket> > global_load_data_packets;

static bool is_file_exist(const char *fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}

static std::string GetAnimationToken(const std::string& filename_in, const std::string& name_in)
{
	return filename_in + std::string("_") + name_in;
}

static bool 
LoadDataPacket(const std::string& filename_in)
{
    if(!is_file_exist(filename_in.c_str()))
    {
        return false;
    }
    
	if (global_load_data_packets.count(filename_in) > 0)
	{
		// file already loaded, just return
		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	//Changed!
	//////////////////////////////////////////////////////////////////////////
		std::shared_ptr<CreatureModule::CreatureLoadDataPacket> new_packet =
			std::make_shared<CreatureModule::CreatureLoadDataPacket>();

		bool is_zip = false;
		if (filename_in.substr(filename_in.find_last_of(".") + 1) == "zip") {
			is_zip = true;
		}

		if (is_zip)
		{
			// load zip archive
			CreatureModule::LoadCreatureZipJSONData(filename_in, *new_packet);
		}
		else {
			// load regular JSON
			CreatureModule::LoadCreatureJSONData(filename_in, *new_packet);
		}
		global_load_data_packets[filename_in] = new_packet;

		return true;
}

static void load_animation(const std::string& filename_in, const std::string& name_in)
{
	auto cur_token = GetAnimationToken(filename_in, name_in);
	if (global_animations.count(cur_token) > 0)
	{
		// animation already exists, just return
		return;
	}

	auto load_data = global_load_data_packets[filename_in];

	std::shared_ptr<CreatureModule::CreatureAnimation> new_animation =
		std::make_shared<CreatureModule::CreatureAnimation>(*load_data, name_in);

	global_animations[cur_token] = new_animation;
}

static bool add_loaded_animation(CreatureModule::CreatureManager * creature_manager, const std::string& filename_in, const std::string& name_in)
{
	auto cur_token = GetAnimationToken(filename_in, name_in);
	if (global_animations.count(cur_token) > 0)
	{
		creature_manager->AddAnimation(global_animations[cur_token]);
		creature_manager->SetIsPlaying(true);
		creature_manager->SetShouldLoop(true);
		return true;
	}
	else {
		std::cout << "ERROR! ACreatureActor::AddLoadedAnimation() Animation with filename: " << filename_in << " and name: " << name_in << " not loaded!" << std::endl;
	}

	return false;
}

void 
CreatureGodot::load_json(const String& filename_in)
{
    auto global_path = Globals::get_singleton()->globalize_path(filename_in);
    std::cout<<"CreatureGodot::load_json() - Loading file: "<<global_path.utf8()<<std::endl;
    
    std::string load_filename(global_path.utf8());
    auto can_load = LoadDataPacket(load_filename);
    if(!can_load)
    {
        std::cout<<"CreatureGodot::load_json() - ERRROR! Could not load file: "<<load_filename<<std::endl;
        return;
    }
    std::cout<<"CreatureGodot::load_json() - Finished loading file: "<<load_filename<<std::endl;

    auto json_data = global_load_data_packets[load_filename];
    auto cur_creature = std::shared_ptr<CreatureModule::Creature>(new CreatureModule::Creature(*json_data));    
    manager = std::shared_ptr<CreatureModule::CreatureManager> (new CreatureModule::CreatureManager(cur_creature));
    
    auto all_animation_names = manager->GetCreature()->GetAnimationNames();
    auto first_animation_name = all_animation_names[0];
    for (auto& cur_name : all_animation_names)
	{
        std::cout<<"CreatureGodot::load_json() - Trying to load animation: "<<cur_name<<std::endl;
		load_animation(load_filename, cur_name);
		add_loaded_animation(manager.get(), load_filename, cur_name);
        std::cout<<"CreatureGodot::load_json() - Loaded animation: "<<cur_name<<std::endl;
	}
    
    manager->SetActiveAnimationName(first_animation_name);
}

void CreatureGodot::blend_to_animation(String animation_name, float blend_delta)
{
    auto real_anim_name = std::string(animation_name.utf8());
    manager->AutoBlendTo(real_anim_name, blend_delta);
}

void CreatureGodot::set_should_loop(bool flag_in)
{
    manager->SetShouldLoop(flag_in);
}

void 
CreatureGodot::set_anim_speed(float value_in)
{
    anim_speed = value_in;
}

float 
CreatureGodot::get_anim_speed() const{
    return anim_speed;   
}

void CreatureGodot::update_animation(float delta)
{
    if(manager)
    {
        manager->Update(delta * anim_speed);
        
        // resize points, uvs and indices array
        if(points.size() == 0)
        {
            points.resize(manager->GetCreature()->GetTotalNumPoints());
            uvs.resize(points.size());
            indices.resize(manager->GetCreature()->GetTotalNumIndices());
        }
        
        // fill in rendering data
        
        // indices/topology
        auto cur_indices = manager->GetCreature()->GetGlobalIndices();
        for(size_t i = 0; i < indices.size(); i++)
        {
            indices[i] = cur_indices[i];
        }
        
        // points and uvs
        auto cur_pts = manager->GetCreature()->GetRenderPts();
        auto cur_uvs = manager->GetCreature()->GetGlobalUvs();
        
        for(size_t i = 0; i < points.size(); i++)
        {
            glm::float32 cur_x = cur_pts[i * 3];
            glm::float32 cur_y = cur_pts[i * 3 + 1];
            points[i] = Vector2(cur_x, -cur_y);
            
            glm::float32 cur_u = cur_uvs[i * 2];
            glm::float32 cur_v = cur_uvs[i * 2 + 1];
            uvs[i] = Vector2(cur_u, cur_v);

        }
        
        update();
    }
}

Rect2 CreatureGodot::get_item_rect() const {


	if (rect_cache_dirty){
		int l =points.size();
		item_rect=Rect2();
		for(int i=0;i<l;i++) {
			Vector2 pos = points[i];
			if (i==0)
				item_rect.pos=pos;
			else
				item_rect.expand_to(pos);
		}
		item_rect=item_rect.grow(20);
		rect_cache_dirty=false;
	}


	return item_rect;


}

void CreatureGodot::set_offset(const Vector2& p_offset) {

	offset=p_offset;
	rect_cache_dirty=true;
	update();
}

Vector2 CreatureGodot::get_offset() const {

	return offset;
}

void CreatureGodot::edit_set_pivot(const Point2& p_pivot) {

	set_offset(p_pivot);
}

Point2 CreatureGodot::edit_get_pivot() const {

	return get_offset();
}
bool CreatureGodot::edit_has_pivot() const {

	return true;
}

void CreatureGodot::_notification(int p_what) {


	switch(p_what) {

		case NOTIFICATION_DRAW: {
             if((points.size() < 3) || (uvs.size() < 3))
             {
                 return;
             }
             
             if(manager)
             {
                manager->SetMirrorY(mirror_y);
             }
            
            Vector<Color> colors;
			colors.push_back(color);
			VS::get_singleton()->canvas_item_add_triangle_array(get_canvas_item(),indices,points,colors,uvs,texture.is_valid()?texture->get_rid():RID());

		} break;
	}
}

void CreatureGodot::set_color(const Color& p_color){

	color=p_color;
	update();
}

Color CreatureGodot::get_color() const{

	return color;
}

void CreatureGodot::set_asset_filename(const String& filename_in)
{
    asset_filename = filename_in;
    load_json(filename_in);
    update_animation(0.1f);
}

String CreatureGodot::get_asset_filename() const
{
    return asset_filename;
}

void CreatureGodot::set_texture(const Ref<Texture>& p_texture){

	texture=p_texture;

	/*if (texture.is_valid()) {
		uint32_t flags=texture->get_flags();
		flags&=~Texture::FLAG_REPEAT;
		if (tex_tile)
			flags|=Texture::FLAG_REPEAT;
		texture->set_flags(flags);
	}*/
	update();
}

Ref<Texture> CreatureGodot::get_texture() const{

	return texture;
}

void CreatureGodot::set_mirror_y(bool flag_in)
{
    mirror_y = flag_in;
    update();
}

bool CreatureGodot::get_mirror_y() const
{
    return mirror_y;
}

void CreatureGodot::set_active_item_swap(const String& region_name, int swap_idx)
{
    if(manager)
    {
        manager->GetCreature()->SetActiveItemSwap(std::string(region_name.utf8()), swap_idx);
    }   
}

void CreatureGodot::remove_active_item_swap(const String& region_name)
{
    if(manager)
    {
        manager->GetCreature()->RemoveActiveItemSwap(std::string(region_name.utf8()));
    }       
}

void CreatureGodot::set_anchor_points_active(bool flag_in)
{
    if(manager)
    {
        manager->GetCreature()->SetAnchorPointsActive(flag_in);
    }
}

void CreatureGodot::make_point_cache(const String& animation_name_in, int gap_step)
{
    if(manager)
    {
        manager->MakePointCache(std::string(animation_name_in.utf8()), gap_step);
    }
}

Vector2 
CreatureGodot::get_bone_pos(const String& bone_name, float slide_factor)
{
    Vector2 ret_pt(0,0);
    
    if(manager)
    {
        auto  render_composition = manager->GetCreature()->GetRenderComposition();
    	auto& bones_map = render_composition->getBonesMap();
        
        std::string real_name(bone_name.utf8());
        if(bones_map.count(real_name) == 0)
        {
            return Vector2(0,0);
        }        
        
        auto cur_bone = bones_map[real_name];
        auto pt1 = cur_bone->getWorldStartPt();
		auto pt2 = cur_bone->getWorldEndPt();
        auto rel_vec = (pt2 - pt1) * slide_factor;     
        auto set_vec = pt1 + rel_vec;
        
        // local coords
        ret_pt = Vector2(set_vec.x, set_vec.y);
        
        // transform to world coords
        get_transform().xform(ret_pt);
    }
    
    return ret_pt;
}

void CreatureGodot::_bind_methods() {

	ObjectTypeDB::bind_method(_MD("set_color","color"),&CreatureGodot::set_color);
	ObjectTypeDB::bind_method(_MD("get_color"),&CreatureGodot::get_color);

	ObjectTypeDB::bind_method(_MD("set_texture","texture"),&CreatureGodot::set_texture);
	ObjectTypeDB::bind_method(_MD("get_texture"),&CreatureGodot::get_texture);

	ObjectTypeDB::bind_method(_MD("set_anim_speed","texture"),&CreatureGodot::set_anim_speed);
	ObjectTypeDB::bind_method(_MD("get_anim_speed"),&CreatureGodot::get_anim_speed);
    
    ObjectTypeDB::bind_method(_MD("set_asset_filename","offset"),&CreatureGodot::set_asset_filename);
	ObjectTypeDB::bind_method(_MD("get_asset_filename"),&CreatureGodot::get_asset_filename);
    
	ObjectTypeDB::bind_method(_MD("set_offset","offset"),&CreatureGodot::set_offset);
	ObjectTypeDB::bind_method(_MD("get_offset"),&CreatureGodot::get_offset);

	ObjectTypeDB::bind_method(_MD("set_mirror_y","offset"),&CreatureGodot::set_mirror_y);
	ObjectTypeDB::bind_method(_MD("get_mirror_y"),&CreatureGodot::get_mirror_y);
    
    ObjectTypeDB::bind_method(_MD("load_json"),&CreatureGodot::load_json);
    ObjectTypeDB::bind_method(_MD("update_animation"),&CreatureGodot::update_animation);
    ObjectTypeDB::bind_method(_MD("blend_to_animation"),&CreatureGodot::blend_to_animation);
    ObjectTypeDB::bind_method(_MD("set_should_loop"),&CreatureGodot::set_should_loop);

    ObjectTypeDB::bind_method(_MD("set_active_item_swap"),&CreatureGodot::set_active_item_swap);
    ObjectTypeDB::bind_method(_MD("remove_active_item_swap"),&CreatureGodot::remove_active_item_swap);
    
    ObjectTypeDB::bind_method(_MD("set_anchor_points_active"),&CreatureGodot::set_anchor_points_active);
    
    ObjectTypeDB::bind_method(_MD("make_point_cache"),&CreatureGodot::make_point_cache);

    ObjectTypeDB::bind_method(_MD("get_bone_pos"),&CreatureGodot::get_bone_pos);

	ADD_PROPERTY( PropertyInfo(Variant::REAL,"anim_speed"),_SCS("set_anim_speed"),_SCS("get_anim_speed"));
	ADD_PROPERTY( PropertyInfo(Variant::STRING,"asset_filename"),_SCS("set_asset_filename"),_SCS("get_asset_filename"));
	ADD_PROPERTY( PropertyInfo(Variant::BOOL,"mirror_y"),_SCS("set_mirror_y"),_SCS("get_mirror_y"));
	ADD_PROPERTY( PropertyInfo(Variant::COLOR,"color"),_SCS("set_color"),_SCS("get_color"));
	ADD_PROPERTY( PropertyInfo(Variant::VECTOR2,"offset"),_SCS("set_offset"),_SCS("get_offset"));
	ADD_PROPERTY( PropertyInfo(Variant::OBJECT,"texture/texture",PROPERTY_HINT_RESOURCE_TYPE,"Texture"),_SCS("set_texture"),_SCS("get_texture"));
}

CreatureGodot::CreatureGodot() {
	color=Color(1,1,1);
	rect_cache_dirty=true;
    anim_speed = 2.0f;
    mirror_y = false;
}