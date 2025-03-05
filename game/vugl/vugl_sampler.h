#ifndef H_VUGL_SAMPLER
#define H_VUGL_SAMPLER

namespace Vugl {

class Sampler {
  public:
    virtual ~Sampler() {};

    virtual VkSampler getVkSampler () const = 0;
    virtual VkImage getVkImage () const = 0;
    virtual VkImageView getVkImageView () const = 0;
};

}

#endif
