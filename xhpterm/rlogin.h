int open_client_connection (char *hostname, int portnum);
int read_rlogin_data (int s);
int send_rlogin_data (int s, char *buf, int nbuf);
int open_rlogin_connection (char *hostname);
