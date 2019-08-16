/*
**************************************************************************************************************
*	@file	debug.c
*	@author Ysheng
*	@version 
*	@date    
*	@brief	debug
***************************************************************************************************************
*/

#ifndef __debug_H
#define __debug_H
#ifdef __cplusplus
 extern "C" {
#endif

#define DEBUG__						1
#define DEBUG_LEVEL	  				2					//调试等级，配合DEBUG调试宏控制调试输出范围,大于该等级的调试不输出	 
	 
#define DEBUG(level, fmt, arg...)  if(level <= DEBUG_LEVEL)	printf(fmt,##arg);  


#ifdef __cplusplus
}
#endif
#endif /*__ pinoutConfig_H */
