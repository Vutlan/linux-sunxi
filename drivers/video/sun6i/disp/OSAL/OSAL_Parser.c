#include "OSAL_Parser.h"

//#ifndef __FPGA_DEBUG__
#if 1
int OSAL_Script_FetchParser_Data(char *main_name, char *sub_name, int value[], int count)
{
	return script_parser_fetch(main_name, sub_name, value, count);
}


#else

int OSAL_Script_FetchParser_Data(char *main_name, char *sub_name, int value[], int count)
{
	return 0;
}

int OSAL_sw_get_ic_ver(void)
{
    return 0;
}

#endif
