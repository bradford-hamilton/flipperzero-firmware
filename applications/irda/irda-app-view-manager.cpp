#include "furi.h"
#include "gui/modules/button_menu.h"
#include "gui/modules/dialog_ex.h"
#include "gui/modules/text_input.h"
#include "irda-app.hpp"
#include <callback-connector.h>

IrdaAppViewManager::IrdaAppViewManager() {
    event_queue = osMessageQueueNew(10, sizeof(IrdaAppEvent), NULL);

    view_dispatcher = view_dispatcher_alloc();
    auto callback = cbc::obtain_connector(this, &IrdaAppViewManager::previous_view_callback);

    gui = static_cast<Gui*>(furi_record_open("gui"));
    view_dispatcher_attach_to_gui(view_dispatcher, gui, ViewDispatcherTypeFullscreen);

    button_menu = button_menu_alloc();
    submenu = submenu_alloc();
    popup = popup_alloc();
    dialog_ex = dialog_ex_alloc();
    text_input = text_input_alloc();

    add_view(ViewType::ButtonMenu, button_menu_get_view(button_menu));
    add_view(ViewType::Submenu, submenu_get_view(submenu));
    add_view(ViewType::Popup, popup_get_view(popup));
    add_view(ViewType::DialogEx, dialog_ex_get_view(dialog_ex));
    add_view(ViewType::TextInput, text_input_get_view(text_input));

    view_set_previous_callback(button_menu_get_view(button_menu), callback);
    view_set_previous_callback(submenu_get_view(submenu), callback);
    view_set_previous_callback(popup_get_view(popup), callback);
    view_set_previous_callback(dialog_ex_get_view(dialog_ex), callback);
    view_set_previous_callback(text_input_get_view(text_input), callback);
}

IrdaAppViewManager::~IrdaAppViewManager() {
    view_dispatcher_remove_view(
        view_dispatcher, static_cast<uint32_t>(IrdaAppViewManager::ViewType::ButtonMenu));
    view_dispatcher_remove_view(
        view_dispatcher, static_cast<uint32_t>(IrdaAppViewManager::ViewType::TextInput));
    view_dispatcher_remove_view(
        view_dispatcher, static_cast<uint32_t>(IrdaAppViewManager::ViewType::DialogEx));
    view_dispatcher_remove_view(
        view_dispatcher, static_cast<uint32_t>(IrdaAppViewManager::ViewType::Submenu));
    view_dispatcher_remove_view(
        view_dispatcher, static_cast<uint32_t>(IrdaAppViewManager::ViewType::Popup));

    submenu_free(submenu);
    popup_free(popup);
    button_menu_free(button_menu);
    dialog_ex_free(dialog_ex);
    text_input_free(text_input);

    view_dispatcher_free(view_dispatcher);
    furi_record_close("gui");
    osMessageQueueDelete(event_queue);
}

void IrdaAppViewManager::switch_to(ViewType type) {
    view_dispatcher_switch_to_view(view_dispatcher, static_cast<uint32_t>(type));
}

TextInput* IrdaAppViewManager::get_text_input() {
    return text_input;
}

DialogEx* IrdaAppViewManager::get_dialog_ex() {
    return dialog_ex;
}

Submenu* IrdaAppViewManager::get_submenu() {
    return submenu;
}

Popup* IrdaAppViewManager::get_popup() {
    return popup;
}

ButtonMenu* IrdaAppViewManager::get_button_menu() {
    return button_menu;
}

osMessageQueueId_t IrdaAppViewManager::get_event_queue() {
    return event_queue;
}

void IrdaAppViewManager::receive_event(IrdaAppEvent* event) {
    if(osMessageQueueGet(event_queue, event, NULL, 100) != osOK) {
        event->type = IrdaAppEvent::Type::Tick;
    }
}

void IrdaAppViewManager::send_event(IrdaAppEvent* event) {
    osStatus_t result = osMessageQueuePut(event_queue, event, 0, 0);
    furi_check(result == osOK);
}

uint32_t IrdaAppViewManager::previous_view_callback(void* context) {
    if(event_queue != NULL) {
        IrdaAppEvent event;
        event.type = IrdaAppEvent::Type::Back;
        send_event(&event);
    }

    return VIEW_IGNORE;
}

void IrdaAppViewManager::add_view(ViewType view_type, View* view) {
    view_dispatcher_add_view(view_dispatcher, static_cast<uint32_t>(view_type), view);
}