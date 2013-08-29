/* passwords.c */
/***************************************************************************
*  Copyright 2003 -   Steven Shipway <steve@cheshire.demon.co.uk>          *
*                     Put "nospam" in subject to avoid spam filter         *
*                                                                          *
*  This program is free software; you can redistribute it and/or modify    *
*  it under the terms of the GNU General Public License as published by    *
*  the Free Software Foundation; either version 2 of the License, or       *
*  (at your option) any later version.                                     *
*                                                                          *
*  This program is distributed in the hope that it will be useful,         *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of          *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
*  GNU General Public License for more details.                            *
*                                                                          *
*  You should have received a copy of the GNU General Public License       *
*  along with this program; if not, write to the Free Software             *
*  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA               *
*  02111-1307, USA.                                                        *
***************************************************************************/


#include "wand_head.h"

#define PASS "poppycrash"

/***************************************************
*                      scrn_passwd                 *
*            reads password num into passwd        *
****************************************************/
int scrn_passwd(num, passwd)
    int num;
    char *passwd;
{
        long position;
        FILE *fp;

        position = PASSWD;
        if(position < 0)
                position = -position;
        while(position > 200000)
                position -= 200000;
        if(position > 198500)
                position -=198500;
        if((fp = fopen(DICTIONARY,"r")) == NULL)
                return 0;
        fseek(fp,position,ftell(fp));
        while(fgetc(fp) != '\n');
        fscanf(fp,"%s\n",passwd);
        /* read a word into passwd */
        fclose(fp);
        return (1);
}

/************************************************
*                    main                       *
*************************************************/
main(argc,argv)
int argc;
char *argv[];
{
        int scr,position,mpw = 0;
        char pass[20];
        char pw[100];
        if(argc < 2) {
                printf("Usage: %s screen1 screen2 ...\n",argv[0]);
                exit(-1);
        }
        position = 0;
        while(++position < argc) {
                if(atoi(argv[position])<1) {
                        printf("Option not known: %s.\n",argv[position]);
                        continue;
                }
                scr = atoi(argv[position]);
                if((scr < 100) && (mpw == 0)) {
                        printf("You need clearance to see passwords below 100.\n");
                        printf("Enter master password:");
                        scanf("%s",pw);
                        if(strncmp(pw,PASS,strlen(PASS))) {
                                printf("Foo, charlatan!\n");
                                exit(-1);
                        }
                        mpw = 1;
                }
                if( ! scrn_passwd((scr-1),pass)) {
                        printf("%s: Cannot access %s.\n",argv[0],DICTIONARY);
                        exit(-1);
                }
                printf("Screen %d password is '%s'.\n",scr,pass);
        }
}
