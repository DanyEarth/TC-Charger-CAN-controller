#include "MenuManager.h"
#include <avr/pgmspace.h>

MenuManager::MenuManager(const MenuItem *root, unsigned char itemCount)
{
  menuRoot = root;
  rootMenuItemCount = itemCount;
  
  currentMenu = menuRoot;
  currentMenuItemCount = rootMenuItemCount;
  currentMenuItemIndexPos = 0;
  menuStackCount = 0;
}

// ---------------------------------------------------
void MenuManager::reset()
{
  currentMenu = menuRoot;
  currentMenuItemCount = rootMenuItemCount;
  currentMenuItemIndexPos = 0;
  menuStackCount = 0;
}

// ---------------------------------------------------
char *MenuManager::getParentItemName(char *buf)
{
  *buf = 0;
  
  MenuStackItem *msi = peekMenuItemOnStack();

  if (msi != 0)
  {
    strcpy_P(buf, (char *)pgm_read_word(&(msi->menu[msi->itemIndexPos].name)));
  }
  return buf;
}

// ---------------------------------------------------
char *MenuManager::getItemName(char *buf, unsigned char idx)
{
  return strcpy_P(buf, (char *)pgm_read_word(&(currentMenu[idx].name)));
}

// ---------------------------------------------------
unsigned char MenuManager::itemHasChildren(unsigned char idx)
{
  return pgm_read_byte(&(currentMenu[idx].childItemCount)) > 0;
}

// ---------------------------------------------------
char *MenuManager::getCurrentItemName(char *buf)
{
  return strcpy_P(buf, (char *)pgm_read_word(&(currentMenu[currentMenuItemIndexPos].name)));
}

// ---------------------------------------------------
const unsigned char MenuManager::getCurrentItemCmdId()
{
  return pgm_read_byte(&(currentMenu[currentMenuItemIndexPos].id));
}

// ---------------------------------------------------
const unsigned char MenuManager::getMenuItemCount()
{
  return currentMenuItemCount;
}

// ---------------------------------------------------
const MenuItem *MenuManager::getMenuItem()
{
  return currentMenu;
}
// ---------------------------------------------------
const unsigned char MenuManager::moveToItem(unsigned char itemNo)
{
  if (itemNo < (currentMenuItemCount))
  {
    currentMenuItemIndexPos = itemNo;
    return 1;
  }
  return 0;
}

// ---------------------------------------------------
const unsigned char MenuManager::getCurrentItemIndex()
{
  return currentMenuItemIndexPos;
}

// ---------------------------------------------------
unsigned char MenuManager::moveToNextItem()
{
  if (currentMenuItemIndexPos < (currentMenuItemCount -1))
  {
    currentMenuItemIndexPos++;
    return 1;
  }
  return 0;
}

// ---------------------------------------------------
unsigned char MenuManager::moveToPreviousItem()
{
  if (currentMenuItemIndexPos > 0)
  {
    currentMenuItemIndexPos--;
    return 1;
  }
  return 0;
}

// ---------------------------------------------------
const unsigned char MenuManager::currentItemHasChildren()
{
  return pgm_read_byte(&(currentMenu[currentMenuItemIndexPos].childItemCount)) > 0;
}

// ---------------------------------------------------
const unsigned char MenuManager::currentMenuHasParent()
{
  return stackHasItems();
}

// ---------------------------------------------------
void MenuManager::descendToChildMenu()
{
  if (currentItemHasChildren())
  {
    pushMenuOnStack(currentMenu, currentMenuItemIndexPos, currentMenuItemCount);
    
    currentMenuItemCount = pgm_read_byte(&(currentMenu[currentMenuItemIndexPos].childItemCount));
    currentMenu = (const MenuItem *) pgm_read_word(&(currentMenu[currentMenuItemIndexPos].childMenu));
    currentMenuItemIndexPos = 0;
  }
}

// ---------------------------------------------------
void MenuManager::ascendToParentMenu()
{
  if (currentMenuHasParent())
  {
    MenuStackItem *msi = popMenuItemFromStack();

    currentMenu = msi->menu;
    currentMenuItemCount = msi->itemCount;
    currentMenuItemIndexPos = msi->itemIndexPos;
  }
}


// ---------------------------------------------------
unsigned char MenuManager::handleNavigation(unsigned char (*getNavAction)(), void (*refreshDisplay)(unsigned char))
{
  unsigned char menuMode = MENU_REMAIN;
  unsigned char action = getNavAction();

  if (action == MENU_ITEM_SELECT || action == MENU_BACK)      // enter menu item, or sub menu, or ascend to parent, or cancel.
  {
    if (getCurrentItemCmdId() == 0 || action == MENU_BACK)
    {
      if (!currentMenuHasParent())
      {
        menuMode = MENU_EXIT;
        reset();
      }
      else
      {
        ascendToParentMenu();
        refreshDisplay(REFRESH_ASCEND);
      }
    }
    else if (currentItemHasChildren())
    {
      descendToChildMenu();
      refreshDisplay(REFRESH_DESCEND);
    }
    else
    {
      menuMode = MENU_INVOKE_ITEM;
    }
  }
  else if (action == MENU_ITEM_PREV) // move prev
  {
    if (moveToPreviousItem())
    {
      refreshDisplay(REFRESH_MOVE_PREV);
    }
  }
  else if (action == MENU_ITEM_NEXT) // move next
  {
    if (moveToNextItem())
    {
      refreshDisplay(REFRESH_MOVE_NEXT);
    }
  }

  return menuMode;
}


// ---------------------------------------------------
const unsigned char MenuManager::stackHasItems()
{
  return menuStackCount > 0;
}

// ---------------------------------------------------
void MenuManager::pushMenuOnStack(const MenuItem *menu, unsigned char indexPos, unsigned char itemCount)
{
  if (menuStackCount < (sizeof (menuStack) / sizeof(MenuStackItem)))
  {
    menuStack[menuStackCount].itemIndexPos = indexPos;
    menuStack[menuStackCount].itemCount = itemCount;
    menuStack[menuStackCount].menu = menu;
    menuStackCount++;
  }
}

// ---------------------------------------------------
MenuStackItem *MenuManager::popMenuItemFromStack()
{
  MenuStackItem *menuStackItem = 0;
  
  if (stackHasItems())
  {
    menuStackCount--;
    menuStackItem = &(menuStack[menuStackCount]);
  }

  return menuStackItem;
}


// ---------------------------------------------------
MenuStackItem *MenuManager::peekMenuItemOnStack()
{
  MenuStackItem *menuStackItem = 0;
  
  if (stackHasItems())
  {
    menuStackItem = &(menuStack[menuStackCount-1]);
  }
  return menuStackItem;
}
