// SPDX-License-Identifier: GPL-2.0

#ifndef ZH_RENDERING_LINE
#define ZH_RENDERING_LINE

#include <vector>

#include "../Color.h"
#include "../vugl/vugl_context.h"

namespace ZH {

class LineRenderer {
  public:
    class Lines {
      friend LineRenderer;
      public:
        void setMatrix(const glm::mat4&);
        void writeMatrix(size_t frameIdx);

      private:
        bool broken = false;
        std::shared_ptr<Vugl::ElementBuffer> linesAndColors;
        std::shared_ptr<Vugl::DescriptorSet> descriptor;
        std::shared_ptr<Vugl::UniformBuffer> buffer;
        uint64_t frameIdxSet = 0;
        alignas(16) glm::mat4 mvp;
    };

    LineRenderer(Vugl::Context&);

    bool preparePipeline(Vugl::RenderPass&);
    void bindPipeline(Vugl::CommandBuffer&);

    Lines createLines(const std::vector<glm::vec3>&, const std::vector<Color>&);

    void renderLines(Lines&, Vugl::CommandBuffer&);

  private:
    Vugl::Context& vuglContext;
    std::shared_ptr<Vugl::Pipeline> pipeline;
};

};

#endif
