/* Copyright (C) 1996 Jeff Moffatt 

   xhpterm: hpterm emulator with imbedded freevt3k

   This file is distributed under the GNU General Public License.
   You should have received a copy of the GNU General Public License
   along with this file; if not, write to the Free Software Foundation,
   Inc., 675 Mass Ave, Cambridge MA 02139, USA.

   Original: Jeff Moffatt, 2 APR 96


   9-MAY-96
      Status readbacks (Esc ^, Esc ~)            Done 
      DC1-DC2-DC1 trigger                        Done
      Obey straps G and H                        Done
      Block mode                                 Done
      Format mode                                Done
      Modify Mode                                Done
      Enter key                                  Done
      Send Display command (Esc d)               Done
      Fix function key programming               Done
      Function key readback (Esc j Esc d Esc k)  Done
      Use mouse to press function keys           Done
      Insert character mode                      Done
      Fix escape code parser                     Done
      Fix break key                              Done
      SPOW(B) latch                              Done
      Send Cursor Position mode                  Done
      Relative cursor positioning                Done
      Auto exit when connection closes           Done
      Insert character during format mode        Done
      Defining form with FORMSPEC doesn't work   Done
      Delete character (Esc P)                   Done

   Things to do:

   Near Future
      Fix X11 window re-size (doesn't scroll to fit new size)
      Fix re-draw problems (performance)
      132 column mode
      Memory lock
      Back tab
      Timed reads

   Future
      Improve keyboard mapping (Alt-M, etc)
      Learn how to make X11 do blinking
      Display functions using tiny letters
      Transmit only fields
      Delete character with wrap
      Insert character with wrap
      Line drawing character set
      Lock/Unlock configuration menus
      Allow display enhancements in key labels
      Keyboard lock

*/
#include <string.h>
#include <stdlib.h>
#include "hpterm.h"
#include "conmgr.h"
#include "x11glue.h"
/*******************************************************************/
#define SHOW_DC1_COUNT 0 
#define DEBUG_BREAK 1 
#define IGNORE_KEYBOARD_LOCK 1
#define IGNORE_USER_SYSTEM_LOCK 1
#define DEBUG_BLOCK_MODE 0
/*******************************************************************/
#define strdup(x) (strcpy(malloc(strlen(x)+1),(x)))
#define MIN(a,b)  (((a)<(b)) ? (a) : (b))
#define MAX(a,b)  (((a)>(b)) ? (a) : (b))
/*******************************************************************/
extern struct conmgr *con;
/*******************************************************************/
#define ASC_DC1 17
#define ASC_DC2 18
static void do_function_button(int);    /* Forward */
static void update_labels(void);        /* Forward */
static void update_cursor(void);        /* Forward */
static void hpterm_rxchar(char);        /* Forward */
struct row * find_cursor_row(void);     /* Forward */
static void do_home_up(void);           /* Forward */
static void do_forward_tab(void);       /* Forward */
/*******************************************************************/
/*
**  Transmit buffer contains characters that are waiting
**  to be given to conmgr.c
*/
unsigned char tx_head=0;
unsigned char tx_tail=0;
char tx_buff[256];
/*******************************************************************/
/*
**  Someday, all routines might have 'term' as their first argument
**  When that happens, this static variable can be removed
*/
static struct hpterm *term=0;
/*
**  Below this line being phased out  
**  They should be part of struct hpterm
*/
static int state=0;
static int cr=0,cc=0;
static int parm=0;
static int nparm=0;
static int sign=0;
static int attr=0;
static int keyn=0;
static int llen=0;
static int slen=0;
static int SPOW_latch=0;
/*******************************************************************/
void term_flush_tx (struct conmgr *con) {
/*
**  Flush buffer from terminal emulator to connection manager
*/
    char ch,termchar;

    while (tx_head != tx_tail) {
        ch = tx_buff[tx_head++];
        if (term->RemoteMode) {
	    conmgr_send (con, &ch, 1);
/*
**          Check if read trigger can be cleared
*/
	    if (term->BlockMode && term->LinePage_D) {
		termchar = term->BlkTerminator;
	    } else {
		termchar = 13;
            }
            if (ch==termchar) {
              term->DC1Count = 0;
              term->DC2Count = 0;
#if SHOW_DC1_COUNT
              update_labels();
#endif
            }
        } else {
/*
**          Terminal is not in remote mode -- perform local echo
*/
            hpterm_rxfunc (term, &ch, 1);
        }
    }
}
/*******************************************************************/
void do_update (int r, struct row *rp, int start_col, int end_col)
/*
**  Perform a partial update of the display
**  Accepts 'r' from 0 to 23. (or more) (or less)
**  Accepts 'start_col' and 'end_col' from 0 to 79.
*/
{
    int i,j,e;
    int n,c;
    int style;

    i = start_col;
    j = MIN(rp->nbchars, end_col+1);
    while (i < j) {
        n = 1;
	while (i+n < j && !(rp->disp[i+n] & HPTERM_ANY_ENHANCEMENT)) {
	    n++;
        }
	e = i;
	while (e && !(rp->disp[e] & HPTERM_ANY_ENHANCEMENT)) {
	    e--;
        }
        style = rp->disp[e] & 0xF;
/*
**      Erase old text first, unless we are using inverse.  When
**      using inverse, we don't need to erase first (Saves time
**      and prevents flashing.)
*/
	if (!(style & HPTERM_INVERSE_MASK)) {
	    disp_erasetext (r, i, n);
        }
        disp_drawtext (style, r, i, &(rp->text[i]), n);
	i += n;
    }
/*
**  Erase the rest of the region 
*/
    if (i < end_col + 1) {
	disp_erasetext (r, i, end_col + 1 - i);
    }
/*
**  Check for needing to update cursor
*/
    if (cr==r && cc>=start_col && cc<=end_col) {
	if (cc >= rp->nbchars) {
	    style = 0;
        } else {
	    e = cc;
	    while (e && !(rp->disp[e] & HPTERM_ANY_ENHANCEMENT)) {
		e--;
            }
	    style = rp->disp[e] & 0xF;
        }
        disp_drawcursor (style, cr, cc);
    }
}
/******************************************************************/
static void request_update (struct row *rp, int start_col, int end_col) {
/*
**  Flag row rp as needing a partial update.
**  Coordinates with previous updates of same row
*/
    if (!rp->changes) {
	rp->changes = 1;
	rp->cstart = start_col;
	rp->cend = end_col;
    } else {
	rp->cstart = MIN(rp->cstart, start_col);
	rp->cend  = MAX(rp->cend, end_col);
    }
}
/******************************************************************/
void update_row (int r, struct row *rp) {

    request_update (rp, 0, term->nbcols - 1);
}
/******************************************************************/
static void update_cursor () {

    struct row *rp;
    int ee,style;

    if (!term->update_all) {
        rp = find_cursor_row();
	request_update (rp, cc, cc);
    }
}
/******************************************************************/
void update_display () {
/*
**  Update entire display
*/
    int r;
    struct row *rp;

    rp = term->dptr;
    for (r=0; r < term->nbrows; r++) {
	if (term->update_all) {
	   do_update (r, rp, 0, term->nbcols - 1);
        } else if (rp->changes) {
	   do_update (r, rp, rp->cstart, rp->cend);
        }
	rp->changes = 0;
        rp = rp->next;
    }
    term->update_all = 0;
}
/*****************************************************************/
void update_menus () {

    struct row *rp;

    rp = term->menu1;
    if (term->update_menus) {
        do_update (term->nbrows, rp, 0, term->nbcols - 1);
    } else if (rp->changes) {
	do_update (term->nbrows, rp, rp->cstart, rp->cend);
    }
    rp->changes = 0;

    rp = term->menu2;
    if (term->update_menus) {
	do_update (term->nbrows+1, rp, 0, term->nbcols - 1);
    } else if (rp->changes) {
	do_update (term->nbrows+1, rp, rp->cstart, rp->cend);
    }
    rp->changes = 0;

    term->update_menus = 0;
}
/*****************************************************************/
void term_redraw () {
/*
**  This routine is called in response to an X Expose event
**  It does a total re-draw
*/
    term->update_all = 1;
    term->update_menus = 1;
    update_display();
    update_menus();
}
/*****************************************************************/
void term_update () {
/*
**  Perform screen updates that have been deferred
*/
    update_display();
    update_menus();
}
/*****************************************************************/
void remove_row (struct row *rp) {
/*
**  Remove row from the linked list
*/
    if (rp->prev) {
       rp->prev->next = rp->next;
    } else {
       term->head = rp->next;
    }
    if (rp->next) {
       rp->next->prev = rp->prev;
    } else {
       term->tail = rp->prev;
    }
}
/****************************************************************/
void insert_before (struct row *ip, struct row *rp) {
/*
**  Insert row ip in front of row rp
*/
    if (rp->prev) {
	rp->prev->next = ip;
	ip->prev = rp->prev;
    } else {
	term->head = ip;
	ip->prev = 0;
    }
    ip->next = rp;
    rp->prev = ip;
}
/****************************************************************/
void insert_after (struct row *ip, struct row *rp) {
/*
**  Insert row ip after row rp
*/
    if (rp->next) { 
       rp->next->prev = ip;
       ip->next = rp->next;
    } else {
       term->tail = ip;
       ip->next = 0;
    }
    ip->prev = rp;
    rp->next = ip;
}
/***************************************************************/
void clear_row (struct row *rp) {

    rp->nbchars = 0;
    memset (rp->text, ' ', 132);
    memset (rp->disp, 0, 132);
}
/*****************************************************************/
struct row * find_cursor_row (void) {
/*
**  Return pointer to the row containing the cursor 
*/
    int ii;
    struct row *rp;
    
    ii = cr;
    rp = term->dptr;
    while (ii && rp->next) {
        rp = rp->next;
        ii--;
    }
    if (ii) {
	printf ("find_cursor_row: ii=%d\n", ii);
    }
    return (rp);
}
/*****************************************************************/
struct row * find_bottom_row () {
/*
**  Return pointer to the bottom row on the screen
*/
    int ii;
    struct row *rp;

    ii = term->nbrows - 1;
    rp = term->dptr;
    while (ii && rp->next) {
	rp = rp->next;
	ii--;
    }
    if (ii) {
	printf ("find_bottom_row: ii=%d\n", ii);
    }
    return (rp);
}
/*****************************************************************/
void erase_cursor () {

    struct row *rp;

    rp = find_cursor_row ();
    request_update (rp, cc, cc);
}
/*****************************************************************/
void show_system () {
/*
**  Display system menu above function buttons
*/
    static char m1[] =
" device  margins/ service   modes            enhance   define            config ";
    static char m2[] =
"control  tabs/col   keys                      video    fields             keys  ";
    strcpy (term->menu1->text, m1);
    strcpy (term->menu2->text, m2);
    term->menu1->nbchars = strlen(m1);
    term->menu2->nbchars = strlen(m2);
}
/*****************************************************************/
void show_device_control () {
/*
**  Display device control menu above function buttons
*/
    static char m1[] =
" LOGGING    TO       TO     Select                                              ";
    static char m2[] =
"   ON      FILE     PIPE   Filename                                             ";

    strcpy (term->menu1->text, m1);
    strcpy (term->menu2->text, m2);
    term->menu1->nbchars = strlen(m1);
    term->menu2->nbchars = strlen(m2);
}
/****************************************************************/
void show_margins_tabs () {
/*
**  Display margin and tabs menu above function buttons
*/
    static char m1[] =
" START     SET     CLEAR   CLR ALL             LEFT    RIGHT   CLR ALL   TAB =  ";
    static char m2[] =
" COLUMN    TAB      TAB      TABS             MARGIN   MARGIN  MARGINS   SPACES ";
    strcpy (term->menu1->text, m1);
    strcpy (term->menu2->text, m2);
    term->menu1->nbchars = strlen(m1);
    term->menu2->nbchars = strlen(m2);
}
/****************************************************************/
void show_config_keys () {
/*
**  Display config keys menu above function buttons
*/
    static char m1[] =
" global           datacomm ext dev           terminal                           ";
    static char m2[] =
" config            config   config            config                            ";

    strcpy (term->menu1->text, m1);
    strcpy (term->menu2->text, m2);
    term->menu1->nbchars = strlen(m1);
    term->menu2->nbchars = strlen(m2);
}
/***************************************************************/
void show_terminal_config () {
/*
**  Display terminal config menu above function buttons
*/
    static char m1[] =
"                  DEFAULT                                                       ";
    static char m2[] =
"                   CONFIG                             USASCII   ROMAN8   ASCII8 ";
    strcpy (term->menu1->text, m1);
    strcpy (term->menu2->text, m2);
    term->menu1->nbchars = strlen(m1);
    term->menu2->nbchars = strlen(m2);
}
/*****************************************************************/
void show_modes () {
/*
**  Display modes above function buttons
*/
    static char m1[] =
"  LINE    MODIFY   BLOCK    REMOTE            SMOOTH   MEMORY  DISPLAY    AUTO  ";
    static char m2[] = 
" MODIFY    ALL      MODE     MODE             SCROLL    LOCK   FUNCTNS     LF   ";

    strcpy (term->menu1->text, m1);
    strcpy (term->menu2->text, m2);
    term->menu1->nbchars = strlen(m1);
    term->menu2->nbchars = strlen(m2);

    if (term->LineModify)   term->menu2->text[7]='*';
    if (term->ModifyAll)    term->menu2->text[7+9]='*';
    if (term->BlockMode)    term->menu2->text[7+18]='*';
    if (term->RemoteMode)   term->menu2->text[7+27]='*';

    if (term->SmoothScroll) term->menu2->text[79-27]='*';
    if (term->MemoryLock)   term->menu2->text[79-18]='*';
    if (term->DisplayFuncs) term->menu2->text[79-9]='*';
    if (term->AutoLineFeed) term->menu2->text[79]='*';
}
/*****************************************************************/
static void update_labels () {
/*
**  Bring term->menu1 and term->menu2 up to date
*/
    struct udf *u;
    int i,j,k,n,s;
    char ch;
/*
**  Set the default display enhancements for the function labels
*/
    memset (term->menu1->disp, 0, 132);
    memset (term->menu2->disp, 0, 132);
    for (i=0; i<8; i++) {
	j = i * 9;
	if (i > 3) j += 9;
	term->menu1->disp[j] = HPTERM_INVERSE_MASK | HPTERM_HALFBRIGHT_MASK;
	term->menu1->disp[j+8] = HPTERM_END_ENHANCEMENT;
	term->menu2->disp[j] = HPTERM_INVERSE_MASK | HPTERM_HALFBRIGHT_MASK;
	term->menu2->disp[j+8] = HPTERM_END_ENHANCEMENT;
    }

    if (term->Message) {
 
	memset (term->menu1->text, ' ', 132);
	memset (term->menu2->text, ' ', 132);
	memset (term->menu1->disp, 0, 132);
	memset (term->menu2->disp, 0, 132);
	i = strlen(term->Message);
	j = 0;
	if (i>80) {
	    j = i - 80;
	    i = 80;
        }
	if (j>80) {
	    j = 80;
        }
	if (i) memcpy (term->menu1->text, term->Message, i);
	if (j) memcpy (term->menu2->text, term->Message+i, j);
	term->menu1->nbchars = i;
	term->menu2->nbchars = j;

    } else if (term->KeyState == ks_modes) {
	show_modes();
    } else if (term->KeyState == ks_system) {
	show_system();
    } else if (term->KeyState == ks_device_control) {
	show_device_control();
    } else if (term->KeyState == ks_margins_tabs) {
	show_margins_tabs();
    } else if (term->KeyState == ks_config_keys) {
	show_config_keys();
    } else if (term->KeyState == ks_terminal_config) {
	show_terminal_config();

    } else if (term->KeyState == ks_user) {
	memset (term->menu1->text, ' ', 132);
	memset (term->menu2->text, ' ', 132);
	for (i=0; i<8; i++) {
	    u = term->UserDefKeys[i];
	    j = i * 9;
	    k = 0;
	    s = 0;
	    if (i > 3) j += 9;
	    for (n=0; n < u->LabelLength; n++) {
		ch = u->Label[n];
		if (k < 8) {
		    term->menu1->text[j+k] = ch;
                } else if (k < 16) {
		    term->menu2->text[j+k-8] = ch;
                }
		k++;
            }
        }
	term->menu1->nbchars = 80;
	term->menu2->nbchars = 80;

    } else {
	term->menu1->nbchars = 0;
	term->menu2->nbchars = 0;
    }

    if (!term->Message && term->InsertMode) {
        term->menu2->text[39] = 'I';
        term->menu2->text[40] = 'C';
        term->menu2->disp[39] = HPTERM_HALFBRIGHT_MASK;
        term->menu2->disp[41] = HPTERM_END_ENHANCEMENT;
    }
/*
**  The following is enabled when debugging the DC1 handshaking 
*/
#if SHOW_DC1_COUNT 
    if (term->menu1->nbchars > 50) {
        char temp[20];
        sprintf (temp, "DC1=%d", term->DC1Count);
        memcpy (term->menu1->text + 38, temp, strlen(temp));
    }
    if (term->menu2->nbchars > 50) {
        char temp[20];
        sprintf (temp, "DC2=%d", term->DC2Count);
        memcpy (term->menu2->text + 38, temp, strlen(temp));
    }
#endif
    term->update_menus = 1;
}
/*****************************************************************/
void do_carriage_return () {

    cc = term->LeftMargin;
    SPOW_latch = 1;
}
/*****************************************************************/
void do_line_feed () {

    struct row *rp;

    cr++;
    if (cr >= term->nbrows) {     /* while? */
        rp = find_bottom_row();
	if (!rp->next) {
	    rp = term->head;
	    remove_row (rp);
	    clear_row (rp);
	    insert_after (rp, term->tail);
        }
        term->dptr = term->dptr->next;	
	term->update_all = 1;
        cr--;
    }
    SPOW_latch = 0;
}
/*****************************************************************/
void do_back_space () {

    if (cc) cc--;
}
/*****************************************************************/
int is_cursor_protected () {
/*
**  Returns 1 if the cursor is positioned in a protected region
*/
    struct row *rp;
    int ee;

    rp = find_cursor_row();
    ee = cc;
    if (ee >= rp->nbchars) ee = rp->nbchars - 1;
    while (ee >= 0) {
	if (rp->disp[ee] & HPTERM_START_FIELD) return(0);
	if (rp->disp[ee] & HPTERM_END_FIELD) return (1);
	ee--;
    }
    return (1);
}
/*****************************************************************/
void goto_next_field () {
/*
**  Position the cursor at the start of the next unprotected field,
**  scrolling the display if needed.  If there is no next field,
**  the cursor position and display are left unchanged.
*/
    struct row *rp,*save_dptr;
    int save_cr, save_cc;
/*
**  If the cursor is in an unprotected field, move it out 
**  of the field
*/
    rp = find_cursor_row();
    if (!is_cursor_protected()) {
	while (cc < rp->nbchars && !(rp->disp[cc] & HPTERM_END_FIELD)) {
	    cc++;
        }
    }

    save_dptr = term->dptr;
    save_cr = cr;
    save_cc = cc;

    rp = find_cursor_row();
    while (rp) {
	while (cc < rp->nbchars) {
	    if (rp->disp[cc] & HPTERM_START_FIELD) {
		return;
            } else {
		cc++;
            }
        }
	rp = rp->next;
	if (rp) {
	    cc = 0;
	    do_line_feed();
	}
    }

    term->dptr = save_dptr;
    cr = save_cr;
    cc = save_cc;
}
/*****************************************************************/
void do_delete_char () {
/*
**  Perform delete character function
*/
    struct row *rp;
    int ii;

    rp = find_cursor_row();
    if (term->FormatMode) {
        if (is_cursor_protected()) return;
        ii = cc;
        while (ii+1 < rp->nbchars && !(rp->disp[ii+1] & HPTERM_END_FIELD)) {
            rp->text[ii] = rp->text[ii+1];
            ii++;
        }
        rp->text[ii] = ' ';
        update_row (cr, rp);
    } else if (cc < rp->nbchars) {
        ii = cc;
        while (ii+1 < rp->nbchars) {
            rp->text[ii] = rp->text[ii+1];
            rp->disp[ii] = rp->disp[ii+1];
            ii++;
        }
        rp->text[ii] = ' ';
        rp->disp[ii] = 0;
        rp->nbchars--;
        update_row (cr, rp);
    }
}
/*****************************************************************/
void display_char (ch)
/*
**  Put a character onto the display.
*/
    char ch;
{
    struct row *rp;
    int jj,ee,style;
    int fillflag=0;
/*
**  Make sure cursor is in an unprotected field
*/
    if (term->FormatMode) {
	if (is_cursor_protected()) do_forward_tab();
	if (is_cursor_protected()) return;
    }
/*
**  Find current row in memory
*/
    rp = find_cursor_row();
/*
**  Push characters right if insert is on
*/
    if (term->InsertMode && !term->FormatMode) {
	jj = rp->nbchars;
	if (jj > term->RightMargin) {
	    jj = term->RightMargin;
        } else if (jj == 132) {
	    jj--;
        } else {
	    rp->nbchars++;
        }
	while (jj > cc) {
	    rp->text[jj] = rp->text[jj-1];
	    rp->disp[jj] = rp->disp[jj-1];
	    if (jj == cc + 1) {
		rp->disp[jj] = 0;
            }
	    jj--;
	}
    } else if (term->InsertMode && term->FormatMode) {
        jj = cc;
        while (jj < rp->nbchars && !(rp->disp[jj] & HPTERM_END_FIELD)) {
            jj++;
        }
        jj--;
        while (jj > cc) {
            rp->text[jj] = rp->text[jj-1];
            jj--;
        }
    }
/*
**  Store character into memory
*/
    if (ch==' ' && term->SPOW_B && SPOW_latch) {
        /* Spaces do not overwrite */
    } else {
        rp->text[cc] = ch;
    }
    if (cc > rp->nbchars) fillflag = 1;
    if (cc >= rp->nbchars) rp->nbchars = cc+1;
/*
**  Update screen
*/
    if (!term->update_all) {
	if (term->InsertMode || fillflag) {
	    update_row (cr, rp);        
        } else {
	    request_update (rp, cc, cc);
        }
    }
/*
**  Advance cursor position
*/
    cc++;
    if (cc >= term->nbcols || cc >= term->RightMargin) {
        if (term->InhEolWrp_C) {
            cc--;
        } else {
	    do_carriage_return();
	    do_line_feed();
        }
    }
    if (term->FormatMode && is_cursor_protected()) {
	do_forward_tab();
    }
}
/*****************************************************************/
void do_esc_atsign() { }
void set_security() { }
void do_delete_char_with_wrap() { }
void do_hard_reset() { }
void do_bell() { }
void do_modem_disconnect() { }
/*****************************************************************/
static void do_soft_reset () {

    do_bell ();
    term->EnableKybd = 1;
    if (term->Message) {
        free (term->Message);
        term->Message = 0;
    }
    term->DisplayFuncs = 0;
    term->RecordMode = 0;
}
/*****************************************************************/
static void set_display_enh (char ch) {

    struct row *rp;
    
    rp = find_cursor_row();

    if (ch == '@') {
	rp->disp[cc] = (rp->disp[cc] & 0xE0) | HPTERM_END_ENHANCEMENT;
    } else {
	rp->disp[cc] = (rp->disp[cc] & 0xE0) | (ch & 0xF);
    }
    if (cc >= rp->nbchars) rp->nbchars = cc + 1;
    if (!term->update_all) update_row (cr, rp);
}
/*****************************************************************/
static void set_start_field () {

    struct row *rp;

    rp = find_cursor_row();
    rp->disp[cc] = (rp->disp[cc] & 0x1F) | HPTERM_START_FIELD;
    if (cc >= rp->nbchars) rp->nbchars = cc + 1;
}
/*****************************************************************/
static void set_start_tx_only_field () {

    struct row *rp;

    rp = find_cursor_row();
    rp->disp[cc] = (rp->disp[cc] & 0x1F) | HPTERM_START_TX_ONLY;
    if (cc >= rp->nbchars) rp->nbchars = cc + 1;
}
/*****************************************************************/
static void set_end_field () {

    struct row *rp;

    rp = find_cursor_row();
    rp->disp[cc] = (rp->disp[cc] & 0x1F) | HPTERM_END_FIELD;
    if (cc >= rp->nbchars) rp->nbchars = cc + 1;
}
/*****************************************************************/
static void set_tab () {

    term->TabStops[cc] = 1;
}
/*****************************************************************/
static void clear_tab () {

    term->TabStops[cc] = 0;
}
/*****************************************************************/
static void clear_all_tabs () {

    int ii;
    for (ii=0; ii<132; ii++) {
      term->TabStops[cc] = 0;
    }
}
/*****************************************************************/
static void do_forward_tab () {

    int done;

    if (term->FormatMode) {
/*
**      Move cursor to the beginning of the next unprotected field.
**      At the last field, the cursor wraps around to the beginning 
**      of the first field.
*/
        cc++;
        goto_next_field();
        if (is_cursor_protected()) {
            do_home_up();
        }

    } else {

        if (cc < term->LeftMargin) cc = term->LeftMargin;

        done=0;
        while (!done) {
	    cc++;
	    if (cc >= term->nbcols || cc >= term->RightMargin) {
	        do_line_feed();
	        cc = term->LeftMargin;
	        done=1;
            } else if (term->TabStops[cc]) {
	        done=1;
            }
        }
    }
    SPOW_latch = 0;
}
/*****************************************************************/
static void do_back_tab () {

}
/*****************************************************************/
void do_cursor_up () {
/*
**  Perform 'Cursor Up' function
*/
    erase_cursor();
    if (cr) cr--;
    else cr = term->nbrows - 1;
    update_cursor();
}
/*****************************************************************/
void do_cursor_down () {
/*
**  Perform 'Cursor Down' function
*/
    erase_cursor();
    cr++;
    if (cr >= term->nbrows) {
      cr=0;
    }
    update_cursor();
}
/*****************************************************************/
void do_cursor_right () {
/*
**  Perform 'Cursor Right' function
*/
    erase_cursor();
    cc++;
    if (cc >= term->nbcols) {
        cc=0;
	cr++;
        if (cr >= term->nbrows) {  
          cr=0;
        }
    }
    update_cursor();
}
/*****************************************************************/
void do_cursor_left () {
/*
**  Perform 'Cursor Left' function
*/
    erase_cursor();
    cc--;
    if (cc<0) {
        cc = term->nbcols - 1;
        cr--;
        if (cr<0) {
          cr = term->nbrows - 1;
        }
    }
    update_cursor();
}
/*****************************************************************/
static void do_home_up () {
/*
**  Perform 'Home Up' function
*/
    int ii;

    cr=0;
    cc=0;
    term->dptr = term->head;

    if (term->FormatMode) {
	if (is_cursor_protected()) goto_next_field();
	if (is_cursor_protected()) {
	    cr = 0;
	    cc = 0;
	    term->dptr = term->head;
        }
    }
    term->update_all = 1;
    SPOW_latch = 0;
}
/*****************************************************************/
void do_home_down () {
/*
**  Perform 'Home Down' function
*/
    struct row *rp;
    int ii,jj;
/*
**  Find last line of memory that is in use
*/
    rp = term->tail;
    while (!rp->nbchars && rp->prev) {
	rp = rp->prev;
    }
/*
**  Put cursor column at end of this line
*/
    cc = rp->nbchars;
/*
**  Position this line as low on the display as possible
*/
    cr=0;
    while (rp->prev && (cr+1 < term->nbrows)) {
	rp = rp->prev;
	cr++;
    }
    term->dptr = rp;
/*
**  Now do a carriage return and line feed to put cursor on a blank line
*/
    do_carriage_return();
    do_line_feed();
    
    term->update_all = 1;
}
/*****************************************************************/
void do_clear_display () {
/*
**  Perform 'Clear Display' function
*/
    struct row *rp;
    int i,j,k;

    if (term->FormatMode) {
        rp = find_cursor_row();
	j = cc;
	k = is_cursor_protected();
	while (rp) {
	    while (j < rp->nbchars) {
		if (rp->disp[j] & HPTERM_END_FIELD) k=1;
		if (rp->disp[j] & HPTERM_START_FIELD) k=0;
		if (!k) rp->text[j] = ' ';
		if (!k) rp->disp[j] &= 0xE0;
		j++;
	    }
	    rp = rp->next;
	    j = 0;
            k = 1;
	}
	term->update_all = 1;
	return;
    }
/*
**  Find row cr in memory
*/
    rp = find_cursor_row();
/*
**  Clear current row to end of line
*/
    for (j=cc; j<132; j++) {
        rp->text[j] = ' ';
	rp->disp[j] = 0;
    }
    rp->nbchars = cc;
/*
**  Erase all rows that follow 
*/
    while (rp->next) {
	rp = rp->next;
	clear_row (rp);
    }
    term->update_all = 1;
}
/****************************************************************/
void do_clear_line () {
/*
**  Perform 'Clear Line' function
*/
    struct row *rp;
    int i,j;
/*
**  Find row cr in memory
*/
    rp = find_cursor_row();

    if (term->FormatMode) {
        if (is_cursor_protected()) {
            /* No action */
        } else {
            /* Clear text from current cursor position to end of field */
            j = cc;
            while (j < rp->nbchars && !(rp->disp[j] & HPTERM_END_FIELD)) {
                rp->text[j++] = ' ';
            }
        }

    } else {
/*
**      Clear current row to end of line
*/
        for (j=cc; j<132; j++) {
            rp->text[j] = ' ';
	    rp->disp[j] = 0;
        }
        rp->nbchars = cc;
    }
    if (!term->update_all) {
        update_row (cr, rp);
    }
}
/*****************************************************************/
void do_insert_line () {
/*
**  Perform 'Insert Line' function
*/
    struct row *rp,*ip;
/*
**  Ignored if in Format mode
*/
    if (term->FormatMode) return;
/*
**  Find row cr in memory
*/
    rp = find_cursor_row();
/*
**  Get the new row and insert into linked list
*/
    if (term->head != term->dptr) {
	ip = term->head;
    } else {
	ip = term->tail;
    }
    remove_row (ip);
    clear_row (ip);
    insert_before (ip, rp);
/*
**  Take care of inserting on first line of screen
*/
    if (!cr) {
        term->dptr = ip;
    }

    term->update_all = 1;
    cc = 0;
}
/*****************************************************************/
void do_delete_line () {
/*
**  Perform 'Delete Line' function
*/
    struct row *rp,*ip;
/*
**  Ignored if in Format mode
*/
    if (term->FormatMode) return;
/*
**  Find row cr in memory
*/
    rp = find_cursor_row();
/*
**  Handle case of deleting first line of screen
*/
    if (!cr) {
        term->dptr = term->dptr->next;
    }
/*
**  Remove row from linked list
*/
    remove_row (rp);
/*
**  Move row to end of linked list
*/
    insert_after (rp, term->tail);
/*
**  Erase the row that we moved
*/
    clear_row (rp);
    term->update_all = 1;
    cc = 0;
}
/*****************************************************************/
void do_roll_up () {
/*
**  Perform 'Roll Text Up' function
*/
    struct row *rp;
    
    rp = find_bottom_row();
    if (rp->next) {
        term->dptr = term->dptr->next;
        term->update_all = 1;
    }
}
/****************************************************************/
void do_roll_down () {
/*
**  Perform 'Roll Text Down' function
*/
    if (term->dptr->prev) {
        term->dptr = term->dptr->prev;
        term->update_all = 1;
    }
}
/*****************************************************************/
void do_next_page () {
/*
**  Perform 'Next Page' function
*/
    struct row *rp;
    int ii;

    rp = find_bottom_row();
    ii = term->nbrows;
    while (ii && rp->next) {
        term->dptr = term->dptr->next;
	rp = rp->next;
        ii--;
    }
    term->update_all = 1;
}
/****************************************************************/
void do_previous_page () {
/*
**  Perform 'Previous Page' function
*/
    int ii;

    ii = term->nbrows;
    while (ii && term->dptr->prev) {
        term->dptr = term->dptr->prev;
        ii--;
    }
    term->update_all = 1;
}
/***************************************************************/
void do_esc_amper_a_r (parm)
/*
**  Perform absolute row cursor positioning
*/
    int parm;
{
    struct row *rp;
    int dr;
/*
**  Determine absolute row number of 1st row of screen
*/
    dr = 0;
    rp = term->dptr;
    while (rp->prev) {
        rp = rp->prev;
        dr++;
    }

    if (parm < dr) {
        while (parm < dr && term->dptr->prev) {
            term->dptr = term->dptr->prev;
            dr--;
        }
        cr = 0;
        term->update_all = 1;
    } else if (parm > dr+23) {
        while (parm > dr+23 && term->dptr->next) {
            term->dptr = term->dptr->next;
            dr++;
        }
        cr = 23;
        term->update_all = 1;
    } else {
        cr = parm - dr;
    }
}
/***************************************************************/
void do_esc_amper_a_plus_r (int parm) {
/*
**  Position cursor using cursor relative row offset
*/
    struct row *rp;
    int dr;
/*
**  Determine absolute row number of 1st row on screen
*/
    dr = 0;
    rp = term->dptr;
    while (rp->prev) {
        rp = rp->prev;
        dr++;
    }

    do_esc_amper_a_r (dr + cr + parm);
}
/***************************************************************/
void do_esc_amper_a_minus_r (int parm) {
/*
**  Position cursor using cursor relative row offset
*/
    do_esc_amper_a_plus_r (-parm);
}
/***************************************************************/
void do_esc_amper_a_y (int parm) {
/*
**  Position cursor using screen relative row number
*/
    struct row *rp;
    int dr;
/*
**  Determine absolute row number of 1st row on screen
*/
    dr = 0;
    rp = term->dptr;
    while (rp->prev) {
        rp = rp->prev;
        dr++;
    }

    do_esc_amper_a_r (dr + parm);
}
/***************************************************************/
void send_number (i)
/*
**  Send 3-digit decimal number
*/
    int i;
{
    int a,b,c;
    a = i / 100;
    i = i - a * 100;
    b = i / 10;
    c = i - b * 10;
    tx_buff[tx_tail++] = a + '0';
    tx_buff[tx_tail++] = b + '0';
    tx_buff[tx_tail++] = c + '0';
}
/***************************************************************/
void send_cursor_abs () { 
/*
**  Send cursor position using absolute addressing
**  Does not send the terminator
*/
    struct row *rp;
    int dr;
/*
**  Determine absolute row number of 1st row of screen
*/
    dr = 0;
    rp = term->dptr;
    while (rp->prev) {
        rp = rp->prev;
        dr++;
    }

    tx_buff[tx_tail++] = 27;
    tx_buff[tx_tail++] = '&';
    tx_buff[tx_tail++] = 'a';
    send_number (cc);
    tx_buff[tx_tail++] = 'c';
    send_number (dr + cr);
    tx_buff[tx_tail++] = 'R';
}
/***************************************************************/
void send_cursor_rel () {
/*
**  Send cursor position using relative addressing
**  Does not send the terminator
*/
    tx_buff[tx_tail++] = 27;
    tx_buff[tx_tail++] = '&';
    tx_buff[tx_tail++] = 'a';
    send_number (cc);
    tx_buff[tx_tail++] = 'c';
    send_number (cr);
    tx_buff[tx_tail++] = 'Y';
}
/***************************************************************/
void reset_user_keys () {
/*
**  Reset user keys to the power-on state
*/
    struct udf *u;
    int i;

    for (i=0; i<8; i++) {
	u = term->UserDefKeys[i];
	u->Attribute = 2;
	u->LabelLength = 16;
	memset (u->Label, ' ', 16);
	u->Label[3] = 'f';
	u->Label[4] = '1' + i;
	u->StringLength = 2;
	u->String[0] = 27;
	u->String[1] = 'p' + i;
    }
}
/***************************************************************/
void init_hpterm () {
/*
**  Initialize terminal emulator
*/
    int i,j;
    struct row *rp;

    term = (struct hpterm*) calloc (1, sizeof(struct hpterm));
    term->TerminalId = strdup("X-hpterm");  
    term->RightMargin = 255;
    term->FldSeparator = 29;
    term->BlkTerminator = 30;
    term->TabStops = (char*) calloc (1, 132);
    for (i=0; i<480; i++) {
        rp = (struct row*) calloc (1, sizeof(struct row));
        rp->text = (char*) calloc (1, 132);
        rp->disp = (char*) calloc (1, 132);
	if (!term->head) {
	    term->head = rp;
	    term->tail = rp;
	    rp->next = 0;
	    rp->prev = 0;
        } else {
	    term->head->prev = rp;
	    rp->next = term->head;
	    term->head = rp;
	    rp->prev = 0;
        }
    }
    term->UserDefKeys = (struct udf**) calloc (8, sizeof(struct udf*));
    for (i=0; i<8; i++) {
	term->UserDefKeys[i] = (struct udf*) calloc (1, sizeof(struct udf));
    }
    term->RemoteMode = 1;
    term->EnableKybd = 1;

    term->dptr = term->head;
    term->nbrows = 24;
    term->nbcols = 80;
    term->menu1 = (struct row*) calloc (1, sizeof(struct row));
    term->menu1->text = (char*) calloc (1, 132);
    term->menu1->disp = (char*) calloc (1, 132);
    term->menu2 = (struct row*) calloc (1, sizeof(struct row));
    term->menu2->text = (char*) calloc (1, 132);
    term->menu2->disp = (char*) calloc (1, 132);
    reset_user_keys();
    term->UserSystem = 1;
}
/***************************************************************/
void hpterm_winsize (int nbrows, int nbcols) {
/*
**  Window size was changed by the window system
*/
    struct row *rp;
    int ii;
/*
**  Reserve 2 lines for the menus
**  and don't allow less than 2 lines of data
*/
    nbrows = nbrows - 2;
    if (nbrows < 2) nbrows=2;
/*
**  Don't allow crazy width
*/
    if (nbcols<10) nbcols=10;
    if (nbcols>132) nbcols=132;
/*
**  Try to position the text nicely after a window resize
**  This is broken
*/
    if (nbrows > term->nbrows) {

        rp = term->dptr;
        for (ii=0; ii<nbrows; ii++) {
       	    if (rp->next) {
	        rp = rp->next;
            } else if (term->dptr->prev) {
	        term->dptr = term->dptr->prev;
	    } else {
	        nbrows=ii; 
            }
        }
    } 

    term->nbrows = nbrows;
    term->nbcols = nbcols;
    if (cr >= nbrows) cr = nbrows - 1;
    if (cc >= nbcols) cc = nbcols - 1;
    if (term->MemLockRow >= nbrows) {
	term->MemoryLock = 0;
    }
    term->update_all = 1;
    update_labels();
}
/***************************************************************/
void get_terminal_status (char s[14]) {
/*
**  Build the terminal status message 
*/
    int display_memory_size = 15;
    int sends_secondary = 1;
    int buffer_memory_size = 0;
    int printer_interface_present = 1;
    int apl_firmware_present = 0;
    int has_terminal_id = 1;
    int programmable_terminal = 0;
    int DataSpeedSelect = 0;
    int MemFull = term->tail->nbchars ? 1 : 0;

    memset (s, 0x30, 14);

    s[0] |= display_memory_size;

    if (term->XmitFnctn_A)     s[1] |= 0x1;
    if (term->SPOW_B)          s[1] |= 0x2;
    if (term->InhEolWrp_C)     s[1] |= 0x4;
    if (term->LinePage_D)      s[1] |= 0x8;

    if (term->InhHndShk_G)     s[2] |= 0x4;
    if (term->InhDC2_H)        s[2] |= 0x8;

    if (term->CapsLock)        s[3] |= 0x1;
    if (term->BlockMode)       s[3] |= 0x2;
    if (term->AutoLineFeed)    s[3] |= 0x4;
    if (sends_secondary)       s[3] |= 0x8;

    if (term->CursorSensePending)      s[4] |= 0x1;
    if (term->FunctionKeyPending)      s[4] |= 0x2;
    if (term->EnterKeyPending)         s[4] |= 0x4;
    if (term->SecondaryStatusPending)  s[4] |= 0x8;

    if (term->DeviceStatusPending)     s[6] |= 0x1;
    if (term->DeviceCompletionPending) s[6] |= 0x2;

    s[7] |= buffer_memory_size;

    if (printer_interface_present)     s[8] |= 0x1;
    if (apl_firmware_present)          s[8] |= 0x2;
    if (has_terminal_id)               s[8] |= 0x4;
    if (programmable_terminal)         s[8] |= 0x8;

    if (term->EscXfer_N)               s[10] |= 0x1;

    if (DataSpeedSelect)               s[12] |= 0x2;
    if (term->ChkParity)               s[12] |= 0x8;

    if (term->MemLockRow)              s[13] |= 0x1;
    if (term->MemoryLock)              s[13] |= 0x2;
    if (term->MemoryLock && MemFull)   s[13] |= 0x4;
}
/***************************************************************/
void send_terminator (void) {
/*
**  Send block transfer terminator
**
**  This version based on page 7-2 of 700/92 manual
*/
    if (term->BlockMode && term->LinePage_D) {
        tx_buff[tx_tail++] = term->BlkTerminator;
    } else {
        tx_buff[tx_tail++] = 13;
        if (term->AutoLineFeed) {
            tx_buff[tx_tail++] = 10;
        }
    }
}
/***************************************************************/
void send_primary_status (void) {
/*
**  Send the primary status information 
*/
    char s[14];

    get_terminal_status (s);
    tx_buff[tx_tail++] = 27;
    tx_buff[tx_tail++] = '\\';
    tx_buff[tx_tail++] = s[0];
    tx_buff[tx_tail++] = s[1];
    tx_buff[tx_tail++] = s[2];
    tx_buff[tx_tail++] = s[3];
    tx_buff[tx_tail++] = s[4];
    tx_buff[tx_tail++] = s[5];
    tx_buff[tx_tail++] = s[6];
    send_terminator();

    term->PrimaryStatusPending = 0;
}
/***************************************************************/
void send_secondary_status (void) {
/*
**  Send the secondary status information
*/
    char s[14];

    get_terminal_status (s);
    tx_buff[tx_tail++] = 27;
    tx_buff[tx_tail++] = '|';
    tx_buff[tx_tail++] = s[7];
    tx_buff[tx_tail++] = s[8];
    tx_buff[tx_tail++] = s[9];
    tx_buff[tx_tail++] = s[10];
    tx_buff[tx_tail++] = s[11];
    tx_buff[tx_tail++] = s[12];
    tx_buff[tx_tail++] = s[13];
    send_terminator();

    term->SecondaryStatusPending = 0;
}
/***************************************************************/
void send_device_status (void) {

    tx_buff[tx_tail++] = 27;
    tx_buff[tx_tail++] = '\\';
    tx_buff[tx_tail++] = 'p';
    tx_buff[tx_tail++] = term->DeviceStatusPending + '0';
    tx_buff[tx_tail++] = 0x30;   /* 0x31 = Last print failed */
    tx_buff[tx_tail++] = 0x38;   /* 0x31 = Busy, 0x38 = Completed */
    tx_buff[tx_tail++] = 0x31;   /* 0x31 = Printer Present */
    send_terminator();

    term->DeviceStatusPending = 0;
}
/***************************************************************/
void send_cursor_sense (void) {

    if (term->CursorSensePending == 1) {
        send_cursor_abs();
    } else if (term->CursorSensePending == 2) {
        send_cursor_rel();
    }
    send_terminator();

    term->CursorSensePending = 0;
}
/***************************************************************/
void send_function_key (void) {

    struct udf *u;
    int j,n;

    if (term->SendCursorPos) {     /* Check page 3-3: Need Esc & a? */
        send_cursor_abs();         /* Sends Esc & a xxx c yyy R */
    }

    n = term->FunctionKeyPending;
    if (n==9) {
/*
**      Is really the 'Select' function
**      See top of page 3-10
*/
        tx_buff[tx_tail++] = 27;
        tx_buff[tx_tail++] = '&';
        tx_buff[tx_tail++] = 'P';

    } else {
/*
**      Is really F1 - F8
*/
        u = term->UserDefKeys[n-1];

        for (j=0; j < u->StringLength; j++) {
            tx_buff[tx_tail++] = u->String[j];
        }
    }
    send_terminator();

    if (term->AutoKybdLock) term->EnableKybd = 0; 

    term->FunctionKeyPending = 0;
}
/***************************************************************/
int send_field (struct row *rp) {
/*
**  Send a field of data starting from column cc
**  Updates cc with new cursor column
**  Returns 1 if block terminator was found
**  Returns 0 if end of field was found
*/
    while (cc < rp->nbchars) {
        if (rp->text[cc] == term->BlkTerminator) {
            cc++;
            return (1);
        }
        if (rp->disp[cc] & HPTERM_END_FIELD) {
            cc++;
            return (0);
        }
        tx_buff[tx_tail++] = rp->text[cc++];
    }
    return (0);
}
/***************************************************************/
int send_line (struct row *rp) {
/*
**  Send a line of data starting from column cc
**  Updates cc with new cursor column
**  Returns 1 if block terminator was found
**  Returns 0 if end of line was found
*/
    while (cc < rp->nbchars) {
/*
**      Check for display enhancement escape sequence
*/
        if (rp->disp[cc] & HPTERM_ANY_ENHANCEMENT) {
            tx_buff[tx_tail++] = 27;
            tx_buff[tx_tail++] = '&';
            tx_buff[tx_tail++] = 'd';
            tx_buff[tx_tail++] = (rp->disp[cc] & 0xF) + '@';
        }
/*
**      Check for start of field escape sequence
*/
        if (rp->disp[cc] & HPTERM_START_FIELD) {
            tx_buff[tx_tail++] = 27;
            tx_buff[tx_tail++] = '[';
        }
/*
**      Check for end of field escape sequence
*/
        if (rp->disp[cc] & HPTERM_END_FIELD) {
            tx_buff[tx_tail++] = 27;
            tx_buff[tx_tail++] = ']';
        }
/*
**      Check for block terminator
*/
        if (rp->text[cc] == term->BlkTerminator) {
            cc++;
            return (1);
        }
/*
**      Now transmit character
*/
        tx_buff[tx_tail++] = rp->text[cc++];
    }
    return (0);
}
/***************************************************************/
void send_enter_data (void) {
/*
**  This version based on pages 3-10 through 3-17 of 700/92 manual
*/
    struct row *rp,*re;
    int ccsave,blkterm,count,done;
/*
**  Send cursor position as described on page 3-3
*/
    if (term->BlockMode || term->FormatMode || !(term->LineModify ||
                                                 term->ModifyAll)) {
        if (term->SendCursorPos) {
            send_cursor_abs();
        }
    }

    if (term->UserKeyMenu && term->BlockMode && term->LinePage_D) {
/*
**      Block mode transfer of function key definitions
*/
        int i,j;
        struct udf *u;

        for (i=0; i<8; i++) {
            u = term->UserDefKeys[i];
            tx_buff[tx_tail++] = 27;
            tx_buff[tx_tail++] = '&';
            tx_buff[tx_tail++] = 'f';
            tx_buff[tx_tail++] = u->Attribute + '0';
            tx_buff[tx_tail++] = 'a';
            tx_buff[tx_tail++] = i + '1';
            tx_buff[tx_tail++] = 'k';
            send_number (u->LabelLength);
            tx_buff[tx_tail++] = 'd';
            send_number (u->StringLength);
            tx_buff[tx_tail++] = 'L';
            for (j=0; j < u->LabelLength; j++) {
                tx_buff[tx_tail++] = u->Label[j];
            }
            for (j=0; j < u->StringLength; j++) {
                tx_buff[tx_tail++] = u->String[j];
            }
            tx_buff[tx_tail++] = 13;
            tx_buff[tx_tail++] = 10;
            if (i < 7) {
                term_flush_tx (con);
            } else {
                tx_buff[tx_tail++] = term->BlkTerminator;
            }
        }

    } else if (term->BlockMode && !term->LinePage_D && !term->FormatMode) {
/*
**      Block Line Mode, Format Off
*/
        rp = find_cursor_row();
        if (term->InhDC2_H) cc=0;    /* See page 3-13 */
        blkterm = send_line(rp);
        if (blkterm) {
            tx_buff[tx_tail++] = term->BlkTerminator;
        } else {
            cc=0;
            if (term->AutoLineFeed) do_line_feed();
        }
        tx_buff[tx_tail++] = 13;
        if (term->AutoLineFeed) {
            tx_buff[tx_tail++] = 10;
        }

    } else if (term->BlockMode && !term->LinePage_D && term->FormatMode) {
/*
**      Block Line Mode, Format On
*/
        if (is_cursor_protected()) goto_next_field();
        if (is_cursor_protected()) {
            tx_buff[tx_tail++] = term->BlkTerminator;   
        } else {                                       
	    rp = find_cursor_row();
	    blkterm = send_field (rp);
            if (blkterm) {
                tx_buff[tx_tail++] = term->BlkTerminator;
            }
        }
	tx_buff[tx_tail++] = 13;
	if (term->AutoLineFeed) {
	    tx_buff[tx_tail++] = 10;
        }

    } else if (term->BlockMode && term->LinePage_D && !term->FormatMode) {
/*
**      Block Page Mode, Format Off
*/
        if (term->InhDC2_H) do_home_up();
/*
**      Find last line of used memory
*/
        re = rp = find_cursor_row();
        while (rp->next) {
            rp = rp->next;
            if (rp->nbchars) re=rp;
        }
/*
**      Transmit lines until end of memory or terminator found
*/
        done=0;
        while (!done) {
            rp = find_cursor_row();
            blkterm = send_line(rp);
            if (blkterm) {
                done=1;
            } else {
                tx_buff[tx_tail++] = 13;
		tx_buff[tx_tail++] = 10;
                cc=0;
                do_line_feed();
            }
            if (rp==re) done=1;
            if (!done) term_flush_tx(con);
        }
        tx_buff[tx_tail++] = term->BlkTerminator;
 
    } else if (term->BlockMode && term->LinePage_D && term->FormatMode) {
/*
**      Block Page Mode, Format On
*/
	if (term->InhDC2_H) do_home_up();

	done=0;
        count=0;
	while (!done) {
	    if (is_cursor_protected()) goto_next_field();
	    if (is_cursor_protected()) {
		done=1;
            } else {
                if (count) {
                    tx_buff[tx_tail++] = term->FldSeparator;
                    term_flush_tx(con);
                }
		rp = find_cursor_row();
		blkterm = send_field(rp);
                if (blkterm) done=1;
		count++;
            }
        }
	tx_buff[tx_tail++] = term->BlkTerminator;

    } else if (term->FormatMode) {
/*
**      Character Mode, Format On
*/
        if (is_cursor_protected()) goto_next_field();
        if (is_cursor_protected()) {
            tx_buff[tx_tail++] = term->BlkTerminator;
        } else {
            blkterm = send_field(rp);
            if (blkterm) tx_buff[tx_tail++] = term->BlkTerminator;
        }
        tx_buff[tx_tail++] = 13;
        if (term->AutoLineFeed) {
            tx_buff[tx_tail++] = 10;
        }

    } else if (term->LineModify || term->ModifyAll) {
/*
**      Modify Mode
*/
        rp = find_cursor_row();
        cc = ccsave = term->StartCol;
        blkterm = send_line(rp);
        if (blkterm) {
            tx_buff[tx_tail++] = term->BlkTerminator;
        } else {
            if (term->LocalEcho) cc=0;
            else cc = ccsave;
        }
        tx_buff[tx_tail++] = 13;
        if (term->AutoLineFeed) {
            tx_buff[tx_tail++] = 10;
        }
        if (term->LineModify) {
            term->LineModify = 0;
            if (term->KeyState == ks_modes) update_labels();
        }

    } else {    /* Normal character mode */
/*
**      Character Mode, Format off, Modify off
*/
        rp = find_cursor_row();
        cc = term->LeftMargin;
        blkterm = send_line(rp);
        if (blkterm) {
            tx_buff[tx_tail++] = term->BlkTerminator;
        } else {
            cc=0;
            do_line_feed();
        }
        tx_buff[tx_tail++] = 13;
        if (term->AutoLineFeed) {
            tx_buff[tx_tail++] = 10;
        }
    }
/*
**  Do auto keyboard lock unless transfer was requested by esc d
*/
    if (term->AutoKybdLock && term->EnterKeyPending != 2) {
        term->EnableKybd = 0;
    }
    term->EnterKeyPending = 0;
}
/***************************************************************/
void send_device_completion_status (void) {
/*
**  Needed for External Devices
*/
    send_terminator();
    term->DeviceCompletionPending = 0;
}
/***************************************************************/
void send_terminal_id (void) {

    int ii;

    for (ii=0; term->TerminalId[ii]; ii++) {
        tx_buff[tx_tail++] = term->TerminalId[ii];
    }
    send_terminator();

    term->TerminalIdPending = 0;
}
/***************************************************************/
int check_type1_handshake (void) {
/*
**  Check the status of the type 1 handshake
**  Returns 1 if handshake is finished
**
**  Since type 1 handshake means 'No handshake', this
**  routine always returns 1.
*/
    return (1);
}
/***************************************************************/
int check_type2_handshake (void) {
/*
**  Check the status of the type 2 handshake
**  Returns 1 if handshake is finished
**
**  Type 2 handshake means wait for a DC1, then send the data
*/
    int out;

    if (term->DC1Count) {
        out=1;
    } else {
        out=0;
    }
    return (out);
}
/***************************************************************/
int check_type3_handshake (void) {
/*
**  Check the status of the type 3 handshake
**  Returns 1 if handshake is finished
**
**  Type 3 handshake means wait for the first DC1, send a DC2, wait
**  for the second DC1, then send the data
*/
    int out;

    if (term->DC1Count && !term->DC2Count) {
        tx_buff[tx_tail++] = ASC_DC2;
        if (term->LinePage_D == 0) {
            tx_buff[tx_tail++] = 13;      /* See note on page 2-12 */
            if (term->AutoLineFeed) {     /* of 700/92 manual */
                tx_buff[tx_tail++] = 10;
            }
        }
        term->DC1Count = 0;
        term->DC2Count = 1;
#if SHOW_DC1_COUNT
        update_labels();
#endif
        out=0;
    } else if (term->DC1Count && term->DC2Count) {
        out=1;
    } else {
        out=0;
    }
    return (out);
}
/***************************************************************/
int check_short_block_handshake (void) {
/*
**  Check the status of the short block handshake.
**  Returns 1 if handshake is finished.
**  The actual handshake used depends on the G and H straps.
*/
    int out;

    if (!term->InhHndShk_G) {
        out = check_type2_handshake();
    } else if (!term->InhDC2_H) {
        out = check_type3_handshake();
    } else {
        out = check_type1_handshake();
    }
    return (out);
}
/***************************************************************/
int check_long_block_handshake (void) {
/*
**  Check the status of the long block handshake.
**  Returns 1 if handshake is finished.
**  The actual handshake used depends on the G and H straps.
*/
    int out;

    if (!term->InhDC2_H) {
        out = check_type3_handshake();
    } else {
        out = check_type1_handshake();
    }
    return (out);
}
/****************************************************************/
int check_long_character_handshake (void) {
/*
**  Check the status of the long character handshake.
**  Returns 1 if handshake is finished.
**  The actual handshake used depends on the G and H straps.
*/
    int out;

    if (term->InhHndShk_G && !term->InhDC2_H) {
        out = check_type3_handshake();
    } else {
        out = check_type1_handshake();
    }
    return (out);
}
/***************************************************************/
void check_transfers_pending (void) {
/*
**  Check the status of any terminal-to-computer transfers
**  that are pending.  If the conditions required for the 
**  transfer are satisfied, perform the transfer.
*/
    int ok;

    if (term->PrimaryStatusPending) {
        ok = check_short_block_handshake();
        if (ok) send_primary_status();
    } else if (term->SecondaryStatusPending) {
        ok = check_short_block_handshake();
        if (ok) send_secondary_status();
    } else if (term->DeviceStatusPending) {
        ok = check_short_block_handshake();
        if (ok) send_device_status();
    } else if (term->CursorSensePending) {
        ok = check_short_block_handshake();
        if (ok) send_cursor_sense();
    } else if (term->FunctionKeyPending) {
        if (term->BlockMode && term->LinePage_D) {
            ok = check_long_block_handshake();
        } else {
            ok = check_short_block_handshake();
        }
        if (ok) send_function_key();
    } else if (term->EnterKeyPending) {
        if (term->EnterKeyPending == 2) {  /* Esc d */
            ok = check_short_block_handshake();
        } else if (term->BlockMode) {
            ok = check_long_block_handshake();
        } else {
            ok = check_long_character_handshake();
        }
        if (ok) send_enter_data();
    } else if (term->DeviceCompletionPending) {
        ok = check_short_block_handshake();
        if (ok) send_device_completion_status();
    } else if (term->TerminalIdPending) {
        ok = check_short_block_handshake();
        if (ok) send_terminal_id();
    } else {
        /* No transfers are pending */
    }
    term_flush_tx(con);
}
/***************************************************************/
void hpterm_rxfunc (void *ptr, char *buf, int nbuf) {
/*
**  Process block of characters received from remote computer
*/
    int ii;

    for (ii=0; ii<nbuf; ii++) {
	hpterm_rxchar (buf[ii]);
    }
}
/***************************************************************/
static void hpterm_rxchar (char ch)
/*
**  Process character received from remote computer
*/
{
    int i,j;

    erase_cursor();
/*
**  Set the following true if you think your display has nice
**  symbols for the control characters
*/
#define HAVE_BITMAPS 0

    if (term->DisplayFuncs) {
        if (ch>=32 || HAVE_BITMAPS) display_char(ch);
        else {
          display_char('<');
          display_char(ch/10+'0');
          display_char(ch%10+'0');
          display_char('>');
        }
        if (ch == 10) {
	    do_carriage_return();
	    do_line_feed();
        }
	if (state==1 && ch=='Z' && term->DisplayFuncs==1) {
	    term->DisplayFuncs = 0;
	    if (term->KeyState == ks_modes) update_labels();
        }
        if (ch==27) state=1;
	else state=0;
	goto done;
    } 

    if (ch==ASC_DC1) {
        term->DC1Count++;
#if SHOW_DC1_COUNT
        update_labels();
#endif
        check_transfers_pending();
	goto done;
    }

    switch (state) {
      case 0:
        if (ch == 27) state=1;
        else if (ch == 13) do_carriage_return();
        else if (ch == 10) do_line_feed();
        else if (ch == 7) do_bell();
        else if (ch == 8) do_back_space();
        else if (ch == 9) do_forward_tab();
        else if (ch>=32) {
          display_char (ch);
        }
        break;
      case 1:
        if (ch == '&') {
	    state=2;
        } else if (ch == '1') {
	    set_tab();
	    state=0;
        } else if (ch == '2') {
	    clear_tab();
	    state=0;
        } else if (ch == '3') {
	    clear_all_tabs();
	    state=0;
        } else if (ch == '4') {
	    term->LeftMargin = cc;
	    state=0;
        } else if (ch == '5') {
	    term->RightMargin = cc + 1;
	    state=0;
        } else if (ch == '9') {
	    term->LeftMargin = 0;
	    term->RightMargin = 255;
	    state=0;
        } else if (ch == '@') {
	    do_esc_atsign();
	    state=0;
        } else if (ch == 'A') {
            do_cursor_up();
            state=0;
        } else if (ch == 'B') {
            do_cursor_down();
            state=0;
        } else if (ch == 'C') {
            do_cursor_right();
            state=0;
        } else if (ch == 'D') {
            do_cursor_left();
            state=0;
        } else if (ch == 'E') {
	    do_hard_reset(); 
	    state=0;
        } else if (ch == 'F') {
	    do_home_down();
	    state=0;
        } else if (ch == 'H' || ch == 'h') {
            do_home_up();
            state=0;
        } else if (ch == 'G') {
	    do_carriage_return();
	    state=0;
        } else if (ch == 'I') {
	    do_forward_tab ();
	    state=0;
        } else if (ch == 'J') {
            do_clear_display();
            state=0;
        } else if (ch == 'K') {
            do_clear_line();
            state=0;
        } else if (ch == 'L') {
            do_insert_line();
            state=0;
        } else if (ch == 'M') {
            do_delete_line();          
            state=0;
        } else if (ch == 'N') {
	    term->InsertMode = 2;        /* Insert Char w/wraparound */
            if (term->KeyState) update_labels();
	    state=0;
        } else if (ch == 'O') {
	    do_delete_char_with_wrap();    
	    state=0;
        } else if (ch == 'P') {
	    do_delete_char();      
	    state=0;
        } else if (ch == 'Q') {
	    term->InsertMode = 1;        /* Insert Char w/o wraparound */
            if (term->KeyState) update_labels();
	    state=0;
        } else if (ch == 'R') {
	    term->InsertMode = 0;
	    if (term->KeyState) update_labels();
	    state=0;
        } else if (ch == 'S') {
            do_roll_up();
            state=0;
        } else if (ch == 'T') {
            do_roll_down();
            state=0;
        } else if (ch == 'U') {
            do_next_page();
            state=0;
        } else if (ch == 'V') {
            do_previous_page();
            state=0;
        } else if (ch == 'W') {
#if DEBUG_BLOCK_MODE
    printf ("FormatMode = 1\n");
    fflush (stdout);
#endif
	    term->FormatMode = 1;
	    if (is_cursor_protected()) do_forward_tab();
	    state=0;
        } else if (ch == 'X') {
#if DEBUG_BLOCK_MODE
    printf ("FormatMode = 0\n");
    fflush (stdout);
#endif
	    term->FormatMode = 0;
	    state=0;
        } else if (ch == 'Y') {
	    if (term->DisplayFuncs == 0) {
	        term->DisplayFuncs = 1;
	        if (term->KeyState == ks_modes) update_labels();
            }
	    state=0;
        } else if (ch == 'Z') {
	    if (term->DisplayFuncs == 1) {
	        term->DisplayFuncs = 0;
	        if (term->KeyState == ks_modes) update_labels();
            }
	    state=0;
        } else if (ch == '[') {
	    set_start_field ();
	    state = 0;
        } else if (ch == ']') {
	    set_end_field ();
	    state = 0;
        } else if (ch == '^') {
            term->PrimaryStatusPending = 1;
            check_transfers_pending();
            state=0;
        } else if (ch == '~') {
            term->SecondaryStatusPending = 1;
            check_transfers_pending();
            state=0;
        } else if (ch == 'a') {
            term->CursorSensePending = 1;  /* 1=abs, 2=rel */
            check_transfers_pending();
            state=0;
        } else if (ch == 'b') {
	    term->EnableKybd = 1;
	    state=0;
        } else if (ch == 'c') {
	    term->EnableKybd = 0;
	    state=0;
        } else if (ch == 'd') {
            term->EnterKeyPending = 2;
            term->DC1Count = 0;  /* See note on page 3-18 */
            term->DC2Count = 0;
#if SHOW_DC1_COUNT
            update_labels();
#endif
            check_transfers_pending();
            state=0;
        } else if (ch == 'f') {
	    do_modem_disconnect();
	    state=0;
	} else if (ch == 'g') {
	    do_soft_reset();    
	    state=0;
        } else if (ch == 'i') {
	    do_back_tab();
	    state=0;
        } else if (ch == 'j') {
            term->UserKeyMenu = 1;
            state=0;
        } else if (ch == 'k') {
            term->UserKeyMenu = 0;
            state=0;
        } else if (ch == 'l') {
	    term->MemoryLock = 1;
	    term->MemLockRow = cr;
	    if (term->KeyState == ks_modes) update_labels();
	    state=0;
        } else if (ch == 'm') {
	    term->MemoryLock = 0;
	    if (term->KeyState == ks_modes) update_labels();
	    state=0;
	} else if (ch >= 'p' && ch <= 'w') {
	    /* Ignore incoming function button message */
	    state=0;
        } else if (ch == 'y') {
	    /* Turn display functions on, but esc Z can't clear it */
	    term->DisplayFuncs = 2;
	    if (term->KeyState == ks_modes) update_labels();
	    state=0;
        } else if (ch == '\'') {
            term->CursorSensePending = 2;  /* 1=abs, 2=rel */
            check_transfers_pending();
            state=0;
        } else if (ch == '*') {
	    state=31;
        } else if (ch == '{') {
	    set_start_tx_only_field();
	    state=0;
        } else {
          display_char ('E');
          display_char ('c');
          display_char (ch);
          state=0;
        }
        break;
      case 2:   /* Got esc &, waiting for more */
        if (ch == 'a') {
          parm = 0;
          state = 3;
        } else if (ch == 'd') {
	  parm = 0;
          state = 11;    
        } else if (ch == 'f') {
	  parm = 0;
	  sign = 1;
	  attr = 0;
	  keyn = 1;
	  llen = 0;
	  slen = 1;
	  state = 37;
        } else if (ch == 'j') {
	  parm = 0;
	  state = 35;
        } else if (ch == 'k') {
	  parm = 0;
	  state = 41;
        } else if (ch == 's') {
	  parm = 0;
	  state = 45;
        } else if (ch == 'X') {   /* 700/92 manual page 3-3 says */
	  parm = 0;               /* this is an upper-case X...  */
	  state = 46;             /* Is this true? */
        } else {
	  state=0;
        }
        break;

      case 3:   /* Got esc & a  -- waiting for more */
        if (ch=='+') {
          state = 4;
        } else if (ch=='-') {
          state = 5;
        } else if (ch>='0' && ch<='9') {
          parm = parm * 10 + ch - '0';
        } else if (ch=='c') {
          cc = parm;
          if (cc >= term->nbcols) cc = term->nbcols - 1;
          parm = 0;
        } else if (ch=='y') {
          do_esc_amper_a_y (parm);
          parm = 0;
        } else if (ch=='r') {
          do_esc_amper_a_r (parm);
          parm = 0;
        } else if (ch=='C') {
          cc = parm;
          if (cc >= term->nbcols) cc = term->nbcols - 1;
          state=0;
        } else if (ch=='Y') {
          do_esc_amper_a_y (parm);
          state=0;
        } else if (ch=='R') {
          do_esc_amper_a_r (parm);
          state=0;
        } else {
          state=0;
        }
        break;

      case 4:
        if (ch>='0' && ch<='9') {
          parm = parm * 10 + ch - '0';
        } else if (ch=='c') {
          cc = cc + parm;
          if (cc >= term->nbcols) cc = term->nbcols - 1;
          parm = 0;
          state = 3;
        } else if (ch=='r') {
          do_esc_amper_a_plus_r (parm);  
          parm = 0;
          state = 3;
        } else if (ch=='C') {
          cc = cc + parm;
          if (cc >= term->nbcols) cc = term->nbcols - 1;
          state=0;
        } else if (ch=='R') {
          do_esc_amper_a_plus_r (parm);  
          state=0;
        } else {
          state=0;
        }
        break;

      case 5:
        if (ch>='0' && ch<='9') {
          parm = parm * 10 + ch - '0';
        } else if (ch=='c') {
          cc = cc - parm;
          if (cc<0) cc=0;
          parm = 0;
          state = 3;
        } else if (ch=='r') {
          do_esc_amper_a_minus_r (parm);  
          parm = 0;
          state = 3;
        } else if (ch=='C') {
          cc = cc - parm;
          if (cc<0) cc=0;
          state=0;
        } else if (ch=='R') {
          do_esc_amper_a_minus_r (parm);  
          state=0;
        } else {
          state=0;
        }
        break;

      case 11:   /* Got esc & d, waiting for more */
	if (ch=='s') {
	  set_security();
        } else if (ch=='S') {
	  set_security();
	  state=0;
        } else if (ch>='@' && ch<='O') {
	  set_display_enh (ch);
	  state=0;
        } else {
	  state=0;
        }
	break;

      case 31:   /* Got esc *, waiting for s or d */
	if (ch=='s') {
	  parm=0;
	  state=32;
        } else if (ch=='d') {
	  parm=0;
	  nparm=0;
	  state=42;
        } else {
	  state=0;
        }
	break;

      case 32:   /* Got esc * s, waiting for ^ */
	if (ch>='0' && ch<='9') {
	  parm = parm * 10 + ch - '0';
	} else if (ch=='^') {
          term->TerminalIdPending = 1;
          check_transfers_pending();
	  state=0;
        } else {         
	  state=0;
        }
	break;

      case 35:   /* Got esc & j -- waiting for more */
	if (ch>='0' && ch<='9') {
	  parm = parm * 10 + ch - '0';
	} else if (ch=='@') {
	  term->KeyState = ks_off;
	  update_labels();
	  state=0;
        } else if (ch=='A') {
	  term->KeyState = ks_modes;
	  update_labels();
	  state=0;
        } else if (ch=='B') {
	  term->KeyState = ks_user;
	  update_labels();
	  state=0;
        } else if (ch=='C') {
	  if (term->Message) {
	      free(term->Message);
	      term->Message = 0;
	      update_labels();
          }
	  state=0;
        } else if (ch=='D') {
	  term->MessageMode = parm;
	  state=0;
        } else if (ch=='L') {
	  if (parm) {
	      if (term->Message) free(term->Message);
	      term->Message = (char*) calloc (1, parm+1);
	      nparm=0;
	      state=36;
	  } else state=0;
        } else if (ch=='S') {
	  term->UserSystem = 0;
	  state=0;
        } else if (ch=='R') {
	  term->UserSystem = 1;
	  state=0;
        } else {
	  state=0;
        }
	break;
     
      case 36:   /* Got esc & j <parm> L -- waiting for message */
        term->Message[nparm++] = ch;
	if (nparm==parm) {
	    update_labels();
	    state=0;
        }
	break;

      case 37:   /* Got esc & f  --  waiting for more */
	if (ch>='0' && ch<='9') {
	  parm = parm * 10 + ch - '0';
        } else if (ch=='-') {
	  sign = -1;
        } else if (ch=='E') {
	  keyn = sign * parm;
	  if (keyn>=1 && keyn<=8) {
	    do_function_button (keyn-1);
	    state=0;
          } else if (keyn == -1) {
	    hpterm_kbd_Enter();
	    state=0;
          }
        } else if (ch=='a' || ch=='A') {
	  attr = sign * parm;
	  parm = 0;
	  sign = 1;
        } else if (ch=='k' || ch=='K') {
	  keyn = sign * parm;
	  parm = 0;
	  sign = 1;
        } else if (ch=='d' || ch=='D') {
	  llen = sign * parm;
	  parm = 0;
	  sign = 1;
        } else if (ch=='l' || ch=='L') {
	  slen = sign * parm;
	  parm = 0;
	  sign = 1;
        } else {
	  state=0;
        }
	if (ch=='A' || ch=='K' || ch=='D' || ch=='L') {
          if (keyn>=1 && keyn<=8) {
	    if (attr >= 0 && attr <= 2) {
	        term->UserDefKeys[keyn-1]->Attribute = attr;
            }
	    if (llen >= 0) {
		term->UserDefKeys[keyn-1]->LabelLength = 0;
            }
	    if (slen == -1 || slen > 0) {
		term->UserDefKeys[keyn-1]->StringLength = 0;
	    }
	    if (llen > 0 && llen <= 16) {
		nparm = 0;
		state=38;
            } else if (slen > 0 && slen <= 80) {
		nparm = 0;
		state=39;
            } else {
		state=0;
            }
          } else if (keyn == 0 || keyn == -1) {
            nparm = 0;
            state = 40;
          } else {
            state = 0;
          }
        }
	break;

      case 38:  /* Getting function key label */
	term->UserDefKeys[keyn-1]->Label[nparm++] = ch;
	term->UserDefKeys[keyn-1]->LabelLength = nparm;
	if (nparm == llen) {
	    if (slen > 0 && slen <= 80) {
		nparm = 0;
		state=39;
            } else {
		state=0;
            }
        }
	break;

      case 39:  /* Getting function key string */
	term->UserDefKeys[keyn-1]->String[nparm++] = ch;
	term->UserDefKeys[keyn-1]->StringLength = nparm;
	if (nparm == slen) {
	    state=0;
        }
	break;

      case 40:  /* Getting window title or icon name */
        nparm++;
        if (nparm == llen) {
          state = 0;
        }
        break;

      case 41:   /* Got esc & k -- waiting for number */
        if (ch>='0' && ch<='9') {
          parm = parm * 10 + ch - '0';
        } else if (ch=='a' || ch=='A') {
	  term->AutoLineFeed = parm;
	  if (term->KeyState == ks_modes) update_labels();
        } else if (ch=='b' || ch=='B') {
	  term->BlockMode = parm;
	  if (term->KeyState == ks_modes) update_labels();
#if DEBUG_BLOCK_MODE
    printf ("BlockMode = %d\n", parm);
    fflush (stdout);
#endif
        } else if (ch=='c' || ch=='C') {
	  term->CapsLockMode = parm;
        } else if (ch=='d' || ch=='D') {
	  term->WarningBell = parm;
        } else if (ch=='j' || ch=='J') {
	  term->FrameRate = parm;
        } else if (ch=='k' || ch=='K') {
	  term->AutoKybdLock = parm;
        } else if (ch=='l' || ch=='L') {
	  term->LocalEcho = parm;
        } else if (ch=='m' || ch=='M') {
	  term->ModifyAll = parm;
	  if (term->KeyState == ks_modes) update_labels();
        } else if (ch=='p' || ch=='P') {
	  term->CapsMode = parm;
        } else if (ch=='q' || ch=='Q') {
	  term->KeyClick = parm;
        } else if (ch=='r' || ch=='R') {
	  term->RemoteMode = parm;
	  if (term->KeyState == ks_modes) update_labels();
        } else if (ch=='[') {
	  term->SmoothScroll = parm;
	  if (term->KeyState == ks_modes) update_labels();
          state=0;
        } else if (ch==']') {
	  term->EnterSelect = parm;
          state=0;
        }
        if (ch>='a' && ch<='z') {
          parm=0;
        } else if (ch>='A' && ch<='Z') {
	  state=0;
        }
	break;

      case 42:   /* Got esc * d -- waiting for number */
	if (ch>='0' && ch<='9') {
	  parm = parm * 10 + ch - '0';
	  nparm++;
        } else if (ch=='e' || ch=='E') {
	  term->InverseBkgd = parm;
        } else if (ch=='q' || ch=='Q') {
	  if (nparm) {
	    term->CursorType = parm;
          } else {
	    term->CursorBlanked = 0;
          }
        } else if (ch=='r' || ch=='R') {
	  term->CursorBlanked = 1;
        }
        if (ch>='a' && ch<='z') {
          parm=0;
          nparm=0;
        } else if (ch>='A' && ch<='Z') {
          state=0;
        }
	break;

      case 43:   /* Got esc & w -- waiting for number */
	if (ch>='0' && ch<='9') {
	  parm = parm * 10 + ch - '0';
        } else if (ch=='F') {
	  if (parm==12) {
	    term->ScreenBlanked = 1;
          } else if (parm==13) {
	    term->ScreenBlanked = 0;
          }
	  state=0;
        } else if (ch=='f') {
	  if (parm==6) { 
	    parm=0;
	    state=44;
          } else {
	    state=0;
          }
        } else {
	  state=0;
        }
	break;

      case 44:   /* Got esc & w 6 f -- waiting for number */
	if (ch>='0' && ch<='9') {
	  parm = parm * 10 + ch - '0';
        } else if (ch=='X') {
	  term->Columns = parm;
	  state=0;
        } else {
	  state=0;
        }
	break;

      case 45:   /* Got esc & s -- waiting for number */
	if (ch>='0' && ch<='9') {
	  parm = parm * 10 + ch - '0';
        } else if (ch=='a' || ch=='A') {
	  term->XmitFnctn_A = parm;
        } else if (ch=='b' || ch=='B') {
	  term->SPOW_B = parm;
        } else if (ch=='c' || ch=='C') {
	  term->InhEolWrp_C = parm;
        } else if (ch=='d' || ch=='D') {
	  term->LinePage_D = parm;
#if DEBUG_BLOCK_MODE
    printf ("LinePage_D = %d\n", parm);
    fflush (stdout);
#endif
        } else if (ch=='g' || ch=='G') {
	  term->InhHndShk_G = parm;
#if DEBUG_BLOCK_MODE
    printf ("InhHndShk_G = %d\n", parm);
    fflush (stdout);
#endif
        } else if (ch=='h' || ch=='H') {
	  term->InhDC2_H = parm;
#if DEBUG_BLOCK_MODE
    printf ("InhDC2_H = %d\n", parm);
    fflush (stdout);
#endif
        } else if (ch=='n' || ch=='N') {
	  term->EscXfer_N = parm;
        }
        if (ch>='a' && ch<='z') {
          parm=0;
        } else if (ch>='A' && ch<='Z') {
          state=0;
        } 
        break;

      case 46:   /* Got esc & X -- waiting for number */
	if (ch>='0' && ch<='9') {
	  parm = parm * 10 + ch - '0';
        } else if (ch=='C') {
	  term->SendCursorPos = parm;
	  state=0;
        } else {
	  state=0;
        }
	break;

      case 99:   /* Waiting for upper case letter */
        if (ch>='@' && ch<='Z') state=0;
        break;
      default:
	state=0;
	break;
    }
done:
    update_cursor();
}
/*************************************************************************/
/*     Keyboard handling routines                                        */
/*************************************************************************/ 
static void process_keyboard (char *buf, int nbuf) {
/*
**  Process data from the keyboard
*/
    if (term->RemoteMode && !term->BlockMode 
                         && !term->LineModify
                         && !term->ModifyAll) {
	conmgr_send (con, buf, nbuf);
/*
**      Check if read trigger can be cleared
*/
        if (nbuf==1 && buf[0]==13 && !term->BlockMode) {
            term->DC1Count = 0;
            term->DC2Count = 0;
#if SHOW_DC1_COUNT
            update_labels();
#endif
        }
	if (term->LocalEcho) {
	    hpterm_rxfunc (term, buf, nbuf);
        }
    } else {
	hpterm_rxfunc (term, buf, nbuf);
    }
}
/*************************************************************************/
static void process_keyboard_A (char *buf, int nbuf) {
/*
**  Process keystroke for a key that is controlled by the A strap
*/
    if (term->XmitFnctn_A) {
        process_keyboard (buf, nbuf);
    } else {
        hpterm_rxfunc (term, buf, nbuf);
    }
}
/*************************************************************************/
void hpterm_kbd_ascii (char ch) {
/*
**  Process a normal character typed on the keyboard
*/
    if (term->EnableKybd || IGNORE_KEYBOARD_LOCK) {
        process_keyboard (&ch, 1);
        if (ch==13 && term->AutoLineFeed) {
	    process_keyboard ("\012", 1);
        }
    }
}
/*************************************************************************/
void hpterm_kbd_Break() {
    if (term->RemoteMode) {
#if DEBUG_BREAK
        printf ("Sending Break...\n");
        fflush (stdout);
#endif
        conmgr_send_break (con);
        term->DC1Count = 0;
        term->DC2Count = 0;
#if SHOW_DC1_COUNT
        update_labels();
#endif
    }
}
/*************************************************************************/
void hpterm_kbd_User() {
    if (term->UserSystem || IGNORE_USER_SYSTEM_LOCK) {
        term->KeyState = ks_user;
    }
    update_labels();
}
/*************************************************************************/
void hpterm_kbd_Reset() {
    do_soft_reset();
}
/*************************************************************************/
void hpterm_kbd_Menu() {
    if (term->UserSystem || IGNORE_USER_SYSTEM_LOCK) {
        if (term->KeyState == ks_off) {
	    term->KeyState = ks_user;
        } else {
	    term->KeyState = ks_off;
        }
    }
    update_labels();
}
/*************************************************************************/
void hpterm_kbd_System() {
    if (term->UserSystem || IGNORE_USER_SYSTEM_LOCK) {
        term->KeyState = ks_system;
    }
    update_labels();
}
/*************************************************************************/
void hpterm_kbd_ClearLine() {
    process_keyboard_A ("\033K", 2);
}
/*************************************************************************/
void hpterm_kbd_InsertLine() {
    process_keyboard_A ("\033L", 2);
}
/*************************************************************************/
void hpterm_kbd_InsertChar() {
/*
**  Implement toggle
*/
    if (term->InsertMode) {
	process_keyboard_A ("\033R", 2);  /* Disable */
    } else {
	process_keyboard_A ("\033Q", 2);  /* Enable */
    }
}
/*************************************************************************/
void hpterm_kbd_ClearDisplay() {
    process_keyboard_A ("\033J", 2);
}
/*************************************************************************/
void hpterm_kbd_DeleteLine() {
    process_keyboard_A ("\033M", 2);
}
/*************************************************************************/
void hpterm_kbd_DeleteChar() {
    process_keyboard_A ("\033P", 2);
}
/*************************************************************************/
void hpterm_kbd_BackTab() {
    process_keyboard_A ("\033i", 2);
}
/*************************************************************************/
void hpterm_kbd_KP_BackTab() {
    process_keyboard_A ("\033i", 2);
}
/*************************************************************************/
void hpterm_kbd_Home() {
    process_keyboard_A ("\033H",2);
}
void hpterm_kbd_Up() {
    process_keyboard_A ("\033A",2);
}
void hpterm_kbd_Right() {
    process_keyboard_A ("\033C",2);
}
void hpterm_kbd_Left() {
    process_keyboard_A ("\033D",2);
}
void hpterm_kbd_Down() {
    process_keyboard_A ("\033B",2);
}
void hpterm_kbd_Prev() {
    process_keyboard_A ("\033V",2);
}
void hpterm_kbd_Next() {
    process_keyboard_A ("\033U",2);
}
void hpterm_kbd_RollUp() {
    process_keyboard_A ("\033S",2);
}
void hpterm_kbd_RollDown() {
    process_keyboard_A ("\033T",2);
}
void hpterm_kbd_HomeDown() {
    process_keyboard_A ("\033F",2);
}
/***************************************************************/
void do_mode_button (int i) {
/*
**  Perform mode button keypress
**  Accepts i from 0 to 7
*/
    switch (i) {
	case 0:
	    term->LineModify ^= 1;
	    break;
        case 1:
	    term->ModifyAll ^= 1;
	    break;
        case 2:
	    term->BlockMode ^= 1;
	    break;
        case 3:
	    term->RemoteMode ^= 1;
	    break;
        case 4:
	    term->SmoothScroll ^= 1;
	    break;
        case 5:
	    if (term->MemoryLock) {
		process_keyboard_A ("\033m",2);
            } else {
		process_keyboard_A ("\033l",2);
            }
	    break;
        case 6:
	    term->DisplayFuncs = !term->DisplayFuncs;
	    break;
        case 7:
	    term->AutoLineFeed ^= 1;
	    break;
    }
    update_labels();
}
/***************************************************************/
void do_system_button (int i) {
/*
**  Perform system menu button press
**  Accepts i from 0 to 7
*/
    switch (i) {
	case 0:
	    term->KeyState = ks_device_control;
	    break;
        case 1:
	    term->KeyState = ks_margins_tabs;
	    break;
        case 2:
	    break;
        case 3:
            if (term->UserSystem || IGNORE_USER_SYSTEM_LOCK) {
	        term->KeyState = ks_modes;
            }
	    break;
        case 4:
	    break;
        case 5:
	    break;
        case 6:
	    break;
        case 7:
	    term->KeyState = ks_config_keys;
	    break;
    }
    update_labels();
}
/***************************************************************/
void do_device_control_button (int i) {
}
/***************************************************************/
void do_margins_tabs_button (int i) {
}
/***************************************************************/
void do_config_keys_button (int i) {
    if (i==4) {
	term->KeyState = ks_terminal_config;
	update_labels();
    }
}
/**************************************************************/
void do_terminal_config_button (int i) {
}
/***************************************************************/
static void do_function_button (int i) {
/*
**  Perform function button keypress
**  Accepts i from 0 to 7
*/
    struct udf *u;
    int j;

    if (term->KeyState == ks_off || term->KeyState == ks_user) {
	u = term->UserDefKeys[i];
	if (u->Attribute == 1) {       /* Execute locally */
	    hpterm_rxfunc (term, u->String, u->StringLength);
        } else {                       /* Transmit to host */
            term->FunctionKeyPending = i + 1;
            check_transfers_pending();
        }
    } else if (term->KeyState == ks_modes) {
	do_mode_button (i);
    } else if (term->KeyState == ks_system) {
	do_system_button (i);
    } else if (term->KeyState == ks_device_control) {
	do_device_control_button (i);
    } else if (term->KeyState == ks_margins_tabs) {
	do_margins_tabs_button (i);
    } else if (term->KeyState == ks_config_keys) {
	do_config_keys_button (i);
    } else if (term->KeyState == ks_terminal_config) {
	do_terminal_config_button (i);
    } else {
	/* More System functions here */
    }
}
/***************************************************************/
void hpterm_kbd_F1() {
    do_function_button (0);
}
void hpterm_kbd_F2() {
    do_function_button (1);
}
void hpterm_kbd_F3() {
    do_function_button (2);
}
void hpterm_kbd_F4() {
    do_function_button (3);
}
void hpterm_kbd_F5() {
    do_function_button (4);
}
void hpterm_kbd_F6() {
    do_function_button (5);
}
void hpterm_kbd_F7() {
    do_function_button (6);
}
void hpterm_kbd_F8() {
    do_function_button (7);
}
/*********************************************************************/
void hpterm_kbd_Enter() {

    if (!term->RemoteMode) {
        /* Ignore Enter in local mode */
    } else {
        term->EnterKeyPending = 1;
        check_transfers_pending();
    }
}
/*********************************************************************/
void hpterm_kbd_Select() {

    term->FunctionKeyPending = 9;
    check_transfers_pending();
}
/*********************************************************************/
void hpterm_kbd_KP_Enter() {
/*
**  Process keypad's Enter key
*/
    if (term->EnterSelect) {
        hpterm_kbd_Select();
    } else {
        hpterm_kbd_Enter();
    }
}
/*********************************************************************/
void hpterm_mouse_click (int row, int col) {
/*
**  Process a single mouse click
**  For now, this can only be used to press a function button
*/
    int i,j;

    if (row >= term->nbrows) {
	for (i=0; i<8; i++) {
	    j = i * 9;
	    if (i > 3) j += 9;
	    if (col>=j && col<=j+7) {
		do_function_button(i);
	    }
        }
    }
}
/*********************************************************************/




