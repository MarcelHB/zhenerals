#ifndef H_GFX_TEXTURE_LOOKUP
#define H_GFX_TEXTURE_LOOKUP

#include "../common.h"
#include "../ResourceLoader.h"
#include "../inis/MappedImageINI.h"

namespace ZH::GFX {

// This one extracts texture data (key name, dimensions) from the INIs
class TextureLookup {
  public:
    TextureLookup(ResourceLoader&);

    bool load();
    OptionalCRef<INIImage> getTexture(const std::string&);

  private:
    ResourceLoader& iniLoader;
    MappedImageINI::MappedImages textures;
};

}

#endif
