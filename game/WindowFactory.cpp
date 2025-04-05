#include "Logging.h"
#include "WindowFactory.h"
#include "formats/WNDFile.h"

#include "GUI/wnd/WindowAndLayout.h"

namespace ZH {

WindowFactory::WindowFactory(const Config& config)
  : loader {{
      "PatchWindow.big",
      "WindowZH.big",
      "ZH_Generals/Window.big"
    }, config.baseDir}
{}

std::shared_ptr<GUI::WND::WindowAndLayout> WindowFactory::getWND(const std::string& key) {
  auto stream = loader.getFileStream(key);
  if (!stream) {
    WARN_ZH("WindowFactory", "Could not find {}", key);
    return {};
  }

  auto memoryStream = stream->getStream();
  WNDFile wnd {memoryStream};

  return wnd.parse();
}

}
