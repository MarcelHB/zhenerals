// SPDX-License-Identifier: GPL-2.0

#ifndef H_DDS_FILE
#define H_DDS_FILE

#include <istream>
#include <memory>
#include <utility>

#include "../common.h"
#include "../gfx/HostTexture.h"

namespace ZH {

class DDSFile {
  public:
    DDSFile(std::istream&);

    std::shared_ptr<GFX::HostTexture> getTexture();
  private:
    std::istream& stream;

    std::vector<char> decodeDXT1(Size);
    // if bool is true, the image looks fully transparent
    std::pair<std::vector<char>, bool> decodeDXT5(Size);
};

}

#endif
