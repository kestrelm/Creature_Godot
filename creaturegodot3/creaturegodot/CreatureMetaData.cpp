#include "CreatureMetaData.h"
#include "gason.h"
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
