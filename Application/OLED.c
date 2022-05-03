#include "defines.h"
#include "IP_init.h"

RECT SCREEN = {0, RESOLUTION_X-1, 0, RESOLUTION_Y-1};

RECT UPPERMOST_LINE = {LEFTHALF_SX, RIGHTHALF_EX, UPPERMOST_SY, UPPERMOST_EY};
RECT UPPER_LINE     = {LEFTHALF_SX, RIGHTHALF_EX, UPPER_SY,     UPPER_EY};
RECT MIDDLE_LINE    = {LEFTHALF_SX, RIGHTHALF_EX, MIDDLE_SY,    MIDDLE_EY};
RECT LOWER_LINE     = {LEFTHALF_SX, RIGHTHALF_EX, LOWER_SY,     LOWER_EY};
RECT LOWERMOST_LINE = {LEFTHALF_SX, RIGHTHALF_EX, LOWERMOST_SY, LOWERMOST_EY};

const RECT* LINES[]   = {&UPPERMOST_LINE, &UPPER_LINE, &MIDDLE_LINE, &LOWER_LINE, &LOWERMOST_LINE};

const RECT UPPERMOSTLEFT_HALF   = {LEFTHALF_SX,  LEFTHALF_EX,  UPPERMOST_SY,  UPPERMOST_EY};
const RECT UPPERMOSTRIGHT_HALF  = {RIGHTHALF_SX, RIGHTHALF_EX, UPPERMOST_SY,  UPPERMOST_EY};
const RECT UPPERLEFT_HALF       = {LEFTHALF_SX,  LEFTHALF_EX,  UPPER_SY,      UPPER_EY};
const RECT UPPERRIGHT_HALF      = {RIGHTHALF_SX, RIGHTHALF_EX, UPPER_SY,      UPPER_EY};
const RECT MIDDLELEFT_HALF      = {LEFTHALF_SX,  LEFTHALF_EX,  MIDDLE_SY,     MIDDLE_EY};
const RECT MIDDLERIGHT_HALF     = {RIGHTHALF_SX, RIGHTHALF_EX, MIDDLE_SY,     MIDDLE_EY};
const RECT LOWERLEFT_HALF       = {LEFTHALF_SX,  LEFTHALF_EX,  LOWER_SY,      LOWER_EY};
const RECT LOWERRIGHT_HALF      = {RIGHTHALF_SX, RIGHTHALF_EX, LOWER_SY,      LOWER_EY};
const RECT LOWERMOSTLEFT_HALF   = {LEFTHALF_SX,  LEFTHALF_EX,  LOWERMOST_SY,  LOWERMOST_EY};
const RECT LOWERMOSTRIGHT_HALF  = {RIGHTHALF_SX, RIGHTHALF_EX, LOWERMOST_SY,  LOWERMOST_EY};

const RECT UPPERMOSTLEFT_THIRD   = {LEFTTHIRD_SX,   LEFTTHIRD_EX,   UPPERMOST_SY, UPPERMOST_EY};
const RECT UPPERMOSTCENTER_THIRD = {CENTERTHIRD_SX, CENTERTHIRD_EX, UPPERMOST_SY, UPPERMOST_EY};
const RECT UPPERMOSTRIGHT_THIRD  = {RIGHTTHIRD_SX,  RIGHTTHIRD_EX,  UPPERMOST_SY, UPPERMOST_EY};
const RECT UPPERLEFT_THIRD       = {LEFTTHIRD_SX,   LEFTTHIRD_EX,   UPPER_SY,     UPPER_EY};
const RECT UPPERCENTER_THIRD     = {CENTERTHIRD_SX, CENTERTHIRD_EX, UPPER_SY,     UPPER_EY};
const RECT UPPERRIGHT_THIRD      = {RIGHTTHIRD_SX,  RIGHTTHIRD_EX,  UPPER_SY,     UPPER_EY};
const RECT MIDDLELEFT_THIRD      = {LEFTTHIRD_SX,   LEFTTHIRD_EX,   MIDDLE_SY,    MIDDLE_EY};
const RECT MIDDLECENTER_THIRD    = {CENTERTHIRD_SX, CENTERTHIRD_EX, MIDDLE_SY,    MIDDLE_EY};
const RECT MIDDLERIGHT_THIRD     = {RIGHTTHIRD_SX,  RIGHTTHIRD_EX,  MIDDLE_SY,    MIDDLE_EY};
const RECT LOWERLEFT_THIRD       = {LEFTTHIRD_SX,   LEFTTHIRD_EX,   LOWER_SY,     LOWER_EY};
const RECT LOWERCENTER_THIRD     = {CENTERTHIRD_SX, CENTERTHIRD_EX, LOWER_SY,     LOWER_EY};
const RECT LOWERRIGHT_THIRD      = {RIGHTTHIRD_SX,  RIGHTTHIRD_EX,  LOWER_SY,     LOWER_EY};
const RECT LOWERMOSTLEFT_THIRD   = {LEFTTHIRD_SX,   LEFTTHIRD_EX,   LOWERMOST_SY, LOWERMOST_EY};
const RECT LOWERMOSTCENTER_THIRD = {CENTERTHIRD_SX, CENTERTHIRD_EX, LOWERMOST_SY, LOWERMOST_EY};
const RECT LOWERMOSTRIGHT_THIRD  = {RIGHTTHIRD_SX,  RIGHTTHIRD_EX,  LOWERMOST_SY, LOWERMOST_EY};

const RECT* LEFT_HALVES[]  = {&UPPERMOSTLEFT_HALF,   &UPPERLEFT_HALF,   &MIDDLELEFT_HALF,   &LOWERLEFT_HALF,   &LOWERMOSTLEFT_HALF};
const RECT* RIGHT_THIRDS[] = {&UPPERMOSTRIGHT_THIRD, &UPPERRIGHT_THIRD, &MIDDLERIGHT_THIRD, &LOWERRIGHT_THIRD, &LOWERMOSTRIGHT_THIRD};

const RECT HEADER  = {LEFTHALF_SX,   RIGHTHALF_EX,   UPPERMOST_SY, UPPERMOST_EY};
const RECT CAPTION = {LEFTTHIRD_SX,  CENTERTHIRD_EX, UPPERMOST_SY, UPPERMOST_EY};
const RECT TIME    = {RIGHTTHIRD_SX, RIGHTTHIRD_EX,  UPPERMOST_SY, UPPERMOST_EY};

const pU8 DeviceType[] = { "ИВТМ-7М4-1", "ИВТМ-7М4", "ИКВ-8" };

static U32  TempTime;

#pragma data_alignment = 4
U8 main_screen[128][8];

void GUI_ScreenSaver(void);
void (*Screen)(void);
MENU* Menu;

char Display1[64];
char Display2[64];
char Display3[64];

/* процедыры упраления TFT ------------------------------------------------------------ */
/* запись управляющих регистров ТФТ ------------------ */
// __WEO012864G
static const U8 oled_init_seq1[] =   { 
                                            0xD5, 0xA0,   // DCLK = Fosc ()/ 1
                                            
                                            0x20,         // Horizontal Addressing Mode  
                                            0x21, 0, 127,
                                            0x22, 0, 7,
                                            
                                            0xD3, 0x01,   // Set Display Offset = 1
                                            
                                            0xC8,         //   Row  address   63  is mapped  to COM0
                                            0xA1,         //   Column address 127 is mapped to SEG0
                                                                                        
                                            0xAF          // Display ON
                                     };

static const U8 oled_set_adr_atbegin1[] =   { 
                                              0x21, 0, 127, 
                                              0x22, 0, 7 
                                            };

// __WEG012864M
static const U8 oled_init_seq2[] =   { 
                                            0xD5, 0xA0,   // DCLK = Fosc ()/ 1

                                            0x20,         // Horizontal Addressing Mode 
                                            0x21, 0, 127,
                                            0x22, 0, 7,
                                            
                                            0xD3, 0x01,   // Set Display Offset = 1
                                            
//                                            0xDA, 0x12, 
                                            0xC8,         //   Row  address   63  is mapped  to COM0
                                            0xA0,         //   Column address 0   is mapped to SEG0                                            

                                            0xAF          // Display ON
                                     };

static const U8 oled_set_adr_atbegin2[] =   { 
                                              0x21, 0, 127, 
                                              0x22, 0, 7 
                                            };

// __SSD1305_CHINAMAMA
static const U8 oled_init_seq3[] =   { 
                                            0xD5, 0xA0,   // DCLK = Fosc ()/ 1
                                            
                                            0x20,         // Horizontal Addressing Mode  
                                            0x21, 2, 129,
                                            0x22, 0, 7,
                                            
                                            0xD3, 0x00,   // Set Display Offset = 0
                                            
                                            0xC8,         // Scan Direction from COM[63] to COM0
                                            0xA1,         // Column address 127 is mapped to SEG0
                                                                                        
                                            0xAF          // Display ON
                                     };

static const U8 oled_set_adr_atbegin3[] =   { 
                                              0x21, 2, 129, 
                                              0x22, 0, 7 
                                            };

/* байт по SPI передать/принять */
#pragma optimize=none
U8  shift_byte_spi3(U8 byte)
{
  SPI3_DR = byte; 
  while  (!SPI3_SR_bit.TXE); 
  while  (SPI3_SR_bit.BSY); 
  return SPI3_DR; 
}

void write_control_OLED(pU8 commandBuffer, U32 datanum)
{
  OLED_COMMAND;
  OLED_CS_LOW;
  
  for(U32 i = 0; i < datanum; i++)
    shift_byte_spi3( commandBuffer[i] );
  
  OLED_CS_HIGH;
}

/* загрузка графической паямяти из буффера ------ */
void load_GRAM_OLED(pU8 buffer)
{ 
  switch( ConfigFlags & DISPLAY_MASK )
  {
    case __SDD1309_WEO012864G:    write_control_OLED( (pU8)oled_set_adr_atbegin1, sizeof(oled_set_adr_atbegin1)); break;
    case __SDD1309_WEG012864M:    write_control_OLED( (pU8)oled_set_adr_atbegin2, sizeof(oled_set_adr_atbegin2)); break;
    case __SDD1305_CHINANONAME1:  write_control_OLED( (pU8)oled_set_adr_atbegin3, sizeof(oled_set_adr_atbegin3)); break;
  }
  
  OLED_DATA;
  OLED_CS_LOW;
  
  if( buffer != NULL )  for(U32 i = 0; i < 8 * 128; i++, buffer++) shift_byte_spi3( *buffer); 
  else                  for(U32 i = 0; i < 8 * 128; i++)           shift_byte_spi3(0);  
  
  OLED_CS_HIGH;
}

/* инициализация OLED */
#pragma optimize=none
void OLED_ini(void)
{  
  OLED_RES_LOW;
  for(U32 i = 0; i < 72*40; i++)   __no_operation();    // 40 us  
  OLED_RES_HIGH;
  
  for(U32 i = 0; i < 72*1000; i++) __no_operation();    // 1 ms
  
  switch( ConfigFlags & DISPLAY_MASK )
  {
    case __SDD1309_WEO012864G:    write_control_OLED( (pU8)oled_init_seq1, sizeof(oled_init_seq1)); break;
    case __SDD1309_WEG012864M:    write_control_OLED( (pU8)oled_init_seq2, sizeof(oled_init_seq2)); break;
    case __SDD1305_CHINANONAME1:  write_control_OLED( (pU8)oled_init_seq3, sizeof(oled_init_seq3)); break;
  }  
  
  load_GRAM_OLED(NULL);
}
/* процедыры упраления TFT ------------------------------------------------------------ */

void SetBit(Int8U* byte, Int8U position, bool bit)
{
  if (bit)
    *byte |= (1 << position);
  else
    *byte &= ~(1 << position);
}

bool GetBit(Int8U* byte, Int8U position)
{
  return *byte & (1 << position);
}

bool OutOfRange(Int8U sX, Int8U eX, Int8U sY, Int8U eY)
{
  if ((sX > RESOLUTION_X) || (eX > RESOLUTION_X)) return true;
  if ((sY > RESOLUTION_Y) || (eY > RESOLUTION_Y)) return true;
  if ((sX > eX) || (sY > eY)) return true;
  
  return false;
}

// зажигает/гасит экран
void OLED_Screen(bool light)
{
  Int8U* buffer = &main_screen[0][0];
  for (int i = 0; i < sizeof(main_screen); i++)
  {
    *buffer = light ? 0xFF : 0x00;
    buffer++;
  }
}

// загружает данные
void OLED_Load(Int8U sX, Int8U eX, Int8U sY, Int8U eY, Int8U* data, bool light)
{
  Int8U width, height;
  width  = eX - sX + 1;
  height = eY - sY + 1;
  
  if (OutOfRange(sX, eX, sY, eY)) return;

  bool light_;
  for (int i = 0; i < height; i++)
  {
    for (int j = 0; j < width; j++)
    {
      light_ = GetBit(&data[i*((width-1)/8+1)+j/8], 7-j%8);
      if (!light) light_ = !light_;
      SetBit(&main_screen[sX+j][(sY+i)/8], (sY+i)%8, light_);
    }
  }
}

// зажигает/гасит прямоугольник
void OLED_Rect(const RECT* rect, bool light)
{
  Int8U width, height;
  width  = rect->eX - rect->sX + 1;
  height = rect->eY - rect->sY + 1;
  
  if (OutOfRange(rect->sX, rect->eX, rect->sY, rect->eY)) return;

  for (int i = 0; i < height; i++)
  {
    for (int j = 0; j < width; j++)
    {
      SetBit(&main_screen[rect->sX+j][(rect->sY+i)/8], (rect->sY+i)%8, light);
    }
  }
}

// зажигает/гасит растянутый прямоугольник
void OLED_RectStretch(const RECT* rect, Int8S lx, Int8S rx, Int8S uy, Int8S ly, bool light)
{
  RECT r = *rect;
  
  r.sX += lx;
  r.eX += rx;
  r.sY += uy;
  r.eY += ly;
  OLED_Rect(&r, light);
}

// инвертирует прямоугольник
void OLED_Invert(const RECT* rect)
{
  Int8U width, height;
  width  = rect->eX - rect->sX + 1;
  height = rect->eY - rect->sY + 1;
  
  if (OutOfRange(rect->sX, rect->eX, rect->sY, rect->eY)) return;

  bool light;
  for (int i = 0; i < height; i++)
  {
    for (int j = 0; j < width; j++)
    {
      light = !GetBit(&main_screen[rect->sX+j][(rect->sY+i)/8], (rect->sY+i)%8);
      SetBit(&main_screen[rect->sX+j][(rect->sY+i)/8], (rect->sY+i)%8, light);
    }
  }
}

// рисует изображение
void OLED_Image(const tImage* image, Int8U sX, Int8U sY, bool light)
{
  Int8U eX, eY;
  eX = sX + image->width  - 1;
  eY = sY + image->height - 1;
  
  OLED_Load(sX, eX, sY, eY, (Int8U*)image->data, light);
}

// выводит символ
void OLED_Char(const tChar* chr, Int8U sX, Int8U sY, bool light)
{
  OLED_Image(chr->image, sX, sY, light);
}

// выводит строку
void OLED_String(char* string, Int8U sX, Int8U sY, const tFont* font, bool light)
{
  while (*string)
  {
    for (int i = 0; i < font->length; i++)
    {
      if (*string == font->chars[i].code)
      {
        OLED_Char(&font->chars[i], sX, sY, light);
        sX += font->chars[i].image->width;
        break;
      }
    }
    string++;
  }
}

void OLED_String_Emphasize(char* string, Int8U sX, Int8U sY, const tFont* font, bool light, int emphasize)
{
  int e = 0;
  
  while (*string)
  {
    for (int i = 0; i < font->length; i++)
    {
      if (*string == font->chars[i].code)
      {
        OLED_Char(&font->chars[i], sX, sY, e != emphasize ? light : !light);
        sX += font->chars[i].image->width;
        break;
      }
    }
    e++;
    string++;
  }
}

void OLED_WriteStringInRectLeft(char* string, const RECT* rect, Int8S x_offset, Int8S y_offset, const tFont* font, bool light)
{
  Int8U height, sX, sY;
  height = OLED_StringHeight(string, font);
  sX = rect->sX + x_offset;
  sY = abs((rect->sY + rect->eY + 1)/2 - height/2) + y_offset;
  
  OLED_String(string, sX, sY, font, light);
}

void OLED_WriteStringInRectCenter(char* string, const RECT* rect, Int8S x_offset, Int8S y_offset, const tFont* font, bool light)
{
  Int8U width, height, sX, sY;
  width  = OLED_StringWidth(string, font);
  height = OLED_StringHeight(string, font);
  sX = abs((rect->sX + rect->eX + 1)/2 - width/2) + x_offset;
  sY = abs((rect->sY + rect->eY + 1)/2 - height/2) + y_offset;
  
  if (sX + width > RESOLUTION_X)
  {
    Int8U delta = sX + width - RESOLUTION_X;
    sX = sX - delta;
  }
  
  OLED_String(string, sX, sY, font, light);
}

void OLED_WriteStringInRectCenter_Emphasize(char* string, const RECT* rect, Int8S x_offset, Int8S y_offset, const tFont* font, bool light, int emphasize)
{
  Int8U width, height, sX, sY;
  width  = OLED_StringWidth(string, font);
  height = OLED_StringHeight(string, font);
  sX = abs((rect->sX + rect->eX + 1)/2 - width/2) + x_offset;
  sY = abs((rect->sY + rect->eY + 1)/2 - height/2) + y_offset;
  
  OLED_String_Emphasize(string, sX, sY, font, light, emphasize);
}

void OLED_WriteStringInRectRight(char* string, const RECT* rect, Int8S x_offset, Int8S y_offset, const tFont* font, bool light)
{
  Int8U width, height, sX, sY;
  width  = OLED_StringWidth(string, font);
  height = OLED_StringHeight(string, font);
  sX = rect->eX - width - x_offset;
  sY = abs((rect->sY + rect->eY + 1)/2 - height/2) + y_offset;
  
  OLED_String(string, sX, sY, font, light);
}

// вычисляет ширину строки
int OLED_StringWidth(char* string, const tFont* font)
{
  int width = 0;  

  while (*string)
  {
    for (int i = 0; i < font->length; i++)
      if (*string == font->chars[i].code)
      {
        width += font->chars[i].image->width;
        break;
      }
    string++;
  }
  
  return width;
}

// вычисляет высоту строки
int OLED_StringHeight(char* string, const tFont* font)
{
  return font->chars[0].image->height;
}

// рисует изображение в центре прямоугольника
void OLED_DrawImageInRectCenter(const tImage* image, const RECT* rect, bool light)
{
  Int8U sX, sY;
  sX = rect->sX + (rect->eX - rect->sX)/2 - image->width/2;
  sY = rect->sY + (rect->eY - rect->sY)/2 - image->height/2;
  OLED_Image(image, sX, sY, light);
}

// подчёркивает прямоугольник со смещением
void OLED_UnderlineRect(const RECT* rect, Int8S x_offset, Int8S y_offset, bool light)
{
  RECT r;
  r.sX = rect->sX+x_offset;
  r.eX = rect->eX-x_offset;
  r.sY = rect->eY-y_offset;
  r.eY = rect->eY-y_offset;
  
  OLED_Rect(&r, light);
}

// подчёркивает прямоугольник со смещением и сдвигом
void OLED_UnderlineRectShift(const RECT* rect, Int8S x_offset, Int8S y_offset, Int8S x_shift, Int8S y_shift, bool light)
{
  RECT r;
  r.sX = rect->sX+x_offset+x_shift;
  r.eX = rect->eX-x_offset+x_shift;
  r.sY = rect->eY-y_offset+y_shift;
  r.eY = rect->eY-y_offset+y_shift;
  
  OLED_Rect(&r, light);
}

void ProceedUp(char* c, Int8U* exceed)
{
  *exceed = 0;
  
  switch (*c)
  {
  case '+':
    *c = '-';
    break;
  case '-':
    *c = '+';
    break;
    
  case '9':
    *c = '0';
    *exceed = 1;
    break;
    
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
    *c += 1;
    break;
  }
}

void ProceedDown(char* c)
{
  switch (*c)
  {
  case '+':
    *c = '-';
    break;
  case '-':
    *c = '+';
    break;
    
  case '0':
    *c = '9';
    break;
    
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    *c -= 1;
    break;
  }
}

void strinsert(char* str, char c, int pos, int max_length)
{
  int length = strlen(str);
  if (length > max_length) return;
  
  length++;
  str[length] = 0;
  
  for (int i = length - 1; i > pos; i--)
  {
    str[i] = str[i - 1];
  }
  str[pos] = c;
}

Int8U octet_size(char* str, int pos)
{
  int length = strlen(str);

  int start = pos;
  while ((start > 0) && (str[start - 1] != '.'))
  {
    start--;
  }
  
  int end = pos;
  while ((end < length - 1) && (str[end + 1] != '.'))
  {
    end++;
  }
  
  return (end - start + 1);
}

// функция ввода значения char
bool GUI_KeyboardChar(char* value, char* caption, bool is_signed, bool is_ip)
{
  static Int8U selected = 0;
  int min;
  
  if (is_signed || is_ip)
  {
    min = 0;
  }
  else
  {
    min = 1;
  }
  selected = min;
  
  while (1)
  {
    OLED_Screen(false);
    GUI_DrawHeader(caption);
    
    OLED_WriteStringInRectCenter_Emphasize(value, &MIDDLE_LINE, 0, 0, &Tahoma16, true, selected);

    load_GRAM_OLED((pU8)main_screen);
    
    Int8U key = OS_WaitSingleEventTimed(0xFF, KEY_WAIT_TIME);
    if (key == NO_KEY_PRESSED) continue;
    
    if (key == KEY_UP_SHORT)
    {
      Int8U exceed;
      ProceedUp(&value[selected], &exceed);
      
      if (exceed)
      {
        if ((!is_ip) && (selected == 1))
        {
          strinsert(value, '1', 1, 25);
          //selected++; ???
        }
        
        if ((is_ip) && (octet_size(value, selected) < 3))
        {
          strinsert(value, '1', selected, 25);
        }
      }

      continue;
    }
    
    if (key == KEY_DOWN_SHORT)
    {
      ProceedDown(&value[selected]);
      continue;
    }
    
    if (key == KEY_ENTER_SHORT)
    {
      selected++;
      if (value[selected] == '.')
      {
        selected++;
      }
      
      if (selected >= strlen(value))
      {
        selected = 0;
        return true;
      }
      
      continue;
    }
    
    if (key == KEY_CANCEL_SHORT)
    {
      if (selected <= min)
      {
        return false;
      }
      
      selected--;
      if (value[selected] == '.')
      {
        selected--;
      }
    }
  }
}

bool GUI_KeyboardInt(int* value, char* caption, bool is_signed, int min, int max)
{
  char _int[25];
  if (is_signed)
    sprintf(_int, "%+d", *value);
  else
    sprintf(_int, " %d", *value);
  if (!GUI_KeyboardChar(_int, caption, is_signed, false))
    return false;
  
  sscanf(_int, "%d", value);
  if ((min != 0) || (max != 0))
  {
    if (*value < min) *value = min;
    if (*value > max) *value = max;
  }
  return true;
}

bool GUI_KeyboardFloat(float* value, char* caption, bool is_signed, float min, float max, int decimal)
{
  char _float[25];
  if (is_signed)
    sprintf(_float, "%+.*f", decimal, *value);
  else
    sprintf(_float, " %.*f", decimal, *value);
  if (!GUI_KeyboardChar(_float, caption, is_signed, false))
    return false;
  
  sscanf(_float, "%f", value);
  if ((min != 0) || (max != 0))
  {
    if (*value < min) *value = min;
    if (*value > max) *value = max;
  }
  return true;
}

bool GUI_KeyboardIP(Int32U* value, char* caption)
{
  char _ip[25];
  parse_ip(value, _ip);
  if (!GUI_KeyboardChar(_ip, caption, false, true))
  {
    return false;
  }
  
  pack_ip(value, _ip);
  return true;
}

void GUI_ProgressBar(const RECT* rect, Int8U current_step, Int8U overall_steps)
{
  RECT rect_ = *rect;
  
  OLED_Rect(&rect_, true);
  rect_.sX++;
  rect_.eX--;
  rect_.sY++;
  rect_.eY--;
  OLED_Rect(&rect_, false);
  
  rect_.eX = (float)rect_.eX * (float)current_step/(float)overall_steps;
  OLED_Rect(&rect_, true);
}

void GUI_LoadingScreen(char* action, bool result, char* error_message, Int8U current_step, Int8U overall_steps)
{
  OLED_Screen(false);
  OLED_WriteStringInRectCenter("ПРОВЕРКА СИСТЕМЫ", &UPPERMOST_LINE, 0, 0, &Tahoma12, true);
  GUI_ProgressBar(&UPPER_LINE, current_step, overall_steps);
  
  OLED_WriteStringInRectCenter(action, &MIDDLE_LINE, 0, 0, &Tahoma12, true);
  if (result)
  {
    OLED_WriteStringInRectCenter("ПРОЙДЕН", &LOWER_LINE, 0, 0, &Tahoma12, true);
  }
  else
  {
    OLED_WriteStringInRectCenter("ОШИБКА", &LOWER_LINE, 0, 0, &Tahoma12, true);
    OLED_WriteStringInRectCenter(error_message, &LOWERMOST_LINE, 0, 0, &Tahoma12, true);
  }
  
  load_GRAM_OLED((pU8)main_screen);
}

// устанавливает функцию графического пользовательского интерфейса
void GUI_SetGUI(void (*screen_function)(void))
{
  if (screen_function)
  {
    Screen = screen_function;
  
    Screen();
    load_GRAM_OLED((pU8)main_screen);
  }
}

void GUI_SetMenu(MENU* menu)
{
  if (menu)
  {
    Menu   =  menu;
    Screen = &GUI_MenuMachine;
    
    Screen();
    load_GRAM_OLED((pU8)main_screen);
  }
}

void GUI_ReloadScreen()
{
  load_GRAM_OLED((pU8)main_screen);
}


// рисует заголовок
void GUI_DrawHeader(char* caption)
{
  //OLED_Rect(&HEADER, true);
  //GUI_DrawTime();
  
  if (caption)
  {
    OLED_Rect(&HEADER, true);
    OLED_WriteStringInRectLeft(caption, &CAPTION, 1, 0, &Tahoma12, false);
    GUI_DrawTime(false);
  }
  else
  {
    OLED_Rect(&HEADER, false);
    
    Display1[0] = 0;
    
    /*
    switch(ProtocolNumber)
    {
    case 0:       strcat(Display1, "\x00A6 ");
                  break;
                  
    case 1:       strcat(Display1, "\x00A8 ");
                  break;                                         
    }    
    */
    if (errors_present())
    {
      strcat(Display1, "\x00BB ");
    }
    
    if (system_flags.wifi_on || system_flags.gsm_on)
    {
      strcat(Display1, "\x0084 ");
    }
    
    if (system_flags.ethernet_on)
    {
      strcat(Display1, "\x00AB ");
    }
    /*
    if ( SavedCounter )
    {
      strcat(Display1, "\x00A7 ");
    }
    */
    if (system_flags.usb_on)
    {
      strcat(Display1, "\x00A4 ");
    }
    
    if (system_flags.mqtt_on)
    {
      strcat(Display1, "\x00A0 ");
    }
    
    OLED_WriteStringInRectLeft(Display1, &HEADER, 0, 0, &Tahoma12, true);
    
    GUI_DrawTime(true);
  }
}

// рисует время
void GUI_DrawTime(bool highlight)
{
  int y, m, d, h, mi, s;
  ParseDate(&Time, &s, &mi, &h, &d, &m, &y);
  sprintf(Display1, "%.2d:%.2d", h, mi);
  OLED_WriteStringInRectRight(Display1, &TIME, 0, 0, &Tahoma12, highlight);
}

void GUI_DrawBattery(const RECT* rect, Int8U power)
{
  RECT r = {rect->sX + 18, rect->eX - 18, rect->sY + 3, rect->eY};
  OLED_Rect(&r, true);

  r.sX += 1;
  r.eX -= 1;
  r.sY += 1;
  r.eY -= 1;
  OLED_Invert(&r);
  
  r.sX += 1;
  r.eX -= 1;
  r.sY += 1;
  r.eY -= 1;
  r.eX = r.sX + (r.eX - r.sX)*(power/100.);
  OLED_Rect(&r, true);
  
  r.sX = rect->eX - 18;
  r.eX = r.sX + 1;
  OLED_Rect(&r, true);
}

Int8U menu_size(MENU* menu)
{
  Int8U size = 0;
  
  const MENU_ITEM** menu_item = &menu->items[0];
  while (*menu_item != NULL)
  {
    if ((*menu_item)->visible) size++;
    menu_item++;
  }
  
  return size;
}

void menu_up(MENU* menu)
{
  menu->selected--;
  if (menu->selected > menu_size(menu) - 1)
  {
    menu->selected = menu_size(menu) - 1;
  }
}

void menu_down(MENU* menu)
{
  menu->selected++;
  if (menu->selected > menu_size(menu) - 1)
  {
    menu->selected = 0;
  }
}

void menu_enter(MENU* menu)
{
  if (menu->selected != 255)
  {
    U8 j = 0;
    for (U8 i = 0; i < menu->selected; )
    {
      if (!menu->items[j]->visible)   j++;
      else                          { j++; i++; }
    }
    
    while(!menu->items[j]->visible) j++;

    if (menu->items[j]->enter_click)
    {
      menu->items[j]->enter_click();
    }
  }
}

void menu_cancel(MENU* menu)
{
  if (menu->cancel_click)
  {
    menu->cancel_click();
  }
}


/* меню машина */
void GUI_MenuMachine()
{
  static U32  noPressTimer;
  
  //draw
  OLED_Screen(false);
  
  bool highlight;
  
  if (Menu->selected != 255)
  {
    GUI_DrawHeader(Menu->caption);
  }
  else
  {
    GUI_DrawHeader(NULL);
  }
  
  // Иницилизация меню (видимость элементов и т.д.)
  if (Menu->initialize)
  {
    Menu->initialize();
  }
  
  // Определение первого и последнего пунктов меню на экране
  U8 max_last = menu_size(Menu);
  if (max_last > 3)
  {
    max_last = 3;
  }
  
  if ((Menu->first == 0) && (Menu->last == 0))  // первая инициализация Меню
  {
    Menu->first = 0;
    Menu->last  = max_last;
  }
  
  if (Menu->selected != 255)           // 
  {
    if (Menu->selected < Menu->first)  // вверх по меню
    {
      Menu->first = Menu->selected;
      Menu->last = Menu->first + 3;
    }
    if (Menu->selected > Menu->last)  // вниз по меню
    {
      Menu->last = Menu->selected;
      Menu->first = Menu->last - 3;
    }
  }
  
  if (Menu->selected == 255)
  {
    Menu->first = 0;
    Menu->last  = max_last;
  }
  
  // Вывод куска меню, определённого выше
  RECT* rect_line;
  RECT* rect_left;
  RECT* rect_right;
  const MENU_ITEM** menu_item = &Menu->items[0];
  
  for (U8 i = 0; i < Menu->first; menu_item++)
  {
    if ((*menu_item)->visible)   i++;
  }
  
  U8 i = 0;
  
  while ((i < 4) && (*menu_item))
  {
    if ((*menu_item)->visible)
    {
      rect_line  = (RECT*)LINES[i + 1];
      rect_left  = (RECT*)LEFT_HALVES[i + 1];
      rect_right = (RECT*)RIGHT_THIRDS[i + 1];
      highlight  = !(Menu->selected == Menu->first + i);
      OLED_Rect(rect_line, !highlight);

      (*menu_item)->caption();
    
      if ((*menu_item)->style == MENU_ITEM_STYLE_CENTER)
      {
        OLED_WriteStringInRectCenter(Display1, rect_line, 0, 0, &Tahoma12, highlight);
      }
      
      if ((*menu_item)->style == MENU_ITEM_STYLE_SIDE)
      {
        OLED_WriteStringInRectLeft(Display1, rect_left, 1, 0, &Tahoma12, highlight);
        OLED_WriteStringInRectCenter(Display2, rect_right, 0, 0, &Tahoma12, highlight);
      }
      
      if (i < 3)
      {
        OLED_UnderlineRectShift(rect_line, 16, 0, 0, 1, true);
      }
      
      i++;
    }    
    menu_item++;
  }
  
  // Вывод на экран стрелок-подсказок (если необходимо)
  if (Menu->selected != 255)
  {
    if (Menu->first > 0)
    {
      highlight = !(Menu->selected == Menu->first);
      OLED_WriteStringInRectRight("\x008b", &UPPER_LINE, 0, 0, &Tahoma12, highlight);
    }
  
    if (Menu->last < menu_size(Menu) - 1)
    {
      highlight = !(Menu->selected == Menu->last);
      OLED_WriteStringInRectRight("\x009b", &LOWERMOST_LINE, 0, 0, &Tahoma12, highlight);
    }
  }
  
  //press
  Int8U key = OS_WaitSingleEventTimed(0xFF, KEY_WAIT_TIME);
  if (key == NO_KEY_PRESSED) 
  {
    noPressTimer++;
    if( noPressTimer * KEY_WAIT_TIME > KEY_NOPRESS_TIMEOUT )       menu_cancel(Menu);
    if( noPressTimer * KEY_WAIT_TIME > KEY_NOPRESS_SCREEN_SAVER ) 
    {
      GUI_SetGUI ( GUI_ScreenSaver );
      noPressTimer = 0;
    }      
    return;
  }
  else noPressTimer = 0;
  
  if (key == KEY_UP_SHORT)
  {
    menu_up(Menu);
    return;
  }
  
  if (key == KEY_DOWN_SHORT)
  {
    menu_down(Menu);
    return;
  }
  
  if (key == KEY_ENTER_SHORT)
  {
    menu_enter(Menu);
    return;
  }
  
  if (key == KEY_CANCEL_SHORT)
  {
    menu_cancel(Menu);
    return;
  }
}
/* меню машина */

// показывает экран сохранения настроек и перезапускает модем
void  SaveReloadScreenHWR(void)
{
      
      OLED_Screen(0);
    
      OLED_WriteStringInRectCenter("СОХРАНЕНИЕ НАСТРОЕК", &UPPERMOST_LINE, 0, 0, &Tahoma12, true);
      OLED_WriteStringInRectCenter("    ВЫПОЛНЕНО!     ", &UPPER_LINE, 0, 0, &Tahoma12, true);
      load_GRAM_OLED((pU8)main_screen);
      OS_Delay(2000);
    
      OLED_WriteStringInRectCenter("ПЕРЕЗАПУСК МОДЕМА", &LOWER_LINE, 0, 0, &Tahoma12, true);
      OLED_WriteStringInRectCenter(" ВЫПОЛНЯЕТСЯ ... ", &LOWERMOST_LINE, 0, 0, &Tahoma12, true);
      load_GRAM_OLED((pU8)main_screen);
      OS_Delay(2000);      
    
      if( IS_USB_CONNECT )
      {
        USB_DeInit();
        OS_Delay(200);
      }
      HW_RESET;
}

// показывает приветственный экран с логотипом
void GUI_LogoScreen()
{
  sprintf(Display1," РМ-2-L Версия ПО %s ", Version);
  
  OLED_Screen(true);
  OLED_DrawImageInRectCenter(&Logo, &SCREEN, false);
  OLED_WriteStringInRectCenter(Display1, &LOWERMOST_LINE, 0, 0, &Tahoma12, true);
  load_GRAM_OLED((pU8)main_screen);
  OS_Delay(3000);
}

// список приборов на связи - для просмотра
void GUI_RadioListBrowser()
{
  static U8             selected = 255;
  static U32            noPressTimer;
  
  //draw
  OLED_Screen(false);
  
  GUI_DrawHeader("Приборы на связи");

  bool highlight = true;

  if (CurrentCounter == 0)
  {
    OLED_WriteStringInRectCenter("Нет приборов", &MIDDLE_LINE, 0, 0, &Tahoma12, highlight);
    OLED_WriteStringInRectCenter("на связи", &LOWER_LINE, 0, 0, &Tahoma12, highlight);
  }
  
  if (CurrentCounter != 0)
  {
    if (selected == 255)
    {
      selected = get_next_device_on_air(255);
    }
    
    sprintf(Display1, "Прибор #%d", CurrentList[selected].Adr);
    OLED_WriteStringInRectCenter(Display1, &UPPER_LINE, 0, 0, &Tahoma12, highlight);
    if (get_previous_device_on_air(selected) < selected)
      OLED_WriteStringInRectCenter("<<", &UPPERLEFT_THIRD, 0, 0, &Tahoma12, highlight);
    if (get_next_device_on_air(selected) > selected)
      OLED_WriteStringInRectCenter(">>", &UPPERRIGHT_THIRD, 0, 0, &Tahoma12, highlight);
    OLED_UnderlineRectShift(&UPPER_LINE, 8, 0, 0, 1, highlight);
    
    if (CurrentList[selected].Errors & (1 << 0))
    {
      sprintf(Display1, "--- °C");
    }
    else
    {
      sprintf(Display1, "%.1f °C", (float)CurrentList[selected].Tempr/10);
    }

    if (CurrentList[selected].Errors & (1 << 1))
    {
      sprintf(Display2, "--- %%");
    }
    else
    {
      sprintf(Display2, "%d %%", CurrentList[selected].Humidy);
    }

    if (CurrentList[selected].Errors & (1 << 2))
    {
      sprintf(Display3, "--- ммРт");
    }
    else
    {
      sprintf(Display3, "%d ммРт", CurrentList[selected].Pressure);
    }
    
    if (CurrentList[selected].Errors & (1 << 7))
    {
      OLED_WriteStringInRectCenter(Display1, &MIDDLELEFT_THIRD, 0, 0, &Tahoma12, highlight);
      OLED_WriteStringInRectCenter(Display2, &MIDDLECENTER_THIRD, 0, 0, &Tahoma12, highlight);
      OLED_WriteStringInRectCenter(Display3, &MIDDLERIGHT_THIRD, 0, 0, &Tahoma12, highlight);
    }
    else
    {
      OLED_WriteStringInRectCenter(Display1, &MIDDLELEFT_HALF, 0, 0, &Tahoma12, highlight);
      OLED_WriteStringInRectCenter(Display2, &MIDDLERIGHT_HALF, 0, 0, &Tahoma12, highlight);
    }
    
    GUI_DrawBattery(&LOWERLEFT_HALF, CurrentList[selected].PowLev);

#if     defined(__V2XX__)    
    sprintf(Display1, "%d дБм", RSSI[selected]);
#elif   defined(__V3XX__)
    sprintf(Display1, "%d дБм", CurrentList[selected].RSSI);    
#else
  #error  "Version Not defined!"
#endif
    
    OLED_WriteStringInRectCenter(Display1, &LOWERRIGHT_HALF, 0, 0, &Tahoma12, highlight);
  
    int y, m, d, h, mi, s;
    ParseDate(&CurrentList[selected].Time, &s, &mi, &h, &d, &m, &y);
    sprintf(Display1, "%.2d:%.2d:%.2d", h, mi, s);
#if     defined(__V2XX__)     
    sprintf(Display2, "Был на связи %s", Display1);
#elif   defined(__V3XX__)
    sprintf(Display2, "%s  %s", DeviceType[CurrentList[selected].Type], Display1); 
#else
  #error  "Version Not defined!"
#endif      
    OLED_WriteStringInRectCenter(Display2, &LOWERMOST_LINE, 0, 0, &Tahoma12, highlight);
  }
  
  //press
  Int8U key = OS_WaitSingleEventTimed(0xFF, KEY_WAIT_TIME);
  if (key == NO_KEY_PRESSED) 
  {
    noPressTimer++;
    if( noPressTimer * KEY_WAIT_TIME > KEY_NOPRESS_TIMEOUT )      key = KEY_CANCEL_SHORT;
  }
  else noPressTimer = 0;
  
  if (key == KEY_UP_SHORT)
  {
    selected = get_next_device_on_air(selected);
    return;
  }
  
  if (key == KEY_DOWN_SHORT)
  {
    selected = get_previous_device_on_air(selected);
    return;
  }
  
  if (key == KEY_ENTER_SHORT)
  {
    return;
  }
  
  if (key == KEY_CANCEL_SHORT)
  {
    selected = 255;
    GUI_SetMenu(&MainMenu);
    return;
  }
}

void GUI_ScreenSaver(void)
{
  static S16 sX   = 64;
  static S16 sY   = 0;
  static S16 sigX = 1, sigY = 1;
  char   sym = '*';
  
  OLED_Screen(false);
  
  for (U16 i = 0; i < Tahoma12.length; i++)
  {
    if ( sym == Tahoma12.chars[i].code) 
    {
      OLED_Char(&Tahoma12.chars[i], sX, sY, true);
        sX += (S16)(Tahoma12.chars[i].image->width/2 * sigX);    
          sY += (S16)(Tahoma12.chars[i].image->height/4 * sigY);
          if ( (sX + Tahoma12.chars[i].image->width) > RESOLUTION_X - 1 ) { sigX = -sigX; sX = RESOLUTION_X - 1 - Tahoma12.chars[i].image->width; }
          if ( ( sX < 0 ) )                                               { sigX = -sigX; sX = 0; }
        if ( (sY + Tahoma12.chars[i].image->height) > RESOLUTION_Y - 1 )  { sigY = -sigY; sY = RESOLUTION_Y - 1 - Tahoma12.chars[i].image->height; }
        if ( ( sY < 0 ) )                                                 { sigY = -sigY; sY = 0; }
    }
  }
  
  Int8U key = OS_WaitSingleEventTimed(0xFF, 100);
  if (key == NO_KEY_PRESSED) return;
  
  GUI_SetMenu(&MainMenu);      
}

// Экран вывода состояния модема: ошибки, версия, серийный номер
void GUI_Information()
{
  static Int8U selected = 0;  
  static Int8U first    = 0;
  static Int8U last     = 3;
  
  //draw
  OLED_Screen(false);
  
  GUI_DrawHeader("Состояние прибора");
  
  char* list[6];
  memset(list, 0, 6 * sizeof(char*));
  
  Int8U size = 2;
  if (errors_flags.timeinvalid)
  {
    list[size - 2] = OS_malloc(20);
    sprintf(list[size - 2], "  Сброшены часы    ");
    size++;
  }
  else if (errors_flags.TimeSyncReq)
  {
    list[size - 2] = OS_malloc(20);
    sprintf(list[size - 2], " Неактульное время ");
    size++;
  }  
  
  if (errors_flags.wifi_fail)
  {
    list[size - 2] = OS_malloc(20);
    sprintf(list[size - 2], "Ошибка перед. WiFi");
    size++;
  }
  if (errors_flags.gsm_fail)
  {
    list[size - 2] = OS_malloc(20);
    sprintf(list[size - 2], "Ошибка перед. GSM");
    size++;
  }  
  if (errors_flags.radio_fail)
  {
    list[size - 2] = OS_malloc(20);
    sprintf(list[size - 2], "Ошибка передат. РМ");
    size++;
  }
  if (errors_flags.currentMemEnd)
  {
    list[size - 2] = OS_malloc(20);
    sprintf(list[size - 2], "Много передатчиков ");
    size++;
  }
  if (errors_flags.savedMemEnd)
  {
    list[size - 2] = OS_malloc(20);
    sprintf(list[size - 2], "Переполнение буфера");
    size++;
  }  
  if (errors_flags.LSEfail)
  {
    list[size - 2] = OS_malloc(20);
    sprintf(list[size - 2], "RTC генер. неиспр.");
    size++;
  }  
  
  if (last > size) last = size;

  if (selected < first)
  {
    first = selected;
    last = first + 3;
  }
  if (selected > last)
  {
    last = selected;
    first = last - 3;
  }

  bool highlight;
  RECT* rect_line;
  RECT* rect_left;
  RECT* rect_right;
  for (U8 i = first; i <= last; i++)
  {
    rect_line = (RECT*)LINES[i - first + 1];
    rect_left = (RECT*)LEFT_HALVES[i - first + 1];
    rect_right = (RECT*)RIGHT_THIRDS[i - first + 1];
    highlight = !(selected == i);
    OLED_Rect(rect_line, !highlight);
    
    if (i == 0)
    {
      OLED_WriteStringInRectLeft("Тех. номер", rect_left, 1, 0, &Tahoma12, highlight);
      sprintf(Display2, "%s", SerialNumber);
      OLED_WriteStringInRectCenter(Display2, rect_right, 0, 0, &Tahoma12, highlight);
      OLED_UnderlineRectShift(rect_line, 16, 0, 0, 1, true);
    }
    
    else if (i == 1)
    {
      OLED_WriteStringInRectLeft("Версия ПО", rect_left, 1, 0, &Tahoma12, highlight);
      sprintf(Display2, "%s", Version);
      OLED_WriteStringInRectCenter(Display2, rect_right, 0, 0, &Tahoma12, highlight);
      OLED_UnderlineRectShift(rect_line, 16, 0, 0, 1, true);
    }
    
    else
    {
      U8 j = i - 2;
      if ((list[j]) && (*list[j] != 0))
      {
        OLED_WriteStringInRectCenter(list[j], rect_line, 0, 0, &Tahoma12, highlight);
        OLED_UnderlineRectShift(rect_line, 16, 0, 0, 1, true);        
      }
    }
  }
  
  U8 j = 0;
  while (list[j])
  {
    OS_free(list[j]);
    list[j] = NULL;
    j++;
  }

  if (first > 0)
  {
    highlight = !(selected == first);
    OLED_WriteStringInRectRight("\x008b", &UPPER_LINE, 0, 0, &Tahoma12, highlight);
  }
  
  if (last < size - 1)
  {
    highlight = !(selected == last);
    OLED_WriteStringInRectRight("\x009b", &LOWERMOST_LINE, 0, 0, &Tahoma12, highlight);
  }
  
  //press
  Int8U key = OS_WaitSingleEventTimed(0xFF, KEY_WAIT_TIME);
  if (key == NO_KEY_PRESSED) return;
  
  if (key == KEY_UP_SHORT)
  {
    selected--;
    if (selected > size - 1) selected = size - 1;
    return;
  }
  
  if (key == KEY_DOWN_SHORT)
  {
    selected++;
    if (selected > size - 1) selected = 0;
    return;
  }
  
  if (key == KEY_ENTER_SHORT)
  {
    return;
  }
  
  if (key == KEY_CANCEL_SHORT)
  {
    selected = 0;
    GUI_SetMenu(&MainMenu);
    return;
  }
}


/* Экран настроек RS: начало */
void CAPTION_RSAddr()
{
  sprintf(Display1, "Адрес прибора");
  sprintf(Display2, "%d", RsAdr);
}

void ENTER_RSAddr()
{
  int value = RsAdr;
  if (GUI_KeyboardInt(&value, "Адрес прибора", false, 1, 128))
  {
    OS_Use(&SemaRAM);
    RsAdr = value;
    UARTMenu.changed = 1;
    OS_Unuse(&SemaRAM);
  }
}

void CAPTION_RSSpeed()
{
  sprintf(Display1, "Скорость связи");
  sprintf(Display2, "%d", RsSpeed);
}

void ENTER_RSSpeed()
{
  OS_Use(&SemaRAM);
  increment_speed(&RsSpeed);
  UARTMenu.changed = 1;
  OS_Unuse(&SemaRAM);
  
  changeRSspeed();
}

const MENU_ITEM MI_RSAddr  = {&CAPTION_RSAddr,  &ENTER_RSAddr,  MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_SIDE};
const MENU_ITEM MI_RSSpeed = {&CAPTION_RSSpeed, &ENTER_RSSpeed, MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_SIDE};

const MENU_ITEM* UARTMenu_items[] = {&MI_RSAddr, &MI_RSSpeed, NULL};

void UARTMenu_cancel()
{
  if( UARTMenu.changed )
  {
    OS_Use(&SemaRAM);
      writeConfigFlashWithCHK();      
    OS_Unuse(&SemaRAM);
  }
  UARTMenu.selected = UARTMenu.changed = 0;
  GUI_SetMenu(&MainMenu);
}

MENU UARTMenu = {"Связь по RS", UARTMenu_items, 0, 0, 0, &UARTMenu_cancel, NULL};
/* Экран настроек RS: окончание */

/* Экран настроек Ethernet: начало */
extern MENU EthernetMenu;

void CAPTION_NetName()
{
  char  hostName[32];
  IP_GetHostName(hostName);
  sprintf(Display1, "Имя");
  sprintf(Display2, "%s", hostName);
}

void CAPTION_MAC()
{
  Int8U MAC[6];
  IP_GetHWAddr(0, MAC, 6);
  sprintf(Display1, "MAC");
  sprintf(Display2, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5]);
}

void CAPTION_EthDHCP()
{
  sprintf(Display1, "DHCP");
  sprintf(Display2, "%s", TCPIPConfig.UseDHCP ? "Да" : "Нет");
}

void ENTER_EthDHCP()
{
  OS_Use(&SemaRAM);
  TCPIPConfig.UseDHCP = !TCPIPConfig.UseDHCP;
  EthernetMenu.changed = 1;
  OS_Unuse(&SemaRAM);
}

void CAPTION_EthIP()
{
  Int32U ip;
  
  if (!TCPIPConfig.UseDHCP)
  {
    ip = TCPIPConfig.IP;
  }
  else
  {
    ip = IP_GetIPAddr(0);
  }
  sprintf(Display1, "IP");
  parse_ip(&ip, Display2);
}

void ENTER_EthIP()
{
  if (!TCPIPConfig.UseDHCP)
  {
    Int32U value = TCPIPConfig.IP;
    if (GUI_KeyboardIP(&value, "IP-адрес"))
    {
      OS_Use(&SemaRAM);
      TCPIPConfig.IP = value;
      EthernetMenu.changed = 1;
      OS_Unuse(&SemaRAM);
    }
  }
}

void CAPTION_EthMask()
{
  Int32U mask;
  if (!TCPIPConfig.UseDHCP)
  {
    mask = TCPIPConfig.Mask;
  }
  else
  {
    mask = IP_GetAddrMask(0);
  }
  sprintf(Display1, "Маска");
  parse_ip(&mask, Display2);
}

void ENTER_EthMask()
{
  if (!TCPIPConfig.UseDHCP)
  {
    Int32U value = TCPIPConfig.Mask;
    if (GUI_KeyboardIP(&value, "Маска подсети"))
    {
      OS_Use(&SemaRAM);
      TCPIPConfig.Mask = value;
      EthernetMenu.changed = 1;
      OS_Unuse(&SemaRAM);
    }
  }
}

void CAPTION_EthGate()
{
  Int32U gate;
  if (!TCPIPConfig.UseDHCP)
  {
    gate = TCPIPConfig.Gate;
  }
  else
  {
    gate = IP_GetGWAddr(0);
  }
  sprintf(Display1, "Шлюз");
  parse_ip(&gate, Display2);
}

void ENTER_EthGate()
{
  if (!TCPIPConfig.UseDHCP)
  {
    Int32U value = TCPIPConfig.Gate;
    if (GUI_KeyboardIP(&value, "Шлюз"))
    {
      OS_Use(&SemaRAM);
      TCPIPConfig.Gate = value;
      EthernetMenu.changed = 1;
      OS_Unuse(&SemaRAM);
    }
  }
}

const MENU_ITEM MI_NetName    = {&CAPTION_NetName,    NULL,           MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_SIDE};
const MENU_ITEM MI_MAC        = {&CAPTION_MAC,        NULL,           MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_SIDE};
const MENU_ITEM MI_EthDHCP    = {&CAPTION_EthDHCP,    &ENTER_EthDHCP, MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_SIDE};
const MENU_ITEM MI_EthIP      = {&CAPTION_EthIP,      &ENTER_EthIP,   MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_SIDE};
const MENU_ITEM MI_EthMask    = {&CAPTION_EthMask,    &ENTER_EthMask, MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_SIDE};
const MENU_ITEM MI_EthGate    = {&CAPTION_EthGate,    &ENTER_EthGate, MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_SIDE};
const MENU_ITEM* EthernetMenu_items[] = {&MI_NetName, &MI_MAC, &MI_EthDHCP, &MI_EthIP, &MI_EthMask, &MI_EthGate, NULL};

void EthernetMenu_cancel()
{
  if( EthernetMenu.changed )
  {
    OS_Use(&SemaRAM);
      writeConfigFlashWithCHK();      
    OS_Unuse(&SemaRAM);
    Initialize_TCPIP_Config(0);
  }
  
  EthernetMenu.selected = EthernetMenu.changed = 0;
  GUI_SetMenu(&MainMenu);
}

MENU EthernetMenu = {"Связь по Ethernet", EthernetMenu_items, 0, 0, 0, &EthernetMenu_cancel, NULL};
/* Экран настроек Ethernet: окончание */

/* Экран настроек WiFi: начало */
extern MENU WiFiMenu;

void CAPTION_WiFiProt()
{
  sprintf(Display1, "Prot: %s", WiFiConfig.modbus ? "ModBus" : "Eksis");
}

void ENTER_WiFiProt()
{
  OS_Use(&SemaRAM);
  WiFiConfig.modbus = !WiFiConfig.modbus;
  WiFiMenu.changed = 1;
  OS_Unuse(&SemaRAM);  
}

void CAPTION_WiFiEnabled()
{
  sprintf(Display1, "WiFi");
  sprintf(Display2, "%s", WiFiConfig.wifi_use ? "включен" : "выключен");
}

void ENTER_WiFiEnabled()
{
  OS_Use(&SemaRAM);
    WiFiConfig.wifi_use = !WiFiConfig.wifi_use;
    WiFiMenu.changed = 1;
    WiFiMenu.first = WiFiMenu.last = 0;
  OS_Unuse(&SemaRAM);  
}

void CAPTION_AccessPoint()
{
  WiFiConfig.NetName[63] = 0;
  if (WiFiConfig.NetName[0] == 0)
  {
    sprintf(Display1, "Log: <Имя сети не задано>");
  }
  else
  {
    sprintf(Display1, "Log: %s", WiFiConfig.NetName);
  }
}

const MENU_ITEM MI_WiFiEnabled = {&CAPTION_WiFiEnabled, &ENTER_WiFiEnabled, MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_SIDE};
      MENU_ITEM MI_AccessPoint = {&CAPTION_AccessPoint,  NULL,              MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_CENTER};
      MENU_ITEM MI_WiFiProt    = {&CAPTION_WiFiProt,    &ENTER_WiFiProt,    MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_CENTER};

const MENU_ITEM* WiFiMenu_items[] = {&MI_WiFiEnabled, &MI_AccessPoint, &MI_WiFiProt, NULL};

void WiFiMenu_cancel()
{
  if( WiFiMenu.changed )
  {
    OS_Use(&SemaRAM);
      writeConfigFlashWithCHK();
      system_flags.reiniwifi = 1;
      OS_EVENT_Set( &Uart2RXComplete);
    OS_Unuse(&SemaRAM);
  }
  WiFiMenu.selected = WiFiMenu.changed = 0;
  GUI_SetMenu(&MainMenu);
}

#pragma optimize=none
void WiFiMenu_initialize()
{
  MI_AccessPoint.visible  = WiFiConfig.wifi_use ? MENU_ITEM_VISIBLE : MENU_ITEM_HIDDEN;
  MI_WiFiProt.visible     = WiFiConfig.wifi_use ? MENU_ITEM_VISIBLE : MENU_ITEM_HIDDEN;
}

MENU WiFiMenu = {"Связь по WiFi", WiFiMenu_items, 0, 0, 0, &WiFiMenu_cancel, &WiFiMenu_initialize};
/* Экран настроек WiFi: окончание */


/* Экран настроек MQTT: начало */
extern MENU             MQTTMenu;

void CAPTION_MQTTEnabled()
{
  sprintf(Display1, "MQTT");
  sprintf(Display2, "%s", MQTTConfig.mqtt_use ? "включено" : "выключено");
}

void ENTER_MQTTEnabled()
{
  OS_Use(&SemaRAM);
    MQTTConfig.mqtt_use = !MQTTConfig.mqtt_use;    
    MQTTMenu.changed    = 1;
    MQTTMenu.first = MQTTMenu.last = 0;
  OS_Unuse(&SemaRAM);  
}

void CAPTION_ServerIP()
{  
  sprintf(Display1, "%s", MQTTConfig.SeverHostName);
  Display2[0] = 0;
}

void CAPTION_ServerPort()
{  
  sprintf(Display1, "MQTT Port");
  sprintf(Display2, "%d", MQTTConfig.ServerPort);
}

void ENTER_ServerPort()
{
  int value = MQTTConfig.ServerPort;
  if (GUI_KeyboardInt(&value, "MQTT Port", false, 1024, 65535 ))
  {
    OS_Use(&SemaRAM);
      MQTTConfig.ServerPort = value;
      MQTTMenu.changed      = 1;
    OS_Unuse(&SemaRAM);
  }
}

void CAPTION_MQTTQos()
{  
  sprintf(Display1, "MQTT Qos");
  sprintf(Display2, "%d", MQTTConfig.mqtt_qos);
}

void ENTER_MQTTQos()
{
  MQTTMenu.changed      = 1;
  MQTTConfig.mqtt_qos++;
  if( MQTTConfig.mqtt_qos > 2 ) MQTTConfig.mqtt_qos = 0;  
}

void CAPTION_MQTTInterface()
{  
  sprintf(Display1, "MQTT Interface");
  switch( MQTTConfig.mqtt_interface )
  {
    case MQTT_THR_ETH:  sprintf(Display2, "Ethernet"); break;
    case MQTT_THR_GSM:  sprintf(Display2, "GSM");      break;
    case MQTT_THR_WIFI: sprintf(Display2, "WiFi");     break;
  }    
}

void ENTER_MQTTInterface()
{  
  for( MQTTMenu.changed = 1;;) 
  {
    MQTTConfig.mqtt_interface++;
    if( MQTTConfig.mqtt_interface > 2 ) MQTTConfig.mqtt_interface = 0;
    switch( MQTTConfig.mqtt_interface )
    {
      case MQTT_THR_ETH:  return;
      case MQTT_THR_GSM:  if( GSMConfig.gsm_present )    return;
                          else break;      
      case MQTT_THR_WIFI: // if( WiFiConfig.wifi_present )  return; пока не сделано!
                          // else break;
                          break;
    }
  } 
}

const MENU_ITEM MI_MQTTEnabled = {&CAPTION_MQTTEnabled,   &ENTER_MQTTEnabled,   MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_SIDE};
      MENU_ITEM MI_ServerIP    = {&CAPTION_ServerIP,       NULL,                MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_CENTER};
      MENU_ITEM MI_ServerPort  = {&CAPTION_ServerPort,    &ENTER_ServerPort,    MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_SIDE};
      MENU_ITEM MI_MQTTQos     = {&CAPTION_MQTTQos,       &ENTER_MQTTQos,       MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_SIDE};      
      MENU_ITEM MI_MQTTInerface= {&CAPTION_MQTTInterface, &ENTER_MQTTInterface, MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_SIDE};            
      
const MENU_ITEM* MQTTMenu_items[] = {&MI_MQTTEnabled, &MI_ServerIP, &MI_ServerPort, &MI_MQTTQos, &MI_MQTTInerface, NULL};

void MQTTMenu_cancel()
{
  MQTTMenu.selected = 0;
  if( MQTTMenu.changed )
  {
    OS_Use(&SemaRAM);
      writeConfigFlashWithCHK();
      system_flags.mqtt_ini = 1;
    OS_Unuse(&SemaRAM);
  }
  MQTTMenu.selected = MQTTMenu.changed = 0;
  GUI_SetMenu(&MainMenu);
}

#pragma optimize=none
void MQTTMenu_initialize()
{
  MI_ServerIP.visible = MI_ServerPort.visible = MI_MQTTQos.visible =  MI_MQTTInerface.visible = MQTTConfig.mqtt_use ? MENU_ITEM_VISIBLE : MENU_ITEM_HIDDEN;  
}

MENU MQTTMenu = {"Настройки MQTT", MQTTMenu_items, 0, 0, 0, &MQTTMenu_cancel, &MQTTMenu_initialize};
/* Экран настроек MQTT: окончание */


/* Экран настроек даты и времени: начало */
extern MENU             DateTimeMenu;

void CAPTION_NTP()
{
  sprintf(Display1, "NTP");
  sprintf(Display2, "%s", NTPConfig.ntpuse ? "включено" : "выключено");
}

void ENTER_NTPUse()
{
  NTPConfig.ntpuse      = !NTPConfig.ntpuse;    
  DateTimeMenu.changed  = 1;
  DateTimeMenu.first    = DateTimeMenu.last = 0;
  
}

void CAPTION_NTPServer()
{  
  sprintf(Display1, "%s", NTPConfig.NTPSeverName);
  Display2[0] = 0;
}

void CAPTION_NTPTimeZ()
{  
  sprintf(Display1, "Часовой пояс");
  sprintf(Display2, "%d", NTPConfig.TimeZone);
}

void ENTER_NTPTimeZ()
{
  int value = NTPConfig.TimeZone;
  if (GUI_KeyboardInt(&value, "Часовой пояс", false, -11, 11 ))
  {
    NTPConfig.TimeZone    = value;
    DateTimeMenu.changed  = 1;
  }
}
  

void CAPTION_Year()
{
  int y, m, d, h, mi, s;
  ParseDate(&TempTime, &s, &mi, &h, &d, &m, &y);
  sprintf(Display1, "Год");
  sprintf(Display2, "%d", y);
}

void ENTER_Year()
{
  int y, m, d, h, mi, s;
  ParseDate(&TempTime, &s, &mi, &h, &d, &m, &y);
  if (GUI_KeyboardInt(&y, "Год", false, 2020, 2100))
  {
    s = 0;
    TempTime = EncodeDate(s, mi, h, d, m, y);
    DateTimeMenu.changed = 2;
  }
}

void CAPTION_Month()
{
  int y, m, d, h, mi, s;
  ParseDate(&TempTime, &s, &mi, &h, &d, &m, &y);
  sprintf(Display1, "Месяц");
  sprintf(Display2, "%d", m);
}

void ENTER_Month()
{
  int y, m, d, h, mi, s;
  ParseDate(&TempTime, &s, &mi, &h, &d, &m, &y);
  if (GUI_KeyboardInt(&m, "Месяц", false, 1, 12))
  {
    s = 0;
    TempTime = EncodeDate(s, mi, h, d, m, y);
    DateTimeMenu.changed = 2;
  }
}

void CAPTION_Day()
{
  int y, m, d, h, mi, s;
  ParseDate(&TempTime, &s, &mi, &h, &d, &m, &y);
  sprintf(Display1, "День");
  sprintf(Display2, "%d", d);
}

void ENTER_Day()
{
  int y, m, d, h, mi, s;
  ParseDate(&TempTime, &s, &mi, &h, &d, &m, &y);
  if (GUI_KeyboardInt(&d, "День", false, 1, 31))
  {
    s = 0;
    TempTime = EncodeDate(s, mi, h, d, m, y);
    DateTimeMenu.changed = 2;
  }
}

void CAPTION_Hour()
{
  int y, m, d, h, mi, s;
  ParseDate(&TempTime, &s, &mi, &h, &d, &m, &y);
  sprintf(Display1, "Часы");
  sprintf(Display2, "%d", h);
}

void ENTER_Hour()
{
  int y, m, d, h, mi, s;
  ParseDate(&TempTime, &s, &mi, &h, &d, &m, &y);
  if (GUI_KeyboardInt(&h, "Часы", false, 0, 23))
  {
    s = 0;
    TempTime = EncodeDate(s, mi, h, d, m, y);
    DateTimeMenu.changed = 2;
  }
}

void CAPTION_Minute()
{
  int y, m, d, h, mi, s;
  ParseDate(&TempTime, &s, &mi, &h, &d, &m, &y);
  sprintf(Display1, "Минуты");
  sprintf(Display2, "%d", mi);
}

void ENTER_Minute()
{
  int y, m, d, h, mi, s;
  ParseDate(&TempTime, &s, &mi, &h, &d, &m, &y);
  if (GUI_KeyboardInt(&mi, "Минуты", false, 0, 59))
  {
    s = 0;
    TempTime = EncodeDate(s, mi, h, d, m, y);
    DateTimeMenu.changed = 2;
  }
}

const MENU_ITEM MI_NTP        = {&CAPTION_NTP,        &ENTER_NTPUse,      MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_SIDE};
      MENU_ITEM MI_NTPServer  = {&CAPTION_NTPServer,   NULL,              MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_CENTER};
      MENU_ITEM MI_NTPTimeZ   = {&CAPTION_NTPTimeZ,   &ENTER_NTPTimeZ,    MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_SIDE};
      MENU_ITEM MI_Year       = {&CAPTION_Year,       &ENTER_Year,        MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_SIDE};
      MENU_ITEM MI_Month      = {&CAPTION_Month,      &ENTER_Month,       MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_SIDE};
      MENU_ITEM MI_Day        = {&CAPTION_Day,        &ENTER_Day,         MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_SIDE};
      MENU_ITEM MI_Hour       = {&CAPTION_Hour,       &ENTER_Hour,        MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_SIDE};
      MENU_ITEM MI_Minute     = {&CAPTION_Minute,     &ENTER_Minute,      MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_SIDE};
      
const MENU_ITEM* DateTimeMenu_items[] = { &MI_NTP, &MI_NTPServer, &MI_NTPTimeZ, &MI_Year, &MI_Month, &MI_Day, &MI_Hour, &MI_Minute, NULL };

void DateTimeMenu_cancel()
{
  if( DateTimeMenu.changed == 1 )
  {
    OS_Use(&SemaRAM);
      writeConfigFlashWithCHK();
    OS_Unuse(&SemaRAM);
  }
  
  if( DateTimeMenu.changed == 2 )
  {
    OS_Use(&SemaRAM);
    Time = TempTime;
    RTC_SetCounter( Time ); 
    
    errors_flags.TimeSyncReq = errors_flags.timeinvalid = 0; 
    set_bkp_reg( ERR_BASE, 0);
    set_bkp_reg( TIMESYNC_BASE,     Time & 0xffff);
    set_bkp_reg( TIMESYNC_BASE + 1, Time >> 16);
    OS_Unuse(&SemaRAM);
  }
  
  DateTimeMenu.changed = DateTimeMenu.selected = 0;
  GUI_SetMenu(&MainMenu);
}

#pragma optimize=none
void DateTimeMenu_initialize()
{
  MI_Year.visible      = MI_Month.visible = MI_Day.visible =  MI_Hour.visible = MI_Minute.visible = NTPConfig.ntpuse ? MENU_ITEM_HIDDEN : MENU_ITEM_VISIBLE;
  MI_NTPServer.visible = MI_NTPTimeZ.visible = NTPConfig.ntpuse ? MENU_ITEM_VISIBLE : MENU_ITEM_HIDDEN;
}

MENU DateTimeMenu = {"Дата и время", DateTimeMenu_items, 0, 0, 0, &DateTimeMenu_cancel, &DateTimeMenu_initialize };
/* Экран настроек даты и времени: окончание */

/* Экран выбора протокола */
void CAPTION_ProtChoose()
{
  switch(ProtocolNumber)
  {
    case 0:       sprintf(Display1, "1 - односторонний"); sprintf(Display2, "");
                  break;
                  
    case 1:       sprintf(Display1, "2 - двусторонний"); sprintf(Display2, "");
                  break;                                         
  }
}

void ENTER_ProtChoose()
{
  OS_Use(&SemaRAM);
   ProtocolNumber ^= 1;
  OS_Unuse(&SemaRAM);  
}

const MENU_ITEM MI_ProtChoose     = {&CAPTION_ProtChoose, &ENTER_ProtChoose, MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_CENTER};
const MENU_ITEM* ProtMenu_items[] = {&MI_ProtChoose, NULL};

void ProtMenu_cancel()
{  
  if( TempTime != ProtocolNumber) 
  {
    OS_Use(&SemaRAM);
      writeConfigFlashWithCHK();      
    OS_Unuse(&SemaRAM);
    
    SaveReloadScreenHWR();
  } 
  else  GUI_SetMenu(&MainMenu);
}

MENU ProtMenu = {"Выбор протокола", ProtMenu_items, 0, 0, 0, &ProtMenu_cancel, NULL };
/* Экран выбора протокола */

/* Главный экран: начало */
void CAPTION_DevicesOnAir()
{
  sprintf(Display1, "Приборов на связи: %d", CurrentCounter);
}

void ENTER_DevicesOnAir()
{
  GUI_SetGUI(&GUI_RadioListBrowser);
}

void CAPTION_RxChannel()
{
  sprintf(Display1, "Приём: %2d к, %1d с", RadioChannelModem, BW_MODEM + 1);
}

void ENTER_RxChannel()
{
  int value = RadioChannelModem;
  if (GUI_KeyboardInt(&value, "Канал приёма", false, 1, 15))
  {
    OS_Use(&SemaRAM);
    RadioChannelModem = value;
    writeConfigFlashWithCHK();
    CurrentCounter = SavedCounter = 0;
    memset((void *) &CurrentList[0], 0, sizeof(CurrentList) );
    memset((void *) &SavedList[0], 0, sizeof(SavedList) );    
    OS_Unuse(&SemaRAM);
    OS_SignalEvent(REBAND_RADIO, &OS_F868);    
  }

  value = BW_MODEM + 1;
  if (GUI_KeyboardInt(&value, "Скорость приёма", false, 1, 4))
  {
    OS_Use(&SemaRAM);
    RadioBW = (RadioBW & ~0x03) + (value - 1);
    writeConfigFlashWithCHK();
    CurrentCounter = SavedCounter = 0;
    memset((void *) &CurrentList[0], 0, sizeof(CurrentList) );
    memset((void *) &SavedList[0], 0, sizeof(SavedList) );        
    OS_Unuse(&SemaRAM);
    OS_SignalEvent(REBAND_RADIO, &OS_F868);    
  }
}

void CAPTION_TxChannel()
{
  if( RadioChannelRetrans )  sprintf(Display1, "Передача: %2d к, %2d с, %2d м", RadioChannelRetrans, BW_RETRANS + 1, (RadioPowerRetrans & 0xf) + 2 );
  else                       sprintf(Display1, "Ретрансляция выключена");
}

void ENTER_TxChannel()
{
  int value = RadioChannelRetrans;
  if (GUI_KeyboardInt(&value, "Канал передачи", false, 0, 15))
  {
    OS_Use(&SemaRAM);
    RadioChannelRetrans = value;
    writeConfigFlashWithCHK();
    OS_Unuse(&SemaRAM);
  }
  
  value = BW_RETRANS + 1;
  if (GUI_KeyboardInt(&value, "Скорость передачи", false, 1, 4))
  {
    OS_Use(&SemaRAM);
    RadioBW        = (RadioBW & ~0x30) + ((value - 1) << 4);
    writeConfigFlashWithCHK();
    OS_Unuse(&SemaRAM);
  }

  value = RadioPowerRetrans + 2;
  if (GUI_KeyboardInt(&value, "Мощность передачи", false, 2, 17))
  {
    OS_Use(&SemaRAM);
    RadioPowerRetrans   = value - 2;
    writeConfigFlashWithCHK();
    OS_Unuse(&SemaRAM);
  }  
}

void CAPTION_DevicesInBuffer()
{
  sprintf(Display1, "В буфере: %d", SavedCounter);
}

void CAPTION_UART()
{
  sprintf(Display1, "RS-232(485): %d/%d", RsAdr, RsSpeed);
}

void ENTER_UART()
{
  GUI_SetMenu(&UARTMenu);
}

void CAPTION_Ethernet()
{
  Int32U ip;
  
  ip = IP_GetIPAddr(0);
  parse_ip(&ip, Display2);
  sprintf(Display1, "Ethernet: %s", Display2);
}

void CAPTION_Reboot()
{  
  if( HotRebootTime ) sprintf(Display1, "Перезагрузка %d час.", HotRebootTime / 3600);
  else                sprintf(Display1, "Перезагрузка выкл.");
}

void ENTER_Reboot()
{
  U32 value = HotRebootTime / 3600;
  if (GUI_KeyboardInt(&value, "Перезагрузка, час.", false, 0, 999 ))
  {
    OS_Use(&SemaRAM);
      HotRebootTime = value * 3600;
      writeConfigFlashWithCHK();
      system_flags.hotreboot_ini = 1;
    OS_Unuse(&SemaRAM);
  }
}

void ENTER_Ethernet()
{
  GUI_SetMenu(&EthernetMenu);
}

void CAPTION_WiFi()
{
  if (system_flags.wifi_on)
  {
    parse_ip(&WiFiConfig.WIFI_TCPIP.IP, Display2);
    sprintf(Display1, "WiFi: %s", Display2);
  }
  else
  {
    if( WiFiConfig.wifi_use ) sprintf(Display1, "WiFi: Нет подключения");
    else                      sprintf(Display1, "WiFi: Выключено      ");
  }
}

void ENTER_WiFi()
{
  GUI_SetMenu(&WiFiMenu);
}

void CAPTION_GSM()
{  
  if( GSMConfig.gsm_use )   
  {
    if (system_flags.gsm_on)  sprintf(Display1, "GSM: Подключено");
    else                      sprintf(Display1, "GSM: Нет подключения");
  }
  else                        sprintf(Display1, "GSM: Выключено      ");
}

void ENTER_GSM()
{
  OS_Use(&SemaRAM);
    GSMConfig.gsm_use = !GSMConfig.gsm_use;
    writeConfigFlashWithCHK();
  OS_Unuse(&SemaRAM);
  
  OS_SignalEvent( BIT7, &OS_GSM);
}

void CAPTION_MQTT()
{
  if (system_flags.mqtt_on)
  {
    sprintf(Display1, "MQTT: %18s", MQTTConfig.SeverHostName);
  }
  else
  {
    if( MQTTConfig.mqtt_use ) sprintf(Display1, "MQTT: Нет подключения");
    else                      sprintf(Display1, "MQTT: Выключено      ");
  }
}

void ENTER_MQTT()
{
  GUI_SetMenu( &MQTTMenu);
}

void CAPTION_DateTime()
{
  int y, m, d, h, mi, s;
  ParseDate(&Time, &s, &mi, &h, &d, &m, &y);
  sprintf(Display1, "Время: %.2d.%.2d.%.4d %.2d:%.2d", d, m, y, h, mi);
}

void ENTER_DateTime()
{
  TempTime = Time;
  GUI_SetMenu(&DateTimeMenu);
}

void CAPTION_Prot()
{
  sprintf(Display1, "Радио протокол: %d", ProtocolNumber + 1);
}

void ENTER_Prot()
{
  TempTime = ProtocolNumber;
  GUI_SetMenu(&ProtMenu);
}

void CAPTION_Info()
{
  errors_present() ? sprintf(Display1, "Состояние прибора (!)") : sprintf(Display1, "Состояние прибора");
}

void ENTER_Info()
{
  GUI_SetGUI(&GUI_Information);
}

const MENU_ITEM MI_DevicesOnAir    = {&CAPTION_DevicesOnAir,    &ENTER_DevicesOnAir, MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_CENTER};
const MENU_ITEM MI_RxChannel       = {&CAPTION_RxChannel,       &ENTER_RxChannel,    MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_CENTER};
const MENU_ITEM MI_TxChannel       = {&CAPTION_TxChannel,       &ENTER_TxChannel,    MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_CENTER};
const MENU_ITEM MI_DevicesInBuffer = {&CAPTION_DevicesInBuffer, NULL,                MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_CENTER};

const MENU_ITEM MI_Prot            = {&CAPTION_Prot,            &ENTER_Prot,         MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_CENTER};

const MENU_ITEM MI_Ethernet        = {&CAPTION_Ethernet,        &ENTER_Ethernet,     MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_CENTER};
      MENU_ITEM MI_WiFi            = {&CAPTION_WiFi,            &ENTER_WiFi,         MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_CENTER};
      MENU_ITEM MI_GSM             = {&CAPTION_GSM,             &ENTER_GSM,          MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_CENTER};
const MENU_ITEM MI_UART            = {&CAPTION_UART,            &ENTER_UART,         MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_CENTER};

const MENU_ITEM MI_MQTT            = {&CAPTION_MQTT,            &ENTER_MQTT,         MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_CENTER};
const MENU_ITEM MI_DateTime        = {&CAPTION_DateTime,        &ENTER_DateTime,     MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_CENTER};
const MENU_ITEM MI_Reboot          = {&CAPTION_Reboot,          &ENTER_Reboot,       MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_CENTER};

const MENU_ITEM MI_Info            = {&CAPTION_Info,            &ENTER_Info,         MENU_ITEM_VISIBLE, MENU_ITEM_STYLE_CENTER};

#if     defined(__V2XX__)
const MENU_ITEM* MainMenu_items[] = {&MI_DevicesOnAir, &MI_RxChannel, &MI_TxChannel, &MI_DevicesInBuffer, &MI_Ethernet, &MI_WiFi, &MI_UART, &MI_DateTime, &MI_Reboot, &MI_Info, NULL};
#elif   defined(__V3XX__)
const MENU_ITEM* MainMenu_items[] = {&MI_DevicesOnAir, &MI_RxChannel, &MI_TxChannel, &MI_DevicesInBuffer, &MI_Prot, &MI_Ethernet, &MI_WiFi, &MI_GSM, &MI_UART, &MI_MQTT, &MI_DateTime, &MI_Reboot, &MI_Info, NULL};
#endif

void MainMenu_cancel()
{
  MainMenu.selected = 255;
}

void MainMenu_initialize()
{
  MI_WiFi.visible = WiFiConfig.wifi_present ? MENU_ITEM_VISIBLE : MENU_ITEM_HIDDEN;
  MI_GSM.visible  = GSMConfig.gsm_present   ? MENU_ITEM_VISIBLE : MENU_ITEM_HIDDEN;
}

MENU MainMenu = {"Главный экран", MainMenu_items, 0, 0, 255, &MainMenu_cancel, &MainMenu_initialize};
/* Главный экран: окончание */


void OLED_Task(void) 
{
  GUI_LogoScreen();
  GUI_SetMenu(&MainMenu);
    
  for(;;)
  {
    Screen();
    load_GRAM_OLED((pU8)main_screen);    
  }
}
