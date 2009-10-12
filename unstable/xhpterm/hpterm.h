/*
   **  hpterm.h  --  Header file for HP terminal emulator
 */
/*********************************************************************/
struct row
{
  int nbchars;			/* Number of characters in this row */
  char *text;			/* Pointer to character buffer [132] */
  char *disp;			/* Pointer to display enhancements [132] */
  struct row *prev;		/* Pointer to next row of memory */
  struct row *next;		/* Pointer to previous row of memory */
};
/*********************************************************************/
/*
   **  Possible states of the function keys
 */
enum ks
{
  ks_off = 0,
  ks_modes = 1,
  ks_user = 2,
  ks_system = 3,
  ks_device_control,
  ks_margins_tabs,
  ks_config_keys,
  ks_terminal_config
};
/*********************************************************************/
/*
   **  Masks for display enhancements bits
 */
#define HPTERM_BLINK_MASK 0x1
#define HPTERM_INVERSE_MASK 0x2
#define HPTERM_UNDERLINE_MASK 0x4
#define HPTERM_HALFBRIGHT_MASK 0x8
#define HPTERM_END_ENHANCEMENT 0x10

#define HPTERM_ANY_ENHANCEMENT 0x1F

#define HPTERM_START_FIELD 0x20
#define HPTERM_START_TX_ONLY 0x40
#define HPTERM_END_FIELD 0x80
/*********************************************************************/
struct hpterm
{
/*
   **  Global configuration
 */
  int FrameRate;		/* Vertical refresh rate in Hertz (ignored) */
  int DisplayOff;		/* Screen saver minutes (ignored) */
  int InverseBkgd;		/* =1 if video is reversed (ignored) */
  int Columns;			/* Screen width, 80 or 132 */
  int CursorType;		/* 0=Block, 1=Line */
  int KeyClick;			/* 0=No, 1=Yes (ignored) */
  int AutoRepeat;		/* 0=No, 1=Yes */
  int WarningBell;		/* 0=Bell off, 1=Bell sounds */
  int StaticCursor;		/* 0=Cursor blinks, 1=Doesn't blink */
/*
   **  Terminal Configuration
 */
  char *Datacomm;		/* Datacomm port */
  char *Keyboard;		/* Keyboard type, default=USASCII */
  char *TerminalId;		/* Terminal Id string, default=70092 */
  char *Language;		/* User's language, default=ENGLISH */
  int LocalEcho;		/* 0=Off, 1=On */
  int CapsLock;			/* 0=Off, 1=On */
  int StartCol;			/* Starting column for xmit line function */
  int MarginBell;		/* 0=CTRL-G only, 1=CTRL-G and margin */
  int XmitFnctn_A;		/* 0=Execute locally, 1=Transmit to host */
  int SPOW_B;			/* 0=Spaces overwrite, 1=Use SPOW mode */
  int InhEolWrp_C;		/* 0=Auto CRLF, 1=Stay in last column */
  int LinePage_D;		/* 0=Line mode, 1=Page mode */
  int InhHndShk_G;		/* 0=No, 1=Yes */
  int InhDC2_H;			/* 0=No, 1=Yes */
  int EscXfer_N;		/* 0=No, 1=Yes */
  int FldSeparator;		/* 31=US */
  int BlkTerminator;		/* 30=RS */
  int ReturnEnter;		/* 0=No, 1=Yes */
  int ReturnDef;		/* 13=CR */
  int TabSpaces;		/* 0=Off, 1=On */
  int NumPadTab;		/* 0=Tab, 1=Enter, 2=Return */
  int PrintMode;		/* 0=Fields, 1=All */
  int TermMode;			/* 0=HP, 1=EM100, 2=EM52, 3=EM220 (ignored) */
/*
   **  Datacomm Configuration
 */
  int BaudRate;			/* 75..38400 */
  int Parity;			/* 0=NONE/8, 1=ZEROES, 2=ODD, 3=ONES, 4=EVEN */
  int EnqAck;			/* 0=No, 1=Yes */
  int Asterisk;			/* 0=OFF, 1=CS, 2=DM, 3=RR */
  int ChkParity;		/* 0=No, 1=Yes */
  int SR_CH;			/* 0=LO, 1=HI */
  int RecvPace;			/* 0=NONE, 1=XON/XOFF */
  int XmitPace;			/* 0=NONE, 1=XON/XOFF */
  int CS_CB_Xmit;		/* 0=NO, 1=YES */
/*
   **  External Device Configuration
 */
  int PrinterBaud;		/* 75..38400 */
  int PrinterParity;		/* 0=NONE/8 1=ZEROES, 2=ODD, 3=ONES, 4=EVEN */
  int PrinterNulls;		/* 0..255 */
  int PrinterType;		/* 0=ROMAN8, 1=EXT ROMAN */
  int SRRXmit;			/* 0=No, 1=Yes */
  int SRRInvert;		/* 0=No, 1=Yes */
  int PrinterPace;		/* 0=NONE, 1=XON/XOFF */
  int PrinterCSCB;		/* 0=No, 1=Yes */
/*
   **  Keyboard Control Modes
 */
  int RemoteMode;		/* 0=Local Mode, 1=Remote Mode */
  int BlockMode;		/* 0=Character Mode, 1=Block Mode */
  int FormatMode;		/* 0=Disable, 1=Enable */
  int AutoKybdLock;		/* 0=Disable, 1=Enable */
  int SendCursorPos;		/* 0=Disable, 1=Enable */
  int LineModify;		/* 0=Off, 1=On */
  int ModifyAll;		/* 0=Disable, 1=Enable */
  int ReadAllMode;		/* 0=Disable, 1=Enable */
  int SmoothScroll;		/* 0=Disable, 1=Enable */
  int MemoryLock;		/* 0=Disable, 1=Enable */
  int MemLockRow;		/* Row number of mem lock, 0..nbrows-1 */
  int DisplayFuncs;		/* 0=Disable, 1=Enable */
  int AutoLineFeed;		/* 0=Disable, 1=Enable */
  int CapsMode;			/* 0=Disable, 1=Enable */
  int CapsLockMode;		/* 0=Disable, 1=Enable */
  int EnterSelect;		/* 0=Enter, 1=Select */
  struct udf
  {
    int Attribute;		/* 0=Normal, 1=Local only, 2=Transmit only */
    int LabelLength;		/* Number of chars in label */
    char Label[16];		/* Text of key label */
    int StringLength;		/* Number of chars in string */
    char String[80];		/* Text of key string */
  }
  **UserDefKeys;
  int UserKeyMenu;		/* 0=Off, 1=On (Esc j, Esc k) */
  enum ks KeyState;		/* 0=Off, 1=modes, 2=user labels, ... */
  int UserSystem;		/* 0=Disable, 1=Enable  (&jR, &jS) */
  int MessageLength;		/* Number of chars from Ec&j--L */
  char *Message;		/* Text of Ec&j--L message */
  int MessageMode;		/* Number from Ec&j-D message */
  struct row *menu1;		/* 1st row of function button display */
  struct row *menu2;		/* 2nd row of function button display */
  int EnableKybd;		/* 0=Disable, 1=Enable */
  int InsertMode;		/* 0=Disable, 1=Enable, 2=Wrap (EcR, EcQ, EcN) */
/*
   **  Display Control
 */
  int ScreenBlanked;		/* 0=Normal, 1=Blanked */
  int CursorBlanked;		/* 0=Normal, 1=Blanked */
/*
   **  Margins and tabs
 */
  int LeftMargin;		/* 0..nbcol-1 */
  int RightMargin;		/* 0..nbcol-1 */
  char *TabStops;		/* 0=Tab clear, 1=Tab set [132] */
/*
   **  Printer and Data Logging
 */
  int DestDevice;		/* 3=Display, 4=Printer */
  int DataLogging;		/* 0=Off, 1=Top Logging, 2=Bottom Logging */
  int RecordMode;		/* 0=Off, 1=On */
  int RecordEndChar;		/* Character which ends record mode, 0=None */
/*
   **  Transfer Pending Flags
 */
  int PrimaryStatusPending;	/* 0=No, 1=Esc ^ received */
  int SecondaryStatusPending;	/* 0=No, 1=Esc ~ received */
  int DeviceStatusPending;	/* 0=No, 1=Esc & p 4 ^ received */
  int CursorSensePending;	/* 0=No, 1=Absolute, 2=Relative */
  int FunctionKeyPending;	/* 0=No, 1..8=User key number, 9=Select */
  int EnterKeyPending;		/* 0=No, 1=Kybd, 2=Esc d */
  int DeviceCompletionPending;	/* 0=No, 1=Yes */
  int TerminalIdPending;	/* 0=No, 1=Yes */
  int DC1Count;			/* Number of DC1s we have rec'd */
  int DC2Count;			/* Number of DC2s we have sent */
/*
   **  Memory buffer
 */
  struct row *head;		/* Pointer to first row of memory */
  struct row *tail;		/* Pointer to last row of memory */
/*
   **  Cursor and scroll window
 */
#if 0
  int cr;			/* Cursor row number, 0..nbrows-1 */
  int cc;			/* Cursor column number, 0..nbcols-1 */
#endif
  struct row *dptr;		/* Pointer to first row on screen */
#if defined(MEMLOCK_2000)
  struct row *MemLockRP;	/* Pointer to last locked row on screen */
#endif
  int nbrows;			/* Number of rows on screen, default=24 */
  int nbcols;			/* Number of columns on screen, 80 or 132 */
/*
   **  Flags to control deferred update
 */
  int update_all;		/* =1 if scrollable region needs updating */
  int update_menus;		/* =1 if function key menus need updating */
};

void set_display_functions (void);
void clear_display_functions (void);
#if defined(MEMLOCK_2000)
void do_roll_down(void);
void do_roll_up(void);
#endif
void init_hpterm (void);
void hpterm_winsize (int nbrows, int nbcols);
void hpterm_mouse_click (int row, int col);
void term_update (void);
void term_redraw (void);
void hpterm_rxfunc (void *, char *, int);
void hpterm_kbd_ascii (char);
void hpterm_kbd_Reset ();
void hpterm_kbd_Break ();
void hpterm_kbd_Menu ();
void hpterm_kbd_User ();
void hpterm_kbd_System ();
#if defined(MEMLOCK_2000)
void hpterm_kbd_Modes ();
void hpterm_kbd_Clear ();
#endif
void hpterm_kbd_ClearDisplay ();
void hpterm_kbd_ClearLine ();
void hpterm_kbd_InsertLine ();
void hpterm_kbd_DeleteLine ();
void hpterm_kbd_InsertChar ();
void hpterm_kbd_DeleteChar ();
void hpterm_kbd_BackTab ();
void hpterm_kbd_KP_BackTab ();
void hpterm_kbd_F1 ();
void hpterm_kbd_F2 ();
void hpterm_kbd_F3 ();
void hpterm_kbd_F4 ();
void hpterm_kbd_F5 ();
void hpterm_kbd_F6 ();
void hpterm_kbd_F7 ();
void hpterm_kbd_F8 ();
void hpterm_kbd_Home ();
void hpterm_kbd_Left ();
void hpterm_kbd_Up ();
void hpterm_kbd_Right ();
void hpterm_kbd_Down ();
void hpterm_kbd_Prev ();
void hpterm_kbd_Next ();
void hpterm_kbd_RollUp ();
void hpterm_kbd_RollDown ();
void hpterm_kbd_HomeDown ();
void hpterm_kbd_Enter ();
void hpterm_kbd_Select ();
void hpterm_kbd_KP_Enter ();
#if defined(MEMLOCK_2000)
void dump_display ();
#endif
