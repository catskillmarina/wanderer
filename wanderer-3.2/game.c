/* File game.c */
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

extern int move_monsters();
extern void showname();
extern int jumpscreen();
extern int check();
extern void showpass();
extern void draw_symbol();
extern void display();
extern int fall();
extern void map();
extern void redraw_screen();
extern struct mon_rec *make_monster();

/***************************************************************
*         function declarations    from this file              *
****************************************************************/
char *playscreen(int *num, long *score, int *bell, int maxmoves,char keys[10]);

extern int debug_disp;
extern int edit_mode;
extern int saved_game;
extern int record_file;
extern char screen[NOOFROWS][ROWLEN+1];
extern char *edit_memory;
extern char *memory_end;

struct mon_rec start_of_list = {0,0,0,0,0,NULL,NULL};
struct mon_rec *last_of_list, *tail_of_list;

/*************************************************************** 
*               function playscreen (the game)                 *
****************************************************************/
/********************************************************************
* Actual game function - Calls fall() to move boulders and arrows   *
*        recursively.                                               *
*   Variable explaination :                                         *
*     All the var names make sense to ME, but some people           *
*     think them a bit confusing... :-) So heres an explanation.    *
*   x,y : where you are                                             *
*   nx,ny : where you're trying to move to                          *
*   sx,sy : where the screen window on the playing area is          *
*   mx,my : where the monster is                                    *
*   tx,ty : teleport arrival                                        *
*   bx,by : baby monster position                                   *
*   nbx,nby : where it wants to be                                  *
*   lx,ly : the place you left when teleporting                     *
*   nf : how many diamonds youve got so far                         *
*   new_disp : the vector the baby monster is trying                *
*********************************************************************/

char *playscreen(int *num, long *score, int *bell, int maxmoves, char keys[10])
{
    int  x,y,nx,ny,deadyet =0,
         sx = -1,sy = -1,tx = -1,ty = -1,lx = 0,ly = 0,mx = -1,my = -1,
         newnum, recording = 0, diamonds = 0, nf = 0, tmpx, tmpy;
    char (*frow)[ROWLEN+1] = screen,
         ch, *memory_ptr,
         buffer[25];
    struct mon_rec *monster;
    static char     howdead[25];  /* M001 can't use auto var for return value */

    tail_of_list = &start_of_list;
    memory_ptr = edit_memory;

    for(x=0;x<=ROWLEN;x++)
        for(y=0;y<NOOFROWS;y++)
        {
            if((screen[y][x] == '*')||(screen[y][x] == '+'))
                diamonds++;
            if(screen[y][x] == 'A')     /* note teleport arrival point &  */
            {                       /* replace with space */
                tx = x;
                ty = y;
                 screen[y][x] = ' ';
            }
            if(screen[y][x] == '@')
            {
                sx = x;
                sy = y;
            }
            if(screen[y][x] == 'M')     /* Put megamonster in */
            {
                mx = x;
                my = y;
            }
            if(screen[y][x] == 'S')   /* link small monster to pointer chain */
            {
            if((monster = make_monster(x,y)) == NULL)
            {
                strcpy(howdead,"running out of memory");
                return howdead;
            }
            if(!viable(x,y-1))     /* make sure its running in the correct */
            {                  /* direction..                          */
                monster->mx = 1;
                monster->my = 0;
            }
            else if(!viable(x+1,y))
            {
                monster->mx = 0;
                monster->my = 1;
            }
            else if(!viable(x,y+1))
            {
                monster->mx = -1;
                monster->my = 0;
            }
            else if(!viable(x-1,y))
            {
                monster->mx = 0;
                monster->my = -1;
            }
        }
        if(screen[y][x] == '-')
                screen[y][x] = ' ';
    };
    x=sx;
    y=sy;
    if((x == -1)&&(y == -1))              /* no start position in screen ? */
    {
        strcpy(howdead,"a screen design error");
        return(howdead);
    }
    
    if(maxmoves < 1) maxmoves = -1;
    
    update_game:        /* M002  restored game restarts here        */
    
    redraw_screen(bell,maxmoves,*num,*score,nf,diamonds,mx,sx,sy,frow);
    
/* ACTUAL GAME FUNCTION - Returns method of death in string  */
    
    while(deadyet == 0)
    {
        switch(recording)
        {
          case 1:
              ch = getch();
              *memory_ptr++ = ch;
               memory_end++;
               break;
         case 2:
              ch = *memory_ptr++;
              if( ch == '\0' )
                    ch = ')';
              break;
         default:
              ch = getch();
        }
        if((record_file != -1)&&(ch != 'q')) write(record_file,&ch,1);
        
        nx=x;
        ny=y;
        
        if((ch == keys[3]) && (x <(ROWLEN-1)))
                        /* move about - but thats obvious */
            nx++;
        if((ch == keys[2]) && (x > 0))
            nx--;
        if((ch == keys[1]) && (y <(NOOFROWS-1)))
            ny++;
        if((ch == keys[0]) && (y > 0))
            ny--;
        if(ch == '1')                  /* Add or get rid of that awful sound */
        {
            move(17,56);
            *bell = 1;
            (void) addstr("Bell ON ");
            move(16,0);
            refresh();
            continue;
        }
        if(ch == '0')
        {
            *bell = 0;
            move(17,56);
            (void) addstr("Bell OFF");
            move(16,0);
            refresh();
            continue;
        }
        if(ch == '~')                             /* level jump */
        {
            if(edit_mode)
                continue;
            if((newnum = jumpscreen(*num)) == 0)
            {
                strcpy(howdead,"a jump error.");
                return howdead;
            }
            if(newnum != *num)
            {                   /* Sorry Greg, no points for free */
                sprintf(howdead,"~%c",newnum);
                return howdead;
            }
            continue;
        }
        if(ch == '!')                      /* look at the map */
        {
            if(debug_disp)
                continue;
            map(frow);
            display(sx,sy,frow,*score);
            continue;
        }
        if(ch == 'q')
        {
            strcpy(howdead,"quitting the game");
            return howdead;
        }
        if(ch == '?')
        {
            helpme(1);
            if(debug_disp)
                map(frow);
            else
                display(sx,sy,frow,*score);
            continue;
        }
        if((ch == '@')&&(!debug_disp))
        {
            sx = x;
            sy = y;
            display(sx,sy,frow,*score);
            continue;
        }
        if(ch == '#')
        {
            debug_disp = 1 - debug_disp;
            if(debug_disp)
                    map(frow);
            else
            {
                     for(tmpy=0;tmpy<=(NOOFROWS+1);tmpy++)
                     {
                            move(tmpy,0);
                            for(tmpx=0;tmpx<=(ROWLEN+2);tmpx++)
                                        addch(' ');
                     }
                    sx = x; sy = y;
                    display(sx,sy,frow,*score);
            }
            continue;
        }
        if((ch == 'W') || ( (int)ch == 12 ))
        {
            redraw_screen(bell,maxmoves,*num,*score,nf,diamonds,mx,sx,sy,frow);
            continue;
        }
    
    /* Edit screen memory functions */
        if(edit_memory)
        {
            if((ch == '(') && (recording == 0)) 
                                      /* start recording from beginning */
            {
                memory_end = memory_ptr = edit_memory;
                move(10,53);
                addstr(" -Recording-");
                refresh();
                recording = 1;
                continue;
            }
            if((ch == ')') && ( recording != 0 ))
                                         /* stop recording or playback */
            {
                recording = 0;
                move(10,53);
                addstr(" -- End --  ");
                refresh();
                move(10,53);
                addstr(" -Occupied- ");
                continue;
            }
            if((ch == '*') && (recording == 0))  /* playback memory */
            {
                memory_ptr = edit_memory;
                recording = 2;
                move(10,53);
                addstr(" -Playback- ");
                refresh();
                continue;
            }
            if((ch == '&') && (recording == 0))/* extend recording,either from*/
            {                                /* end or from checkpoint        */
                recording = 1;
                move(10,53);
                addstr(" -Recording-");
                refresh();
                if(*(memory_ptr -1) != '-')
                        memory_ptr--;
                continue;
            }
            if((ch == '+') && (recording == 0) && (memory_ptr < memory_end))
            {
            /* continue from checkpoint */
                recording = 2;
                move(10,53);
                addstr(" -Playback- ");
                refresh();
                continue;
            }
            if(( ch == '-') && (recording != 0)) 
                                            /*create or react to a checkpoint*/
            {
            /* checkpoint */
                move(10,53);
                addstr("-Checkpoint-");
                refresh();
                move(10,53);
                if( recording == 2)
                {
                    recording = 0;
                    addstr(" -Occupied- ");
                } else addstr(" -Recording-");
                continue;
            }
        }
        /* end of memory functions */
        
        /* M002  Added save/restore game feature.  Gregory H. Margo        */
        if(ch == 'S')           /* save game */
        {
                extern        struct        save_vars        zz;
        
            /* stuff away important local variables to be saved */
            /* so the game state may be acurately restored        */
                zz.z_x                = x;
                zz.z_y                = y;
                zz.z_sx                = sx;
                zz.z_sy                = sy;
                zz.z_tx                = tx;
                zz.z_ty                = ty;
                zz.z_mx                = mx;
                zz.z_my                = my;
                zz.z_diamonds        = diamonds;
                zz.z_nf                = nf;
        
                save_game(*num, score, bell, maxmoves, &start_of_list,tail_of_list);
                /* NOTREACHED ... unless there's been an error. */
        }
        if(ch == 'R')            /* restore game */
        {
                extern        struct        save_vars        zz;
        
                restore_game(num,score,bell,&maxmoves,&start_of_list,&tail_of_list);
        
                /* recover important local variables */
                x                = zz.z_x;
                y                = zz.z_y;
                sx                = zz.z_sx;
                sy                = zz.z_sy;
                tx                = zz.z_tx;
                ty                = zz.z_ty;
                mx                = zz.z_mx;
                my                = zz.z_my;
                diamonds        = zz.z_diamonds;
                nf                = zz.z_nf;
        
                goto update_game;        /* the dreaded goto        */
        }
        
        if(screen[ny][nx] == 'C')
        {
            screen[ny][nx] = ':';
            *score+=4;
            if(maxmoves != -1)
                maxmoves+=250;
        }
        switch(screen[ny][nx])
        {
            case '@': break;
            case '*': *score+=9;
                nf++;
        #ifdef NOISY
                if(*bell)printf("\007");
        #endif
            case ':': *score+=1;
                move(3,48);
                sprintf(buffer,"%d\t %d",*score,nf);
                (void) addstr(buffer);
            case ' ':
                screen[y][x] = ' ';
                   screen[ny][nx] = '@';
                if(!debug_disp)
                {
                    draw_symbol((x-sx+5)*3,(y-sy+3)*2,' ');
                    draw_symbol((nx-sx+5)*3,(ny-sy+3)*2,'@');
                }
                else
                {
                    move(y+1,x+1);
                    addch(' ');
                    move(ny+1,nx+1);
                    addch('@');
                }
                deadyet += check(&mx,&my,x,y,nx-x,ny-y,sx,sy,howdead);
                move(16,0);
                refresh();
                y = ny;
                x = nx;
                break;
            case 'O':
                if((nx == 0 )||(nx == ROWLEN)) break;
                if(screen[y][nx*2-x] == 'M')
                {
                    screen[y][nx*2-x] = ' ';
                    mx = my = -1;
                    *score+=100;
                    move(3,48);
                    sprintf(buffer,"%d\t %d\t %d ",*score,nf,diamonds);
                    (void) addstr(buffer);
                    draw_symbol(50,11,' ');
                    move(12,56); addstr("              ");
                        move(13,56); addstr("              ");
                    move(16,0);
                    refresh();
                }
                if(screen[y][nx*2-x] == ' ')
                {
                    screen[y][nx*2-x] = 'O';
                    screen[y][x] = ' ';
                    screen[ny][nx] = '@';
                    if(!debug_disp)
                    {
                        draw_symbol((x-sx+5)*3,(y-sy+3)*2,' ');
                        draw_symbol((nx-sx+5)*3,(ny-sy+3)*2,'@');
                        if(nx*2-x>sx-6&&nx*2-x<sx+6)
                            draw_symbol((nx*2-x-sx+5)*3,(y-sy+3)*2,'O');
                    }
                    else
                    {
                        move(y+1,x+1);
                        addch(' ');
                        move(ny+1,nx+1);
                        addch('@');
                        move(y+1,nx*2-x+1);
                        addch('O');
                    }
                    deadyet += fall(&mx,&my,nx*2-x,y+1,sx,sy,howdead);
                    deadyet += fall(&mx,&my,x*2-nx,y,sx,sy,howdead);
                    deadyet += fall(&mx,&my,x,y,sx,sy,howdead);
                    deadyet += fall(&mx,&my,x,y-1,sx,sy,howdead);
                    deadyet += fall(&mx,&my,x,y+1,sx,sy,howdead);
                    move(16,0);
                    refresh();
                    y = ny;
                    x = nx;
                }
                break;
            case '^':
                if((nx == 0 )||(nx == ROWLEN)) break;
                if(screen[y][nx*2-x] == ' ')
                {
                    screen[y][nx*2-x] = '^';
                    screen[y][x] = ' ';
                    screen[ny][nx] = '@';
                    if(!debug_disp)
                    {
                        draw_symbol((x-sx+5)*3,(y-sy+3)*2,' ');
                        draw_symbol((nx-sx+5)*3,(ny-sy+3)*2,'@');
                        if(nx*2-x>sx-6&&nx*2-x<sx+6)
                            draw_symbol((nx*2-x-sx+5)*3,(y-sy+3)*2,'^');
                    }
                    else
                    {
                        move(y+1,x+1);
                        addch(' ');
                        move(ny+1,nx+1);
                        addch('@');
                        move(y+1,nx*2-x+1);
                        addch('^');
                    }
                    deadyet += fall(&mx,&my,nx*2-x,y-1,sx,sy,howdead);
                    deadyet += fall(&mx,&my,x*2-nx,y,sx,sy,howdead);
                    deadyet += fall(&mx,&my,x,y,sx,sy,howdead);
                    deadyet += fall(&mx,&my,x,y+1,sx,sy,howdead);
                    deadyet += fall(&mx,&my,x,y-1,sx,sy,howdead);
                    move(16,0);
                    refresh();
                    y = ny;
                    x = nx;
                }
                break;
            case '<':
            case '>':
                if((ny == 0) || ( ny == NOOFROWS)) break;
                if(screen[ny*2-y][x] == 'M')
                {
                    screen[ny*2-y][x] = ' ';
                    mx = my = -1;
                    *score+=100;
                    move(3,48);
                    sprintf(buffer,"%d\t %d\t %d ",*score,nf,diamonds);
                    (void) addstr(buffer);
                    draw_symbol(50,11,' ');
                       move(12,56); addstr("              ");
                      move(13,56); addstr("              ");
                    move(16,0);
                    refresh();
                }
                if(screen[ny*2-y][x] == ' ')
                {
                    screen[ny*2-y][x] = screen[ny][nx];
                    screen[y][x] = ' ';
                    screen[ny][nx] = '@';
                    if(!debug_disp)
                    {
                        draw_symbol((x-sx+5)*3,(y-sy+3)*2,' ');
                        draw_symbol((nx-sx+5)*3,(ny-sy+3)*2,'@');
                        if(ny*2-y>sy-4&&ny*2-y<sy+4)
                            draw_symbol((x-sx+5)*3,(ny*2-y-sy+3)*2,screen[ny*2-y][x]);
                    }
                    else
                    {
                        move(y+1,x+1);
                        addch(' ');
                        move(ny+1,nx+1);
                        addch('@');
                        move(ny*2-y+1,x+1);
                        addch(screen[ny*2-y][x]);
                    }
                    deadyet += fall(&mx,&my,x,y,sx,sy,howdead);
                    deadyet += fall(&mx,&my,x-1,(ny>y)?y:(y-1),sx,sy,howdead);
                    deadyet += fall(&mx,&my,x+1,(ny>y)?y:(y-1),sx,sy,howdead);
                    deadyet += fall(&mx,&my,x-1,ny*2-y,sx,sy,howdead);
                    deadyet += fall(&mx,&my,x+1,ny*2-y,sx,sy,howdead);
                    move(16,0);
                    refresh();
                    y = ny;
                    x = nx;
                    }
                break;
            case '~':
                if(((2*nx-x) < 0) ||((ny*2-y) > NOOFROWS)||((ny*2-y) < 0)||((2*nx-x) > ROWLEN)) break;
                if(screen[ny*2-y][nx*2 -x] == 'M')
                {
                    screen[ny*2-y][nx*2-x] = ' ';
                    mx = my = -1;
                    *score+=100;
                    move(3,48);
                    sprintf(buffer,"%d\t %d\t %d ",*score,nf,diamonds);
                    (void) addstr(buffer);
                    draw_symbol(50,11,' ');
                       move(12,56); addstr("              ");
                      move(13,56); addstr("              ");
                    move(16,0);
                    refresh();
                }
                if(screen[ny*2-y][nx*2-x] == ' ')
                {
                    screen[ny*2-y][nx*2-x] = '~';
                    screen[y][x] = ' ';
                    screen[ny][nx] = '@';
                    if(!debug_disp)
                    {
                        draw_symbol((x-sx+5)*3,(y-sy+3)*2,' ');
                        draw_symbol((nx-sx+5)*3,(ny-sy+3)*2,'@');
                        if((ny*2-y>sy-4)&&(ny*2-y<sy+4))
                            draw_symbol((nx*2-x-sx+5)*3,(ny*2-y-sy+3)*2,'~');
                    }
                    else
                    {
                        move(y+1,x+1);
                        addch(' ');
                        move(ny+1,nx+1);
                        addch('@');
                        move(ny*2-y+1,nx*2-x+1);
                        addch('~');
                    }
                    deadyet += check(&mx,&my,x,y,nx-x,ny-y,sx,sy,howdead);
                    move(16,0); refresh();
                    y = ny; x = nx;
                }
                break;
            case '!':
                strcpy(howdead,"an exploding landmine");
                deadyet = 1;
                if(!debug_disp)
                {
                        draw_symbol((x-sx+5)*3,(y-sy+3)*2,' ');
                        draw_symbol((nx-sx+5)*3,(ny-sy+3)*2,'%');
                }
                else
                {
                    move(y+1,x+1);
                    addch(' ');
                    move(ny+1,nx+1);
                    addch('@');
                }
                move(16,0);
                refresh();
                break;
            case 'X':
                if(nf == diamonds)
                {
                    *score+=250;
                    showpass(*num);
                    return NULL;
                }
                break;
            case 'T':
                if(tx > -1)
                {
                    screen[ny][nx] = ' ';
                    screen[y][x] = ' ';
                    lx = x;
                    ly = y;
                    y = ty;
                    x = tx;
                    screen[y][x] = '@';
                    sx = x;
                    sy = y;
                    *score += 20;
                    move(3,48);
                    sprintf(buffer,"%d\t %d\t %d ",*score,nf,diamonds);
                    (void) addstr(buffer);
                    if(!debug_disp)
                        display(sx,sy,frow,*score);
                    else
                        map(frow);
                    deadyet += fall(&mx,&my,nx,ny,sx,sy,howdead);
                    if(deadyet == 0)
                        deadyet = fall(&mx,&my,lx,ly,sx,sy,howdead);
                    if(deadyet == 0)
                        deadyet = fall(&mx,&my,lx+1,ly-1,sx,sy,howdead);
                    if(deadyet == 0)
                        deadyet = fall(&mx,&my,lx+1,ly+1,sx,sy,howdead);
                    if(deadyet == 0)
                        deadyet = fall(&mx,&my,lx-1,ly+1,sx,sy,howdead);
                    if(deadyet == 0)
                        deadyet = fall(&mx,&my,lx-1,ly-1,sx,sy,howdead);
                    move(16,0);
                    refresh();
                }
                else
                {
                    screen[ny][nx] = ' ';
                    printf("Teleport out of order");
                }
                break;
            case 'M':
                strcpy(howdead,"a hungry monster");
                deadyet = 1;
                if(!debug_disp)
                        draw_symbol((x-sx+5)*3,(y-sy+3)*2,' ');
                else
                {
                    move(y+1,x+1);
                    addch(' ');
                }
                move(16,0);
                refresh();
                break;
            case 'S':
                strcpy(howdead,"walking into a monster");
                deadyet = 1;
                if(!debug_disp)
                        draw_symbol((x-sx+5)*3,(y-sy+3)*2,' ');
                else
                {
                    move(y+1,x+1);
                    addch(' ');
                }
                move(16,0);
                refresh();
                break;
            default:
                break;
        }
        if((y == ny) && (x == nx) && (maxmoves>0))
        {
            (void) sprintf(buffer,"Moves remaining = %d ",--maxmoves);
            move(15,48);
            (void) addstr(buffer);
        }
        if(maxmoves == 0)
        {
            strcpy(howdead,"running out of time");
            deadyet = 1;
            if(edit_mode) maxmoves = -1;
        }
        if(!debug_disp)
        {
            if ((x<(sx-3))&& (deadyet ==0))  /* screen scrolling if necessary */
            {
                sx-=6;
                if(sx < 4)
                    sx = 4;
                display(sx,sy,frow,*score);
            }
            if ((y<(sy-2))&& (deadyet == 0))
            {
                sy-=5;
                if(sy < 2)
                    sy = 2;
                display(sx,sy,frow,*score);
            }
            if ((x>(sx+3)) && (!deadyet))
            {
                sx+=6;
                if(sx>(ROWLEN -5))
                    sx = ROWLEN -5;
                display(sx,sy,frow,*score);
            }
            if ((y>(sy+2))&& (!deadyet))
            {
                sy+=5;
                if(sy > (NOOFROWS-3))
                    sy = NOOFROWS -3;
                display(sx,sy,frow,*score);
            }
        }
        
        deadyet += move_monsters(&mx,&my,score,howdead,sx,sy,nf,*bell,x,y,diamonds);
        
        if((edit_mode)&&(deadyet))         /* stop death if testing */
        {
            recording = 0;
            move(10,53);
            addstr("-Occupied- ");
            if(!debug_disp)
                move(18,0);
            else
                move(20,0);
            addstr("You were killed by ");
            addstr(howdead);
            addstr("\nPress 'c' to continue.");
            refresh();
            ch=getch();
            if(ch == 'c')
                deadyet = 0;
            if(!debug_disp)
                move(18,0);
             else
                move(20,0);
            addstr("                                                              ");
            addstr("\n                      ");
            refresh();
        }
    }
    return(howdead);
}
