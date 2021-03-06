/*
=================================================================================
 Name        : pcd8544_cli.c
 Version     : 0.1

 Inspirated by http://github.com/binerry/RaspberryPi/tree/master/libraries/c/PCD8544

 Copyright (C) 2013 by Ronan Guilloux, @arno_u_loginlux, http://github.com/ronanguilloux

 Description :
     A simple PCD8544 LCD (Nokia3310/5110) for Raspberry Pi for displaying some string.

     Makes use of

     * WiringPI-library of Gordon Henderson (https://projects.drogon.net/raspberry-pi/wiringpi/)
	 * Raspberry Pi PCD8544 Library
	 * http://binerry.de/post/25787954149/pcd8544-library-for-raspberry-pi.

	 Recommended connection (http://www.raspberrypi.org/archives/384):
	 LCD pins      Raspberry Pi
	 LCD1 - GND    P06  - GND
	 LCD2 - VCC    P01 - 3.3V
	 LCD3 - CLK    P11 - GPIO0
	 LCD4 - Din    P12 - GPIO1
	 LCD5 - D/C    P13 - GPIO2
	 LCD6 - CS     P15 - GPIO3
	 LCD7 - RST    P16 - GPIO4
	 LCD8 - LED    P01 - 3.3V

================================================================================
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.
================================================================================
 */
#include <wiringPi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include "PCD8544.h"

// pin setup
int _din = 1;
int _sclk = 0;
int _dc = 2;
int _rst = 4;
int _cs = 3;

// lcd contrast
int contrast = 45;

// concatenate any number of strings
#include <stdarg.h>       // va_*

char* concat(int count, ...)
{
    va_list ap;
    int i;

    // Find required length to store merged string
    int len = 1; // room for NULL
    va_start(ap, count);
    for(i=0 ; i<count ; i++)
        len += strlen(va_arg(ap, char*));
    va_end(ap);

    // Allocate memory to concat strings
    char *merged = calloc(sizeof(char),len);
    int null_pos = 0;

    // Actually concatenate strings
    va_start(ap, count);
    for(i=0 ; i<count ; i++)
    {
        char *s = va_arg(ap, char*);
        strcpy(merged+null_pos, s);
        null_pos += strlen(s);
    }
    va_end(ap);

    return merged;
}

//int main (void)
int main(int argc, char *argv[])
{
  // check wiringPi setup
  if (wiringPiSetup() == -1)
  {
  printf("wiringPi-Error\n");
    exit(1);
  }

  // init and clear lcd
  LCDInit(_sclk, _din, _dc, _cs, _rst, contrast);
  LCDclear();

  // get system usage / info
  struct sysinfo sys_info;
  if(sysinfo(&sys_info) != 0)
  {
      printf("sysinfo-Error\n");
  }

  if (argc > 3)
  {
    printf("Only one or two argument expected\n");
    exit(1);
  }

  int i;
  char *command;
  char *output;
  char *file_path;
  char *file_content_buffer = NULL;


  output = argv[1];

  for (i = 1; i < argc; i++) {

      /* Check for a switch (leading "-"). */
      if (argv[i][0] == '-') {

          /* Use the next character to decide what to do. */
          switch (argv[i][1]) {

              case 'i':   //a_value = atoi(argv[++i]);
                  command = "hostname -I";
                  output = getCommandLineOutput(command);
                  break;

              // Help
              case 'h':
                  printf("usage: pcd8544_cli [options] <text to display>\n");
                  printf("Available options:\n");
                  printf("\t-h: show this help on usage & options\n");
                  printf("\t-i: show IP address\n");
                  printf("\t-d: show datetime\n");
                  printf("\t-f <file_path>: show the content if the file\n");
                  output = "";
                  break;

              case 'c':   //c_value = argv[++i];
                          break;

              case 'd':
                for (;;)
                {
                  command = "date";
                  output = getCommandLineOutput(command);
                  LCDclear();

                  LCDdrawstring(0, 0, output);
                  LCDdisplay();
                  delay(100);
                }

                  break;

              case 'f':
                file_path = argv[2];
                size_t size = 0;
                for (;;)
                {
                  size = 0;
                  FILE *fp = fopen(file_path, "r");
                  fseek(fp, 0, SEEK_END); 
                  size = ftell(fp);
                  rewind(fp);
                  file_content_buffer = malloc((size + 1) * sizeof(*file_content_buffer));
                  fread(file_content_buffer, size, 1, fp);
                  file_content_buffer[size] = '\0';
  
                  LCDclear();
                  LCDdrawstring(0, 0, file_content_buffer);
                  LCDdisplay();
                  delay(100);
                }

                  break;

              default:
                  printf("pcd8544_cli: invalid option\n");
                  printf("Type « sudo ./pcd8544_cli -h » for more informations\n");
                  output = "";

          }
      }
  }

  // build screen
  LCDdrawstring(0, 0, output);
  LCDdisplay();

  return 0;
}
