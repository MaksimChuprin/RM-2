#include "defines.h"
#include "IP_init.h"
#include <string.h>
#include "core/net.h"
#include "core/bsd_socket.h"
#include "core/socket.h"

// webserver port 502
void TCP80_Task(void) 
{
  #pragma data_alignment = 4
  //static U8           web_buffer[256];  
  static U8           web_buffer[2];  

  static SOCKTYPE     socket80, socket80_w;
  int                 addrlen, result;
  struct  sockaddr    saddr80;
  timeval             timeout;
  
  // open server's sockets          
  do 
  {
    socket80 = ListenAtTcpAddr(WEB_PORT);
    OS_Delay(100);
  } while(socket80 == SOCKET_ERROR); 
    
  // prosess command socket
  for(;;)
  {    
    // wait for link 
    while (IP_IFaceIsReady(0) == 0)  
    {
      OS_Delay(100);
    }
    
    // look for incoming connection
    addrlen                 = sizeof(saddr80);
    socket80_w              = accept(socket80, &saddr80, &addrlen);        
    if(socket80_w == SOCKET_ERROR)      continue;             
    
    timeout.tv_sec  = 5; 
    timeout.tv_usec = 0;
    setsockopt(socket80_w, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));  // Set socket timeout 5 sec
            
    // web port
    for(int i = 0, r = 0; ; i++)
    {
      if( i == 0 )
      {
        result = recv(socket80_w, (char*)web_buffer, sizeof(web_buffer), 0);
        if( result == sizeof(web_buffer) )  
        {
          timeout.tv_sec  = 0;
          timeout.tv_usec = 200000;
          setsockopt(socket80_w, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));  // Set socket timeout 0.2 sec
        }
        else break;
      }
      else   
      {
        r = recv(socket80_w, (char*)&web_buffer[32], sizeof(web_buffer) - 32, 0); // dummy read
        if( ( r == -1) || ( r == 0 ) || ( r < sizeof(web_buffer) - 32 ) ) break;
      }
    }
                            
    if( result > 0 )  
    {
      proceedWEB( socket80_w, web_buffer );
    }
    
    shutdown( socket80_w, SOCKET_SD_BOTH); 
    closesocket(socket80_w);
  }
}

// command port 502
void IP502_Task(void) 
{
  #pragma data_alignment = 4
  static U8           tcp_ansbuffer[TCP_MODBUS_LEN];
  static U8           tcp_querybuffer[512];  
  
  static SOCKTYPE     socket502, socket502_w;
  int                 addrlen, result;
  struct  sockaddr    saddr502;
  timeval             timeout = { 2, 0 };
           
  // open server's sockets
  do 
  {
    socket502 = ListenAtTcpAddr(COMMAND_PORT);
    OS_Delay(100);
  } while(socket502 == SOCKET_ERROR); 
    
  // prosess command socket
  for(;;)
  {    
    // wait for link 
    while (IP_IFaceIsReady(0) == 0)
    {
      OS_Delay(100);
    }
    
    // look for incoming connection
    addrlen                 = sizeof(saddr502);
    socket502_w             = accept(socket502, &saddr502, &addrlen); 
    if(socket502_w == SOCKET_ERROR)      continue;             
         
    setsockopt(socket502_w, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));  // Set socket timeout

    // command port 502
    for(U8 sockAlive = 1; sockAlive;)
    {
      result = recv(socket502_w, (char*)tcp_querybuffer, sizeof(tcp_querybuffer), 0);
      
      if( result > 0 )
      {
        // modbus protocol
        S16 current_query = 0;
        for( ;; )
        {
          memcpy(tcp_ansbuffer, &tcp_querybuffer[current_query], MIN(sizeof(tcp_querybuffer) - current_query, sizeof(tcp_ansbuffer)) );
          U16 anslen = MODBUS_TCP(tcp_ansbuffer);                   
          if( !anslen )   { sockAlive = 0; break; }                     // не модбас паскет - рвем соединение

          send(socket502_w, tcp_ansbuffer, anslen, 0);
          
          current_query += (6 + 256*tcp_querybuffer[current_query+4] + tcp_querybuffer[current_query+5]);
          if( current_query == result )       {  break; }               // все запросы обработаны    
          if( (current_query + 7) > result )  { sockAlive = 0; break; } // фрагментированный запрос  - рвем соединение
        }        
      }
      else break;
      
    }
    shutdown( socket502_w, SOCKET_SD_BOTH );
    closesocket( socket502_w );
  }
}

// UDP Socket 1337
void UDP_Task(void) 
{
  SOCKTYPE  sock;
  struct    sockaddr_in     TargetAddr;
  int       Len;
  timeval   timeout  = { 1, 0 };
  
  #pragma data_alignment = 4
  static U8                 udp_buffer[UDP_BUF_LEN];

  // open UDP port
  while( 1 )
  {
    sock = OpenAtUdpAddr(UDP_PORT);
    if( sock != SOCKET_ERROR ) break;
    OS_Delay(100);
  }

  //
  TargetAddr.sin_family       = AF_INET;
  TargetAddr.sin_port         = htons(UDP_PORT);
  TargetAddr.sin_addr.s_addr  = htonl(INADDR_ANY);
  
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));  // Set socket timeout

  for(;;)
  {
    // wait for link 
    while (IP_IFaceIsReady(0) == 0)
    {
      OS_Delay(100);
    }
        
    Len = sizeof(TargetAddr);
    Len = recvfrom(sock, (char*)udp_buffer, sizeof(udp_buffer), 0, (struct sockaddr*)&TargetAddr, &Len);
    if (Len > 0) 
    {
      // prosess eksis  protocol         
      Len = EKSIS_WIFI_TCP(udp_buffer);
      sendto(sock, (char*)udp_buffer, Len, 0, (struct sockaddr*)&TargetAddr, sizeof(TargetAddr));
    }
  }
}

// open socket in server mode
static SOCKTYPE ListenAtTcpAddr(U16 port) 
{
  SOCKTYPE            sock;
  struct sockaddr_in  addr;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock != SOCKET_ERROR) 
  {
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      =   AF_INET;
    addr.sin_port        =   htons(port);
    addr.sin_addr.s_addr =   htonl(INADDR_ANY);
    bind(sock, (struct sockaddr *)&addr, sizeof(addr));
    listen(sock, 1);
  }
  return sock;
}

// open UDP socket
static SOCKTYPE OpenAtUdpAddr(U16 port) 
{
  SOCKTYPE            sock;
  struct sockaddr_in  addr;

  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock != SOCKET_ERROR) 
  {
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      =   AF_INET;
    addr.sin_port        =   htons(port);
    addr.sin_addr.s_addr =   htonl(INADDR_ANY);
    bind(sock, (struct sockaddr *)&addr, sizeof(addr));
  }
  return sock;
}
