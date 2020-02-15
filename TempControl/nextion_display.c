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
void send_ffffff();

void nextion_display_create_menu(menu_t *menu, const uint8_t page_count)
{	
	menu->page_count = page_count;
	menu->pages = calloc(page_count, sizeof(page_t));
}

void nextion_display_create_page(page_t *page, const uint8_t item_count)
{
	page->item_count = item_count;
	
	//*page->items = calloc(item_count, sizeof(item_t*));
	for(uint8_t i=0; i<item_count;i++)
	{
		page->items[i] = (item_t*)malloc(sizeof(item_t*));//calloc(item_count, sizeof(item_t));
	}
}

void nextion_display_create_item(item_t *item, char *id)
{
	item->string_id = id;
}

uint8_t nextion_display_add_page(menu_t *menu, page_t page)
{
	static uint8_t current=0;
	
	if(current < menu->page_count)
	{
		menu->pages[current] = page;
		current++;
		return 1;
	}
	return 0;
}

uint8_t nextion_display_add_element(page_t *page, item_t *item){
	static uint8_t current = 0;
	if(current < page->item_count){
		page->items[current] = item;
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
	for(uint8_t i = 0; i < menu->page_count; i++)
	{
		for(uint8_t j = 0; j<menu->pages[i].item_count; j++)
		{
			value = calloc(3, sizeof(uint8_t));
			if(menu->pages[i].items[j]->value>=0)
			sprintf(value,"%d",menu->pages[i].items[j]->value);
			else
			sprintf(value,"%+d",menu->pages[i].items[j]->value);
			usart_write(_usart, menu->pages[i].items[j]->string_id, strlen(menu->pages[i].items[j]->string_id));
			while(!usart_tx_check(_usart));
			usart_write(_usart, value, strlen(value));
			while(!usart_tx_check(_usart));
			send_ffffff();
			free(value);
			//_delay_us(200);
		}
	}
}

void nextion_display_send_value(uint8_t value, char *string_id)
{
	char *string_value = calloc(3, sizeof(uint8_t));
	if(value>=0)
	sprintf(string_value,"%d",value);
	else
	sprintf(string_value,"%+d",value);
	usart_write(_usart, string_id, strlen(string_id));
	while(!usart_tx_check(_usart));
	usart_write(_usart, string_value, strlen(string_value));
	while(!usart_tx_check(_usart));
	send_ffffff();
	free(string_value);
}

void send_ffffff()
{
	uint32_t var = 0xFFFFFF;
	usart_write(_usart,&var,3);
	while(!usart_tx_check(_usart));
}