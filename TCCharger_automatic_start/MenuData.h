#ifndef _sampleMenu_
#define _sampleMenu_
#include "MenuManager.h"
#include <avr/pgmspace.h>

/*
Generated using LCD Menu Builder at http://lcd-menu-bulder.cohesivecomputing.co.uk/.
*/

enum sampleMenuCommandId
{
  mnuCmdBack = 0,
  mnuCmdCurrentRoot,
  mnuCmdCurrentSub,
  mnuCmdSOCRoot,
  mnuCmdSOCSub,
  mnuCmdStartCAN,
  mnuCmdCancomm
};

PROGMEM const char sampleMenu_back[] = "Back";
PROGMEM const char sampleMenu_exit[] = "Exit";

PROGMEM const char sampleMenu_1_1[] = "Max Current";
PROGMEM const MenuItem sampleMenu_List_1[] = {{mnuCmdCurrentSub, sampleMenu_1_1}, {mnuCmdBack, sampleMenu_back}};

PROGMEM const char sampleMenu_2_1[] = "Target SOC";
PROGMEM const MenuItem sampleMenu_List_2[] = {{mnuCmdSOCSub, sampleMenu_2_1}, {mnuCmdBack, sampleMenu_back}};

PROGMEM const char sampleMenu_3_1[] = "Start CAN-BUS";
PROGMEM const MenuItem sampleMenu_List_3[] = {{mnuCmdCancomm, sampleMenu_3_1}, {mnuCmdBack, sampleMenu_back}};

PROGMEM const char sampleMenu_1[] = "1 Set Current";
PROGMEM const char sampleMenu_2[] = "2 Set SOC";
PROGMEM const char sampleMenu_3[] = "3 CAN-BUS comm";
PROGMEM const MenuItem sampleMenu_Root[] = {{mnuCmdCurrentRoot, sampleMenu_1, sampleMenu_List_1, menuCount(sampleMenu_List_1)}, {mnuCmdSOCRoot, sampleMenu_2, sampleMenu_List_2, menuCount(sampleMenu_List_2)}, {mnuCmdStartCAN, sampleMenu_3, sampleMenu_List_3, menuCount(sampleMenu_List_3)}, {mnuCmdBack, sampleMenu_exit}};

/*
case mnuCmdCurrentSub :
  break;
case mnuCmdSOCSub :
  break;
case mnuCmdCancomm :
  break;
*/

/*
<?xml version="1.0"?>
<RootMenu xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema">
  <Config IdPrefix="mnuCmd" VarPrefix="sampleMenu" UseNumbering="true" IncludeNumberHierarchy="false"
          MaxNameLen="15" MenuBackFirstItem="false" BackText="Back" ExitText="Exit" AvrProgMem="true"/>
  <MenuItems>  
    <Item Id="CurrentRoot" Name="Set Current">
   <MenuItems>
        <Item Id="CurrentSub" Name="Max Current"/>
          </MenuItems>
   </Item>
    <Item Id="SOCRoot" Name="Set SOC">
  <MenuItems>
        <Item Id="SOCSub" Name="Target SOC"/>
          </MenuItems>
    </Item>
 <Item Id="StartCAN" Name="CAN-BUS comm">
  <MenuItems>
        <Item Id="Cancomm" Name="Start CAN-BUS"/>
          </MenuItems>
    </Item>
  </MenuItems>
</RootMenu>
*/
#endif
