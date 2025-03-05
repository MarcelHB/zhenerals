#ifndef H_MAPPED_IMAGE_INI
#define H_MAPPED_IMAGE_INI

#include <istream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "Dimensions.h"

namespace ZH {

struct INIImage {
  std::string texture;
  Size size;
  Point topLeft;
  Point bottomRight;

  Size effectiveSize() const {
    return {
        static_cast<Size::underlying_type>(bottomRight.x - topLeft.x)
      , static_cast<Size::underlying_type>(bottomRight.y - topLeft.y)
    };
  }
};

class MappedImageINI {
  public:
    using MappedImages = std::unordered_map<std::string, INIImage>;

    MappedImageINI(std::istream& instream);
    MappedImages parse();

  private:
    std::istream& stream;
    std::vector<char> readBuffer;

    void advanceStream();
    std::string consumeComment();
    std::string getToken();
    std::optional<uint16_t> parseInteger();
    std::optional<uint16_t> parseInteger(const std::string&);
    uint16_t parseIntegerFromCoord(const std::string&);
    bool parseCoords(INIImage& iniImage);
    bool parseMappedImage(MappedImages& mappedImages);
    bool parseTexture(INIImage& iniImage);
};

}

#endif
