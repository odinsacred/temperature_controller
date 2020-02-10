/*
* nextion_display.h
*
* Created: 10.02.2020 13:43:38
*  Author: odins
*/


#ifndef NEXTION_DISPLAY_H_
#define NEXTION_DISPLAY_H_
#include "usart.h"

typedef struct _item_t
{
	int8_t value;
	char *string_id;
}item_t;


typedef struct _page_t
{
	item_t *items;
	uint8_t item_count;
}page_t;

typedef struct _menu_t
{
	page_t *pages;
	uint8_t page_count;
}menu_t;


menu_t nextion_display_create_menu(const uint8_t page_count);
void nextion_display_add_page(menu_t *menu, const uint8_t item_count);
uint8_t nextion_display_add_element(page_t *page, char *id);
//void nextion_display_create_item(const element_t cur_temp, const element_t high, const element_t low, const element_t param);
void nextion_display_init(usart_t *usart);
void nextion_display_refresh(menu_t *menu);
#endif /* NEXTION_DISPLAY_H_ */