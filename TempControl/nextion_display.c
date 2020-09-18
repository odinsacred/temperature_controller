/*
* nextion_display.c
*
* Created: 10.02.2020 13:43:21
*  Author: odins
*/
#define F_CPU 16000000
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>
#include <avr/io.h>
#include "nextion_display.h"

usart_t _usart = NULL;
void send_ffffff(void);
void send_item(item_t *item);

void nextion_display_init(usart_t *usart)
{
	_usart = *usart;
	usart_reset(_usart);
}

void nextion_display_create_row(display_row *row, const char *whole_id,const char *frac_id,const char *conn_id){
	row->whole.string_id = whole_id;
	row->frac.string_id = frac_id;
	row->conn.string_id = conn_id;
}

void nextion_display_refresh_row(display_row *row){
	send_item(&row->whole);
	send_item(&row->frac);
	send_item(&row->conn);
}

void send_item(item_t *item){
	char value[3];
	if(item->value>=0)
	sprintf(value,"%d",item->value);
	else
	sprintf(value,"%+d",item->value);
	usart_write(_usart, item->string_id, strlen(item->string_id));
	usart_write(_usart, value, strlen(value));
	send_ffffff();
}

void send_ffffff(void)
{
	uint32_t var = 0xFFFFFF;
	usart_write(_usart,&var,3);
}

//void nextion_display_create_menu(menu_t *menu)
//{
//menu->page_count = PAGES;
//}

//void nextion_display_create_page(page_t *page)
//{
//page->item_count = ITEMS;
//}

//void nextion_display_create_item(item_t *item, const char *id)
//{
//item->string_id = id;
//}

//uint8_t nextion_display_add_page(menu_t *menu, page_t *page)
//{
//static uint8_t current=0;
//
//if(current < menu->page_count)
//{
//menu->pages[current] = page;
//current++;
//return 1;
//}
//return 0;
//}

//uint8_t nextion_display_add_item(page_t *page, item_t *item){
//static uint8_t current = 0;
//if(current < page->item_count){
//page->items[current] = item;
//current++;
//return 1;
//}
//return 0;
//}

//void nextion_display_refresh(menu_t *menu)
//{
//char *value=NULL;
//for(uint8_t i = 0; i < menu->page_count; i++)
//{
//for(uint8_t j = 0; j<menu->pages[i]->item_count; j++)
//{
//value = calloc(3, sizeof(uint8_t));
//if(menu->pages[i]->items[j]->value>=0)
//sprintf(value,"%d",menu->pages[i]->items[j]->value);
//else
//sprintf(value,"%+d",menu->pages[i]->items[j]->value);
//usart_write(_usart, menu->pages[i]->items[j]->string_id, strlen(menu->pages[i]->items[j]->string_id));
////while(!usart_tx_check(_usart));
//usart_write(_usart, value, strlen(value));
////while(!usart_tx_check(_usart));
//send_ffffff();
//_delay_ms(100);
//free(value);
//}
//}
//}