/* File encrypt.c */
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

/* Uses seeded random xor to encrypt because setkey doesnt work on our
   system.                                                                 */

crypt_file(name)
char *name;
{
char buffer[1024];
int fd,length,loop;

if((fd = open(name,O_RDONLY)) == -1) {
        endwin();
        sprintf(buffer,"Wanderer: cannot open %s",name);
        perror(buffer);
        exit(1);
}
if((length = read(fd,buffer,1024)) < 1) {
        endwin();
        sprintf(buffer,"Wanderer: read error on %s",name);
        perror(buffer);
        exit(1);
}
close(fd);

/* Right, got it in here, now to encrypt the stuff */

addstr("Running crypt routine....\n");
refresh();

srand(BLURFL);
for(loop=0;loop<length;loop++)
        buffer[loop]^=rand();

if((fd = open(name,O_WRONLY|O_TRUNC))== -1) {
        endwin();
        sprintf(buffer,"Wanderer: cannot write to %s",name);
        perror(buffer);
        exit(1);
}
if(write(fd,buffer,length)!=length) {
        endwin();
        sprintf(buffer,"Wanderer: write error on %s",name);
        perror(buffer);
        exit(1);
}
close(fd);

/* ok, file now contains encrypted/decrypted game. */
/* lets go back home... */
}
