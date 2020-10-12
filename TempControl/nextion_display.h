/*
* nextion_display.h
*
* Created: 10.02.2020 13:43:38
*  Author: odins
*/


#ifndef NEXTION_DISPLAY_H_
#define NEXTION_DISPLAY_H_
#include "usart.h"

#define PAGES 1
#define ITEMS 30
#define DEATHBAND 1

enum
{
	INT_VALUE,
	BOOL_VALUE
}value_type;

typedef struct _item_t
{
	int8_t prev_value;
	int8_t value;
	const char *string_id;
}item_t;

typedef struct{
	item_t whole;
	item_t frac;
	item_t conn;
}display_row;




void nextion_display_init(usart_t *usart);
void nextion_display_create_row(display_row *row, const char *whole_id,const char *frac_id,const char *conn_id);
void nextion_display_refresh_row(display_row *row);

//void nextion_display_refresh(menu_t *menu);
//void nextion_display_create_menu(menu_t *menu);
//void nextion_display_create_page(page_t *page);
//void nextion_display_create_item(item_t *item, const char *id);

//uint8_t nextion_display_add_page(menu_t *menu, page_t *page);
//uint8_t nextion_display_add_item(page_t *page, item_t *item);
//void nextion_display_create_item(const element_t cur_temp, const element_t high, const element_t low, const element_t param);

//typedef struct _page_t
//{
//uint8_t item_count;
//item_t *items[ITEMS];
//}page_t;

//typedef struct _menu_t
//{
//uint8_t page_count;
//page_t *pages[PAGES];
//}menu_t;

#endif /* NEXTION_DISPLAY_H_ */