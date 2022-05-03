#include "stdbool.h"

// Структура точки
typedef struct
{
  Int8U X, Y;
} POINT;

// Структура прямоугольника
typedef struct
{
  Int8U sX, eX;
  Int8U sY, eY;
} RECT;

// Структура элемента меню
typedef struct
{
  void (*caption)    (void);
  void (*enter_click)(void);
  Int8U visible: 1;
  Int8U style:   2;
} MENU_ITEM;

#define MENU_ITEM_HIDDEN        0
#define MENU_ITEM_VISIBLE       1

#define MENU_ITEM_STYLE_CENTER  0
#define MENU_ITEM_STYLE_SIDE    1

// Структура меню
typedef struct
{
  char* caption;
  const MENU_ITEM** items;
  Int8U first;
  Int8U last;
  Int8U selected;
  void (*cancel_click)(void);
  void (*initialize)  (void);
  Int8U changed;
} MENU;

Int8U menu_size  (MENU* menu);
void  menu_up    (MENU* menu);
void  menu_down  (MENU* menu);
void  menu_enter (MENU* menu);
void  menu_cancel(MENU* menu);

#define RESOLUTION_X            128
#define RESOLUTION_Y            64

#define UPPERMOST_SY            0
#define UPPERMOST_EY            11
#define UPPER_SY                13
#define UPPER_EY                24
#define MIDDLE_SY               26
#define MIDDLE_EY               37
#define LOWER_SY                39
#define LOWER_EY                50
#define LOWERMOST_SY            52
#define LOWERMOST_EY            63

#define LEFTHALF_SX             0
#define LEFTHALF_EX             63
#define RIGHTHALF_SX            64
#define RIGHTHALF_EX            127

#define LEFTTHIRD_SX            0
#define LEFTTHIRD_EX            41
#define CENTERTHIRD_SX          43
#define CENTERTHIRD_EX          84
#define RIGHTTHIRD_SX           86
#define RIGHTTHIRD_EX           127

#define KEY_WAIT_TIME            100
#define KEY_NOPRESS_TIMEOUT      120000
#define KEY_NOPRESS_SCREEN_SAVER 300000

#define MEASURES_SCREEN         0
#define CONTROLS_SCREEN         1

#define MESSAGEBOX_OK           0
#define MESSAGEBOX_YESNO        1

extern U8                           main_screen[128][8];

void OLED_ini                      (void);
void write_control_OLED            (pU8 commandBuffer, U32 datanum);
void load_GRAM_OLED                (pU8 buffer);

void OLED_Screen(bool light);
void OLED_Load(Int8U sX, Int8U eX, Int8U sY, Int8U eY, Int8U* data, bool light);
void OLED_Rect(const RECT* rect, bool light);
void OLED_RectStretch(const RECT* rect, Int8S lx, Int8S rx, Int8S uy, Int8S ly, bool light);
void OLED_Invert(const RECT* rect);

void OLED_Image(const tImage* image, Int8U sX, Int8U sY, bool light);
void OLED_Char(const tChar* chr, Int8U sX, Int8U sY, bool light);

void OLED_String(char* string, Int8U sX, Int8U sY, const tFont* font, bool light);
void OLED_StringEmphasize(char* string, Int8U sX, Int8U sY, const tFont* font, bool light, int emphasize);

void OLED_WriteStringInRectLeft(char* string, const RECT* rect, Int8S x_offset, Int8S y_offset, const tFont* font, bool light);
void OLED_WriteStringInRectCenter(char* string, const RECT* rect, Int8S x_offset, Int8S y_offset, const tFont* font, bool light);
void OLED_WriteStringInRectCenter_Emphasize(char* string, const RECT* rect, Int8S x_offset, Int8S y_offset, const tFont* font, bool light, int emphasize);
void OLED_WriteStringInRectRight(char* string, const RECT* rect, Int8S x_offset, Int8S y_offset, const tFont* font, bool light);
int  OLED_StringWidth(char* string, const tFont* font);
int  OLED_StringHeight(char* string, const tFont* font);

void OLED_DrawImageInRectCenter(const tImage* image, const RECT* rect, bool light);

void OLED_UnderlineRect(const RECT* rect, Int8S x_offset, Int8S y_offset, bool light);
void OLED_UnderlineRectShift(const RECT* rect, Int8S x_offset, Int8S y_offset, Int8S x_shift, Int8S y_shift, bool light);

bool GUI_KeyboardChar(char* value, char* caption, bool is_signed, bool is_ip);
bool GUI_KeyboardInt(int* value, char* caption, bool is_signed, int min, int max);
bool GUI_KeyboardFloat(float* value, char* caption, bool is_signed, float min, float max, int decimal);
bool GUI_KeyboardIP(Int32U* value, char* caption);

bool GUI_MessageBox(char* caption, char* text, Int8U timeout, Int8U type);

void GUI_ProgressBar(const RECT* rect, Int8U current_step, Int8U overall_steps);
void GUI_LoadingScreen(char* action, bool result, char* error_message, Int8U current_step, Int8U overall_steps);

void GUI_SetGUI (void (*screen_function)(void));
void GUI_SetMenu(MENU* menu);
void GUI_ReloadScreen();

void GUI_LogoScreen();

void GUI_DrawHeader(char* caption);
void GUI_DrawTime(bool highlight);
void GUI_DrawBattery(const RECT* rect, Int8U power);

void GUI_RadioListBrowser();
void GUI_Information();

extern MENU UARTMenu;
extern MENU EthernetMenu;
extern MENU WiFiMenu;
extern MENU DateTimeMenu;
extern MENU MainMenu;

void GUI_MenuMachine();

void OLED_Task();

extern OS_STACKPTR U32                StackOLED[]; 
extern OS_TASK                        OS_OLED;

extern const tFont                    CourierNew16;
extern const tFont                    CourierNew8;
extern const tFont                    Tahoma16;
extern const tFont                    Tahoma12;
extern const tImage                   Logo;

extern RECT UPPERMOST12_LINE;
extern RECT UPPER12_LINE;
extern RECT MIDDLE12_LINE;
extern RECT LOWER12_LINE;
extern RECT LOWERMOST12_LINE;