#include <sys/types.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>
#include <ctype.h>
#include <dirent.h>
#include <libgen.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <argp.h>
#include <stdbool.h>
#include <wiringPi.h>
#include "RPI-OLED-Font.h"
#define byte unsigned char
#define Brightness 0xCF
#define X_WIDTH 128
//======================================
#define MAX_BUF 1024
#define PID_LIST_BLOCK 32
//======================================


char *PID, *PROCESS, *INTERFACE;

const char *argp_program_version = "OLED-Display RPi-Info 0.9";
const char *argp_program_bug_address = "https://github.com/tobiasvogel/RPi-OLED/issues";
static char doc[] = "This Tool lets you display IP-Address, Netmask, Process Info, CPU Temperature, Core Voltage and Uptime using a SSD1306-OLED-Display on a Raspberry Pi (2).";
static char args_doc[] = ""; // ToDo: Remove if not used"
static struct argp_option options[] = {
  { "interface", 'i', "INTERFACE", 0, "Specify default Network Interface" },
  { "process", 'p', "PROCESS", 0, "Specify Process to observe [default: sshd]" },
  { "pid", 'd', "PID", 0, "Specify PID of Process to observe" },
  { "celsius", 'c', 0, 0, "Use Celsius for Temperatures [default]" },
  { "fahrenheit", 'f', 0, 0, "Use Fahrenheit for Temperatures" },
  { "verbose", 'v', 0, 0, "Print data to stdout" },
  { "quiet", 'q', 0, 0, "Don't print any output" },
  { 0 }
};

struct arguments {
  enum { FAHRENHEIT, CELSIUS } unit;
  enum { PROCESS_NAME, PROCESS_ID } watch;
  enum { QUIET, VERBOSE } verbosity;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  struct arguments *arguments = state->input;
  switch (key) {
  case 'c':
    arguments->unit = CELSIUS;
    break;
  case 'f':
    arguments->unit = FAHRENHEIT;
    break;
  case 'v':
    arguments->verbosity = VERBOSE;
    break;
  case 'q':
    arguments->verbosity = QUIET;
    break;
  case 'p':
    arguments->watch = PROCESS_NAME;
    PROCESS = arg;
    break;
  case 'd':
    arguments->watch = PROCESS_ID;
    PID = arg;
    break;
  case 'i':
    INTERFACE = arg;
    break;
  case ARGP_KEY_ARG:
    return 0;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc, 0, 0, 0 };

char ipaddress[] = "               ";
char netmask[] = "               ";
char hwaddr[] = "  :  :  :  :  :  ";

void LCD_Init(void);
void LCD_CLS(void);
void LCD_P6x8Str(byte x,byte y,byte ch[]);
void LCD_Fill(byte dat);

void LCD_WrDat(unsigned char dat) {
  unsigned char i=8;
  digitalWrite(10, 0); //LCD_CS=0;
  digitalWrite(5, 1); //LCD_DC=1;
  digitalWrite(14, 0); //LCD_SCL=0;
  while(i--) {
    if(dat&0x80) {
      digitalWrite(12, 1);   //LCD_SDA=1;
    } else {
      digitalWrite(12, 0);   //LCD_SDA=0;
    }
    digitalWrite(14, 1); //LCD_SCL=1;
    ;;;
    digitalWrite(14, 0); //LCD_SCL=0;
    dat<<=1;
  }
  digitalWrite(10, 1); //LCD_CS=1;
}
void LCD_WrCmd(unsigned char cmd) {
  unsigned char i=8;
  digitalWrite(10, 0); //LCD_CS=0;
  digitalWrite(5, 0); //LCD_DC=0;
  digitalWrite(14, 0); //LCD_SCL=0;
  while(i--) {
    if(cmd&0x80) {
      digitalWrite(12, 1);   //LCD_SDA=1;
    } else {
      digitalWrite(12, 0);   //LCD_SDA=0;
    }
    digitalWrite(14, 1); //LCD_SCL=1;
    ;;;
    digitalWrite(14, 0); //LCD_SCL=0;;
    cmd<<=1;;
  }
  digitalWrite(10, 1); //LCD_CS=1;
}
void LCD_Set_Pos(unsigned char x, unsigned char y) {
  LCD_WrCmd(0xb0+y);
  LCD_WrCmd(((x&0xf0)>>4)|0x10);
  LCD_WrCmd((x&0x0f)|0x00);
}
void LCD_Fill(unsigned char bmp_dat) {
  unsigned char y,x;
  for(y=0; y<8; y++) {
    LCD_WrCmd(0xb0+y);
    LCD_WrCmd(0x01);
    LCD_WrCmd(0x10);
    for(x=0; x<X_WIDTH; x++)
      LCD_WrDat(bmp_dat);
  }
}
void LCD_CLS(void) {
  unsigned char y,x;
  for(y=0; y<8; y++) {
    LCD_WrCmd(0xb0+y);
    LCD_WrCmd(0x01);
    LCD_WrCmd(0x10);
    for(x=0; x<X_WIDTH; x++)
      LCD_WrDat(0);
  }
}
void LCD_DLY_ms(unsigned int ms) {
  unsigned int a;
  while(ms) {
    a=1335;
    while(a--);
    ms--;
  }
  return;
}
void LCD_Init(void) {
  digitalWrite(14, 1); //LCD_SCL=1;
  digitalWrite(10, 1); //LCD_CS=1;
  digitalWrite(6, 0); //LCD_RST=0;
  delay(50); //LCD_DLY_ms(50);
  digitalWrite(6, 1); //LCD_RST=1;
  LCD_WrCmd(0xae);//--turn off oled panel
  LCD_WrCmd(0x00);//---set low column address
  LCD_WrCmd(0x10);//---set high column address
  LCD_WrCmd(0x40);//--set start line address Set Mapping RAM Display Start Line (0x00~0x3F)
  LCD_WrCmd(0x81);//--set contrast control register
  LCD_WrCmd(Brightness); // Set SEG Output Current Brightness
  LCD_WrCmd(0xa1);//--Set SEG/Column Mapping
  LCD_WrCmd(0xc8);//Set COM/Row Scan Direction
  LCD_WrCmd(0xa6);//--set normal display
  LCD_WrCmd(0xa8);//--set multiplex ratio(1 to 64)
  LCD_WrCmd(0x3f);//--1/64 duty
  LCD_WrCmd(0xd3);//-set display offset Shift Mapping RAM Counter (0x00~0x3F)
  LCD_WrCmd(0x00);//-not offset
  LCD_WrCmd(0xd5);//--set display clock divide ratio/oscillator frequency
  LCD_WrCmd(0x80);//--set divide ratio, Set Clock as 100 Frames/Sec
  LCD_WrCmd(0xd9);//--set pre-charge period
  LCD_WrCmd(0xf1);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
  LCD_WrCmd(0xda);//--set com pins hardware configuration
  LCD_WrCmd(0x12);
  LCD_WrCmd(0xdb);//--set vcomh
  LCD_WrCmd(0x40);//Set VCOM Deselect Level
  LCD_WrCmd(0x20);//-Set Page Addressing Mode (0x00/0x01/0x02)
  LCD_WrCmd(0x02);//
  LCD_WrCmd(0x8d);//--set Charge Pump enable/disable
  LCD_WrCmd(0x14);//--set(0x10) disable
  LCD_WrCmd(0xa4);// Disable Entire Display On (0xa4/0xa5)
  LCD_WrCmd(0xa6);// Disable Inverse Display On (0xa6/a7)
  LCD_WrCmd(0xaf);//--turn on oled panel
  LCD_Fill(0x00); // fill with "black" pixel
  LCD_Set_Pos(0,0);
}
//==============================================================
// LCD_P6x8Str(unsigned char x,unsigned char y,unsigned char *p)
//==============================================================
void LCD_P6x8Str(unsigned char x,unsigned char y,unsigned char ch[]) {
  unsigned char c=0,i=0,j=0;
  while (ch[j]!='\0') {
    c =ch[j]-32;
    if(x>126) {
      x=0;
      y++;
    }
    LCD_Set_Pos(x,y);
    for(i=0; i<6; i++)
      LCD_WrDat(F6x8[c][i]);
    x+=6;
    j++;
  }
}
//==================================
void getIpaddress(char *if_name) {
  struct ifreq ifr;
  size_t if_name_len=strlen(if_name);
  if (if_name_len<sizeof(ifr.ifr_name)) {
    memcpy(ifr.ifr_name,if_name,if_name_len);
    ifr.ifr_name[if_name_len]=0;
  } else {
    printf("ERROR: interface name is too long\n");
    exit(1);
  }
  int fd=socket(AF_INET,SOCK_DGRAM,0);
  if (fd==-1) {
    printf("ERROR: %s\n",strerror(errno));
    exit(1);
  }
  if (ioctl(fd,SIOCGIFADDR,&ifr)==-1) {
    int temp_errno=errno;
    close(fd);
    printf("ERROR: %s\n",strerror(temp_errno));
    exit(1);
  }
  close(fd);
  struct sockaddr_in* ipaddr = (struct sockaddr_in*)&ifr.ifr_addr;
  strcpy(ipaddress, (char *)inet_ntoa(ipaddr->sin_addr));
  fd=socket(AF_INET,SOCK_DGRAM,0);
  if (fd==-1) {
    printf("ERROR: %s\n",strerror(errno));
    exit(1);
  }
  if (ioctl(fd,SIOCGIFNETMASK,&ifr)==-1) {
    int temp_errno=errno;
    close(fd);
    printf("ERROR: %s\n",strerror(temp_errno));
    exit(1);
  }
  close(fd);
  struct sockaddr_in* nmask = (struct sockaddr_in*)&ifr.ifr_netmask;
  strcpy(netmask, (char *)inet_ntoa(nmask->sin_addr));
  fd=socket(AF_UNIX,SOCK_DGRAM,0);
  if (fd==-1) {
    printf("ERROR: %s\n",strerror(errno));
    exit(1);
  }
  if (ioctl(fd,SIOCGIFHWADDR,&ifr)==-1) {
    int temp_errno=errno;
    close(fd);
    printf("ERROR: %s\n",strerror(temp_errno));
    exit(1);
  }
  close(fd);
  if (ifr.ifr_hwaddr.sa_family!=ARPHRD_ETHER) {
    printf("ERROR: not an Ethernet interface\n");
    exit(1);
  }
  const unsigned char* mac=(unsigned char*)ifr.ifr_hwaddr.sa_data;
  sprintf(hwaddr, "%02X:%02X:%02X:%02X:%02X:%02X",
          mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);

}

byte getProcessRunning(char *pname) {
  DIR *dirp;
  FILE *fp;
  struct dirent *entry;
  int *pidlist, pidlist_index = 0, pidlist_realloc_count = 1;
  char path[MAX_BUF], read_buf[MAX_BUF];

  dirp = opendir ("/proc/");
  if (dirp == NULL) {
    perror ("Fail");
    printf("ERROR: failed to read tree of running processes\n");
    exit(1);
    return;
  }

  pidlist = malloc (sizeof (int) * PID_LIST_BLOCK);
  if (pidlist == NULL) {
    printf("ERROR: failed generating a list of running processes\n");
    exit(1);
    return;
  }

  while ((entry = readdir (dirp)) != NULL) {
    if (atoi(entry->d_name)) {
      strcpy (path, "/proc/");
      strcat (path, entry->d_name);
      strcat (path, "/comm");

      /* A file may not exist, it may have been removed.
       * dut to termination of the process. Actually we need to
       * make sure the error is actually file does not exist to
       * be accurate.
       */
      fp = fopen (path, "r");
      if (fp != NULL) {
        fscanf (fp, "%s", read_buf);
        if (strcmp (read_buf, pname) == 0) {
          /* add to list and expand list if needed */
          pidlist[pidlist_index++] = atoi (entry->d_name);
          if (pidlist_index == PID_LIST_BLOCK * pidlist_realloc_count) {
            pidlist_realloc_count++;
            pidlist = realloc (pidlist, sizeof (int) * PID_LIST_BLOCK * pidlist_realloc_count); //Error check todo
            if (pidlist == NULL) {
              printf("ERROR: something went wrong here...\n");
              exit(1);
              return;
            }
          }
        }
        fclose (fp);
      }
    }
  }


  closedir (dirp);
  pidlist[pidlist_index] = -1; /* indicates end of list */
  if (pidlist[0] != -1) {
    return 1;
  } else {
    return 0;
  }
}

char *toUp(char *cstring) {
  int i = 0;
  char str[255];
  strcpy(str,cstring);
  char c;
  while  (str[i]) {
    c = str[i];
    str[i] = toupper(c);
    i++;
  }
  char *returnString;
  strcpy(returnString, str);
  return returnString;
}
char *getVoltage(void) {
  char voltbuff[100];
  char vVal[7] = {0};
  FILE *pipe = popen("/opt/vc/bin/vcgencmd measure_volts core", "r" );
  if (pipe == NULL ) {
    printf("ERROR: error getting system voltage");
    return;
  } else {
    while(!feof(pipe) )
      fgets( voltbuff, 100, pipe );

    pclose(pipe);
  }
  vVal[0] = voltbuff[5];
  vVal[1] = voltbuff[6];
  vVal[2] = voltbuff[7];
  vVal[3] = voltbuff[8];
  vVal[4] = voltbuff[9];
  vVal[5] = voltbuff[10];
  vVal[6] = '\0';
  char *retVal = vVal;
  return retVal;
}


char *getCpuTemp(void) {
  char tempbuff[100];
  char tempVal[5] = {0};
  FILE *pipe = popen("/opt/vc/bin/vcgencmd measure_temp", "r" );
  if (pipe == NULL ) {
    printf("ERROR: error getting temp");
    return;
  } else {
    while(!feof(pipe) )
      fgets( tempbuff, 100, pipe );

    pclose(pipe);
  }
  tempVal[0] = tempbuff[5];
  tempVal[1] = tempbuff[6];
  tempVal[2] = tempbuff[7];
  tempVal[3] = tempbuff[8];
  tempVal[4] = '\0';
  char *retVal = tempVal;
  return retVal;
}
void main(int argc, char *argv[]) {

  struct arguments arguments;

  arguments.unit = CELSIUS;
  arguments.verbosity = VERBOSE;
  arguments.watch = PROCESS_NAME;
  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  unsigned char i=0;
  wiringPiSetup();
  pinMode(10, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(6, OUTPUT);
  getIpaddress(INTERFACE);
  char displayip[24] = {0};
  char displaynmask[24] = {0};
  char displayhwa[24] = {0};
  char displaypid[24] = {0};
  char displaytemp[24] = {0};
  char displayvolts[24] = {0};
  char displayuptime[24] = {0};
  if (arguments.verbosity) {
    printf("IPv4: %s\n", ipaddress);
  }
  sprintf(displayip, "IPv4: %15s", ipaddress);
  if (arguments.verbosity) {
    printf("NETM: %s\n", netmask);
  }
  sprintf(displaynmask, "NETMSK: %13s", netmask);
  if (arguments.verbosity) {
    printf("HW: %s\n", hwaddr);
  }
  sprintf(displayhwa, "HW: %17s", hwaddr);

  if ((int)arguments.watch == 0) {
    byte procStatus = getProcessRunning(PROCESS);
    char *upperCase = toUp(PROCESS);
    strcat(upperCase, ":");
    if (procStatus == 1) {
      if (arguments.verbosity) {
        printf("%s running\n", upperCase);
      }
      sprintf(displaypid, "%-13s running", upperCase);
    } else {
      if (arguments.verbosity) {
        printf("%s not running\n", upperCase);
      }
      sprintf(displaypid, "%-9s not running", upperCase);
    }
  } else {
    int checkpid;
    checkpid = kill(atoi(PID), 0);
    if (checkpid == 0) {
      if (arguments.verbosity) {
        printf("PID(%s) is running\n", PID);
      }
      sprintf(displaypid, "PID %-13s running", PID);
    } else {
      if (arguments.verbosity) {
        printf("PID(%s) is not running\n", PID);
      }
      sprintf(displaypid, "PID %-9s not running", PID);
    }
  }
  char *myTemp = getCpuTemp();
  char myTemperature[36];
  if (!(arguments.unit)) {
    double myFahrenheit = atof(myTemp);
    myFahrenheit = (myFahrenheit * 1.8);
    myFahrenheit = (myFahrenheit + 32);
    sprintf(myTemperature, "%g", myFahrenheit);
  }
  if (arguments.unit) {
    if (arguments.verbosity) {
      printf("CPU Temp: %s °C\n", myTemp);
    }
    sprintf(displaytemp, "CPU Temp: %8s 'C", myTemp);
  } else {
    if (arguments.verbosity) {
      printf("CPU Temp: %s °F\n", myTemperature);
    }
    sprintf(displaytemp, "CPU Temp: %8s 'F", myTemperature);
  }
  char *myVoltage = getVoltage();
  if (arguments.verbosity) {
    printf("Voltage: %s\n", myVoltage);
  }
  sprintf(displayvolts, "Core Voltage: %5s V", myVoltage);
  struct sysinfo info;
  sysinfo(&info);
  sprintf(displayuptime, "Uptime: %5s%02ld:%02ld:%02ld", " ", info.uptime/3600, info.uptime%3600/60, info.uptime%60);
  LCD_Init();
  LCD_P6x8Str(0,7,displayuptime);
  LCD_P6x8Str(0,6,displayvolts);
  LCD_P6x8Str(0,5,displaytemp);
  LCD_P6x8Str(0,4,displaypid);
  LCD_P6x8Str(0,3,displayhwa);
  LCD_P6x8Str(0,2,displaynmask);
  LCD_P6x8Str(0,1,displayip);
}
