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
  mnuCmdSettingsRoot,
  mnuCmdCurrentSub1,
  mnuCmdCurrentSub,
  mnuCmdSOCSub1,
  mnuCmdSOCSub,
  mnuCmdNumberSub1,
  mnuCmdNumberSub,
  mnuCmdStartCAN,
  mnuCmdCancomm
};

PROGMEM const char sampleMenu_back[] = "Back";
PROGMEM const char sampleMenu_exit[] = "Exit";

PROGMEM const char sampleMenu_1_1_1[] = "Max Current";
PROGMEM const MenuItem sampleMenu_List_1_1[] = {{mnuCmdCurrentSub, sampleMenu_1_1_1}, {mnuCmdBack, sampleMenu_back}};

PROGMEM const char sampleMenu_1_2_1[] = "Target SOC";
PROGMEM const MenuItem sampleMenu_List_1_2[] = {{mnuCmdSOCSub, sampleMenu_1_2_1}, {mnuCmdBack, sampleMenu_back}};

PROGMEM const char sampleMenu_1_3_1[] = "No. of chargers?";
PROGMEM const MenuItem sampleMenu_List_1_3[] = {{mnuCmdNumberSub, sampleMenu_1_3_1}, {mnuCmdBack, sampleMenu_back}};

PROGMEM const char sampleMenu_1_1[] = "Set Current";
PROGMEM const char sampleMenu_1_2[] = "Set SOC";
PROGMEM const char sampleMenu_1_3[] = "Set # charger";
PROGMEM const MenuItem sampleMenu_List_1[] = {{mnuCmdCurrentSub1, sampleMenu_1_1, sampleMenu_List_1_1, menuCount(sampleMenu_List_1_1)}, {mnuCmdSOCSub1, sampleMenu_1_2, sampleMenu_List_1_2, menuCount(sampleMenu_List_1_2)}, {mnuCmdNumberSub1, sampleMenu_1_3, sampleMenu_List_1_3, menuCount(sampleMenu_List_1_3)}, {mnuCmdBack, sampleMenu_back}};

PROGMEM const char sampleMenu_2_1[] = "Start CAN-BUS";
PROGMEM const MenuItem sampleMenu_List_2[] = {{mnuCmdCancomm, sampleMenu_2_1}, {mnuCmdBack, sampleMenu_back}};

PROGMEM const char sampleMenu_1[] = "1 Set Parameter";
PROGMEM const char sampleMenu_2[] = "2 CAN-BUS comm";
PROGMEM const MenuItem sampleMenu_Root[] = {{mnuCmdSettingsRoot, sampleMenu_1, sampleMenu_List_1, menuCount(sampleMenu_List_1)}, {mnuCmdStartCAN, sampleMenu_2, sampleMenu_List_2, menuCount(sampleMenu_List_2)}, {mnuCmdBack, sampleMenu_exit}};

/*
case mnuCmdCurrentSub :
  break;
case mnuCmdSOCSub :
  break;
case mnuCmdNumberSub :
  break;
case mnuCmdCancomm :
  break;
*/

/*
<RootMenu xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:xsd="http://www.w3.org/2001/XMLSchema">
  <Config IdPrefix="mnuCmd" VarPrefix="sampleMenu" UseNumbering="true" IncludeNumberHierarchy="false"
          MaxNameLen="15" MenuBackFirstItem="false" BackText="Back" ExitText="Exit" AvrProgMem="true"/>
  <MenuItems>  
    <Item Id="SettingsRoot" Name="Set Parameters">
   <MenuItems>
        <Item Id="CurrentSub1" Name="Set Current">
            <MenuItems>
                <Item Id="CurrentSub" Name="Max Current"/>
            </MenuItems>
        </Item>
        <Item Id="SOCSub1" Name="Set SOC">
            <MenuItems>
                <Item Id="SOCSub" Name="Target SOC"/>
            </MenuItems>
        </Item>
        <Item Id="NumberSub1" Name="Set # chargers">
            <MenuItems>
                <Item Id="NumberSub" Name="No. of chargers?"/>
            </MenuItems>
        </Item>
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
