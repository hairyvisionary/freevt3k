/*
 * @(#) vmstypes.h: 93/09/15-17:02:00
 */
typedef struct
	{
	int		word1;
	int		word2;
	} QUAD_WORD;	

typedef struct
	{
	unsigned char	class;
	unsigned char	type;
	unsigned short	msg_len;
	char		msg[256];
	} GET_MSG;

typedef struct
	{
	unsigned char	class;
	unsigned char	type;
	unsigned short	page_width;
	unsigned long	basic;
	unsigned long	extended;
	} TRM_CHR;

typedef struct
	{
	unsigned short	status;
	unsigned short	line_speed;
	unsigned short	fill;
	unsigned char	par_flags;
	unsigned char	unused;
	} TT_SENSE_IOSB;

typedef struct
	{
	unsigned short	status;
	unsigned short	terminator_offset;
	unsigned char	terminator;
	unsigned char	unused;
	unsigned short	terminator_size;
	} TT_READ_IOSB;

typedef struct
	{
	unsigned short	status;
	unsigned short	byte_cnt;
	unsigned int	unused;
	} TT_WRITE_IOSB;

typedef struct
	{
	unsigned int	nil;
	unsigned int	ctrl_char;
	} TERMINATOR_MASK;

/*
typedef struct
	{
	unsigned int	length;
	unsigned char	*addr;
	} NFB_DESC;

typedef struct
	{
	unsigned char	function;
	unsigned int	nil;
	} NFB;

typedef struct
	{
	unsigned int	length;
	unsigned char	*addr;
	} NCB_DESC;
*/

#define MAX_NCB			(128)
typedef struct
	{ /* Mailbox data format */
	short int	msg;
	short int	unit;
	char		name[MAX_NCB];
	} MBX_DATA;

#define RETSIGTYPE		void
