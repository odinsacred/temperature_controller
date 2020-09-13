/* 
 * File:   assert.h
 * Author: odins
 *
 * Created on 6 сентября 2020 г., 22:14
 */

#ifndef ASSERT_H
#define	ASSERT_H

#ifdef	__cplusplus
extern "C" {
#endif
#include <stdbool.h>
#include <stdint.h>    

void assert(bool value, uint8_t blink);


#ifdef	__cplusplus
}
#endif

#endif	/* ASSERT_H */

