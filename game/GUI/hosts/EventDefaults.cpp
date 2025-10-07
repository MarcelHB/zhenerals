// SPDX-License-Identifier: GPL-2.0

#include "EventDefaults.h"

namespace ZH::GUI::Hosts {

static void assignButtonClickSounds(ZH::GUI::Component& button, EventDispatcher& dispatcher) {
  button.addClickListener([&dispatcher](const Component&) {
    auto sfxName = new std::string {"GUIClick"};
    dispatcher.fire(ZH::GameEvent::PLAY_SFX, sfxName);
  });
}

void assignComponentDefaultEvents(ZH::GUI::Component& component, EventDispatcher& dispatcher) {
  switch (component.getType()) {
    case ZH::GUI::WND::Window::Type::PUSHBUTTON:
      assignButtonClickSounds(component, dispatcher);
      break;
    default: /* OK */
      break;
  }

  for (auto& child : component.getChildren()) {
    assignComponentDefaultEvents(*child, dispatcher);
  }
}

}
