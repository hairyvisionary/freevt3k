static void show_tty_error (char *funcname, int errnum);
int open_tty_connection (char *devicename);
int read_tty_data (int s);
int send_tty_data (int s, char *buf, int nbuf);
