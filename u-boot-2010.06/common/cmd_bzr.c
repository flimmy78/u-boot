#include <common.h>
#include <command.h>

extern void buzzer_notify(unsigned long msecs, unsigned long times);

int do_buzzer(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
    int i = 0;
    unsigned long args[2] = {55, 1};

    if(argc > 3) argc = 0;
    if(argc > 0) argc --;

    for(i = 0; i < argc; i ++) {
        args[i] = (unsigned long)simple_strtoul(argv[i+1], NULL, 10);
    }

    buzzer_notify(args[0], args[1]);

    return 0;
}

U_BOOT_CMD(
	bzr,    3,	1,  do_buzzer,
	"bzr - buzzer sound out.\n",
	"\t- holding time(unit:10ms) & times.\n"
	"\t- e.g. bzr 55 2\n"
	);


