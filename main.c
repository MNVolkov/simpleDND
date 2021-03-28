/*
     Simple DND screen for Amazfit Bip BipOS
	(C) Maxim Volkov  2021 <Maxim.N.Volkov@ya.ru>
	
	пример реализации окна DND в BipOS для Amazfit Bip
	
	v.1.0
	Описание программы:
	Экран DND содержит:
	+	строку состояния (основано на библиотеке BipStatusbarLibrary @ugers ( https://github.com/ugers/BipStatusbarLibrary )
	+	четыре кнопки быстрого запуска приложений (фонарик, поиск телефона, переключение режимов DND, калькулятор)
	
	
*/

#include <libbip.h>
#include "main.h"
#include "BipStatusbarLib.h"

//	структура меню экрана - для каждого экрана своя
struct regmenu_ screen_data = {				
						55,							//	current_screen 
						1,							//	номер главного экрана, значение 0-255, для пользовательских окон лучше брать от 50 и выше
						0,							//	0
						dispatch_screen,			//	указатель на функцию обработки тача, свайпов, долгого нажатия кнопки
						key_press_screen, 			//	указатель на функцию обработки нажатия кнопки
						screen_job,					//	указатель на функцию-колбэк таймера 
						0,							//	0
						show_screen,				//	указатель на функцию отображения экрана
						0,							//	
						0							//	долгое нажатие кнопки
					};

					
int main(int proc, char** argv){	//	здесь переменная argv не определена
	show_screen((void*) proc);	
}

void show_screen (void *proc){
struct app_data_** 	app_data_p = get_ptr_temp_buf_2(); 	//	указатель на указатель на данные экрана 
struct app_data_ *	app_data;					//	указатель на данные экрана


// проверка источника запуска процедуры
if ( (proc == *app_data_p) ){ // возврат из оверлейного экрана (входящий звонок, уведомление, будильник, цель и т.д.) или из дочернего экрана

	app_data = *app_data_p;					//	указатель на данные необходимо сохранить для исключения 
											//	высвобождения памяти функцией reg_menu
	*app_data_p = NULL;						//	обнуляем указатель для передачи в функцию reg_menu	

	// 	создаем новый экран, при этом указатель temp_buf_2 был равен 0 и память не была высвобождена	
	reg_menu(&screen_data, 0); 				// 	menu_overlay=0
	
	*app_data_p = app_data;						//	восстанавливаем указатель на данные после функции reg_menu
	
	//   здесь проводим действия при возврате из оверлейного экрана: восстанавливаем данные и т.д.
	
	
} else { // если запуск функции произошел впервые т.е. из меню 
	
	// создаем экран (регистрируем в системе)
	reg_menu(&screen_data, 0);

	// выделяем необходимую память и размещаем в ней данные, (память по указателю, хранящемуся по адресу temp_buf_2 высвобождается автоматически функцией reg_menu другого экрана)
	*app_data_p = (struct app_data_ *)pvPortMalloc(sizeof(struct app_data_));
	app_data = *app_data_p;		//	указатель на данные
	
	// очистим память под данные
	_memclr(app_data, sizeof(struct app_data_));
	
	//	получим указатель на данные запущенного процесса структура Elf_proc_
	app_data->proc = get_proc_by_addr(main);
	
	// запомним адрес указателя на функцию в которую необходимо вернуться после завершения данного экрана
	if ( proc && app_data->proc->elf_finish ) 			//	если указатель на возврат передан, то возвоащаемся на него
		app_data->ret_f = app_data->proc->elf_finish;
	else					//	если нет, то на циферблат
		app_data->ret_f = show_watchface;
	
	// здесь проводим действия которые необходимо если функция запущена впервые из меню: заполнение всех структур данных и т.д.
	
}

// здесь выполняем отрисовку интерфейса, обновление (перенос в видеопамять) экрана выполнять не нужно

draw_screen();

// ставим таймер вызова screen_job в мс в зависимости нужна ли заставка
set_update_period(1,  DISPLAY_UPDATE_PERIOD);
}

void key_press_screen(){
struct app_data_** 	app_data_p = get_ptr_temp_buf_2(); 	//	указатель на указатель на данные экрана 
struct app_data_ *	app_data = *app_data_p;				//	указатель на данные экрана

// вызываем функцию возврата (для экрана DND обычно это циферблат), в качестве параметра указываем адрес функции нашего приложения
show_menu_animate(app_data->ret_f, (int)main, ANIMATE_UP);	
};

void screen_job(){
// при необходимости можно использовать данные экрана в этой функции
//struct app_data_** 	app_data_p = get_ptr_temp_buf_2(); 	//	указатель на указатель на данные экрана 
//struct app_data_ *	app_data = *app_data_p;				//	указатель на данные экрана

// делаем периодическое действие: анимация, увеличение счетчика, обновление экрана,
// отрисовку интерфейса, обновление (перенос в видеопамять) экрана выполнять нужно

draw_screen();
repaint_screen_lines(0, 176);

set_update_period(1, DISPLAY_UPDATE_PERIOD); // обновляем экран через заданное время
}


int dispatch_screen (void *param){
struct app_data_** 	app_data_p = get_ptr_temp_buf_2(); 	//	указатель на указатель на данные экрана 
struct app_data_ *	app_data = *app_data_p;				//	указатель на данные экрана

struct gesture_ *gest = param;
int result = 0;

switch (gest->gesture){

	case GESTURE_CLICK: {	
		
			int btn = BTN_NONE;
			
			if ( ( gest->touch_pos_y < 30 ) ) {	//	нажатие на статусбар
								
				int btn_index = gest->touch_pos_x / (176/3);
				
				//определяем какая кнопка нажата
				if (btn_index == 0) 	//	левая сторона статусбара - дата
					btn = BTN_DATE;
				else 
				if (btn_index == 1){	//	средняя часть статусбара - время
					btn = BTN_TIME;
				}
				else 					//	то что осталось - правая сторона - батарейка
					btn = BTN_BATTERY;
				
			}  else 
			if (gest->touch_pos_y < 98){	// верхняя часть экрана							
					int btn_index = gest->touch_pos_x / (176/2);

					//определяем какая кнопка нажата
					if (btn_index == 0) {	//	левая кнопка
						btn = BTN_APP_11;
					}
					else {					//	то что осталось - правая кнопка
						btn = BTN_APP_12;
					}
				
			} else {								
					int btn_index = gest->touch_pos_x / (176/2);

					//определяем какая кнопка нажата
					if (btn_index == 0){ 	//	левая кнопка
						btn = BTN_APP_21;
					}
					else {					//	то что осталось - правая кнопка
						btn = BTN_APP_22;
					}
			}  
	
			if (btn != BTN_NONE)
				vibrate(1,70,0);
				
			
			if (btn != BTN_APP_12){
				send_host_app_msg(CMD_FIND_PHONE_STOP);
				app_data->find_phone_active=0;
			}
			
			switch (btn){
				case BTN_DATE:
				case BTN_TIME:
				case BTN_BATTERY:{
					//	ничего не делаем
					break;
				}
				
				case BTN_APP_11:	{
					//	при нажатии на кнопку 1	-	DND
					char dnd;
					get_generic_data(GENERIC_DATA_DND_MODE, &dnd);
					dnd++;
					dnd%=3;
					set_generic_data(GENERIC_DATA_DND_MODE, &dnd);
					break;
				}
				case BTN_APP_12:	{
					
					//	при нажатии на кнопку 2	-	поиск телефона
					app_data->find_phone_active++;
					app_data->find_phone_active%=2;
					if (app_data->find_phone_active)
						send_host_app_msg(CMD_FIND_PHONE_START);
					else 
						send_host_app_msg(CMD_FIND_PHONE_STOP);
					break;
				}
				case BTN_APP_21:	{
					//	при нажатии на кнопку 3	-	фонарик
					int result = load_elf_by_name("flash", show_watchface, 0, 0, NULL);
					repaint_screen();
					if ( result == ERROR_NONE){
						Elf_proc_* proc = get_proc_by_addr(main);
						proc->ret_f = NULL;
						elf_finish(main);	//	выгрузить Elf из памяти
						
						return ERROR_NONE;
					}
					break;
				}
				case BTN_APP_22:	{
					//	при нажатии на кнопку 4	-	калькулятор
					int result = load_elf_by_name("calc", show_watchface, 0, 0, NULL);
					repaint_screen();
					if ( result == ERROR_NONE){
						Elf_proc_* proc = get_proc_by_addr(main);
						proc->ret_f = NULL;
						elf_finish(main);	//	выгрузить Elf из памяти
						
						return ERROR_NONE;
					}
					break;
				}
				//	если нажатие вне тач зон
				default: break;
			}
		
		
		draw_screen();
		repaint_screen_lines(0, 176);
	
		break;
	};
		
		case GESTURE_SWIPE_RIGHT: 	//	свайп направо
		case GESTURE_SWIPE_LEFT: 
		case GESTURE_SWIPE_DOWN: {	// свайп вверх
			break;
		};
		case GESTURE_SWIPE_UP: {	// справа вверх
			return show_menu_animate(app_data->ret_f, (int)main, ANIMATE_UP);
		};
		
		default:{	// что-то пошло не так...
			
			break;
		};		
		
	}


	
//ставим таймер вызова screen_job в мс
set_update_period(1, DISPLAY_UPDATE_PERIOD);

return result;
};


// Отрисовка экрана 
void draw_screen(){
	
	void draw_button(int x, int y, int width, int height){
	draw_filled_rect_bg(x, y, x+width,y+height);
	}

struct app_data_** 	app_data_p = get_ptr_temp_buf_2(); 	//	указатель на указатель на данные экрана 
struct app_data_ *	app_data = *app_data_p;				//	указатель на данные экрана

set_bg_color(COLOR_BLACK);
fill_screen_bg();
set_graph_callback_to_ram_1();

// подгружаем шрифты
load_font();

show_statusbar(3, COLOR_BLACK,COLOR_WHITE);

// рисуем 4 кнопки
set_bg_color(COLOR_WHITE);
set_fg_color(COLOR_BLACK);

// draw_filled_rect(MARGIN_LEFT, 		20+MARGIN_TOP,		 	 88-MARGIN_RIGHT/2,		 98-MARGIN_BOTTOM/2);	//	BTN_APP_11
// draw_filled_rect(88+MARGIN_RIGHT/2, 20+MARGIN_TOP,			176-MARGIN_RIGHT/2, 	 98-MARGIN_BOTTOM/2);	//	BTN_APP_12
// draw_filled_rect(MARGIN_LEFT, 		98+MARGIN_BOTTOM/2, 	 88-MARGIN_RIGHT/2, 	176-MARGIN_BOTTOM/2);	//	BTN_APP_21
// draw_filled_rect(88+MARGIN_RIGHT/2, 98+MARGIN_BOTTOM/2, 	176-MARGIN_RIGHT/2, 	176-MARGIN_BOTTOM/2);	//	BTN_APP_22

#define BTN_WIDTH	82
#define BTN_HEIGHT	72
#define BTN_X_1		4
#define BTN_X_2		4+BTN_WIDTH+4
#define BTN_Y_1		20+4
#define BTN_Y_2		20+4+BTN_HEIGHT+4


draw_button(BTN_X_1, BTN_Y_1,	BTN_WIDTH, BTN_HEIGHT);	//	DND
draw_button(BTN_X_2, BTN_Y_1,	BTN_WIDTH, BTN_HEIGHT);	//	FIND_PHONE
draw_button(BTN_X_1, BTN_Y_2,	BTN_WIDTH, BTN_HEIGHT);	//	FLASH
draw_button(BTN_X_2, BTN_Y_2,	BTN_WIDTH, BTN_HEIGHT);	//	CALC

// text_out_center("DND", 44, 49);
// text_out_center("FIND", 132, 49);
// text_out_center("FLASH", 44, 132);
// text_out_center("CALC", 132, 132);

// если отклюился bluetooth поменять иконку на неактивную
if (!IS_BT_CONNECTED){
	 app_data->find_phone_active = 0;
}

char dnd;
int dnd_icon;
get_generic_data(GENERIC_DATA_DND_MODE, &dnd);

switch (dnd){
default: 
case DND_MODE_OFF:  dnd_icon = RES_ICON_SILENCE_OFF; break;
case DND_MODE_ON:  	dnd_icon = RES_ICON_SILENCE_ON; break;
case DND_MODE_AUTO:	dnd_icon = RES_ICON_SILENCE_AUTO; break;
}

struct res_params_ res_params;	//	параметры графического реурса
get_res_params(ELF_INDEX_SELF, dnd_icon, &res_params);

int find_phone_icon = app_data->find_phone_active?RES_ICON_FIND_PHONE_ACTIVE:RES_ICON_FIND_PHONE_STOPPED;

show_elf_res_by_id(ELF_INDEX_SELF, dnd_icon, 			BTN_X_1+(BTN_WIDTH-res_params.width)/2, BTN_Y_1+(BTN_HEIGHT-res_params.height)/2);
show_elf_res_by_id(ELF_INDEX_SELF, find_phone_icon,		BTN_X_2+(BTN_WIDTH-res_params.width)/2, BTN_Y_1+(BTN_HEIGHT-res_params.height)/2);
show_elf_res_by_id(ELF_INDEX_SELF, RES_ICON_FLASH, 		BTN_X_1+(BTN_WIDTH-res_params.width)/2, BTN_Y_2+(BTN_HEIGHT-res_params.height)/2);
show_elf_res_by_id(ELF_INDEX_SELF, RES_ICON_CALC, 		BTN_X_2+(BTN_WIDTH-res_params.width)/2, BTN_Y_2+(BTN_HEIGHT-res_params.height)/2);


};

