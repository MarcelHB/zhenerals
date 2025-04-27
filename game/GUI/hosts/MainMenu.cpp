#include "../../Logging.h"
#include "EventDefaults.h"
#include "MainMenu.h"

namespace ZH::GUI::Hosts {

MainMenu::MainMenu(
    WindowFactory& windowFactory
  , ComponentFactory& componentFactory
  , EventDispatcher& eventDispatcher
  , Size viewport
) : windowFactory(windowFactory)
  , componentFactory(componentFactory)
  , eventDispatcher(eventDispatcher)
  , viewport(viewport)
{}

bool MainMenu::init() {
  auto mainMenuWnd = windowFactory.getWND("window\\menus\\mainmenu.wnd");
  if (!mainMenuWnd) {
    return false;
  }

  rootWindow = componentFactory.createComponent(mainMenuWnd->getWindow(), mainMenuWnd->getLayout());
  if (!rootWindow) {
    ERROR_ZH("MainMenu", "Failed to set up window components.");
    return false;
  }

  assignComponentDefaultEvents(*rootWindow, eventDispatcher);

  rootWindow->setPositionOffset({
      viewport.x / 2 - rootWindow->getSize().x / 2
    , viewport.y / 2 - rootWindow->getSize().y / 2
  });

  campaignMenu = rootWindow->findByName("MainMenu.wnd:MapBorder");
  campaignMenu->setHidden(true);

  campaignSubMenu = rootWindow->findByName("MainMenu.wnd:MapBorder3");
  campaignSubMenu->setHidden(true);

  difficultyMenu = rootWindow->findByName("MainMenu.wnd:MapBorder4");
  difficultyMenu->setHidden(true);

  multiplayerMenu = rootWindow->findByName("MainMenu.wnd:MapBorder1");
  multiplayerMenu->setHidden(true);

  exitButton = rootWindow->findByName("MainMenu.wnd:ButtonExit");
  exitButton->addClickListener([this](const Component&) {
    eventDispatcher.fire(GameEvent::EXIT);
  });

  std::vector<std::string> buttonsToHide {
      "MainMenu.wnd:ButtonChinaLoadGame"
    , "MainMenu.wnd:ButtonChinaRecentSave"
    , "MainMenu.wnd:ButtonGLALoadGame"
    , "MainMenu.wnd:ButtonGLARecentSave"
    , "MainMenu.wnd:ButtonUSALoadGame"
    , "MainMenu.wnd:ButtonUSARecentSave"
  };

  rootWindow->setDrawImage(false);

  for (auto& buttonName : buttonsToHide) {
    auto button = rootWindow->findByName(buttonName);
    if (button) {
      button->setHidden(true);
    }
  }

  return true;
}

std::shared_ptr<Component> MainMenu::getRootComponent() {
  return rootWindow;
}

}
