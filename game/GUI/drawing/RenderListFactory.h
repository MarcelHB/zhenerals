#ifndef H_GUI_DRAWING_RENDER_LIST_FACTORY
#define H_GUI_DRAWING_RENDER_LIST_FACTORY

#include <unordered_map>

#include <glm/glm.hpp>

#include "../../Color.h"
#include "../Component.h"
#include "../Button.h"
#include "../Label.h"
#include "../Window.h"
#include "../../gfx/TextureCache.h"
#include "../../gfx/font/FontManager.h"
#include "../../MurmurHash.h"
#include "../../vugl/vugl_context.h"

namespace ZH::GUI::Drawing {

enum class ImageIndex : size_t {
    DEFAULT = 0
  , LEFT = 0
  , LEFT_LOCKED = 1
  , MIDDLE_LOCKED = 3
  , RIGHT_LOCKED = 4
  , MIDDLE = 5
  , RIGHT = 6
};

enum class Pipeline {
    NONE
  , DEFAULT
  , TEXTURE
  , FONT
};

struct TextRenderConfig {
  enum class TextFlags {
      NOTHING = 0
    , CENTERED = 0x1
  };

  TextRenderConfig(const Component& component, const Font& font)
    : component(component)
    , font(font)
  {};

  const Component& component;
  const Font& font;
  BitField<TextFlags> flags;
  IntFlatBox bbox;
  Color tint = {255, 255, 255, 255};
};

struct UIMatrices {
  alignas(16) glm::mat4 mvp;
  alignas(16) glm::mat4 uv;
};

struct UIFontMatrix {
  alignas(16) glm::mat4 viewport;
};

struct TextureBundle {
  std::shared_ptr<Vugl::CombinedSampler> texture;
  std::shared_ptr<Vugl::UniformBuffer> matrices;
  std::shared_ptr<Vugl::DescriptorSet> descriptorSet;

  Point position;
  Size size;
};

using NineTextures = std::array<std::optional<TextureBundle>, 9>;

class FrameDisposable {
  public:
    uint8_t getMisses() const;

    void increaseMiss();
    void decreaseMiss();

  private:
    uint8_t misses = 0;
};

struct ResourceBundle : public FrameDisposable {
    NineTextures enabledTextures;
    NineTextures highlightTextures;
};

struct TextHolderBundle : public FrameDisposable {
    glm::mat4 matrix;
    std::shared_ptr<Vugl::DescriptorSet> descriptorSet;
    std::shared_ptr<Vugl::UniformBuffer> matrixBuffer;
};

struct TextCacheKey {
  TextCacheKey (const std::u16string&, uint8_t, bool);

  MurmurHash hash;
};

struct TextCacheKeyHash {
  size_t operator() (const TextCacheKey& key) const {
    return key.hash.value;
  }
};

struct TextCacheKeyCmp {
  bool operator() (const TextCacheKey& a, const TextCacheKey& b) const {
    return a.hash.value == b.hash.value;
  }
};

struct TextCacheEntry : public FrameDisposable {
    TextCacheEntry (Vugl::ElementBuffer&&, size_t);

    Vugl::ElementBuffer vertexBuffer;
    size_t numVertices;
};

struct RenderComponent {
  Point positionOffset;
  Point position;
  Size size;
};

class RenderListFactory {
  public:
    RenderListFactory(
        Vugl::Context&
      , Component&
      , GFX::TextureCache&
      , GFX::Font::FontManager&
    );
    RenderListFactory(const RenderListFactory&) = delete;

    std::shared_ptr<Vugl::CommandBuffer> createRenderList(size_t, Vugl::RenderPass&);
    void onQueueSubmitted();
  private:
    Vugl::Context& vuglContext;
    GFX::TextureCache& textureCache;
    GFX::Font::FontManager& fontManager;
    Component& rootComponent;

    glm::mat4 viewportMatrix;
    std::shared_ptr<Vugl::ElementBuffer> rectangleVertUV;
    std::shared_ptr<Vugl::Pipeline> uiPipeline;
    std::shared_ptr<Vugl::Pipeline> uiTexturePipeline;

    std::shared_ptr<Vugl::Pipeline> fontPipeline;
    std::unordered_map<
        ZH::GFX::Font::FontKey
      , std::shared_ptr<Vugl::CombinedSampler>
    > fontTextures;
    std::shared_ptr<Vugl::UniformBuffer> fontMatrix;

    Pipeline currentPipeline = Pipeline::NONE;

    std::unordered_map<uint64_t, ResourceBundle> resourceMap;
    std::unordered_map<uint64_t, TextHolderBundle> textHolderResourceMap;
    std::unordered_map<TextCacheKey, TextCacheEntry, TextCacheKeyHash, TextCacheKeyCmp> textBuffers;

    void createRectangularRenderList(
        const Component&
      , Vugl::CommandBuffer&
      , size_t
      , const RenderComponent&
      , const std::optional<INIImage>& texture
      , TextureBundle& textureBundle
    );

    void createRenderList(Component&, Vugl::CommandBuffer&, size_t, OptionalCRef<RenderComponent>);
    void createChildrenRenderList(Component&, Vugl::CommandBuffer&, size_t, OptionalCRef<RenderComponent>);
    void createButtonRenderList(Button&, Vugl::CommandBuffer&, size_t, OptionalCRef<RenderComponent>);
    void createLabelRenderList(Label&, Vugl::CommandBuffer&, size_t, OptionalCRef<RenderComponent>);
    void createWindowRenderList(Window&, Vugl::CommandBuffer&, size_t, OptionalCRef<RenderComponent>);

    std::string getDebugString(const std::u16string&);
    TextureBundle& getButtonTexture(
        const Button&
      , ResourceBundle&
      , size_t
    );

    ResourceBundle& findOrCreateResourceBundle(const Component&);
    TextHolderBundle& findOrCreateTextHolderBundle(const Component&);
    bool fillRectVertexData();
    bool prepareCommandBuffer(Vugl::CommandBuffer&, Vugl::RenderPass&);
    bool preparePipelines(Vugl::RenderPass&);
    void prepareTextHolderDescriptorSet(TextHolderBundle&, const std::u16string&, const TextRenderConfig& config);

    void renderText(const std::u16string&, const TextRenderConfig& config, Vugl::CommandBuffer&, size_t);
    OptionalRef<TextCacheEntry> createTextElements(const std::u16string&, const TextRenderConfig& config);

    bool switchToFont(uint8_t size, bool bold);
    void switchToPipeline(Pipeline, Vugl::CommandBuffer&);

    void writePositionMatrices(
        const RenderComponent&
      , const TextureBundle&
      , size_t
    );
};

}

#endif
