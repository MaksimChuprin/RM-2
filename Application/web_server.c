#include "defines.h"

#define MAG7_GAS_CH4  0
#define MAG7_GAS_O2   1
#define MAG7_GAS_CO2  2
#define MAG7_GAS_CO   3
#define MAG7_GAS_NH3  4
#define MAG7_GAS_H2S  5
#define MAG7_GAS_NO2  6
#define MAG7_GAS_SO2  7

const char* MAG7_GASES_NAMES[]    = {"Метан",          "Кислород",      "Диоксид углерода", "Монооксид углерода", "Аммиак",           "Сероводород",      "Диоксид азота",    "Диоксид серы"};
const char* MAG7_GASES_SYMBOLS[]  = {"CH<sub>4</sub>", "O<sub>2</sub>", "CO<sub>2</sub>",   "CO",                 "NH<sub>3</sub>",   "H<sub>2</sub>S",   "NO<sub>2</sub>",   "SO<sub>2</sub>"};
const char* MAG7_GASES_UNITS[]    = {"%",              "%",             "ppm",              "мг/м<sup>3</sup>",   "мг/м<sup>3</sup>", "мг/м<sup>3</sup>", "мг/м<sup>3</sup>",  "мг/м<sup>3</sup>"};
const int   MAG7_GASES_DECIMALS[] = {2,                2,               0,                   2,                   2,                  2,                  2,                   2};

int send_sock_cyclone(SOCKTYPE pOutput, pU8 str)
{
  int     len    = strlen(str);
  int     result = 0;
  int     pointerBuf;
  
  for(pointerBuf = 0;len > 0; )
  {
    result      = send(pOutput, &str[pointerBuf], len, 0);
    if( result == -1)         break;
    
    pointerBuf += result;
    len        -= result;
  }
  return result;
}

int _IP_WEBS_SendString(SOCKTYPE pOutput, pU8 str)
{
  int     result;
  
  result = send_sock_cyclone(pOutput, str);
  if(result >= 0) return send_sock_cyclone(pOutput, "\r\n");
  return result;
}

// WEB server!
void proceedWEB(SOCKTYPE pOutput, pU8 buffer)
{
  #if defined(__V2XX__)
  return;
  #endif
  
  if (strncmp(buffer, "GET ", 4) != 0) 
  {
    return;
  }
    
  buffer[3] = '_';
  char* c = strchr(buffer, 0x20);
  if ((!c) || (c - &buffer[4] > 24)) return; 
  
  char request[32];
  
  memset(request, 0x00, 32);
  memcpy(request, &buffer[4], c - &buffer[4]);

  if ((strcmp(request, "/") == 0) || (strcmp(request, "/current.html") == 0))
  {
    WS_SendCurrentPage(pOutput, buffer);
  }
  else if (strcmp(request, "/archive.html") == 0)
  {
    WS_SendArchivePage(pOutput, buffer);
  }
  else if (strcmp(request, "/status.html") == 0)
  {
    WS_SendStatusPage(pOutput, buffer);
  }
  else
  {
    _IP_WEBS_SendString(pOutput, "HTTP/1.0 404 Not Found");
    _IP_WEBS_SendString(pOutput, ""); // break before data
  }
}

void WS_SendHead(SOCKTYPE pOutput, pU8 buffer)
{
  _IP_WEBS_SendString(pOutput, "HTTP/1.0 200 OK");  
  _IP_WEBS_SendString(pOutput, "Content-Type: text/html");
  _IP_WEBS_SendString(pOutput, "");
  
  _IP_WEBS_SendString(pOutput, "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">");
  _IP_WEBS_SendString(pOutput, "<html xmlns=\"http://www.w3.org/1999/xhtml\">");

  _IP_WEBS_SendString(pOutput, "<html>");
  _IP_WEBS_SendString(pOutput, "<head>");
  _IP_WEBS_SendString(pOutput, "<meta charset=\"windows-1251\">");
    
  sprintf(buffer, "<title>%s %s</title>", ID, Version);
  _IP_WEBS_SendString(pOutput, buffer);
  
  WS_SendCSS(pOutput);
  
  _IP_WEBS_SendString(pOutput, "</head>");
}

void WS_SendCSS(SOCKTYPE pOutput)
{
  _IP_WEBS_SendString(pOutput, "<style type=\"text/css\">");
  
  _IP_WEBS_SendString(pOutput, "body");
  _IP_WEBS_SendString(pOutput, "{");
  _IP_WEBS_SendString(pOutput, "	font-family: \"Trebuchet MS\", Arial, Helvetica, sans-serif;");
  _IP_WEBS_SendString(pOutput, "	font-size: 14px;");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, "body a");
  _IP_WEBS_SendString(pOutput, "{");
  _IP_WEBS_SendString(pOutput, "	color: #134e74;");
  _IP_WEBS_SendString(pOutput, "	cursor: pointer;");
  _IP_WEBS_SendString(pOutput, "	text-decoration: underline;");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, ".selected");
  _IP_WEBS_SendString(pOutput, "{");
  _IP_WEBS_SendString(pOutput, "	background-color: #C8FAC8;");
  _IP_WEBS_SendString(pOutput, "	font-weight: bold;");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, "#header");
  _IP_WEBS_SendString(pOutput, "{");
  _IP_WEBS_SendString(pOutput, "	display: block;");
  _IP_WEBS_SendString(pOutput, "	margin-top: 0;");
  _IP_WEBS_SendString(pOutput, "	border-bottom: 2px solid;");
  _IP_WEBS_SendString(pOutput, "	width: 100%;");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, "#rm-info");
  _IP_WEBS_SendString(pOutput, "{");
  _IP_WEBS_SendString(pOutput, "	display: block;");
  _IP_WEBS_SendString(pOutput, "	font-size: 24px;");
  _IP_WEBS_SendString(pOutput, "	font-weight: bold;");
  _IP_WEBS_SendString(pOutput, "	text-align: left;");
  _IP_WEBS_SendString(pOutput, "	color: #175e8c;");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, "#page-generation-time");
  _IP_WEBS_SendString(pOutput, "{");
  _IP_WEBS_SendString(pOutput, "	display: block;");
  _IP_WEBS_SendString(pOutput, "	font-style: italic;");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, "#logo");
  _IP_WEBS_SendString(pOutput, "{");
  _IP_WEBS_SendString(pOutput, "	position: absolute;");
  _IP_WEBS_SendString(pOutput, "	top: 0;");
  _IP_WEBS_SendString(pOutput, "	right: 0;");
  _IP_WEBS_SendString(pOutput, "	margin: 16px 8px 16px 8px;");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, "@media (max-width: 480px)");
  _IP_WEBS_SendString(pOutput, "{");
  _IP_WEBS_SendString(pOutput, "   #logo");
  _IP_WEBS_SendString(pOutput, "   { ");
  _IP_WEBS_SendString(pOutput, "      position: relative;");
  _IP_WEBS_SendString(pOutput, "	  display: block;");
  _IP_WEBS_SendString(pOutput, "   }");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, "#eksis-website-link img");
  _IP_WEBS_SendString(pOutput, "{");
  _IP_WEBS_SendString(pOutput, "	display: block;");
  _IP_WEBS_SendString(pOutput, "	margin: 0 auto;");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, ".links");
  _IP_WEBS_SendString(pOutput, "{");
  _IP_WEBS_SendString(pOutput, "	margin: 16px 0;");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, ".link");
  _IP_WEBS_SendString(pOutput, "{");
  _IP_WEBS_SendString(pOutput, "	box-shadow: inset 0px 1px 0px 0px #bbdaf7;");
  _IP_WEBS_SendString(pOutput, "	background-color: #99dbff;");
  _IP_WEBS_SendString(pOutput, "	border-radius: 6px;");
  _IP_WEBS_SendString(pOutput, "	border: 1px solid #649bd3;");
  _IP_WEBS_SendString(pOutput, "	display: inline-block;");
  _IP_WEBS_SendString(pOutput, "	color: #ffffff;");
  _IP_WEBS_SendString(pOutput, "	font-family: Arial;");
  _IP_WEBS_SendString(pOutput, "	font-size: 15px;");
  _IP_WEBS_SendString(pOutput, "	padding: 6px 24px;");
  _IP_WEBS_SendString(pOutput, "	text-decoration: none;");
  _IP_WEBS_SendString(pOutput, "	text-shadow: 0px 1px 0px #528ecc;");
  _IP_WEBS_SendString(pOutput, "	margin: 0 8px 4px 0;");
  _IP_WEBS_SendString(pOutput, "	box-sizing: border-box;");
  _IP_WEBS_SendString(pOutput, "	width: 220px;");
  _IP_WEBS_SendString(pOutput, "	text-align: center;");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, "@media (max-width: 480px)");
  _IP_WEBS_SendString(pOutput, "{");
  _IP_WEBS_SendString(pOutput, "   .link");
  _IP_WEBS_SendString(pOutput, "   { ");
  _IP_WEBS_SendString(pOutput, "		width: 100%;");
  _IP_WEBS_SendString(pOutput, "		box-sizing: border-box;");
  _IP_WEBS_SendString(pOutput, "		text-align: center;");
  _IP_WEBS_SendString(pOutput, "   }");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, ".link:hover");
  _IP_WEBS_SendString(pOutput, "{");
  _IP_WEBS_SendString(pOutput, "	background-color: #79bbef;");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, ".devices");
  _IP_WEBS_SendString(pOutput, "{");
  _IP_WEBS_SendString(pOutput, "	display: flex;");
  _IP_WEBS_SendString(pOutput, "	flex-wrap: wrap;");
  _IP_WEBS_SendString(pOutput, "	justify-content: flex-start;");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, "@media (max-width: 480px)");
  _IP_WEBS_SendString(pOutput, "{");
  _IP_WEBS_SendString(pOutput, "   .devices");
  _IP_WEBS_SendString(pOutput, "   { ");
  _IP_WEBS_SendString(pOutput, "      flex-direction: row;");
  _IP_WEBS_SendString(pOutput, "   }");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, ".device");
  _IP_WEBS_SendString(pOutput, "{");
  _IP_WEBS_SendString(pOutput, "	display: inline-block;");
  _IP_WEBS_SendString(pOutput, "	width: 220px;");
  _IP_WEBS_SendString(pOutput, "	vertical-align: top;");
  _IP_WEBS_SendString(pOutput, "	border: 3px solid #1C6EA4;");
  _IP_WEBS_SendString(pOutput, "	border-radius: 15px;");
  _IP_WEBS_SendString(pOutput, "	margin-right: 12px;");
  _IP_WEBS_SendString(pOutput, "	margin-bottom: 12px;");
  _IP_WEBS_SendString(pOutput, "	font-size: 16px;");
  _IP_WEBS_SendString(pOutput, "	box-sizing: border-box;");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, "@media (max-width: 480px)");
  _IP_WEBS_SendString(pOutput, "{");
  _IP_WEBS_SendString(pOutput, "   .device");
  _IP_WEBS_SendString(pOutput, "   { ");
  _IP_WEBS_SendString(pOutput, "		width: 100%;");
  _IP_WEBS_SendString(pOutput, "		margin-right: 0;");
  _IP_WEBS_SendString(pOutput, "   }");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, ".device:hover");
  _IP_WEBS_SendString(pOutput, "{");
  _IP_WEBS_SendString(pOutput, "	border: 3px solid #4CAEC4;");
  _IP_WEBS_SendString(pOutput, "	background-color: #F0FFFF;");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, ".channel");
  _IP_WEBS_SendString(pOutput, "{");
  _IP_WEBS_SendString(pOutput, "	position: relative;");
  _IP_WEBS_SendString(pOutput, "	padding-left: 16px;");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, ".parameter");
  _IP_WEBS_SendString(pOutput, "{");
  _IP_WEBS_SendString(pOutput, "	position: relative;");
  _IP_WEBS_SendString(pOutput, "	margin-left: 16px;");
  _IP_WEBS_SendString(pOutput, "	margin-bottom: -15px;");
  _IP_WEBS_SendString(pOutput, "	font-size: 14px;");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, ".node");
  _IP_WEBS_SendString(pOutput, "{");
  _IP_WEBS_SendString(pOutput, "	white-space: nowrap;");
  _IP_WEBS_SendString(pOutput, "	overflow: clip;");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, ".node-caption");
  _IP_WEBS_SendString(pOutput, "{");
  _IP_WEBS_SendString(pOutput, "	display: inline-block;");
  _IP_WEBS_SendString(pOutput, "	margin: 4px 8px 4px 4px;");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, ".device .node-caption");
  _IP_WEBS_SendString(pOutput, "{");
  _IP_WEBS_SendString(pOutput, "	color: #175e8c;");
  _IP_WEBS_SendString(pOutput, "	font-weight: bold;");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, ".ok::marker");
  _IP_WEBS_SendString(pOutput, "{");
  _IP_WEBS_SendString(pOutput, "	color: #079d13;");
  _IP_WEBS_SendString(pOutput, "}");
  
  _IP_WEBS_SendString(pOutput, ".error::marker");
  _IP_WEBS_SendString(pOutput, "{");
  _IP_WEBS_SendString(pOutput, "	color: #ff0000;");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, ".last-update");
  _IP_WEBS_SendString(pOutput, "{");
  _IP_WEBS_SendString(pOutput, "	display: block;");
  _IP_WEBS_SendString(pOutput, "	margin: 6px 0 0 0;");
  _IP_WEBS_SendString(pOutput, "	font-style: italic;");
  _IP_WEBS_SendString(pOutput, "	text-align: center;");
  _IP_WEBS_SendString(pOutput, "	padding-top: 16px;");
  _IP_WEBS_SendString(pOutput, "}");

  _IP_WEBS_SendString(pOutput, ".info");
  _IP_WEBS_SendString(pOutput, "{");
  _IP_WEBS_SendString(pOutput, "      margin-top: -16px;");  
  _IP_WEBS_SendString(pOutput, "}");
  
  _IP_WEBS_SendString(pOutput, ".info-section");
  _IP_WEBS_SendString(pOutput, "{");
  _IP_WEBS_SendString(pOutput, "	text-align: center;");
  _IP_WEBS_SendString(pOutput, "	font-size: 16px;");
  _IP_WEBS_SendString(pOutput, "	font-weight: bold;");
  _IP_WEBS_SendString(pOutput, "	color: #06891e;");
  _IP_WEBS_SendString(pOutput, "	text-decoration: underline;");
  _IP_WEBS_SendString(pOutput, "}");
  
  _IP_WEBS_SendString(pOutput, ".info .key");
  _IP_WEBS_SendString(pOutput, "{");
  _IP_WEBS_SendString(pOutput, "	white-space: nowrap;");
  _IP_WEBS_SendString(pOutput, "	overflow: clip;");
  _IP_WEBS_SendString(pOutput, "	display: inline-block;");
  _IP_WEBS_SendString(pOutput, "	width: 30%;");
  _IP_WEBS_SendString(pOutput, "}");
  
  _IP_WEBS_SendString(pOutput, ".info .value");
  _IP_WEBS_SendString(pOutput, "{");
  _IP_WEBS_SendString(pOutput, "	white-space: nowrap;");
  _IP_WEBS_SendString(pOutput, "	overflow: clip;");
  _IP_WEBS_SendString(pOutput, "	display: inline-block;");
  _IP_WEBS_SendString(pOutput, "	font-weight: bold;");
  _IP_WEBS_SendString(pOutput, "	color: #175e8c;");
  _IP_WEBS_SendString(pOutput, "}");
  
  _IP_WEBS_SendString(pOutput, "@media (max-width: 480px)");
  _IP_WEBS_SendString(pOutput, "{");
  _IP_WEBS_SendString(pOutput, "   .info .key, .info .value");
  _IP_WEBS_SendString(pOutput, "   { ");
  _IP_WEBS_SendString(pOutput, "      width: auto;");
  _IP_WEBS_SendString(pOutput, "      display: block;");
  _IP_WEBS_SendString(pOutput, "   }");
  _IP_WEBS_SendString(pOutput, "   .info .value");
  _IP_WEBS_SendString(pOutput, "   { ");
  _IP_WEBS_SendString(pOutput, "      margin-left: 30%;");
  _IP_WEBS_SendString(pOutput, "      margin-top: -16px;");  
  _IP_WEBS_SendString(pOutput, "   }");  
  _IP_WEBS_SendString(pOutput, "}");
  
  _IP_WEBS_SendString(pOutput, "</style>");
}

void WS_SendInfo(SOCKTYPE pOutput, pU8 buffer)
{
  int m, d, h, mi, s, y;
  ParseDate( &Time, &s, &mi, &h, &d, &m, &y);

  _IP_WEBS_SendString(pOutput, "<div id=\"header\">");
  
  _IP_WEBS_SendString(pOutput, "<p id=\"rm-info\">");
  sprintf(buffer, "%s %s [%s]", ID, Version, SerialNumber);
  _IP_WEBS_SendString(pOutput, buffer);
  _IP_WEBS_SendString(pOutput, "</p>");
  
  _IP_WEBS_SendString(pOutput, "<p id=\"page-generation-time\">");
  sprintf(buffer, "Время генерации страницы: %02d:%02d:%02d %02d.%02d.%4d", h, mi ,s, d, m, y);
  _IP_WEBS_SendString(pOutput, buffer);
  _IP_WEBS_SendString(pOutput, "</p>");
  
  _IP_WEBS_SendString(pOutput, "<div id=\"logo\">");
  _IP_WEBS_SendString(pOutput, "<a id=\"eksis-website-link\" href=\"https://eksis.ru\" target=\"_blank\"><img src=\"http://www.eksis.ru/bitrix/templates/eksis/images/logo.png\"/></a>");
  _IP_WEBS_SendString(pOutput, "</div>");
  
  _IP_WEBS_SendString(pOutput, "</div>");
}

void WS_SendLinks(SOCKTYPE pOutput, pU8 buffer, U8 selected)
{
  _IP_WEBS_SendString(pOutput, "<div class=\"links\">");

  OS_Use(&SemaRAM);
  if (selected == CURRENT_BUFFER_PAGE)
  {
    _IP_WEBS_SendString(pOutput, "<p class=\"link selected\">");
    sprintf(buffer, "Приборы на связи (%d)", CurrentCounter);
    _IP_WEBS_SendString(pOutput, buffer);
    _IP_WEBS_SendString(pOutput, "</p>");
  }
  else
  {
    _IP_WEBS_SendString(pOutput, "<p class=\"link\">");
    sprintf(buffer, "<a href=\"current.html\">Приборы на связи (%d)</a>", CurrentCounter);
    _IP_WEBS_SendString(pOutput, buffer);
    _IP_WEBS_SendString(pOutput, "</p>");
  } 
  OS_Unuse(&SemaRAM);

  OS_Use(&SemaRAM);
  if (selected == ARCHIVE_BUFFER_PAGE)
  {
    _IP_WEBS_SendString(pOutput, "<p class=\"link selected\">");
    sprintf(buffer, "Записи в памяти (%d)", SavedCounter);
    _IP_WEBS_SendString(pOutput, buffer);
    _IP_WEBS_SendString(pOutput, "</p>");
  }
  else
  {
    _IP_WEBS_SendString(pOutput, "<p class=\"link\">");
    sprintf(buffer, "<a href=\"archive.html\">Записи в памяти (%d)</a>", SavedCounter);
    _IP_WEBS_SendString(pOutput, buffer);
    _IP_WEBS_SendString(pOutput, "</p>");
  } 
  OS_Unuse(&SemaRAM);
  
  OS_Use(&SemaRAM);
  if (selected == STATUS_PAGE)
  {
    _IP_WEBS_SendString(pOutput, "<p class=\"link selected\">");
    if (errors_present())
      sprintf(buffer, "Состояние модема (!)");
    else
      sprintf(buffer, "Состояние модема");
    _IP_WEBS_SendString(pOutput, buffer);
    _IP_WEBS_SendString(pOutput, "</p>");
  }
  else
  {
    _IP_WEBS_SendString(pOutput, "<p class=\"link\">");
    if (errors_present())
      sprintf(buffer, "<a href=\"status.html\">Состояние модема (!)</a>");
    else
      sprintf(buffer, "<a href=\"status.html\">Состояние модема</a>");
    _IP_WEBS_SendString(pOutput, buffer);
    _IP_WEBS_SendString(pOutput, "</p>");
  }
  OS_Unuse(&SemaRAM);
  
  _IP_WEBS_SendString(pOutput, "</div>");
}

void WS_SendCurrentPage(SOCKTYPE pOutput, pU8 buffer)
{
  WS_SendHead(pOutput, buffer);
  
  _IP_WEBS_SendString(pOutput, "<body>");

  WS_SendInfo(pOutput, buffer);
  
  WS_SendLinks(pOutput, buffer, CURRENT_BUFFER_PAGE);
  
  _IP_WEBS_SendString(pOutput, "<div class=\"devices\">");
  OS_Use(&SemaRAM);
  for (int i = 0; i < CurrentCounter; i++)
  {
    WS_SendRadioListStruct(pOutput, buffer, &CurrentList[i]);
  }
  OS_Unuse(&SemaRAM);
  _IP_WEBS_SendString(pOutput, "</div>");
  
  _IP_WEBS_SendString(pOutput, "</body>");
  _IP_WEBS_SendString(pOutput, "</html>");
}

void WS_SendArchivePage(SOCKTYPE pOutput, pU8 buffer)
{
  WS_SendHead(pOutput, buffer);
  
  _IP_WEBS_SendString(pOutput, "<body>");

  WS_SendInfo(pOutput, buffer);
  
  WS_SendLinks(pOutput, buffer, ARCHIVE_BUFFER_PAGE);
  
  _IP_WEBS_SendString(pOutput, "<div class=\"devices\">");
  OS_Use(&SemaRAM);
  if (SavedCounter)
  {
    for (int i = SavedCounter - 1; i >= 0; i--)
    {
      WS_SendRadioListStruct(pOutput, buffer, &SavedList[i]);
    }
  }
  OS_Unuse(&SemaRAM);
  _IP_WEBS_SendString(pOutput, "</div>");
  
  _IP_WEBS_SendString(pOutput, "</body>");
  _IP_WEBS_SendString(pOutput, "</html>");
}

void WS_SendInfoKeyValue(SOCKTYPE pOutput, char* key, char* value)
{
  _IP_WEBS_SendString(pOutput, "<div class=\"info\">");
  if (key)
  {
    _IP_WEBS_SendString(pOutput, "<p class=\"key\">");
    _IP_WEBS_SendString(pOutput, key);
    _IP_WEBS_SendString(pOutput, "</p>");
  }
  if (value)
  {
    _IP_WEBS_SendString(pOutput, "<p class=\"value\">");
    _IP_WEBS_SendString(pOutput, value);
    _IP_WEBS_SendString(pOutput, "</p>");
  }
  _IP_WEBS_SendString(pOutput, "</div>");
}

void WS_SendStatusPage(SOCKTYPE pOutput, pU8 buffer)
{
  WS_SendHead(pOutput, buffer);
  
  _IP_WEBS_SendString(pOutput, "<body>");

  WS_SendInfo(pOutput, buffer);
  
  WS_SendLinks(pOutput, buffer, STATUS_PAGE);

  _IP_WEBS_SendString(pOutput, "<p class=\"info-section\">Параметры приёма и передачи</p>");
  
  WS_SendInfoKeyValue(pOutput, "Протокол обмена", ProtocolNumber ? "двусторонний" : "односторонний");
  
  sprintf(buffer, "%u", RadioChannelModem);
  WS_SendInfoKeyValue(pOutput, "Канал приёма", buffer);

  sprintf(buffer, "%u", (RadioBW & 3) + 1);
  WS_SendInfoKeyValue(pOutput, "Скорость приёма", buffer);
  
  if (RadioChannelRetrans)
  {
    sprintf(buffer, "%u", RadioChannelRetrans);
    WS_SendInfoKeyValue(pOutput, "Канал ретрансляции", buffer);
    
    sprintf(buffer, "%u", BW_RETRANS + 1);
    WS_SendInfoKeyValue(pOutput, "Скорость ретрансляции", buffer);
    
    sprintf(buffer, "%u дБм", (RadioPowerRetrans & 0xf) + 2);
    WS_SendInfoKeyValue(pOutput, "Мощность ретрансляции", buffer);
  }
  else
  {
    WS_SendInfoKeyValue(pOutput, "Канал ретрансляции", "ретрансляция выключена");
  }
  
  _IP_WEBS_SendString(pOutput, "<p class=\"info-section\">Параметры связи по RS-232/RS-485</p>");

  sprintf(buffer, "%u", RsAdr);
  WS_SendInfoKeyValue(pOutput, "Сетевой адрес", buffer);

  sprintf(buffer, "%u", RsSpeed);
  WS_SendInfoKeyValue(pOutput, "Скорость связи", buffer);
  
  _IP_WEBS_SendString(pOutput, "<p class=\"info-section\">Параметры связи по Ethernet</p>");
  
  IP_GetHostName(buffer);
  WS_SendInfoKeyValue(pOutput, "Сетевое имя", buffer);
  
  Int8U MAC[6];
  IP_GetHWAddr(0, MAC, 6);
  sprintf(buffer, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5]);
  WS_SendInfoKeyValue(pOutput, "MAC-адрес", buffer);
  
  WS_SendInfoKeyValue(pOutput, "Использовать DHCP", TCPIPConfig.UseDHCP ? "да" : "нет");

  Int32U ip;
  if (TCPIPConfig.UseDHCP)
     ip = IP_GetIPAddr(0);
  else
    ip = TCPIPConfig.IP;
  parse_ip(&ip, buffer);
  WS_SendInfoKeyValue(pOutput, "IP-адрес", buffer);
  
  if (TCPIPConfig.UseDHCP)
     ip = IP_GetAddrMask(0);
  else
    ip = TCPIPConfig.Mask;
  parse_ip(&ip, buffer);
  WS_SendInfoKeyValue(pOutput, "Маска подсети", buffer);

  if (TCPIPConfig.UseDHCP)
     ip = IP_GetGWAddr(0);
  else
    ip = TCPIPConfig.Gate;
  parse_ip(&ip, buffer);
  WS_SendInfoKeyValue(pOutput, "Шлюз", buffer);

  if (WiFiConfig.wifi_present)
  {
    _IP_WEBS_SendString(pOutput, "<p class=\"info-section\">Параметры связи по WiFi</p>");

    WS_SendInfoKeyValue(pOutput, "Состояние", WiFiConfig.wifi_use ? "включено" : "выключено");
    
    WiFiConfig.NetName[63] = 0;
    if (WiFiConfig.NetName[0])
    {
      sprintf(buffer, "%s", WiFiConfig.NetName);
      WS_SendInfoKeyValue(pOutput, "Имя сети", buffer);
    }
    else
      WS_SendInfoKeyValue(pOutput, "Имя сети", "не задано");

    WS_SendInfoKeyValue(pOutput, "Протокол", WiFiConfig.modbus ? "ModBus" : "Eksis");

    if (system_flags.wifi_on)
    {
      parse_ip(&WiFiConfig.WIFI_TCPIP.IP, buffer);
      WS_SendInfoKeyValue(pOutput, "Состояние", buffer);
    }
    else
      WS_SendInfoKeyValue(pOutput, "Состояние", "нет подключения");
  }
  
  if (GSMConfig.gsm_present)
  {
    _IP_WEBS_SendString(pOutput, "<p class=\"info-section\">Параметры связи по GSM</p>");
    
    if (GSMConfig.gsm_use)
    {
      WS_SendInfoKeyValue(pOutput, "Состояние", system_flags.gsm_on ? "подключено" : "нет подключения");
    }
    else
      WS_SendInfoKeyValue(pOutput, "Состояние", "выключено");
    
    switch (GSMConfig.err_code)
    {
      case 1:
        WS_SendInfoKeyValue(pOutput, "Ошибка", "Нет связи с модулем!");
        break;
      case 2:
        WS_SendInfoKeyValue(pOutput, "Ошибка", "Нет SIM-карты!");
        break;
      case 3:
        WS_SendInfoKeyValue(pOutput, "Ошибка", "Нет связи с сетью!");
        break;
    }

    WS_SendInfoKeyValue(pOutput, "Режим работы", GSMConfig.sms_mode ? "SMS" : "MQTT");
  }

  _IP_WEBS_SendString(pOutput, "<p class=\"info-section\">Параметры передачи данных через MQTT</p>");
  
  WS_SendInfoKeyValue(pOutput, "Состояние", MQTTConfig.mqtt_use ? "включено" : "выключено");
  if (MQTTConfig.mqtt_use)
  {
    switch (MQTTConfig.mqtt_interface)
    {
      case 0:
        WS_SendInfoKeyValue(pOutput, "Интерфейс", "Ethernet");
        break;
      case 1:
        WS_SendInfoKeyValue(pOutput, "Интерфейс", "GSM");
        break;
      case 2:
        WS_SendInfoKeyValue(pOutput, "Интерфейс", "WiFi");
        break;
    }
    
    sprintf(buffer, "%s:%d", MQTTConfig.SeverHostName, MQTTConfig.ServerPort);
    WS_SendInfoKeyValue(pOutput, "Брокер", buffer);

    WS_SendInfoKeyValue(pOutput, "Подключение", system_flags.mqtt_on ? "OK" : "отсутствует!");

    WS_SendInfoKeyValue(pOutput, "Логин", MQTTConfig.UserName);
    
    WS_SendInfoKeyValue(pOutput, "Пароль", MQTTConfig.UserPass);
    
    WS_SendInfoKeyValue(pOutput, "Топик", MQTTConfig.TopicBase);
    
    sprintf(buffer, "%d", MQTTConfig.mqtt_qos);
    WS_SendInfoKeyValue(pOutput, "QoS", buffer);

    sprintf(buffer, "%d с", MQTTConfig.ping_interval);
    WS_SendInfoKeyValue(pOutput, "Пинг-интервал", buffer);

    sprintf(buffer, "%d мин", MQTTConfig.info_interval);
    WS_SendInfoKeyValue(pOutput, "Инфо-интервал", buffer);
  }

  _IP_WEBS_SendString(pOutput, "<p class=\"info-section\">Параметры синхронизации времени через NTP</p>");

  WS_SendInfoKeyValue(pOutput, "Состояние", NTPConfig.ntpuse ? "включено" : "выключено");
  if (NTPConfig.ntpuse)
  {
    sprintf(buffer, "%s", NTPConfig.NTPSeverName);
    WS_SendInfoKeyValue(pOutput, "Сервер", buffer);
    
    sprintf(buffer, "%d", NTPConfig.TimeZone);
    WS_SendInfoKeyValue(pOutput, "Часовой пояс", buffer);
    
    sprintf(buffer, "%d ч", NTPConfig.periodUpdateTime);
    WS_SendInfoKeyValue(pOutput, "Интервал синхронизации", buffer);
    
    sprintf(buffer, "%d дней", NTPConfig.periodValidTime);
    WS_SendInfoKeyValue(pOutput, "Валидность времени", buffer);
  }
  
  _IP_WEBS_SendString(pOutput, "<p class=\"info-section\">Другие параметры</p>");

  int m, d, h, mi, s, y;
  ParseDate( &Time, &s, &mi, &h, &d, &m, &y);
  sprintf(buffer, "%02d:%02d:%02d %02d.%02d.%4d", h, mi ,s, d, m, y);
  WS_SendInfoKeyValue(pOutput, "Дата и время в приборе", buffer);
  
  if (HotRebootTime)
     sprintf(buffer, "каждые %d часов", HotRebootTime / 3600);
  else
    sprintf(buffer, "выключена");
  WS_SendInfoKeyValue(pOutput, "Автоматическая перезагрузка", buffer);
  
  if (errors_present())
  {
    _IP_WEBS_SendString(pOutput, "<p class=\"info-section\">Состояние модема</p>");
  
    if (errors_flags.timeinvalid)
      WS_SendInfoKeyValue(pOutput, NULL, "Неверные дата и время!");

    if (errors_flags.TimeSyncReq)
       WS_SendInfoKeyValue(pOutput, NULL, "Дата и время нуждаются в синхронизации!");

    if (errors_flags.config_fail)
      WS_SendInfoKeyValue(pOutput, NULL, "Конфигурация некорректна!");

    if (errors_flags.wifi_fail)
      WS_SendInfoKeyValue(pOutput, NULL, "WiFi-модуль неисправен!");

    if (errors_flags.radio_fail)
      WS_SendInfoKeyValue(pOutput, NULL, "Радиомодуль неисправен!");
    
    if (errors_flags.gsm_fail)
      WS_SendInfoKeyValue(pOutput, NULL, "GSM-модуль неисправен!");
    
    if (errors_flags.currentMemEnd)
      WS_SendInfoKeyValue(pOutput, NULL, "Буфер текущих записей заполнен!");

    if (errors_flags.savedMemEnd)
      WS_SendInfoKeyValue(pOutput, NULL, "Буфер архивных записей заполнен!");

    if (errors_flags.LSEfail)
      WS_SendInfoKeyValue(pOutput, NULL, "RTC-генератор неисправен!");
  }
  
  _IP_WEBS_SendString(pOutput, "</body>");
  _IP_WEBS_SendString(pOutput, "</html>");
}

void WS_SendRadioListStruct(SOCKTYPE pOutput, pU8 buffer, radio_list_struct_t* rls)
{
  _IP_WEBS_SendString(pOutput, "");
  sprintf(buffer, "<div class=\"device\" id=\"current_%d\">", rls->Adr);
  _IP_WEBS_SendString(pOutput, buffer);
    
  _IP_WEBS_SendString(pOutput, "<div class=\"node\">");
  _IP_WEBS_SendString(pOutput, "<p class=\"node-caption\">");
  switch (rls->Type)
  {
    case IVTM_7M4_OKW:
      sprintf(buffer, "ИВТМ-7 М4-1 (#%d)", rls->Adr);
      _IP_WEBS_SendString(pOutput, buffer);
      break;
      
    case IVTM_7M4_PRT:
      sprintf(buffer, "ИВТМ-7 М4 (#%d)", rls->Adr);
      _IP_WEBS_SendString(pOutput, buffer);
      break;
      
    case MAG_7:
      sprintf(buffer, "ИКВ-8 (#%d)", rls->Adr);
      _IP_WEBS_SendString(pOutput, buffer);
      break;
      
    default:
      _IP_WEBS_SendString(pOutput, "Неизвестный прибор");
  }
  _IP_WEBS_SendString(pOutput, "</p>");
  _IP_WEBS_SendString(pOutput, "</div>");

  _IP_WEBS_SendString(pOutput, "<div class=\"parameter\">");
  _IP_WEBS_SendString(pOutput, "<p class=\"node-caption\">");
  if (!temperature_error(rls))
    sprintf(buffer, "<li class=\"ok\" title=\"Температура\">%.1f [T, °C]</li>", rls->Tempr/10.);
  else
    sprintf(buffer, "<li class=\"error\" title=\"Температура\">--- [T, °C]</li>");
  _IP_WEBS_SendString(pOutput, buffer);
  _IP_WEBS_SendString(pOutput, "</p>");
  _IP_WEBS_SendString(pOutput, "</div>");
  
  if (humidity_present(rls))
  {
    _IP_WEBS_SendString(pOutput, "<div class=\"parameter\">");
    _IP_WEBS_SendString(pOutput, "<p class=\"node-caption\">");
    if (!humidity_error(rls))
      sprintf(buffer, "<li class=\"ok\" title=\"Влажность\">%d [H, %%]</li>", rls->Humidy);
    else
      sprintf(buffer, "<li class=\"error\" title=\"Влажность\">--- [H, %%]</li>");
    _IP_WEBS_SendString(pOutput, buffer);
    _IP_WEBS_SendString(pOutput, "</p>");
    _IP_WEBS_SendString(pOutput, "</div>");
  }
  
  if (pressure_present(rls))
  {
    _IP_WEBS_SendString(pOutput, "<div class=\"parameter\">");
    _IP_WEBS_SendString(pOutput, "<p class=\"node-caption\">");
    if (!pressure_error(rls))
      sprintf(buffer, "<li class=\"ok\" title=\"Давление\">%d [P, мм рт.ст.]</li>", rls->Pressure);
    else
      sprintf(buffer, "<li class=\"error\" title=\"Давление\">--- [P, мм рт.ст.]</li>");
    _IP_WEBS_SendString(pOutput, buffer);
    _IP_WEBS_SendString(pOutput, "</p>");
    _IP_WEBS_SendString(pOutput, "</div>");
  }
  
  if (rls->Type == MAG_7)
  {
    F32 value;
    int measure_index = 0;
    for (int gas_index = 0; gas_index < 8 && measure_index < 6; gas_index++)
    {
      if (rls->Config & (1 << gas_index))
      {
        _IP_WEBS_SendString(pOutput, "<div class=\"parameter\">");
        _IP_WEBS_SendString(pOutput, "<p class=\"node-caption\">");

        if (!(rls->Errors & (1 << 4 + measure_index)))
        {
          if (gas_index == MAG7_GAS_CO2)
            value = rls->Measures[measure_index];
          else
            value = rls->Measures[measure_index]/100.;
          sprintf(buffer, "<li class=\"ok\" title=\"%s\">%.*f [%s, %s]</li>", MAG7_GASES_NAMES[gas_index], MAG7_GASES_DECIMALS[gas_index], value, MAG7_GASES_SYMBOLS[gas_index], MAG7_GASES_UNITS[gas_index]);
        }
        else
        {
          sprintf(buffer, "<li class=\"error\" title=\"%s\">--- [%s, %s]</li>", MAG7_GASES_NAMES[gas_index], MAG7_GASES_SYMBOLS[gas_index], MAG7_GASES_UNITS[gas_index]);
        }
        
        _IP_WEBS_SendString(pOutput, buffer);
        _IP_WEBS_SendString(pOutput, "</p>");
        _IP_WEBS_SendString(pOutput, "</div>");

        measure_index++;
      }
    }
  }

  _IP_WEBS_SendString(pOutput, "<div class=\"parameter\">");
  _IP_WEBS_SendString(pOutput, "<p class=\"node-caption\">");
  sprintf(buffer, "<li class=\"ok\" title=\"Уровень заряда\">%d [B, %%]</li>", rls->PowLev);
  _IP_WEBS_SendString(pOutput, buffer);
  _IP_WEBS_SendString(pOutput, "</p>");
  _IP_WEBS_SendString(pOutput, "</div>");

  _IP_WEBS_SendString(pOutput, "<div class=\"parameter\">");
  _IP_WEBS_SendString(pOutput, "<p class=\"node-caption\">");
  sprintf(buffer, "<li class=\"ok\" title=\"Уровень сигнала\">%d [S, дБм]</li>", rls->RSSI);
  _IP_WEBS_SendString(pOutput, buffer);
  _IP_WEBS_SendString(pOutput, "</p>");
  _IP_WEBS_SendString(pOutput, "</div>");

  _IP_WEBS_SendString(pOutput, "<p class=\"last-update\" title=\"Время получения данных\">");
  int m, d, h, mi, s, y;
  ParseDate(&rls->Time, &s, &mi, &h, &d, &m, &y);
  sprintf(buffer, "%02d:%02d:%02d %02d.%02d.%4d", h, mi ,s, d, m, y);
  _IP_WEBS_SendString(pOutput, buffer);
  _IP_WEBS_SendString(pOutput, "</p>");
    
  _IP_WEBS_SendString(pOutput, "</div>");
}

