#include <fstream>

#include <gtest/gtest.h>

#include "../Config.h"
#include "../ResourceLoader.h"
#include "../gfx/TextureLoader.h"

namespace ZH {

// Steam version v1.05, EN

TEST(TexturesLoaderTest, lookup) {
  Config config;

  auto texturesResourceLoader =
    std::shared_ptr<ResourceLoader>(
      new ResourceLoader {{
          "TexturesZH.big"
        , "TerrainZH.big"
        , "MapsZH.big"
        , "EnglishZH.big"
        , "ZH_Generals/Textures.big"
        , "ZH_Generals/Terrain.big"
        , "ZH_Generals/Maps.big"
        , "ZH_Generals/English.big"
        }
        , config.baseDir
      }
    );

  GFX::TextureLoader textureLoader {*texturesResourceLoader};

  EXPECT_TRUE(textureLoader.getTexture("trwood1.tga"));
  EXPECT_TRUE(textureLoader.getTexture("PRGrey.tga"));
}

}
