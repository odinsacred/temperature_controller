/*
* nextion_display.h
*
* Created: 10.02.2020 13:43:38
*  Author: odins
*/


#ifndef NEXTION_DISPLAY_H_
#define NEXTION_DISPLAY_H_
#include "usart.h"

enum
{
	INT_VALUE,
	BOOL_VALUE
}value_type;

typedef struct _item_t
{
	int8_t value;
	char *string_id;
}item_t;


typedef struct _page_t
{
	uint8_t item_count;
	item_t *items[9];
}page_t;

typedef struct _menu_t
{
	uint8_t page_count;
	page_t *pages;
}menu_t;


void nextion_display_create_menu(menu_t *menu, const uint8_t page_count);
void nextion_display_create_page(page_t *page, const uint8_t item_count);
void nextion_display_create_item(item_t *item, char *id);

uint8_t nextion_display_add_page(menu_t *menu, page_t page);
uint8_t nextion_display_add_element(page_t *page, item_t *item);
//void nextion_display_create_item(const element_t cur_temp, const element_t high, const element_t low, const element_t param);
void nextion_display_init(usart_t *usart);
void nextion_display_refresh(menu_t *menu);
#endif /* NEXTION_DISPLAY_H_ */