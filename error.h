#ifdef VMS
int  VMSerror(int vms_stat);
#endif /* VMS */
int  PortableErrno P((int err));
char *PortableStrerror P((int err));
void PortablePerror P((char *text));
