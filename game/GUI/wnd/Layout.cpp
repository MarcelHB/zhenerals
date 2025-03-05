#include "Layout.h"

namespace ZH::GUI::WND {

OptionalCRef<std::string> Layout::getInitFunction() const {
  return optToCRef(initFunction);
}

OptionalCRef<std::string> Layout::getUpdateFunction() const {
  return optToCRef(updateFunction);
}

OptionalCRef<std::string> Layout::getShutdownFunction() const {
  return optToCRef(shutdownFunction);
}

}
