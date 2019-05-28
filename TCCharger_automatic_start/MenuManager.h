#ifndef MenuManager_h_
#define MenuManager_h_

#define MENU_MANAGER_1_0

typedef struct MenuItem {
  unsigned char  id;
  const char *name;
  const MenuItem *childMenu;
  unsigned char  childItemCount; 
} MenuItem;

#define menuCount(m)  (sizeof(m) / sizeof(MenuItem))

typedef struct MenuStackItem {
  unsigned char itemIndexPos;
  unsigned char itemCount;
  const MenuItem *menu;
} MenuStackItem;

enum MenuMode         // Values returned by MenuManager::handleNavigation().
{
  MENU_REMAIN = 0,    // user is navigating menu.
  MENU_EXIT,          // user exits menu navigation.
  MENU_INVOKE_ITEM    // user has selected menu item.
};

enum MenuNavAction    // Used by call back function to map user input to these values.
{
  MENU_ITEM_PREV = 1,
  MENU_ITEM_NEXT,
  MENU_ITEM_SELECT,
  MENU_BACK
};

enum DisplayRefreshMode   // Used by call back function to instruct display refresh according to menu navigation.
{
  REFRESH_MOVE_PREV,  // user has navigated to previous menu item.
  REFRESH_MOVE_NEXT,  // user has navigated to next menu item.
  REFRESH_ASCEND,     // user has navigated to parent menu.
  REFRESH_DESCEND     // user has navigated to child menu.
};

class MenuManager
{
  public:
    MenuManager (const MenuItem *root, unsigned char itemCount);

    // Resets the menu so it points to the first item of the root menu.
    void reset();

    // Gets the menu item name of the parent. Caller needs to first check if currentMenuHasParent().
    char *getParentItemName(char *buf);
    
    // Gets the menu item name, given item index position
    char *getItemName(char *buf, unsigned char idx);

    // Returns true if specified menu item has child menu items.
    unsigned char itemHasChildren(unsigned char idx);
    
    // gets the current menu item name.
    char *getCurrentItemName(char *buf);
    
    // Gets the current menu item command id.
    const unsigned char getCurrentItemCmdId();

    // Returns the number of items in the current menu.
    const unsigned char getMenuItemCount();
    // Gets the current Menu;
    const MenuItem *getMenuItem();
    
    // Moves to specified menu item. Returns true if successful.
    const unsigned char moveToItem(unsigned char itemIndex);
    // Gets the current menu item index.
    const unsigned char getCurrentItemIndex();
    
    // Moves to next menu item. Returns true if there was an item to move to.
    unsigned char moveToNextItem();
    // Moves to previous menu item. Returns true if there was an item to move to.
    unsigned char moveToPreviousItem();

    // Returns true if current menu item has child menu items.
    const unsigned char currentItemHasChildren();
    // Returns true if current menu has a parent menu.
    const unsigned char currentMenuHasParent();

    // Decends to current item's child menu.
    void descendToChildMenu();
    // Ascends to current item's parent menu.
    void ascendToParentMenu();


    // Handles menu navigation. Returns MenuMode enum value.
    unsigned char handleNavigation(
      unsigned char (*getNavAction)(),        // call back function that returns MenuNavAction enum value.
      void (*refreshDisplay)(unsigned char)   // call back function that updates the menu display
                                              // using paramter of DisplayRefreshMode enum value.
      );
  
  private:
    const MenuItem *menuRoot;
    unsigned char rootMenuItemCount;

    const MenuItem *currentMenu;
    unsigned char currentMenuItemCount;
    unsigned char currentMenuItemIndexPos;
    
    MenuStackItem menuStack[5];
    unsigned char menuStackCount;
    
    const unsigned char stackHasItems();
    void pushMenuOnStack(const MenuItem *menu, unsigned char indexPos, unsigned char itemCount);
    MenuStackItem *popMenuItemFromStack();
    MenuStackItem *peekMenuItemOnStack();
    
};

#endif
