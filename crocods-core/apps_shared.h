#ifndef APPSSHARED_H
#define APPSSHARED_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "plateform.h"

#include "apps_keyboard.h"
#include "apps_tapeplayer.h"
#include "apps_menu.h"
#include "apps_autorun.h"
#include "apps_disk.h"
#include "apps_infopanel.h"
#include "apps_guestinfo.h"
#include "apps_debugger.h"
#include "apps_browser.h"
#include "apps_console.h"

// #include "apps_web.h"
// #include "apps_telnetd.h"

typedef struct {
    int count;
    int begin;
    int selected;
    int mouseSelected;
} AppsListCroco;

u16 appli_begin(core_crocods_t *core, u16 keys_pressed);

#ifdef __cplusplus
}
#endif

#endif // ifndef APPSSHARED_H
