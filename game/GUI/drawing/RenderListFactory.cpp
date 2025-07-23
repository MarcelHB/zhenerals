#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>

#include "../../Logging.h"
#include "../Button.h"
#include "../Label.h"
#include "../Window.h"
#include "RenderListFactory.h"

#define IMAGE_INDEX(imageIndex) \
static_cast<typename std::underlying_type<ImageIndex>::type>(imageIndex)

namespace ZH::GUI::Drawing {

TextCacheKey::TextCacheKey(
    const std::u16string& text
  , uint8_t size
  , bool isBold)
{
  MurmurHash3_32 hasher;

  for (auto it = text.cbegin(); it != text.cend(); ++it) {
    hasher.feed(*it);
  }
  hasher.feed(size);
  hasher.feed(isBold);

  hash = hasher.getHash();
}

TextCacheEntry::TextCacheEntry (Vugl::ElementBuffer&& elementBuffer, size_t numVertices)
  : vertexBuffer(std::move(elementBuffer))
  , numVertices(numVertices)
{}

RenderListFactory::RenderListFactory(
    Vugl::Context& vuglContext
  , Component& component
  , GFX::TextureCache& textureCache
  , GFX::Font::FontManager& fontManager
) : vuglContext(vuglContext)
  , textureCache(textureCache)
  , rootComponent(component)
  , fontManager(fontManager)
{}

std::shared_ptr<Vugl::CommandBuffer> RenderListFactory::createRenderList(
    size_t frameIndex
  , Vugl::RenderPass& renderPass
) {
  TRACY(ZoneScoped);

  for (auto& pair : resourceMap) {
    pair.second.increaseMiss();
  }

  for (auto& pair : textHolderResourceMap) {
    pair.second.increaseMiss();
  }

  for (auto& pair : textBuffers) {
    pair.second.increaseMiss();
  }

  auto commandBuffer = vuglContext.createCommandBuffer(frameIndex, true);
  if (!prepareCommandBuffer(commandBuffer, renderPass)) {
    ERROR_ZH("RenderListFactory", "Could not create command buffer.");
    return {};
  }

  createRenderList(rootComponent, commandBuffer, frameIndex, std::nullopt);
  commandBuffer.closeRendering();

  for (auto it = resourceMap.begin(); it != resourceMap.end();) {
    if (it->second.getMisses() >= 2) {
      it = resourceMap.erase(it);
    } else {
      it++;
    }
  }

  for (auto it = textHolderResourceMap.begin(); it != textHolderResourceMap.end();) {
    if (it->second.getMisses() >= 2) {
      it = textHolderResourceMap.erase(it);
    } else {
      it++;
    }
  }

  for (auto it = textBuffers.begin(); it != textBuffers.end();) {
    if (it->second.getMisses() >= 2) {
      it = textBuffers.erase(it);
    } else {
      it++;
    }
  }

  return std::make_shared<Vugl::CommandBuffer>(std::move(commandBuffer));
}

void RenderListFactory::createRenderList(
    Component& component
  , Vugl::CommandBuffer& commandBuffer
  , size_t frameIndex
  , OptionalCRef<RenderComponent> parent
) {
  if (component.isHidden()) {
    return;
  }

  switch (component.getType()) {
    case GUI::WND::Window::Type::OVERLAY:
      createChildrenRenderList(component, commandBuffer, frameIndex, parent);
      break;
    case GUI::WND::Window::Type::PUSHBUTTON:
      createButtonRenderList(dynamic_cast<Button&>(component), commandBuffer, frameIndex, parent);
      break;
    case GUI::WND::Window::Type::STATICTEXT:
      createLabelRenderList(dynamic_cast<Label&>(component), commandBuffer, frameIndex, parent);
      break;
    case GUI::WND::Window::Type::USER:
      createWindowRenderList(dynamic_cast<Window&>(component), commandBuffer, frameIndex, parent);
      break;
    default:
      WARN_ZH("RenderListFactory", "Render request on unsupported component type");
      break;
  }

  component.setRedrawn();
}

void RenderListFactory::createButtonRenderList(
    Button& button
  , Vugl::CommandBuffer& commandBuffer
  , size_t frameIndex
  , OptionalCRef<RenderComponent> parent
) {
  OptionalCRef<Component::ImagesContainer> images;
  if (button.isHighlighted()) {
    images = std::make_optional(std::cref(button.getHighlightImages()));
  } else {
    images = std::make_optional(std::cref(button.getEnabledImages()));
  }

  RenderComponent renderComponent;
  renderComponent.position = button.getPosition();
  renderComponent.size = images->get()[IMAGE_INDEX(ImageIndex::LEFT)]->effectiveSize();
  if (parent) {
    renderComponent.position += parent->get().positionOffset;
  }

  auto& resources = findOrCreateResourceBundle(button);
  std::optional<INIImage> image;
  auto imageIndex = IMAGE_INDEX(button.isLocked() ? ImageIndex::LEFT_LOCKED : ImageIndex::LEFT);
  auto& texture1 = getButtonTexture(button, resources, imageIndex);
  image = images->get()[imageIndex];

  createRectangularRenderList(
      button
    , commandBuffer
    , frameIndex
    , renderComponent
    , image
    , texture1
  );

  renderComponent.position.x += renderComponent.size.x;
  renderComponent.size.x =
    button.getSize().x
      - images->get()[IMAGE_INDEX(ImageIndex::LEFT)]->effectiveSize().x
      - images->get()[IMAGE_INDEX(ImageIndex::RIGHT)]->effectiveSize().x;

  imageIndex = IMAGE_INDEX(button.isLocked() ? ImageIndex::MIDDLE_LOCKED : ImageIndex::MIDDLE);
  auto& texture2 = getButtonTexture(button, resources, imageIndex);
  image = images->get()[imageIndex];

  createRectangularRenderList(
      button
    , commandBuffer
    , frameIndex
    , renderComponent
    , image
    , texture2
  );

  renderComponent.position.x += renderComponent.size.x;
  renderComponent.size.x = images->get()[IMAGE_INDEX(ImageIndex::RIGHT)]->effectiveSize().x;

  imageIndex = IMAGE_INDEX(button.isLocked() ? ImageIndex::RIGHT_LOCKED : ImageIndex::RIGHT);
  auto& texture3 = getButtonTexture(button, resources, imageIndex);
  image = images->get()[imageIndex];

  createRectangularRenderList(
      button
    , commandBuffer
    , frameIndex
    , renderComponent
    , image
    , texture3
  );

  // EVAL default font?
  auto fontOpt = button.getFont();
  if (fontOpt && button.getText().size() > 0) {
    auto& font = fontOpt->get();
    switchToPipeline(Pipeline::FONT, commandBuffer);
    if (!switchToFont(font.size, font.bold)) {
      return;
    }

    TextRenderConfig renderConfig {button, font};
    renderConfig.flags = BitField<TextRenderConfig::TextFlags>(TextRenderConfig::TextFlags::CENTERED);
    renderConfig.bbox.size = button.getSize();
    renderConfig.bbox.position = button.getPosition() + parent->get().positionOffset;
    renderConfig.tint = Color::White;

    auto textColorOpt = button.getTextColor();
    if (textColorOpt) {
      if (button.isHighlighted() && textColorOpt->get().getHighlight()) {
        renderConfig.tint = *textColorOpt->get().getHighlight();
      } else if (textColorOpt->get().getEnabled()) {
        renderConfig.tint = *textColorOpt->get().getEnabled();
      }
    }

    renderText(button.getText(), renderConfig, commandBuffer, frameIndex);
  }
}

void RenderListFactory::createChildrenRenderList(
    Component& component
  , Vugl::CommandBuffer& commandBuffer
  , size_t frameIndex
  , OptionalCRef<RenderComponent> parent
) {
  for (auto& child : component.getChildren()) {
    createRenderList(*child, commandBuffer, frameIndex, parent);
  }
}

void RenderListFactory::createLabelRenderList(Label&, Vugl::CommandBuffer&, size_t frameIndex, OptionalCRef<RenderComponent>) {
}

void RenderListFactory::createRectangularRenderList(
    const Component& component
  , Vugl::CommandBuffer& commandBuffer
  , size_t frameIndex
  , const RenderComponent& renderComponent
  , const std::optional<INIImage>& texture
  , TextureBundle& textureBundle
) {
  if (vuglContext.isDebuggingAllowed()) {
    commandBuffer.beginDebugLabel(component.getName());
  }

  if (texture && !textureBundle.texture) {
    textureBundle.texture = textureCache.getTextureSampler(texture->texture);
    textureBundle.position = texture->topLeft;
    textureBundle.size = Size (
        static_cast<Size::value_type>(texture->bottomRight.x - texture->topLeft.x)
      , static_cast<Size::value_type>(texture->bottomRight.y - texture->topLeft.y)
    );
  }

  if (!textureBundle.descriptorSet) {
    textureBundle.matrices =
      std::make_shared<Vugl::UniformBuffer>(vuglContext.createUniformBuffer(sizeof(UIMatrices)));

    if (textureBundle.texture) {
      textureBundle.descriptorSet = std::make_shared<Vugl::DescriptorSet>(uiTexturePipeline->createDescriptorSet());
    } else {
      textureBundle.descriptorSet = std::make_shared<Vugl::DescriptorSet>(uiPipeline->createDescriptorSet());
    }

    textureBundle.descriptorSet->assignUniformBuffer(*textureBundle.matrices);
    if (textureBundle.texture) {
      textureBundle.descriptorSet->assignCombinedSampler(*textureBundle.texture);
      vuglContext.uploadResource(*textureBundle.texture);
    }

    textureBundle.descriptorSet->updateDevice();
  }

  if (textureBundle.texture) {
    switchToPipeline(Pipeline::TEXTURE, commandBuffer);
  } else {
    switchToPipeline(Pipeline::DEFAULT, commandBuffer);
  }

  commandBuffer.bindResource(*textureBundle.descriptorSet);
  commandBuffer.draw([this](VkCommandBuffer vkCommandBuffer, uint32_t) {
    vkCmdDraw(vkCommandBuffer, 6, 1, 0, 0);
    return VK_SUCCESS;
  });

  writePositionMatrices(renderComponent, textureBundle, frameIndex);

  if (vuglContext.isDebuggingAllowed()) {
    commandBuffer.endDebugLabel();
  }
}

void RenderListFactory::createWindowRenderList(
    Window& window
  , Vugl::CommandBuffer& commandBuffer
  , size_t frameIndex
  , OptionalCRef<RenderComponent> parent
) {
  RenderComponent renderComponent;
  renderComponent.position = window.getPosition() + window.getPositionOffset();
  renderComponent.size = window.getSize();
  renderComponent.positionOffset = window.getPositionOffset();
  if (parent) {
    renderComponent.position = window.getPosition() + parent->get().positionOffset;
    renderComponent.positionOffset = parent->get().positionOffset;
  }

  auto& resources = findOrCreateResourceBundle(window);

  if (window.isDrawImage()) {
    auto imageIndex = IMAGE_INDEX(ImageIndex::DEFAULT);
    auto& image = window.getEnabledImages()[imageIndex];

    if (!resources.enabledTextures[imageIndex]) {
      resources.enabledTextures[imageIndex] = std::make_optional<TextureBundle>();
    }

    createRectangularRenderList(
        window
      , commandBuffer
      , frameIndex
      , renderComponent
      , image
      , *resources.enabledTextures[imageIndex]
    );
  }

  auto optRenderComponent = std::make_optional(std::cref(renderComponent));
  createChildrenRenderList(
      window
    , commandBuffer
    , frameIndex
    , optRenderComponent
  );
}

ResourceBundle& RenderListFactory::findOrCreateResourceBundle(const Component& component) {
  auto resource = resourceMap.find(component.getID());
  if (resource == resourceMap.cend()) {
    return resourceMap.emplace(component.getID(), ResourceBundle {}).first->second;
  } else {
    resource->second.decreaseMiss();
    return resource->second;
  }
}

TextHolderBundle& RenderListFactory::findOrCreateTextHolderBundle(const Component& component) {
  auto resource = textHolderResourceMap.find(component.getID());
  if (resource == textHolderResourceMap.cend()) {
    return textHolderResourceMap.emplace(component.getID(), TextHolderBundle {}).first->second;
  } else {
    resource->second.decreaseMiss();
    return resource->second;
  }
}

bool RenderListFactory::fillRectVertexData() {
  // two triangles: |\\|
  std::vector<float> data = {
    0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
    1.0f, 1.0f, 0.0f,  1.0f, 1.0f,

    0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
    1.0f, 1.0f, 0.0f,  1.0f, 1.0f
  };

  rectangleVertUV->writeData(data, std::vector<uint16_t>{});
  if (rectangleVertUV->getLastResult() != VK_SUCCESS) {
    return false;
  }

  if (!vuglContext.uploadResource(*rectangleVertUV)) {
    return false;
  }

  return true;
}

bool RenderListFactory::prepareCommandBuffer(
    Vugl::CommandBuffer& commandBuffer
  , Vugl::RenderPass& renderPass
) {
  if (!uiPipeline) {
    if (!preparePipelines(renderPass)) {
      return false;
    }
  }

  commandBuffer.beginRendering(renderPass, {});
  switchToPipeline(Pipeline::DEFAULT, commandBuffer);

  return true;
}

bool RenderListFactory::preparePipelines(Vugl::RenderPass& renderPass) {
  Vugl::PipelineSetup pipelineSetup {vuglContext.getViewport(), vuglContext.getVkSamplingFlag()};
  pipelineSetup.vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  pipelineSetup.vkPipelineDepthStencilCreateInfo.depthTestEnable = VK_FALSE;
  pipelineSetup.vkPipelineColorBlendAttachmentState.blendEnable = VK_TRUE;
  pipelineSetup.vkPipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  pipelineSetup.vkPipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  pipelineSetup.setVSCode(readFile("shaders/ui.vert.spv"));
  pipelineSetup.setFSCode(readFile("shaders/ui.frag.spv"));
  pipelineSetup.reserveUniformBuffer(VK_SHADER_STAGE_VERTEX_BIT);
  pipelineSetup.addVertexInput(VK_FORMAT_R32G32B32_SFLOAT, 0, 12, 0);
  pipelineSetup.addVertexInput(VK_FORMAT_R32G32_SFLOAT, 12, 8, 0);

  uiPipeline =
    std::make_unique<Vugl::Pipeline>(vuglContext.createPipeline(pipelineSetup, renderPass.getVkRenderPass()));
  if (uiPipeline->getLastResult() != VK_SUCCESS) {
    return false;
  }

  Vugl::PipelineSetup texturePipelineSetup {vuglContext.getViewport(), vuglContext.getVkSamplingFlag()};
  texturePipelineSetup.vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  texturePipelineSetup.vkPipelineDepthStencilCreateInfo.depthTestEnable = VK_FALSE;
  texturePipelineSetup.vkPipelineColorBlendAttachmentState.blendEnable = VK_TRUE;
  texturePipelineSetup.vkPipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  texturePipelineSetup.vkPipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  texturePipelineSetup.setVSCode(readFile("shaders/ui.vert.spv"));
  texturePipelineSetup.setFSCode(readFile("shaders/ui_texture.frag.spv"));
  texturePipelineSetup.reserveUniformBuffer(VK_SHADER_STAGE_VERTEX_BIT);
  texturePipelineSetup.reserveCombinedSampler(VK_SHADER_STAGE_FRAGMENT_BIT);
  texturePipelineSetup.addVertexInput(VK_FORMAT_R32G32B32_SFLOAT, 0, 12, 0);
  texturePipelineSetup.addVertexInput(VK_FORMAT_R32G32_SFLOAT, 12, 8, 0);

  uiTexturePipeline =
    std::make_unique<Vugl::Pipeline>(vuglContext.createPipeline(texturePipelineSetup, renderPass.getVkRenderPass()));
  if (uiTexturePipeline->getLastResult() != VK_SUCCESS) {
    return false;
  }

  rectangleVertUV =
    std::make_unique<Vugl::ElementBuffer>(vuglContext.createElementBuffer(0));
  fillRectVertexData();

  auto vp = vuglContext.getViewport();
  viewportMatrix =
    glm::translate(glm::mat4 {1.0f}, glm::vec3 {-1.0f, -1.0f, 0.0f})
      * glm::scale(glm::mat4 {1.0f}, glm::vec3 {1.0f / (0.5f * vp.width), 1.0f / (0.5f * vp.height), 1.0f});

  Vugl::PipelineSetup fontPipelineSetup {vuglContext.getViewport(), vuglContext.getVkSamplingFlag()};
  fontPipelineSetup.vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  fontPipelineSetup.vkPipelineDepthStencilCreateInfo.depthTestEnable = VK_FALSE;
  fontPipelineSetup.vkPipelineColorBlendAttachmentState.blendEnable = VK_TRUE;
  fontPipelineSetup.vkPipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  fontPipelineSetup.vkPipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  fontPipelineSetup.setVSCode(readFile("shaders/ui_font.vert.spv"));
  fontPipelineSetup.setFSCode(readFile("shaders/ui_font.frag.spv"));
  fontPipelineSetup.reserveUniformBuffer(VK_SHADER_STAGE_VERTEX_BIT);
  fontPipelineSetup.reserveCombinedSampler(VK_SHADER_STAGE_FRAGMENT_BIT);
  fontPipelineSetup.addVertexInput(VK_FORMAT_R32G32_SFLOAT, 0, 8, 0);
  fontPipelineSetup.addVertexInput(VK_FORMAT_R32G32_SFLOAT, 8, 8, 0);
  fontPipelineSetup.enablePushConstants(VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::vec4));

  fontPipeline =
    std::make_unique<Vugl::Pipeline>(vuglContext.createPipeline(fontPipelineSetup, renderPass.getVkRenderPass()));
  if (fontPipeline->getLastResult() != VK_SUCCESS) {
    return false;
  }

  return true;
}

std::string RenderListFactory::getDebugString(const std::u16string& text) {
  std::string string {"Text: "};

  for (auto it = text.cbegin(); it != text.cend(); ++it) {
    if (*it >= 32 && *it < 127) {
      auto c = static_cast<char>(*it);
      string.append(&c, 1);
    }
  }

  return string;
}

TextureBundle& RenderListFactory::getButtonTexture(
    const Button& button
  , ResourceBundle& resources
  , size_t locationIndex
) {
  OptionalRef<NineTextures> nineTextures;
  if (button.isHighlighted()) {
    nineTextures = std::make_optional(std::ref(resources.highlightTextures));
  } else {
    nineTextures = std::make_optional(std::ref(resources.enabledTextures));
  }

  if (!nineTextures->get()[locationIndex]) {
    nineTextures->get()[locationIndex] = std::make_optional<TextureBundle>();
  }

  return *nineTextures->get()[locationIndex];
}

void RenderListFactory::prepareTextHolderDescriptorSet(
    TextHolderBundle& bundle
  , const std::u16string& text
  , const TextRenderConfig& config
) {
  auto atlas = fontManager.getFont(config.font.size, config.font.bold);
  size_t totalWidth = 0;
  Point::value_type maxAbove0 = 0;

  for (auto it = text.cbegin(); it != text.cend(); ++it) {
    auto glyph = atlas->getGlyph(*it);
    if (glyph) {
      totalWidth += glyph->get().size.x;
      maxAbove0 = std::max(maxAbove0, glyph->get().above0);
    }
  }

  if (totalWidth > config.bbox.size.x) {
    totalWidth = config.bbox.size.x;
  }

  Point startPos = config.bbox.position + Point {2, 2};
  if (config.flags | TextRenderConfig::TextFlags::CENTERED) {
    startPos = config.bbox.position;
    startPos.x += config.bbox.size.x / 2 - totalWidth / 2;
    startPos.y += config.bbox.size.y / 2 - maxAbove0 / 2;
  }

  auto texture = textureCache.getFontTextureSampler(config.font.size, config.font.bold);
  if (!texture) {
    return;
  }

  auto fontMatrix =
    std::make_shared<Vugl::UniformBuffer>(vuglContext.createUniformBuffer(sizeof(UIFontMatrix)));

  auto descriptorSet = std::make_shared<Vugl::DescriptorSet>(fontPipeline->createDescriptorSet());
  descriptorSet->assignUniformBuffer(*fontMatrix);
  descriptorSet->assignCombinedSampler(*texture);
  descriptorSet->updateDevice();

  bundle.descriptorSet = std::move(descriptorSet);
  bundle.matrixBuffer = std::move(fontMatrix);

  auto modelMatrix = glm::translate(glm::mat4 {1.0f}, glm::vec3 { startPos.x, startPos.y, 0.0f});

  bundle.matrix = viewportMatrix * modelMatrix;
}

void RenderListFactory::renderText(
    const std::u16string& text
  , const TextRenderConfig& config
  , Vugl::CommandBuffer& commandBuffer
  , size_t frameIndex
) {
  TRACY(ZoneScoped);
  if (vuglContext.isDebuggingAllowed()) {
    commandBuffer.beginDebugLabel(getDebugString(text));
  }

  auto& textHolderBundle = findOrCreateTextHolderBundle(config.component);
  if (!textHolderBundle.descriptorSet) {
    prepareTextHolderDescriptorSet(textHolderBundle, text, config);
  }
  if (!textHolderBundle.descriptorSet) {
    return;
  }

  textHolderBundle.matrixBuffer->writeData(textHolderBundle.matrix, frameIndex);
  commandBuffer.bindResource(*textHolderBundle.descriptorSet);

  TextCacheKey key {text, config.font.size, config.font.bold};
  OptionalRef<TextCacheEntry> textCacheEntry;
  auto lookup = textBuffers.find(key);
  if (lookup == textBuffers.cend()) {
    textCacheEntry = createTextElements(text, config);
  } else {
    auto& entry = lookup->second;
    entry.decreaseMiss();

    textCacheEntry = std::make_optional(std::ref(entry));
  }

  auto& elementBuffer = textCacheEntry->get().vertexBuffer;
  auto numVertices = textCacheEntry->get().numVertices;

  commandBuffer.bindResource(elementBuffer);
  commandBuffer.draw([this, numVertices, &config](VkCommandBuffer vkCommandBuffer, uint32_t) {
    glm::vec4 tint = config.tint;
    vkCmdPushConstants(
        vkCommandBuffer
      , fontPipeline->getVkPipelineLayout()
      , VK_SHADER_STAGE_FRAGMENT_BIT
      , 0
      , sizeof(glm::vec4)
      , &tint
    );
    vkCmdDraw(vkCommandBuffer, numVertices / 4, 1, 0, 0);

    return VK_SUCCESS;
  });

  if (vuglContext.isDebuggingAllowed()) {
    commandBuffer.endDebugLabel();
  }
}

OptionalRef<TextCacheEntry> RenderListFactory::createTextElements(
    const std::u16string& text
  , const TextRenderConfig& config
) {
  auto atlas = fontManager.getFont(config.font.size, config.font.bold);
  auto texture = textureCache.getFontTextureSampler(config.font.size, config.font.bold);

  size_t height = 0;
  size_t numGlyphs = 0;

  for (auto it = text.cbegin(); it != text.cend(); ++it) {
    auto glyph = atlas->getGlyph(*it);
    if (glyph) {
      numGlyphs += 1;
      height = glyph->get().size.y; // same height everything r/n
    }
  }

  if (height > config.bbox.size.y) {
    height = config.bbox.size.y;
  }

  std::vector<float> data;
  auto numVertices = numGlyphs * 24;
  data.resize(numVertices);

  auto extent = texture->getExtent();
  size_t i = 0;
  size_t xOffset = 0;
  for (auto it = text.cbegin(); it != text.cend(); ++it) {
    auto glyphOpt = atlas->getGlyph(*it);
    if (!glyphOpt) {
      continue;
    }

    auto& glyph = glyphOpt->get();
    // top-left |\ /
    data[i * 24] = xOffset;
    data[i * 24 + 1] = 0;
    data[i * 24 + 2] = glyph.position.x / (extent.width * 1.0f);
    data[i * 24 + 3] = glyph.position.y / (extent.height * 1.0f);
    // bottom-left
    data[i * 24 + 4] = xOffset;
    data[i * 24 + 5] = height;
    data[i * 24 + 6] = glyph.position.x / (extent.width * 1.0f);
    data[i * 24 + 7] = (glyph.position.y + height) / (extent.height * 1.0f);
    // bottom-right
    data[i * 24 + 8] = xOffset + glyph.size.x;
    data[i * 24 + 9] = height;
    data[i * 24 + 10] = (glyph.position.x + glyph.size.x) / (extent.width * 1.0f);
    data[i * 24 + 11] = (glyph.position.y + height) / (extent.height * 1.0f);

    // top-left \|
    data[i * 24 + 12] = xOffset;
    data[i * 24 + 13] = 0;
    data[i * 24 + 14] = glyph.position.x / (extent.width * 1.0f);
    data[i * 24 + 15] = glyph.position.y / (extent.height * 1.0f);
    // top-right
    data[i * 24 + 16] = xOffset + glyph.size.x;
    data[i * 24 + 17] = 0;
    data[i * 24 + 18] = (glyph.position.x + glyph.size.x) / (extent.width * 1.0f);
    data[i * 24 + 19] = glyph.position.y / (extent.height * 1.0f);
    // bottom-right
    data[i * 24 + 20] = xOffset + glyph.size.x;
    data[i * 24 + 21] = height;
    data[i * 24 + 22] = (glyph.position.x + glyph.size.x) / (extent.width * 1.0f);
    data[i * 24 + 23] = (glyph.position.y + height) / (extent.height * 1.0f);

    xOffset += glyph.size.x;
    ++i;
  }

  TextCacheKey key {text, config.font.size, config.font.bold};
  auto elementBuffer = vuglContext.createElementBuffer(0);
  elementBuffer.writeData(data, std::vector<uint16_t>{});

  if (elementBuffer.getLastResult() != VK_SUCCESS) {
    return {};
  }

  vuglContext.uploadResource(elementBuffer);

  auto result = textBuffers.emplace(std::move(key), TextCacheEntry {std::move(elementBuffer), numVertices});

  return std::make_optional(std::ref(result.first->second));
}

bool RenderListFactory::switchToFont(uint8_t size , bool bold) {
  if (currentPipeline != Pipeline::FONT) {
    return false;
  }

  ZH::GFX::Font::FontKey key {size, bold};

  auto lookup = fontTextures.find(key);
  if (lookup != fontTextures.cend()) {
    return true;
  } else {
    auto texture = textureCache.getFontTextureSampler(size, bold);
    if (!texture) {
      return false;
    }

    vuglContext.uploadResource(*texture);
    fontTextures.emplace(key, std::move(texture));
  }

  return true;
}

void RenderListFactory::switchToPipeline(Pipeline nextPipeline, Vugl::CommandBuffer& commandBuffer) {
  if (nextPipeline == currentPipeline) {
    return;
  }

  if (nextPipeline == Pipeline::TEXTURE) {
    commandBuffer.bindResource(*uiTexturePipeline);
    commandBuffer.bindResource(*rectangleVertUV);
  } else if (nextPipeline == Pipeline::FONT) {
    commandBuffer.bindResource(*fontPipeline);
  } else {
    commandBuffer.bindResource(*uiPipeline);
    commandBuffer.bindResource(*rectangleVertUV);
  }

  currentPipeline = nextPipeline;
}

void RenderListFactory::writePositionMatrices(
    const RenderComponent& component
  , const TextureBundle& bundle
  , size_t frameIndex
) {
  UIMatrices matrices;

  auto size = component.size;
  auto position = component.position;

  if (bundle.texture) {
    auto extent = bundle.texture->getExtent();

    matrices.uv =
      glm::translate(glm::mat4 {1.0f}, glm::vec3 {
          bundle.position.x / (extent.width * 1.0f)
        , bundle.position.y / (extent.height * 1.0f)
        , 1.0f
      })
      * glm::scale(glm::mat4 {1.0f}, glm::vec3 {
          bundle.size.x / (extent.width * 1.0f)
        , bundle.size.y / (extent.height * 1.0f)
        , 1.0f
      });
  }

  auto modelMatrix =
    glm::translate(glm::mat4 {1.0f}, glm::vec3 { position.x, position.y, 0.0f})
      * glm::scale(glm::mat4 {1.0f}, glm::vec3 { size.x, size.y, 1.0f});

  matrices.mvp = viewportMatrix * modelMatrix;

  bundle.matrices->writeData(matrices, frameIndex);
}

void RenderListFactory::onQueueSubmitted() {
}

}
