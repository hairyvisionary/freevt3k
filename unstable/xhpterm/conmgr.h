/*
**  conmgr.h -- Connection manager
*/
enum e_contype {
    e_none = 0,       /* Connection is not established */
    e_tty = 1,        /* Connection is to a tty port */
    e_rlogin = 2,     /* Connection is via rlogin */
    e_vt3k = 3        /* Connection is via vt3k */
};

struct conmgr {
    enum e_contype type;
    char *hostname;
    void *ptr;
    int socket;
    int eof;
};

struct conmgr * conmgr_connect (enum e_contype type, char *hostname);
void conmgr_read (struct conmgr *con);
void conmgr_send (struct conmgr *con, char *buf, int nbuf);
void conmgr_send_break (struct conmgr *con);
void conmgr_close (struct conmgr *con);
void conmgr_rxfunc (long refcon, char *buf, int nbuf);
