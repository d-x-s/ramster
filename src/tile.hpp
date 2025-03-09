#include <iostream>
#include <map>
#include <vector>
#include <tinyECS/components.hpp>

static inline std::map<TEXTURE_ASSET_ID, std::vector<b2Vec2>> TILE_GEOMETRY =
    {
        {TEXTURE_ASSET_ID::SQUARE_TILE_1,
         {{0.0f, 0.0f},
          {127.0f, 0.0f},
          {127.0f, 127.0f},
          {0.0f, 127.0f}}},
        {TEXTURE_ASSET_ID::SQUARE_TILE_2,
         {{0.0f, 0.0f},
          {127.0f, 0.0f},
          {127.0f, 127.0f},
          {0.0f, 127.0f}}},
        {TEXTURE_ASSET_ID::SMOOTH_RAMP_BR,
         {
             {0.0f, 0.0f},
             {119.0f, 0.0f},
             {127.0f, 127.0f},
             {119.0f, 127.0f - 44.0f},
             {111.0f, 127.0f - 62.0f},
             {103.0f, 127.0f - 75.0f},
             {95.0f, 127.0f - 86.0f},
             {87.0f, 127.0f - 93.0f},
             {79.0f, 127.0f - 100.0f},
             {71.0f, 127.0f - 106.0f},
             {63.0f, 127.0f - 110.0f},
             {55.0f, 127.0f - 115.0f},
             {47.0f, 127.0f - 118.0f},
             {39.0f, 127.0f - 121.0f},
             {31.0f, 127.0f - 123.0f},
             {24.0f, 127.0f - 125.0f},
             {15.0f, 127.0f - 126.0f},
             {6.0f, 127.0f - 127.0f},
             {0.0f, 0.0f},
         }},
};
