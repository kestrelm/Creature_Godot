#include "creaturegodot.h"
#include "globals.h"
#include <string>
#include <iostream>

static std::map<std::string, std::shared_ptr<CreatureModule::CreatureAnimation> > global_animations;
static std::map<std::string, std::shared_ptr<CreatureModule::CreatureLoadDataPacket> > global_load_data_packets;

static std::string GetAnimationToken(const std::string& filename_in, const std::string& name_in)
{
	return filename_in + std::string("_") + name_in;
}

static bool 
LoadDataPacket(const std::string& filename_in)
{
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

void CreatureGodot::update_animation(float delta)
{
    if(manager)
    {
        manager->Update(delta);
        
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
            points[i] = Vector2(cur_x, cur_y);
            
            glm::float32 cur_u = cur_uvs[i * 2];
            glm::float32 cur_v = cur_uvs[i * 2 + 1];
            uvs[i] = Vector2(cur_u, cur_v);

        }
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

			Vector<Color> colors;
			colors.push_back(color);
			//Vector<int> indices = Geometry::triangulate_polygon(points);

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

void CreatureGodot::_bind_methods() {

	ObjectTypeDB::bind_method(_MD("set_color","color"),&CreatureGodot::set_color);
	ObjectTypeDB::bind_method(_MD("get_color"),&CreatureGodot::get_color);

	ObjectTypeDB::bind_method(_MD("set_texture","texture"),&CreatureGodot::set_texture);
	ObjectTypeDB::bind_method(_MD("get_texture"),&CreatureGodot::get_texture);
    
	ObjectTypeDB::bind_method(_MD("set_offset","offset"),&CreatureGodot::set_offset);
	ObjectTypeDB::bind_method(_MD("get_offset"),&CreatureGodot::get_offset);
    
    ObjectTypeDB::bind_method(_MD("load_json"),&CreatureGodot::load_json);
    ObjectTypeDB::bind_method(_MD("update_animation"),&CreatureGodot::update_animation);
    ObjectTypeDB::bind_method(_MD("blend_to_animation"),&CreatureGodot::blend_to_animation);
    ObjectTypeDB::bind_method(_MD("set_should_loop"),&CreatureGodot::set_should_loop);

	ADD_PROPERTY( PropertyInfo(Variant::COLOR,"color"),_SCS("set_color"),_SCS("get_color"));
	ADD_PROPERTY( PropertyInfo(Variant::VECTOR2,"offset"),_SCS("set_offset"),_SCS("get_offset"));
	ADD_PROPERTY( PropertyInfo(Variant::OBJECT,"texture/texture",PROPERTY_HINT_RESOURCE_TYPE,"Texture"),_SCS("set_texture"),_SCS("get_texture"));
}

CreatureGodot::CreatureGodot() {
	color=Color(1,1,1);
	rect_cache_dirty=true;
}