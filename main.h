/*
	Simple DND screen for Amazfit Bip BipOS
	(C) Maxim Volkov  2020 <Maxim.N.Volkov@ya.ru>
	
	Requirements: BipOS 0.5.5+
	

*/

#ifndef __APP_DND_H__
#define __APP_DND_H__


#define DISPLAY_UPDATE_PERIOD	500

#define BTN_NONE		0
#define BTN_DATE		1
#define BTN_TIME		2
#define BTN_BATTERY		3
#define BTN_APP_11		4
#define BTN_APP_12		5
#define BTN_APP_21		6
#define BTN_APP_22		7

#define MARGIN_LEFT		6
#define MARGIN_RIGHT	8
#define MARGIN_TOP		8
#define MARGIN_BOTTOM	6

#define RES_ICON_SILENCE_ON			0
#define RES_ICON_SILENCE_OFF		1
#define RES_ICON_SILENCE_AUTO		2
#define RES_ICON_FLASH				3
#define RES_ICON_FIND_PHONE_ACTIVE	5
#define RES_ICON_FIND_PHONE_STOPPED	4
#define RES_ICON_CALC				6
#define RES_ICON_BT_CON				7
#define RES_ICON_BT_NOT_CON			8

struct app_data_{
	void* ret_f;
	Elf_proc_* proc;
	int find_phone_active;
	
} ;

// main.c
void 	show_screen (void *return_screen);		//	функция запуска главного экрана приложения
void 	key_press_screen();						//	функция обработки долгого кнопки
void 	key_long_press();						//	функция обработки долгого нажатия кнопки
int 	dispatch_screen (void *param);			//	функция обработки событий сенсорного экрана
void 	screen_job();							//	функция обработки срабатывания экранного таймера приложения
void	draw_screen();							//	функция отрисовки основного экрана приложения

#endif