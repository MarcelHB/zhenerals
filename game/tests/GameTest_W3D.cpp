#include <fstream>

#include <gtest/gtest.h>

#include "../Config.h"
#include "../ResourceLoader.h"
#include "../formats/W3DFile.h"

namespace ZH {

// Steam version v1.05, EN

TEST(W3DTest, parsing) {
  Config config;
  ResourceLoader w3dLoader {{"W3DZH.big"}, config.baseDir};

  auto fileStream = w3dLoader.getFileStream("art\\w3d\\ptdogwod01_s.w3d");
  auto stream = fileStream->getStream();
  W3DFile w3d {stream};
  auto model = w3d.parse();
  ASSERT_TRUE(model);

  EXPECT_EQ("OBJECT04", model->name);
  EXPECT_EQ("PTDOGWOD01_S", model->containerName);
  EXPECT_EQ(52, model->vertices.size());
  EXPECT_EQ(52, model->normals.size());
  EXPECT_EQ(32, model->triangles.size());
  EXPECT_EQ(1, model->shaderValues.size());
  EXPECT_EQ(1, model->materialPasses.size());
  EXPECT_EQ(1, model->materials.size());
  EXPECT_EQ(1, model->textures.size());
  EXPECT_GE(model->boundingSphereRadius, 0);
}

}
