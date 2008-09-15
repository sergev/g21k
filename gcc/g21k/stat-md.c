static char *__ident__ = "@(#) stat-md.c 2.2@(#)";

#include <fcntl.h>
#include <stdio.h>

#include "insn-codes.h"
#include "tm.h"

static char* stat_file = "Match.stat";
static int file_read_p = 0;
 int rule_use[MAX_INSN_CODE][2];


void
read_stat_file ()
{
    int sfd, len, rnum;

    if (file_read_p) return;
    
    file_read_p = 1;
    for(rnum = 0; rnum < MAX_INSN_CODE; rnum++)
	rule_use[rnum][0] = rule_use[rnum][1] = 0;

    sfd = open(stat_file, O_RDONLY);
    if (sfd >= 0) {
	len = read(sfd, rule_use, sizeof rule_use) / sizeof rule_use[0];
	if (len != MAX_INSN_CODE) {
	    fprintf(stderr,
		    "Warning: didn't read correct # of  elements (%d != %d)\n",
		    len, MAX_INSN_CODE);
	    perror(" == ");
	}
	close(sfd);
    }
    else perror("\nWarning: can't open stat-file for reading ");
}


void write_stat_file()
{
    int sfd, len;

    /*** Do not write if you haven't read the file -- can *****/
    /*** erase the previous history by mistake         ****/

    if (! file_read_p) return;

    sfd = open(stat_file, O_WRONLY | O_CREAT, 0666);
    if (sfd < 0) perror("Warning: can't open stat-file for writing");
    else {
	len = write(sfd, rule_use, sizeof rule_use) / sizeof rule_use[0];
	if (len != MAX_INSN_CODE) {
	    fprintf(stderr,
		    "Warning: didn't write enough elements (%d<%d)\n",
		    len, MAX_INSN_CODE);
	    perror(" -- ");
	}
	close (sfd);
    }
}

/** time 0 means before, 1 -- after **/

void mark_as_used(rule_num, time)
    int rule_num, time;
{
    if (! file_read_p) read_stat_file();

    if (! (time == 0 || time == 1))
	fprintf(stderr, "Warning: wrong time value: %d is not 0 or 1\n", time);
    else rule_use[rule_num][time]++;
}
