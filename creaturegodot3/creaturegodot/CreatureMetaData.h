#pragma once
#include <vector>
#include <functional>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <string>
#include "glm.hpp"

class meshRenderBoneComposition;

namespace CreatureModule {
    class CreatureManager;
}

namespace CreatureModule
{
    class CreatureMetaData
    {
    public:
        CreatureMetaData(const std::string& json_str);
        virtual ~CreatureMetaData();

        bool getIsValid() const { return is_valid; }

        bool buildSkinSwapIndices(
            const std::string& swap_name,
            meshRenderBoneComposition * bone_composition,
            std::function<void(int, int)> indices_callback,
            int& total_size
        );

        void updateIndicesAndPoints(
            glm::uint32 * src_indices,
            std::function<void(int, int)> dst_indices_callback,
            int num_indices,
            const std::string& anim_name,
            int time_in
        );

        bool addSkinSwap(const std::string& swap_name, const std::unordered_set<std::string>& set_in);

        bool removeSkinSwap(const std::string& swap_name);

        void printInfo();

        void resetEvents(const std::string& anim_name);

        bool hasEvents(const std::string& anim_name) const;

        std::string runEvents(const std::string& anim_name, int time_in);

    	void computeMorphWeights(const glm::vec2& img_pt)
    	{
    		morph_data.play_img_pt = img_pt;
    		auto sampleFilterPt = [](
    				float q11, // (x1, y1)
    				float q12, // (x1, y2)
    				float q21, // (x2, y1)
    				float q22, // (x2, y2)
    				float x1,
    				float y1,
    				float x2,
    				float y2,
    				float x,
    				float y)
    		{
    			float x2x1, y2y1, x2x, y2y, yy1, xx1;
    			x2x1 = x2 - x1;
    			y2y1 = y2 - y1;
    			x2x = x2 - x;
    			y2y = y2 - y;
    			yy1 = y - y1;
    			xx1 = x - x1;

    			float denom = (x2x1 * y2y1);
    			float numerator = (
    				q11 * x2x * y2y +
    				q21 * xx1 * y2y +
    				q12 * x2x * yy1 +
    				q22 * xx1 * yy1
    				);

    			return (denom == 0) ? q11 : (1.0f / denom * numerator);
    		};

    		auto lookupVal = [this](int x_in, int y_in, int idx)
    		{
    			auto& cur_space = morph_data.morph_spaces[idx];
    			return static_cast<float>(cur_space[y_in *  morph_data.morph_res + x_in]) / 255.0f;
    		};

    		float x1 = std::floor(img_pt.x);
    		float y1 = std::floor(img_pt.y);
    		float x2 = std::ceil(img_pt.x);
    		float y2 = std::ceil(img_pt.y);

    		for (int i = 0; i < static_cast<int>(morph_data.morph_spaces.size()); i++)
    		{
    			float q11 = (float)lookupVal(x1, y1, i); // (x1, y1)
    			float q12 = (float)lookupVal(x1, y2, i); // (x1, y2)
    			float q21 = (float)lookupVal(x2, y1, i); // (x2, y1)
    			float q22 = (float)lookupVal(x2, y2, i); // (x2, y2)

    			float sample_val = sampleFilterPt(
    				q11, q12, q21, q22, x1, y1, x2, y2, img_pt.x, img_pt.y);
				
    			morph_data.weights[i] = sample_val;
    		}
    	}

    	void computeMorphWeightsNormalised(const glm::vec2& normal_pt)
    	{
    		auto img_pt = normal_pt * glm::vec2(morph_data.morph_res - 1, morph_data.morph_res - 1);
    		img_pt.x = std::max(std::min((float)morph_data.morph_res - 1.0f, img_pt.x), 0.0f);
    		img_pt.y = std::max(std::min((float)morph_data.morph_res - 1.0f, img_pt.y), 0.0f);
    		computeMorphWeights(img_pt);
    	}

    	void computeMorphWeightsWorld(const glm::vec2& world_pt, const glm::vec2& base_pt, float radius)
    	{
    		auto rel_pt = world_pt - base_pt;
    		auto cur_length = glm::length(rel_pt);
    		if (cur_length > radius)
    		{
    			rel_pt = glm::normalize(rel_pt);
    			rel_pt *= radius;
    		}

    		glm::vec2 normal_pt = (rel_pt + glm::vec2(radius, radius)) / (radius * 2.0f);
    		computeMorphWeightsNormalised(normal_pt);
    	}

    	void updateMorphStep(CreatureModule::CreatureManager * manager_in, float delta_step);    

    	struct MorphData
    	{
    		std::vector<std::vector<uint8_t>> morph_spaces;
    		std::string center_clip;
    		std::vector<std::pair<std::string, glm::vec2>> morph_clips;
    		std::vector<float> weights;
    		glm::vec2 bounds_min, bounds_max;
    		int morph_res;
    		std::vector<std::pair<std::string, float>> play_anims_data;
    		std::pair<std::string, float> play_center_anim_data;
    		std::vector<glm::float32> play_pts;
    		glm::vec2 play_img_pt;

    		bool isValid() const {
    			return (morph_spaces.empty() == false);
    		}
    	};

		MorphData& getMorphData() {
			return morph_data;
		}

    protected:

        const std::vector<int> * sampleOrder(const std::string& anim_name, int time_in) const;

        std::unordered_map<int, std::pair<int, int>> mesh_map;
        std::unordered_map<std::string, std::unordered_map<int, std::vector<int>>> anim_order_map;
        std::unordered_map<std::string, std::unordered_map<int, std::pair<std::string, bool>>> anim_events_map;
        std::unordered_map<std::string, std::unordered_set<std::string>> skin_swaps;
        bool is_valid;

    	MorphData morph_data;

    };
}