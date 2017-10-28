#include "creaturegodot.h"
#include "project_settings.h"
#include "engine.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

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

    // load regular JSON
    CreatureModule::LoadCreatureJSONData(filename_in, *new_packet);
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
#ifdef _CREATURE_DEBUG        
        std::cout << "ERROR! ACreatureActor::AddLoadedAnimation() Animation with filename: " << filename_in << " and name: " << name_in << " not loaded!" << std::endl;
#endif        
    }

    return false;
}

// CreatureGodot
CreatureGodot::CreatureGodot() {
    color=Color(1,1,1);
    rect_cache_dirty=true;
    anim_speed = 2.0f;
    mirror_y = false;
    anim_frame = 0.0f;
    indices_process_mode = INDICES_MODE_NONE;    
    reload_data = false;
}

bool 
CreatureGodot::load_json(const String filename_in)
{
    auto global_path = ProjectSettings::get_singleton()->globalize_path(filename_in);
#ifdef _CREATURE_DEBUG        
    std::cout<<"CreatureGodot::load_json() - Loading file: "<<global_path.utf8()<<std::endl;
#endif    
    std::string load_filename(global_path.utf8());
    auto can_load = LoadDataPacket(load_filename);
    if(!can_load)
    {
#ifdef _CREATURE_DEBUG        
        std::cout<<"CreatureGodot::load_json() - ERRROR! Could not load file: "<<load_filename<<std::endl;
#endif
        return false;
    }
#ifdef _CREATURE_DEBUG        
    std::cout<<"CreatureGodot::load_json() - Finished loading file: "<<load_filename<<std::endl;
#endif
    auto json_data = global_load_data_packets[load_filename];
    auto cur_creature = std::shared_ptr<CreatureModule::Creature>(new CreatureModule::Creature(*json_data));    
    manager = std::unique_ptr<CreatureModule::CreatureManager> (new CreatureModule::CreatureManager(cur_creature));
    
    auto all_animation_names = manager->GetCreature()->GetAnimationNames();
    auto first_animation_name = all_animation_names[0];
    for (auto& cur_name : all_animation_names)
    {
#ifdef _CREATURE_DEBUG        
        std::cout<<"CreatureGodot::load_json() - Trying to load animation: "<<cur_name<<std::endl;
#endif        
        load_animation(load_filename, cur_name);
        add_loaded_animation(manager.get(), load_filename, cur_name);
#ifdef _CREATURE_DEBUG        
        std::cout<<"CreatureGodot::load_json() - Loaded animation: "<<cur_name<<std::endl;
#endif        
    }
    
    manager->SetActiveAnimationName(first_animation_name);

    if(!Engine::get_singleton()->is_editor_hint())
    {
#ifdef _CREATURE_DEBUG        
        std::cout<<"CreatureGodot::load_json() -Enabling AutoBlending for: "<<filename_in.utf8()<<std::endl;
#endif        
        manager->SetAutoBlending(true);        
    }
    else {
#ifdef _CREATURE_DEBUG        
        std::cout<<"CreatureGodot::load_json() -In Editor, Disabling Autoblending for: "<<filename_in.utf8()<<std::endl;
#endif        
    }
    anim_name = String(first_animation_name.c_str());
    
    return true;
}

bool CreatureGodot::blend_to_animation(String animation_name, float blend_delta)
{
    auto real_anim_name = std::string(animation_name.utf8());
    if(manager->GetAllAnimations().count(real_anim_name) == 0)
    {
#ifdef _CREATURE_DEBUG        
        std::cout<<"CreatureGodot::blend_to_animation() - ERROR! Animation name: "<<real_anim_name<<" does not exist."<<std::endl;
#endif        
        return false;
    }
    
    manager->AutoBlendTo(real_anim_name, blend_delta);
    
    return true;
}

void CreatureGodot::set_skinswap_name(String name_in)
{
    skinswap_name = name_in;
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

void CreatureGodot::set_anim_frame(float frame_in)
{
    if(manager)
    {
        anim_frame = frame_in;
        manager->setRunTime(frame_in);
        update_animation(0.0f);
    }
}

float CreatureGodot::get_anim_frame() const
{
    return anim_frame;   
}
    
void CreatureGodot::set_anim_name(String name_in)
{
    if(manager)
    {
        auto retval = blend_to_animation(name_in, 1.0f);
        if(retval)
        {
            update_animation(0.0f);
            anim_name = name_in;            
        }
    }
}

String CreatureGodot::get_anim_name() const
{
    return anim_name;
}

void CreatureGodot::enable_skinswap()
{
    indices_process_mode = INDICES_MODE_SKINSWAP;
}

void CreatureGodot::enable_layerorder()
{
    indices_process_mode = INDICES_MODE_ORDER;    
}

void CreatureGodot::disable_skinswap_or_order()
{
    indices_process_mode = INDICES_MODE_NONE;
}

void CreatureGodot::update_colors()
{
    if(manager)
    {
        if(fill_colors.size() != manager->GetCreature()->GetTotalNumPoints())
        {
            fill_colors.resize(manager->GetCreature()->GetTotalNumPoints());
        }

        auto& regions_map = manager->GetCreature()->GetRenderComposition()->getRegionsMap();
        for (auto& cur_region_pair : regions_map)
        {
            auto cur_region = cur_region_pair.second;
            auto start_pt_index = cur_region->getStartPtIndex();
            auto end_pt_index = cur_region->getEndPtIndex();
            auto cur_alpha = cur_region->getOpacity() / 100.0f;
            for (auto i = start_pt_index; i <= end_pt_index; i++)
            {
                fill_colors[i].r = color.r * cur_alpha;
                fill_colors[i].g = color.g * cur_alpha;
                fill_colors[i].b = color.b * cur_alpha;
                fill_colors[i].a = color.a * cur_alpha;
            }
        }
    }
}

void CreatureGodot::update_animation(float delta)
{
    auto hasEvents = [&]()
    {
        if(metadata)
        {
            if(metadata->hasEvents(manager->GetActiveAnimationName()))
            {
                return true;
            }    
        }

        return false;
    };

    auto processEvents = [&]()
    {
        std::string evt_name = 
        metadata->runEvents(
            manager->GetActiveAnimationName(),
            static_cast<int>(manager->getActualRunTime()));
        if(!evt_name.empty())
        {
            emit_signal("CreatureEvent", String(evt_name.c_str()));
        }
    };

    if(manager)
    {
        auto old_time = manager->getActualRunTime();
        bool has_events = hasEvents();                
        if(has_events)
        {
            processEvents();
        }

        manager->Update(delta * anim_speed);

        if((old_time > manager->getActualRunTime()) && has_events)
        {
            metadata->resetEvents(manager->GetActiveAnimationName());
        }
        
        // resize points, uvs and indices array
        if(reload_data)
        {
            points.resize(manager->GetCreature()->GetTotalNumPoints());
            uvs.resize(points.size());
            indices.resize(manager->GetCreature()->GetTotalNumIndices());
            meta_indices.resize(manager->GetCreature()->GetTotalNumIndices());
            reload_data = false;
        }
        
        // fill in rendering data

        // colors
        update_colors();
        
        // indices/topology
        auto cur_indices = manager->GetCreature()->GetGlobalIndices();
        for(size_t i = 0; i < indices.size(); i++)
        {
            indices[i] = cur_indices[i];
        }

        // MetaData indices processing        
        if(indices_process_mode == INDICES_MODE_ORDER)
        {
            process_layerorder(static_cast<int>(manager->getActualRunTime()));
        }
        else if(indices_process_mode == INDICES_MODE_SKINSWAP)
        {
            process_skinswap();
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
                item_rect.position = pos;
            else
                item_rect.expand_to(pos);
        }
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
            bool use_meta_indices = (indices_process_mode != INDICES_MODE_NONE);
            if((points.size() < 3) || (uvs.size() < 3) || (indices.size() % 3 != 0))
            {
                return;
            }

            if(use_meta_indices && (real_meta_indices.size() % 3 != 0))
            {
                return;
            }
            
            if(manager)
            {
               manager->SetMirrorY(mirror_y);
            }

            
            VS::get_singleton()->canvas_item_add_triangle_array(
                get_canvas_item(),
                use_meta_indices ? real_meta_indices : indices,
                points,
                fill_colors,
                uvs,texture.is_valid()?texture->get_rid():RID()
            );

        } break;
    }
}

void CreatureGodot::set_color(Color p_color){

    color=p_color;
    update();
}

Color CreatureGodot::get_color() const{

    return color;
}

void CreatureGodot::set_asset_filename(String filename_in)
{
    auto retval = load_json(filename_in);
    
    if(retval)
    {
        reload_data = true;
        asset_filename = filename_in;
        update_animation(0.1f);     
        rect_cache_dirty = true;   
    }
}

String CreatureGodot::get_asset_filename() const
{
    return asset_filename;
}

void CreatureGodot::set_metadata_filename(String filename_in)
{
    auto global_path = ProjectSettings::get_singleton()->globalize_path(filename_in);
#ifdef _CREATURE_DEBUG        
    std::cout<<"CreatureGodot - Loading MetaData: "<<global_path.utf8()<<std::endl;
#endif    

    if(filename_in.empty())
    {
        metadata_filename = "";
        return;
    }

    if(!is_file_exist(global_path.utf8()))
    {
        return;
    }

    std::ifstream read_file;
    read_file.open(global_path.utf8());
    std::stringstream str_stream;
    str_stream << read_file.rdbuf();
    read_file.close();

    metadata_filename = filename_in;
    metadata = std::unique_ptr<CreatureModule::CreatureMetaData>(
        new CreatureModule::CreatureMetaData(str_stream.str()));
}

String CreatureGodot::get_metadata_filename() const
{
    return metadata_filename;
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

void CreatureGodot::set_active_item_swap(String region_name, int swap_idx)
{
    if(manager)
    {
        manager->GetCreature()->SetActiveItemSwap(std::string(region_name.utf8()), swap_idx);
    }   
}

void CreatureGodot::remove_active_item_swap(String region_name)
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

void CreatureGodot::make_point_cache(String animation_name_in, int gap_step)
{
    if(manager)
    {
        manager->MakePointCache(std::string(animation_name_in.utf8()), gap_step);
    }
}

void CreatureGodot::add_skinswap(String name_in, Vector<String> custom_swap)
{
    if(metadata)
    {
        std::unordered_set<std::string> swap_set;
        for(int i = 0; i < custom_swap.size(); i++)
        {
            swap_set.insert(std::string(custom_swap[i].utf8()));
        }

        metadata->addSkinSwap(std::string(name_in.utf8()), swap_set);
    }
}

void CreatureGodot::remove_skinswap(String name_in)
{
    if(metadata)
    {
        metadata->removeSkinSwap(std::string(name_in.utf8()));
    }
}

void CreatureGodot::process_skinswap()
{
    int real_indices_size = 0;
    auto render_composition = manager->GetCreature()->GetRenderComposition();    
    auto retval = metadata->buildSkinSwapIndices(
        std::string(skinswap_name.utf8()),
        render_composition,
        [&](int idx, int value)
        {
            meta_indices[idx] = value;
        },
        real_indices_size
    );

    if(retval)
    {
        if(real_meta_indices.size() != real_indices_size)
        {
            real_meta_indices.resize(real_indices_size);            
        }

        for(int i = 0; i < real_indices_size; i++)
        {
            real_meta_indices[i] = meta_indices[i];
        }
    }
}

void CreatureGodot::process_layerorder(int time_in)
{
    metadata->updateIndicesAndPoints(
        manager->GetCreature()->GetGlobalIndices(),
        [&](int idx, int value)
        {
            meta_indices[idx] = value;            
        },
        manager->GetCreature()->GetTotalNumIndices(),
        manager->GetActiveAnimationName(),
        time_in        
    );

    if(real_meta_indices.size() != meta_indices.size())
    {
        real_meta_indices.resize(meta_indices.size());            
    }

    for(int i = 0; i < real_meta_indices.size(); i++)
    {
        real_meta_indices[i] = meta_indices[i];
    }
}

Vector2 
CreatureGodot::get_bone_pos(String bone_name, float slide_factor)
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

Vector<String> CreatureGodot::get_anim_clips() const
{
    Vector<String> ret_names;

    if(manager) {
        const auto& all_anims = manager->GetCreature()->GetAnimationNames();
        for(const auto& cur_name : all_anims)
        {
            ret_names.push_back(String(cur_name.c_str()));
        }        
    }

    return ret_names;
}

void CreatureGodot::_bind_methods() {

    ClassDB::bind_method(D_METHOD("set_color","color"),&CreatureGodot::set_color);
    ClassDB::bind_method(D_METHOD("get_color"),&CreatureGodot::get_color);

    ClassDB::bind_method(D_METHOD("set_texture","texture"),&CreatureGodot::set_texture);
    ClassDB::bind_method(D_METHOD("get_texture"),&CreatureGodot::get_texture);

    ClassDB::bind_method(D_METHOD("set_anim_speed","texture"),&CreatureGodot::set_anim_speed);
    ClassDB::bind_method(D_METHOD("get_anim_speed"),&CreatureGodot::get_anim_speed);
    
    ClassDB::bind_method(D_METHOD("set_asset_filename","asset_filename"),&CreatureGodot::set_asset_filename);
    ClassDB::bind_method(D_METHOD("get_asset_filename"),&CreatureGodot::get_asset_filename);

    ClassDB::bind_method(D_METHOD("set_metadata_filename","metadata_filename"),&CreatureGodot::set_metadata_filename);
    ClassDB::bind_method(D_METHOD("get_metadata_filename"),&CreatureGodot::get_metadata_filename);

    ClassDB::bind_method(D_METHOD("set_offset","offset"),&CreatureGodot::set_offset);
    ClassDB::bind_method(D_METHOD("get_offset"),&CreatureGodot::get_offset);

    ClassDB::bind_method(D_METHOD("set_mirror_y","offset"),&CreatureGodot::set_mirror_y);
    ClassDB::bind_method(D_METHOD("get_mirror_y"),&CreatureGodot::get_mirror_y);

    ClassDB::bind_method(D_METHOD("set_anim_frame","offset"),&CreatureGodot::set_anim_frame);
    ClassDB::bind_method(D_METHOD("get_anim_frame"),&CreatureGodot::get_anim_frame);

    ClassDB::bind_method(D_METHOD("set_anim_name","offset"),&CreatureGodot::set_anim_name);
    ClassDB::bind_method(D_METHOD("get_anim_name"),&CreatureGodot::get_anim_name);
    
    ClassDB::bind_method(D_METHOD("load_json"),&CreatureGodot::load_json);
    ClassDB::bind_method(D_METHOD("update_animation"),&CreatureGodot::update_animation);
    ClassDB::bind_method(D_METHOD("blend_to_animation"),&CreatureGodot::blend_to_animation);
    ClassDB::bind_method(D_METHOD("set_should_loop"),&CreatureGodot::set_should_loop);
    ClassDB::bind_method(D_METHOD("get_anim_clips"),&CreatureGodot::get_anim_clips);
    
    ClassDB::bind_method(D_METHOD("set_skinswap_name"),&CreatureGodot::set_skinswap_name);
    ClassDB::bind_method(D_METHOD("add_skinswap"),&CreatureGodot::add_skinswap);
    ClassDB::bind_method(D_METHOD("remove_skinswap"),&CreatureGodot::remove_skinswap);

    ClassDB::bind_method(D_METHOD("enable_skinswap"),&CreatureGodot::enable_skinswap);
    ClassDB::bind_method(D_METHOD("enable_layerorder"),&CreatureGodot::enable_layerorder);
    ClassDB::bind_method(D_METHOD("disable_skinswap_or_order"),&CreatureGodot::disable_skinswap_or_order);
    
    ClassDB::bind_method(D_METHOD("set_active_item_swap"),&CreatureGodot::set_active_item_swap);
    ClassDB::bind_method(D_METHOD("remove_active_item_swap"),&CreatureGodot::remove_active_item_swap);
    
    ClassDB::bind_method(D_METHOD("set_anchor_points_active"),&CreatureGodot::set_anchor_points_active);
    
    ClassDB::bind_method(D_METHOD("make_point_cache"),&CreatureGodot::make_point_cache);

    ClassDB::bind_method(D_METHOD("get_bone_pos"),&CreatureGodot::get_bone_pos);
    
    ADD_PROPERTY( PropertyInfo(Variant::STRING,"anim_name"), "set_anim_name", "get_anim_name");
    ADD_PROPERTY( PropertyInfo(Variant::REAL,"anim_frame"), "set_anim_frame", "get_anim_frame");
    ADD_PROPERTY( PropertyInfo(Variant::REAL,"anim_speed"), "set_anim_speed", "get_anim_speed");
    ADD_PROPERTY( PropertyInfo(Variant::STRING,"asset_filename"), "set_asset_filename", "get_asset_filename");
    ADD_PROPERTY( PropertyInfo(Variant::STRING,"metadata_filename"), "set_metadata_filename", "get_metadata_filename");
    ADD_PROPERTY( PropertyInfo(Variant::BOOL,"mirror_y"), "set_mirror_y", "get_mirror_y");
    ADD_PROPERTY( PropertyInfo(Variant::COLOR,"color"), "set_color", "get_color");
    ADD_PROPERTY( PropertyInfo(Variant::VECTOR2,"offset"), "set_offset", "get_offset");
    ADD_PROPERTY( PropertyInfo(Variant::OBJECT,"texture/texture",PROPERTY_HINT_RESOURCE_TYPE,"Texture"), "set_texture", "get_texture");
}