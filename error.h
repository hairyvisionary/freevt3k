#ifdef VMS
int  VMSerror(int vms_stat);
#endif /* VMS */
int  PortableErrno(int err);
char *PortableStrerror(int err);
void PortablePerror(char *text);
