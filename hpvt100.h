#ifdef __STDC__
void vt3kHPtoVT100(int32 refCon, char *buf, int buf_len);
void vt3kHPtoVT52(int32 refCon, char *buf, int buf_len);
void TranslateKeyboard(char *buf, int *buf_len);
#else
void vt3kHPtoVT100();
void vt3kHPtoVT52();
void TranslateKeyboard();
#endif
