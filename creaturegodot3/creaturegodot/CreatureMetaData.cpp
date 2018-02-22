#include "CreatureMetaData.h"
#include "gason.h"
#include "MeshBone.h"
#include "CreatureModule.h"
#include <memory>
#include <algorithm>
#include <iostream>
#include <cstring>

// Helper functions

class JsonDataHelper
{
public:
    std::unique_ptr<char> chars_data;
    JsonValue base_node;
    JsonAllocator allocator;
};

namespace Base64Lib {
	static const std::string base64_chars =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";

	static inline bool is_base64(uint8_t c) {
		return (isalnum(c) || (c == '+') || (c == '/'));
	}

	std::vector<uint8_t> base64_decode(const std::string& encoded_string) {
		int in_len = static_cast<int>(encoded_string.size());
		int i = 0;
		int j = 0;
		int in_ = 0;
		uint8_t char_array_4[4], char_array_3[3];
		std::vector<uint8_t> ret;

		while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
			char_array_4[i++] = encoded_string[in_]; in_++;
			if (i == 4) {
				for (i = 0; i <4; i++)
					char_array_4[i] = (uint8_t)base64_chars.find(char_array_4[i]);

				char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
				char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
				char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

				for (i = 0; (i < 3); i++)
					ret.push_back(char_array_3[i]);
				i = 0;
			}
		}

		if (i) {
			for (j = i; j <4; j++)
				char_array_4[j] = 0;

			for (j = 0; j <4; j++)
				char_array_4[j] = (uint8_t)base64_chars.find(char_array_4[j]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);
		}

		return ret;
	}
}

// CreatureMetaData
CreatureModule::CreatureMetaData::CreatureMetaData(const std::string& json_str)
{
    is_valid = false;
    // Parse and load in data
    auto loadJsonFromStr = [&](const std::string& str_in, JsonDataHelper& data_helper)
    {
        char *endptr;
        size_t source_size = str_in.size();
        data_helper.chars_data = std::unique_ptr<char>(new char[source_size + 1]);
        data_helper.chars_data.get()[source_size] = 0;
        std::copy(str_in.c_str(), str_in.c_str() + source_size, data_helper.chars_data.get());
        JsonParseStatus status = jsonParse(data_helper.chars_data.get(), &endptr, &data_helper.base_node, data_helper.allocator);
        return status;
    };

    auto getJsonNode = [&](JsonNode& json_obj, const std::string& key)
    {
        JsonNode * ret_node = nullptr;
        for(JsonIterator it = JsonBegin(json_obj.value); it != JsonEnd(json_obj.value); ++it)
        {
            if( std::string((*it)->key) == key) {
                ret_node = *it;
                break;
            }
        }

        return ret_node;
    };

    auto getJsonNodeFromArray = [this](JsonNode& json_obj, int idx)
    {
        int i = 0;
        for(auto it = JsonBegin(json_obj.value); it != JsonEnd(json_obj.value); ++it)
        {
            JsonNode * cur_node = *it;
            if(i == idx)
            {
                return cur_node;
            }

            i++;
        }

        return (JsonNode *)nullptr;
    };

    auto parseMeshes = [&](JsonNode * node_in)
    {
        for(auto it = JsonBegin(node_in->value); it != JsonEnd(node_in->value); ++it)
        {
            JsonNode * cur_node = *it;
            auto cur_id = static_cast<int>(getJsonNode(*cur_node, "id")->value.toNumber());
            auto cur_start_index = static_cast<int>(getJsonNode(*cur_node, "startIndex")->value.toNumber());
            auto cur_end_index = static_cast<int>(getJsonNode(*cur_node, "endIndex")->value.toNumber());

            mesh_map[cur_id] = std::make_pair(cur_start_index, cur_end_index);
        }
    };

    auto parseRegionOrders = [&](JsonNode * node_in)
    {
        for(auto it = JsonBegin(node_in->value); it != JsonEnd(node_in->value); ++it)
        {
            JsonNode * cur_node = *it;
            std::unordered_map<int, std::vector<int>> cur_switch_order_map;
            std::string cur_anim_name(cur_node->key);

            for(auto a_it = JsonBegin(cur_node->value); a_it != JsonEnd(cur_node->value); ++a_it)
            {
                auto sub_node = *a_it;
                auto switch_node = getJsonNode(*sub_node, "switch_order");
                std::vector<int> cur_switch_ints;                
                for(auto s_it = JsonBegin(switch_node->value); s_it != JsonEnd(switch_node->value); ++s_it)
                {
                    auto val_node = *s_it;
                    cur_switch_ints.push_back(static_cast<int>(val_node->value.toNumber()));
                }

                auto cur_switch_time = static_cast<int>(getJsonNode(*sub_node, "switch_time")->value.toNumber());                
                cur_switch_order_map[cur_switch_time] = cur_switch_ints;
            }

            anim_order_map[cur_anim_name] = cur_switch_order_map;
        }
    };

    auto parseEventTriggers = [&](JsonNode * node_in)
    {
        for(auto it = JsonBegin(node_in->value); it != JsonEnd(node_in->value); ++it)
        {
            JsonNode * cur_node = *it;
            std::string cur_anim_name(cur_node->key);
            std::unordered_map<int, std::pair<std::string, bool>> cur_events_map;
            for(auto a_it = JsonBegin(cur_node->value); a_it != JsonEnd(cur_node->value); ++a_it)
            {
                auto cur_events_obj = *a_it;
                auto cur_event_name = std::string(getJsonNode(*cur_events_obj, "event_name")->value.toString());
                auto cur_switch_time = static_cast<int>(getJsonNode(*cur_events_obj, "switch_time")->value.toNumber());
                cur_events_map[cur_switch_time] = std::make_pair(cur_event_name, false);
            }   

            anim_events_map[cur_anim_name] = cur_events_map;
        }
    };

    auto parseSkinSwapList = [&](JsonNode * node_in)
    {
        for(auto it = JsonBegin(node_in->value); it != JsonEnd(node_in->value); ++it)
        {
            JsonNode * cur_node = *it;
            std::unordered_set<std::string> new_swap_set;
            std::string swap_name(cur_node->key);
            auto swap_data = getJsonNode(*cur_node, "swap");
            auto swap_items = getJsonNode(*swap_data, "swap_items");
            for(auto a_it = JsonBegin(swap_items->value); a_it != JsonEnd(swap_items->value); ++a_it)
            {
                auto cur_item = *a_it;
                auto item_name = std::string(cur_item->value.toString());
                new_swap_set.insert(item_name);
            }

            skin_swaps[swap_name] = new_swap_set;
        }
    };

    auto parseMorphTargets = [this, getJsonNode, getJsonNodeFromArray]
    (
        JsonNode * node_morph_targets, 
        JsonNode * node_morph_res, 
        JsonNode * node_morph_space)
    {
		auto morph_obj = node_morph_targets;

		auto morph_center_array = getJsonNode(*morph_obj, "CenterData");
		morph_data.center_clip = getJsonNodeFromArray(*morph_center_array, 1)->value.toString();

		auto morph_shapes_array = getJsonNode(*morph_obj, "MorphShape");
		morph_data.bounds_min = glm::vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
		morph_data.bounds_max = glm::vec2(std::numeric_limits<float>::min(), std::numeric_limits<float>::min());

        for(auto it = JsonBegin(morph_shapes_array->value); it != JsonEnd(morph_shapes_array->value); ++it)
		{
			auto cur_array = *it;
			auto cur_clip = getJsonNodeFromArray(*cur_array, 0)->value.toString();
			auto pts_array = getJsonNodeFromArray(*cur_array, 1);
			glm::vec2 cur_pt(
                (float)getJsonNodeFromArray(*pts_array, 0)->value.toNumber(), 
                (float)getJsonNodeFromArray(*pts_array, 1)->value.toNumber());

			morph_data.bounds_min.x = std::min(morph_data.bounds_min.x, cur_pt.x);
			morph_data.bounds_max.x = std::max(morph_data.bounds_max.x, cur_pt.x);
			morph_data.bounds_min.y = std::min(morph_data.bounds_min.y, cur_pt.y);
			morph_data.bounds_max.y = std::max(morph_data.bounds_max.y, cur_pt.y);

			morph_data.morph_clips.push_back(std::make_pair(cur_clip, cur_pt));
		}
		
		morph_data.morph_res = static_cast<int>(node_morph_res->value.toNumber());
		{
			// Transform to Image space
			for (auto& cur_clip : morph_data.morph_clips)
			{
				auto bounds_size = morph_data.bounds_max - morph_data.bounds_min;
				auto& cur_pt = cur_clip.second;
				cur_pt = (cur_pt - morph_data.bounds_min) / bounds_size * (float)(morph_data.morph_res - 1);
				cur_pt.x = std::min(std::max(0.0f, cur_pt.x), (float)(morph_data.morph_res - 1));
				cur_pt.y = std::min(std::max(0.0f, cur_pt.y), (float)(morph_data.morph_res - 1));
			}
		}


		std::string raw_str = node_morph_space->value.toString();
		auto raw_bytes = Base64Lib::base64_decode(raw_str);
		for (size_t j = 0; j < morph_data.morph_clips.size(); j++)
		{
			int space_size = morph_data.morph_res * morph_data.morph_res;
			int byte_idx = j * space_size;
			std::vector<uint8_t> space_data;
			space_data.resize(space_size);
			std::memcpy(space_data.data(), raw_bytes.data() + byte_idx, space_size * sizeof(uint8_t));
			morph_data.morph_spaces.push_back(space_data);
		}

		morph_data.weights.resize( morph_data.morph_clips.size());        
    };

    // Load in MetaData types
    JsonDataHelper json_data;
    auto status = loadJsonFromStr(json_str, json_data);
    if(status != JSON_PARSE_OK) {
#ifdef _CREATURE_DEBUG        
        std::cout<<"CreatureMetaData() - Error parsing Meta Data JSON!"<<std::endl;
#endif        
        return;
    } 

    JsonNode * json_base = json_data.base_node.toNode();
    JsonNode * node_morph_targets = nullptr, * node_morph_res = nullptr, * node_morph_space = nullptr;
    while(json_base->next != nullptr)
    {
        auto cur_key = std::string(json_base->key);
        auto cur_node = json_base;
        if(cur_key == "meshes")
        {
            parseMeshes(cur_node);
        }
        else if(cur_key == "regionOrders")
        {
            parseRegionOrders(cur_node);
        }
        else if(cur_key == "eventTriggers")
        {
            parseEventTriggers(cur_node);
        }
        else if(cur_key == "skinSwapList")
        {
            parseSkinSwapList(cur_node);
        }        
        else if(cur_key == "MorphTargets")
        {
            node_morph_targets = cur_node;           
        }
        else if(cur_key == "MorphRes")
        {
            node_morph_res = cur_node;
        }
        else if(cur_key == "MorphSpace")
        {
            node_morph_space = cur_node;           
        }

        json_base = json_base->next;
    }

    // Morph Targets Parsing
    if(node_morph_targets && node_morph_res && node_morph_space)
    {
        parseMorphTargets(node_morph_targets, node_morph_res, node_morph_space);
    }

    is_valid = true;
    printInfo();
}

CreatureModule::CreatureMetaData::~CreatureMetaData()
{

}

void CreatureModule::CreatureMetaData::printInfo()
{
#ifdef _CREATURE_DEBUG        
    std::cout<<"\nCreatureMetaData SkinSwaps: "<<std::endl;
    for(const auto& cur_data : skin_swaps)
    {
        std::cout<<"\t-"<<cur_data.first<<std::endl;
    }

    std::cout<<"\nCreatureMetaData Layer Order Animation: "<<std::endl;
    for(const auto& cur_data : anim_order_map)
    {
        std::cout<<"\t-"<<cur_data.first<<" size: "<<cur_data.second.size()<<std::endl;
    }

    std::cout<<"\nCreatureMetaData Events: "<<std::endl;
    for(const auto& cur_data : anim_events_map)
    {
        std::cout<<"\t-"<<cur_data.first<<" size: "<<cur_data.second.size()<<std::endl;
    }
#endif    
}

bool CreatureModule::CreatureMetaData::buildSkinSwapIndices(
    const std::string& swap_name,
    meshRenderBoneComposition * bone_composition,
    std::function<void(int, int)> indices_callback,
    int& total_size
)
{
    total_size = 0;
    if(skin_swaps.count(swap_name) == 0)
    {
        return false;
    }

    const auto& swap_set = skin_swaps[swap_name];
    const auto& regions = bone_composition->getRegions();
    int offset = 0;
	for (auto cur_region : regions)
	{
		if (swap_set.count(cur_region->getName()) > 0)
		{
            for(auto idx = 0; idx < cur_region->getNumIndices(); idx++)
            {
                indices_callback(offset + idx, cur_region->getIndices()[idx]);
            }

            offset += cur_region->getNumIndices();
		}        
    }

    total_size = offset;
    return true;
}

void CreatureModule::CreatureMetaData::updateIndicesAndPoints(
    glm::uint32 * src_indices,
    std::function<void(int, int)> dst_indices_callback,
    int num_indices,
    const std::string& anim_name,
    int time_in
)
{
    auto copyIndices = [&](glm::uint32 * ref_indices, int offset, int write_indices) {
        for(int i = 0; i < write_indices; i++)
        {
            dst_indices_callback(i + offset, ref_indices[i]);
        }
    };

    bool has_data = false;
    auto cur_order = sampleOrder(anim_name, time_in);
    if(cur_order)
    {
        has_data = (cur_order->size() > 0);
    }

    if (has_data)
    {
        // Copy new ordering to destination
        int write_offset = 0;
        int total_num_write_indices = 0;
        for (auto region_id : (*cur_order))
        {
            if (mesh_map.count(region_id) == 0)
            {
                // region not found, just copy and return
                copyIndices(src_indices, 0, num_indices);
                return;
            }

            // Write indices
            auto& mesh_data = mesh_map[region_id];
            auto num_write_indices = mesh_data.second - mesh_data.first + 1;
            auto region_src_ptr = src_indices + mesh_data.first;
            total_num_write_indices += num_write_indices;

            if (total_num_write_indices > num_indices)
            {
                // overwriting boundaries of array, regions do not match so copy and return
                copyIndices(src_indices, 0, num_indices);
                return;
            }

            copyIndices(region_src_ptr, write_offset, num_write_indices);
            write_offset += num_write_indices;
        }
    }
    else {
        // Nothing changded, just copy from source
        copyIndices(src_indices, 0, num_indices);
    }
}

void CreatureModule::CreatureMetaData::updateMorphStep(
    CreatureModule::CreatureManager * manager_in, 
    float delta_step)
{
	auto creature_in = manager_in->GetCreature();
	if (morph_data.play_anims_data.empty())
	{
		auto& all_clips = manager_in->GetAllAnimations();
		morph_data.play_anims_data.resize(morph_data.morph_clips.size());
		for (size_t i = 0; i < morph_data.play_anims_data.size(); i++)
		{
			auto& cur_play_data = morph_data.play_anims_data[i];
			cur_play_data.first = morph_data.morph_clips[i].first;
			const auto& cur_clip_name = cur_play_data.first;
			cur_play_data.second = all_clips[cur_clip_name]->getStartTime();
		}

		if (morph_data.center_clip.size() > 0)
		{
			morph_data.play_center_anim_data.first = morph_data.center_clip;
			const auto& center_clip_name = morph_data.play_center_anim_data.first;
			morph_data.play_center_anim_data.second = all_clips[center_clip_name]->getStartTime();
		}

		morph_data.play_pts.resize(creature_in->GetTotalNumPoints() * 3);
	}

    std::memset(morph_data.play_pts.data(), 0, sizeof(glm::float32) * morph_data.play_pts.size());

	auto updatePoints = [this, creature_in](float ratio_in)
	{
		auto render_pts = morph_data.play_pts.data();
		for (int j = 0; j < creature_in->GetTotalNumPoints() * 3; j += 3)
		{
			render_pts[j] +=
				(creature_in->GetRenderPts()[j] * ratio_in);
			render_pts[j + 1] +=
				(creature_in->GetRenderPts()[j + 1] * ratio_in);
		}
	};

	float center_ratio = 0;
	bool has_center = (morph_data.center_clip.size() > 0);
	if (has_center)
	{
		auto radius = (float)morph_data.morph_res * 0.5f;
		auto test_pt = morph_data.play_img_pt - glm::vec2(morph_data.morph_res / 2, morph_data.morph_res / 2);
		center_ratio = glm::length(test_pt / ((float)morph_data.morph_res * 0.5f));

		const auto& clip_name = morph_data.play_center_anim_data.first;
		manager_in->SetActiveAnimationName(clip_name);
		manager_in->setRunTime(morph_data.play_center_anim_data.second);
		manager_in->Update(delta_step);

		float inv_center_ratio = 1.0f - center_ratio;
		updatePoints(inv_center_ratio);
		morph_data.play_center_anim_data.second = manager_in->getRunTime();
	}

	for (size_t i = 0; i < morph_data.play_anims_data.size(); i++)
	{
		auto& cur_data = morph_data.play_anims_data[i];
		const auto& clip_name = cur_data.first;
		manager_in->SetActiveAnimationName(clip_name);
		manager_in->setRunTime(cur_data.second);
		manager_in->Update(delta_step);

		cur_data.second = manager_in->getRunTime();
		updatePoints((center_ratio > 0) ? (morph_data.weights[i] * center_ratio) : morph_data.weights[i]);
	}

	// Copy to current render points
	std::memcpy(
		creature_in->GetRenderPts(),
		morph_data.play_pts.data(), 
		sizeof(glm::float32) * morph_data.play_pts.size());
}

const std::vector<int> * 
CreatureModule::CreatureMetaData::sampleOrder(const std::string& anim_name, int time_in) const
{
    if (anim_order_map.count(anim_name) > 0)
    {
        const auto& order_table = anim_order_map.at(anim_name);
        if (order_table.empty())
        {
            return nullptr;
        }

        int sample_time = 0;
        for(const auto& order_data : order_table)
        {
            auto cur_time = order_data.first;
            if (time_in >= cur_time)
            {
                sample_time = cur_time;
            }
        }

        return &order_table.at(sample_time);
    }

    return nullptr;
}

bool 
CreatureModule::CreatureMetaData::addSkinSwap(
    const std::string& swap_name, 
    const std::unordered_set<std::string>& set_in)
{
    if (skin_swaps.count(swap_name))
    {
        return false;
    }

    skin_swaps[swap_name] = set_in;
    return true;    
}

bool 
CreatureModule::CreatureMetaData::removeSkinSwap(const std::string& swap_name)
{
    if (skin_swaps.count(swap_name))
    {
        return false;
    }

    skin_swaps.erase(swap_name);
    return true;
}

void CreatureModule::CreatureMetaData::resetEvents(const std::string& anim_name)
{
    if(anim_events_map.count(anim_name) == 0)
    {
        return;
    }

    auto& cur_events = anim_events_map[anim_name];
    for(auto& cur_data : cur_events)
    {
        cur_data.second.second = false;
    }
}

bool  CreatureModule::CreatureMetaData::hasEvents(const std::string& anim_name) const
{
    return anim_events_map.count(anim_name) > 0;
}

std::string CreatureModule::CreatureMetaData::runEvents(const std::string& anim_name, int time_in)
{
    std::string ret_name;
    if(anim_events_map.count(anim_name) == 0)
    {
        return ret_name;
    }

    int greatest_time = 0;
    auto& cur_events = anim_events_map[anim_name];
    for(auto& cur_data : cur_events)
    {
        auto trigger_time = cur_data.first;
        auto& triggers = cur_data.second;
        const auto& trigger_name = triggers.first;
        auto& triggered = triggers.second;

        if(triggered == false)
        {
            triggered = true;
            if(trigger_time >= greatest_time)
            {
                ret_name = trigger_name;
                greatest_time = trigger_time;                    
            }
        }
    }

    return ret_name;
}
