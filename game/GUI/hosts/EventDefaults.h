#ifndef H_GUI_HOSTS_EVENT_DEFAULTS
#define H_GUI_HOSTS_EVENT_DEFAULTS

#include "../../common.h"
#include "../../EventDispatcher.h"
#include "../Button.h"

namespace ZH::GUI::Hosts {

void assignComponentDefaultEvents(ZH::GUI::Component& component, EventDispatcher& dispatcher);

}

#endif
