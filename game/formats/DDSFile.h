#ifndef H_DDS_FILE
#define H_DDS_FILE

#include <istream>
#include <memory>

#include "../gfx/HostTexture.h"

namespace ZH {

class DDSFile {
  public:
    DDSFile(std::istream&);

    std::shared_ptr<GFX::HostTexture> getTexture();
  private:
    std::istream& stream;

    std::vector<char> decodeDXT1(Size);
    std::vector<char> decodeDXT5(Size);
};

}

#endif
