#include "CreatureMetaData.h"
#include "gason.h"
#include "MeshBone.h"
#include <memory>
#include <algorithm>
#include <iostream>


class JsonDataHelper
{
public:
    std::unique_ptr<char> chars_data;
    JsonValue base_node;
    JsonAllocator allocator;
};

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
            std::unordered_map<int, std::string> cur_events_map;
            for(auto a_it = JsonBegin(cur_node->value); a_it != JsonEnd(cur_node->value); ++a_it)
            {
                auto cur_events_obj = *a_it;
                auto cur_event_name = std::string(getJsonNode(*cur_events_obj, "event_name")->value.toString());
                auto cur_switch_time = static_cast<int>(getJsonNode(*cur_events_obj, "switch_time")->value.toNumber());
                cur_events_map[cur_switch_time] = cur_event_name;
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

    // Load in MetaData types
    JsonDataHelper json_data;
    auto status = loadJsonFromStr(json_str, json_data);
    if(status != JSON_PARSE_OK) {
        std::cout<<"CreatureMetaData() - Error parsing Meta Data JSON!"<<std::endl;
        return;
    } 

    JsonNode * json_root = json_data.base_node.toNode();
    for(auto it = JsonBegin(json_root->value); it != JsonEnd(json_root->value); ++it)
    {
        auto cur_key = std::string((*it)->key);
        auto cur_node = *it;
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
    }

    is_valid = true;
}

CreatureModule::CreatureMetaData::~CreatureMetaData()
{

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

