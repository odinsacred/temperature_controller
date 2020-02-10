/*
* nextion_display.c
*
* Created: 10.02.2020 13:43:21
*  Author: odins
*/
#include <stdlib.h>
#include <string.h>
#include "nextion_display.h"

usart_t _usart = NULL;
void send_ffffff();

menu_t nextion_display_create_menu(const uint8_t page_count)
{
	menu_t menu;
	menu.page_count = page_count;
	menu.pages = calloc(page_count, sizeof(page_t));
	return menu;
}

void nextion_display_add_page(menu_t *menu, const uint8_t item_count)
{
	static uint8_t current=0;
	if(current < menu->page_count)
	{
		page_t page;
		page.item_count = item_count;
		page.items = calloc(item_count, sizeof(item_t));
		menu->pages[current] = page;
	}
	
}

uint8_t nextion_display_add_element(page_t *page, char *id){
	static uint8_t current = 0;
	if(current < page->item_count){
		page->items[current].string_id = id;
		current++;
		return 1;
	}
	return 0;
}

void nextion_display_init(usart_t *usart)
{
	_usart = *usart;
}

void nextion_display_refresh(menu_t *menu)
{
	char *value=NULL;
	for(uint8_t i = 0; i<menu->page_count; i++)
	{
		for(uint8_t j = 0;j<menu->pages[i].item_count; j++)
		{
			value = calloc(3, sizeof(uint8_t));
			if(menu->pages[i].items[j].value>=0)
				sprintf(value,"%d",menu->pages[i].items[j].value);	
			else
				sprintf(value,"%+d",menu->pages[i].items[j].value);			
			usart_write(_usart, menu->pages[i].items[j].string_id, strlen(menu->pages[i].items[j].string_id));
			usart_write(_usart, value, strlen(value));
			send_ffffff();
		}
	}
}

void send_ffffff()
{
	uint32_t var = 0xFFFFFF;
	usart_write(_usart,&var,3);
}