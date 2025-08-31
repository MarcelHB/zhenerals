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
  auto models = w3d.parse();
  ASSERT_EQ(1, models.size());

  auto& model = models.front();
  EXPECT_EQ("OBJECT04", model->name);
  EXPECT_EQ("PTDOGWOD01_S", model->containerName);
  EXPECT_EQ(52, model->vertices.size());
  EXPECT_EQ(52, model->normals.size());
  EXPECT_EQ(32, model->triangles.size());
  EXPECT_EQ(1, model->shaderValues.size());
  EXPECT_EQ(1, model->materialPasses.size());
  EXPECT_EQ(1, model->materials.size());
  EXPECT_EQ("8 - Default", model->materials[0]);
  EXPECT_EQ(1, model->textures.size());
  EXPECT_EQ("PTDogwod01_S.tga", model->textures[0]);
  EXPECT_GE(model->boundingSphereRadius, 0);
}

TEST(W3DTest, parsingPtPalm02) {
  Config config;
  ResourceLoader w3dLoader {{"ZH_Generals/W3D.big"}, config.baseDir};

  auto fileStream = w3dLoader.getFileStream("art\\w3d\\ptpalm02.w3d");
  auto stream = fileStream->getStream();
  W3DFile w3d {stream};
  auto models = w3d.parse();
  ASSERT_EQ(2, models.size());
}

TEST(W3DTest, parsingPRG02) {
  Config config;
  ResourceLoader w3dLoader {{"W3DZH.big"}, config.baseDir};

  auto fileStream = w3dLoader.getFileStream("art\\w3d\\prg02.w3d");
  auto stream = fileStream->getStream();
  W3DFile w3d {stream};
  auto models = w3d.parse();
  ASSERT_EQ(1, models.size());
}

TEST(W3DTest, parsingSwing) {
  Config config;
  ResourceLoader w3dLoader {{"ZH_Generals/W3D.big"}, config.baseDir};

  auto fileStream = w3dLoader.getFileStream("art\\w3d\\pmswing.w3d");
  auto stream = fileStream->getStream();
  W3DFile w3d {stream};
  auto models = w3d.parse();
  ASSERT_EQ(3, models.size());
}

}
