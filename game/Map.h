#ifndef H_MAP
#define H_MAP

#include <cstdint>
#include <list>
#include <string>
#include <unordered_map>

#include "Color.h"
#include "Dimensions.h"
#include "formats/Dict.h"
#include "ResourceLoader.h"
#include "Script.h"
#include "inis/TerrainINI.h"

namespace ZH {

struct MapObject {
  glm::vec3 location;
  float angle = 0.0f;
  std::string name;
  Dict properties;
  Color color;

  bool light = false;
  bool waypoint = false;
  bool scorch = false;
};

struct TextureClass {
  uint32_t firstTile = 0;
  uint32_t numTiles = 0;
  uint32_t width = 0;
  std::string name;
  Point position;
};

struct BlendTileInfo {
  size_t blendIdx = 0;
  bool horizontal = 0;
  bool vertical = 0;
  bool rightDiagonal = 0;
  bool leftDiagonal = 0;
  uint8_t inverted = 0;
  bool longDiagonal = false;
  std::optional<uint32_t> customBlendEdgeClass;
};

struct CliffInfo {
  uint16_t tileIndex = 0;
  glm::vec4 u;
  glm::vec4 v;
  bool flip = false;
  bool mutant = false;
};

struct BuildInfo {
  std::string buildingName;
  std::string templateName;
  glm::vec3 position;
  float angle = 0.0f;
  bool prebuilt = false;
  uint32_t numRebuilt = 0;
  std::string script;
  uint32_t health = 0;
  bool whiner = false;
  bool unsellable = false;
  bool repairable = true;
};

struct SideInfo {
  Dict properties;
  std::vector<BuildInfo> buildInfo;
};

struct MapScriptParam {
  // EVAL plenty of type logic in parser
  Script::ParameterType type;
  glm::vec3 positionParam;
  int32_t intParam = 0;
  float floatParam = 0.0;
  std::string stringParam;
};

struct MapScriptAction {
  Script::ActionType type;
  // EVAL type, key type logic in parser
  uint32_t keyType;
  std::vector<MapScriptParam> params;
};

struct MapScriptCondition {
  Script::ConditionType type;
  // EVAL type, keyType, version logic in parser
  uint32_t keyType;
  uint16_t version;
  std::vector<MapScriptParam> params;
};

struct MapScriptOrCondition {
  std::list<MapScriptCondition> conditions;
};

struct MapScript {
  std::string name;
  std::string comment;
  std::string condition;
  std::string action;
  bool active = true;
  bool once = false;
  bool easy = false;
  bool normal = false;
  bool hard = true;
  bool subroutine = false;
  uint32_t delaySec = 0;

  std::list<MapScriptAction> actions;
  std::list<MapScriptAction> actionsOnFalse;
  // last parsed is front
  std::list<MapScriptOrCondition> orConditions;
};

struct MapScriptGroup {
  std::string name;
  bool active = true;
  bool subroutine = false;
};

struct PolygonTrigger {
  std::string name;
  std::string layerName;
  uint32_t id = 0;
  bool water = false;
  bool river = false;
  uint32_t riverStart = 0;
  std::vector<glm::ivec3> points;
};

struct TeamInfo {
  Dict properties;
};

struct Light {
  ColorRGBf ambient;
  ColorRGBf diffuse;
  glm::vec3 position;
};

using Lights = std::array<std::array<Light, 3>, 4>;

struct MapBuilder {
  std::unordered_map<uint32_t, std::string> chunkLabels;

  Dict worldDict;

  Size size;
  uint32_t borderSize = 0;
  std::vector<uint8_t> heightMap;
  std::vector<Point> boundaries;

  std::list<MapObject> objects;

  std::vector<uint16_t> tileIndices;
  std::vector<uint16_t> cliffInfoIndices;
  std::vector<uint16_t> blendTileIndices;
  std::vector<uint16_t> extraBlendTileIndices;
  std::vector<uint8_t> flipStates;
  std::vector<uint8_t> cliffStates;

  std::vector<TextureClass> textureClasses;
  std::vector<TextureClass> edgeTextureClasses;
  std::vector<BlendTileInfo> blendTileInfo;
  std::vector<CliffInfo> cliffInfo;

  std::vector<SideInfo> sides;
  std::vector<TeamInfo> teams;

  // last parsed is front
  std::list<MapScript> scripts;
  std::list<MapScriptGroup> scriptGroups;

  std::vector<PolygonTrigger> polygonTriggers;

  uint32_t timeOfDay = 0;
  Lights lights;
  Lights objectLights;
};

class Map {
  public:
    struct VertexData {
      glm::vec3 position;
      glm::vec3 normal;
      glm::vec2 uv;
      uint32_t textureIdx;
    };

    Map(MapBuilder&&);

    float getCenterHeight(size_t, size_t);
    float getHeight(size_t, size_t, uint8_t);
    const std::vector<uint8_t>& getHeightMap() const;
    const std::vector<std::string>& getTexturesIndex() const;
    const std::vector<VertexData>& getVertexData() const;
    const std::vector<uint32_t>& getVertexIndices() const;

    Size getSize() const;
  private:
    Size size;
    std::vector<uint8_t> heightMap;
    std::vector<std::string> texturesIndex;

    std::vector<VertexData> verticesAndNormals;
    std::vector<uint32_t> vertexIndices;

    void prepareTextureIndex(std::vector<TextureClass>&);
    void setVertexUV(
        VertexData&
      , const std::vector<TextureClass>&
      , uint16_t tileIdx
      , float xOffset = 0.0f
      , float yOffset = 0.0f
    );
    void tesselateHeightMap(
        const std::vector<TextureClass>&
      , const std::vector<uint16_t>& tileIndices
      , const std::vector<uint8_t>& flipStates
    );
};

}

#endif
