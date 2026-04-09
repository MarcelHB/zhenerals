#include <fstream>

#include <gtest/gtest.h>

#include "../Config.h"
#include "../ResourceLoader.h"
#include "../formats/W3DFile.h"
#include "../gfx/Model.h"

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

TEST(W3DTest, multiModelExtremesAndBoundings) {
  Config config;
  ResourceLoader w3dLoader {{"ZH_Generals/W3D.big"}, config.baseDir};

  auto fileStream = w3dLoader.getFileStream("art\\w3d\\cbnhotel01.w3d");
  auto stream = fileStream->getStream();
  W3DFile w3d {stream};
  auto models = w3d.parse();
  ASSERT_EQ(2, models.size());

  auto model1 = Model::fromW3D(*models[0]);
  auto model2 = Model::fromW3D(*models[1]);

  auto extremes1 = model1.getExtremes();
  EXPECT_FLOAT_EQ(11.162782f, extremes1[0].x);
  EXPECT_FLOAT_EQ(-15.6218f,  extremes1[0].y);
  EXPECT_FLOAT_EQ(72.893188f, extremes1[0].z);
  EXPECT_FLOAT_EQ(11.162782f, extremes1[1].x);
  EXPECT_FLOAT_EQ(16.062727f, extremes1[1].y);
  EXPECT_FLOAT_EQ(82.354538f, extremes1[1].z);

  auto extremes2 = model2.getExtremes();
  EXPECT_FLOAT_EQ(-24.42733f,    extremes2[0].x);
  EXPECT_FLOAT_EQ(-18.679104f,   extremes2[0].y);
  EXPECT_FLOAT_EQ(-0.070003346f, extremes2[0].z);
  EXPECT_FLOAT_EQ(27.313433f, extremes2[1].x);
  EXPECT_FLOAT_EQ(18.679104f, extremes2[1].y);
  EXPECT_FLOAT_EQ(75.862373f, extremes2[1].z);
}

}
