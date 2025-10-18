#ifndef H_apps_server_apps_selection_header_H
#define H_apps_server_apps_selection_header_H
#ifdef __cplusplus
extern "C" {
#endif

#ifdef APP_SELECT_ECHO
#include <server_apps/echo_app.h>
#elif APP_SELECT_DEMO
#include <server_apps/echo_app.h>
#else
#error "Server - no app was selected"
#endif
#ifdef __cplusplus
}
#endif

#endif
