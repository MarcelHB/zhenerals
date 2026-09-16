// SPDX-License-Identifier: GPL-2.0

#ifndef H_DDS_FILE
#define H_DDS_FILE

#include <istream>
#include <memory>
#include <utility>

#include "common.h"
#include "gfx/HostTexture.h"

namespace ZH {

class DDSFile {
  public:
    DDSFile(std::istream&);

    std::shared_ptr<GFX::HostTexture> getTexture();
  private:
    enum class Encoding {
        UNKNOWN
      , DXT1
      , DXT5
    };

    std::istream& stream;

    static size_t calculateTotalSize(const Size&, uint32_t, Encoding);
};

}

#endif
