#ifndef H_GUI_HOSTS_MAIN_MENU
#define H_GUI_HOSTS_MAIN_MENU

#include "../../common.h"
#include "../ComponentFactory.h"
#include "../../EventDispatcher.h"
#include "../../WindowFactory.h"

namespace ZH::GUI::Hosts {

class MainMenu {
  public:
    MainMenu(
        WindowFactory& windowFactory
      , ComponentFactory& componentFactory
      , EventDispatcher& eventDispatcher
      , Size viewport
    );

    bool init();
    std::shared_ptr<Component> getRootComponent();
  private:
    WindowFactory& windowFactory;
    ComponentFactory& componentFactory;
    EventDispatcher& eventDispatcher;
    std::shared_ptr<Component> rootWindow;
    Size viewport;

    std::shared_ptr<Component> campaignMenu;
    std::shared_ptr<Component> campaignSubMenu;
    std::shared_ptr<Component> difficultyMenu;
    std::shared_ptr<Component> initialMenu;
    std::shared_ptr<Component> multiplayerMenu;

    std::shared_ptr<Component> exitButton;

    std::shared_ptr<Component> multiplayerButton;
};

}

#endif
