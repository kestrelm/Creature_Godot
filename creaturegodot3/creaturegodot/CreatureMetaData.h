#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <string>

namespace CreatureModule
{
    class CreatureMetaData
    {
    public:
        CreatureMetaData(const std::string& json_str);
        virtual ~CreatureMetaData();

        bool getIsValid() const { return is_valid; }

    protected:
        std::unordered_map<int, std::pair<int, int>> mesh_map;
        std::unordered_map<std::string, std::unordered_map<int, std::vector<int>>> anim_order_map;
        std::unordered_map<std::string, std::unordered_map<int, std::string>> anim_events_map;
        std::unordered_map<std::string, std::unordered_set<std::string>> skin_swaps;
        bool is_valid;
    };
}