#include "shared.h"

MENUITEM MainMenuItems[] = {};

MENU mnuMainMenu = {
	11
#if defined(NOROMLOADER)
		- 1
#endif

	,
	0, (MENUITEM *)&MainMenuItems};
