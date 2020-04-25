/* acpi application yacpi
 * Copyright (C) 2005-2007 Nico Golde <nico@ngolde.de>
 * See COPYING file for license details
 */

#define _GNU_SOURCE

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libacpi.h>
#include <getopt.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include "get_cpu.h"
#include "yacpi.h"


/* needed for print functions to get y coordinates and
 * interval for each output, yeah I know using global variables
 * sucks, but it's PITA to use local ones here */
static int line = 0;
static int interval = 2;

static void
usage (void){
	puts ("YACPI " VERSION " by Nico Golde <nico@ngolde.de>");
	puts ("usage: yacpi [options]\n"
			"options:\n"
			"-v    print version number + release date.\n"
			"-h    print this help.\n\n"
			"-p    plain-text output.\n"
			"-n    display also not found items\n"
			"-l    loop every second\n"
			"-d    delay seconds for loop\n\n"
			"-b    show battery information\n"
			"-a    show ac adapter information\n"
			"-f    show fan information\n"
			"-t    show thermal zone information\n"
			"-c    show cpu frequency\n"
			"-g    show used frequency governor\n\n"
			"The default prints all available information.\n");
	exit(EXIT_SUCCESS);
}

static void
noinfo(void){
	attron(COLOR_PAIR(3));
	printw("no information");
	attroff(COLOR_PAIR(3));
}

static void
color_change (const battery_t *binfo, const int temp) {
	if(temp == 0 && binfo != NULL) {
		if(binfo->batt_state == B_HIGH)
			attron (COLOR_PAIR (1));
		else if(binfo->batt_state == B_MED)
			attron (COLOR_PAIR (2));
		else
			attron (COLOR_PAIR (3));
	}
	else {
		if(temp <= 59)
			attron (COLOR_PAIR (1));
		else if(temp <= 69 && temp >= 60)
			attron (COLOR_PAIR (2));
		else if(temp >= 70)
			attron (COLOR_PAIR (3));
	}
}

static void
helpbox(int x1, int y1, int x2, int y2) {
	int i, j;

	attron(WA_REVERSE);
	for(i = y1; i <= y2; i++) {
		for(j = x1; j <= x2; j++) {
			mvaddch(i, j, ' ');
		}
	}
	attroff(WA_REVERSE);
}

static void
print_window (void){
	int i;

	attron(WA_REVERSE);
	clear();
	/* print white lines and version numbers */
	for (i = 0; i < COLS; i++) {
		mvaddch (0, i, ' ');
		mvaddch (LINES - 1, i, ' ');
	}
	mvaddstr(0, 0, "* YACPI " VERSION " by Nico Golde");
	mvaddstr(LINES - 1, 0, "h:help, r:reload, q:quit ");

	attroff(WA_REVERSE);
	move(line = 0, 0);
}

static void
help (const unsigned int opt) {
	int centerx, centery;
	int input;
	int offset = 13;
	int offsetstr = 11;
	char *helptext = "Press any[tm] key to continue.";

	centerx = COLS / 2;
	centery = LINES / 2;

	helpbox(centerx - 15, centery, centerx + 14, centery + 4);

	attron (WA_REVERSE);
	/* Keys */
	mvprintw(centery + 1, centerx - offset, "q");
	mvprintw(centery + interval, centerx - offset, "r");
	mvprintw(centery + 3, centerx - offset, "h");

	/* Descriptions */
	mvaddstr(centery + 1, centerx - offsetstr, "End Yacpi ...");
	mvaddstr(centery + interval, centerx - offsetstr, "Reload ACPI data ...");
	mvaddstr(centery + 3, centerx - offsetstr, "Show this help ...");
	mvprintw(LINES - 1, COLS - strlen(helptext), "%s", helptext);
	attroff(WA_REVERSE);

	nodelay(stdscr, FALSE);

	input = getch();

	if(opt & LOOP_F)
		nodelay(stdscr, TRUE);

	/* Return input back into input queue so it gets automatically
	   executed. */
	if ((input != '\n') && (input != 'h') && (input != 'q'))
		ungetch(input);

	print_window();
}

static void
print_ac(const int ac_s, global_t *global, const int t, const int not_f){
	adapter_t *ac = &global->adapt;

	if(!ac) return;
	read_acpi_acstate(global);
	if(t){
		if(ac_s == SUCCESS && ac->ac_state == P_BATT)
			printf("| ac = off-line ");
		else if(ac_s == SUCCESS && ac->ac_state == P_AC)
			printf("| ac = on-line ");
		else if(ac_s != SUCCESS && not_f)
			printf("| ac = no info ");
	} else {
		move(line = line + interval, 0);
		printw("%s", COLS <= 48 ? "AC: " : "AC adapter status: ");
		if(ac_s == SUCCESS && ac->ac_state == P_BATT){
			attron(COLOR_PAIR(3));
			printw("off");
			attroff(COLOR_PAIR(3));
		}
		else if(ac_s == SUCCESS && ac->ac_state == P_AC){
			attron(COLOR_PAIR(1));
			printw("on ");
			attroff(COLOR_PAIR(1));
		}
		else if(ac_s != SUCCESS && not_f)
			noinfo();
	}
}

static void
print_cpu(const int t, const int not_f){
	int cur = get_cpu_cur();
	int max = get_cpu_max();

	if(!t) move(line = line + interval, 0);

	if(max && cur){
		if(!t) printw("CPU frequency: %d/%d MHz      ", cur / 1000, max / 1000);
		else printf ("| cpu = %d/%d mhz ", cur / 1000, max / 1000);
	}
	else if(not_f){
		if(!t) printw("CPU frequency: not supported");
		else printf ("| cpu = no info ");
	}
}

static void
print_gov(const int t, const int not_f){
	char *cpu_gov = get_cpu_gov();
	if(!cpu_gov && t && not_f)
		printf("| gov = no info ");
	else if(!cpu_gov && !t && not_f){
		move(line = line + 1, 0);
		printw("CPU governor: not supported");
	}
	if(!cpu_gov) return;

	if(t)
		printf ("| gov = %s ", cpu_gov);
	else{
		move(line = line + interval, 0);
		printw("CPU governor: %s        ", cpu_gov);
	}
	free(cpu_gov);
}

static void
print_fan(const int fan_s, global_t *global, const int t, const int not_f){
	fan_t *fa;
	int i;
	if(fan_s == SUCCESS){
		for(i=0; i<global->fan_count; i++){
			/* read fan state */
			read_acpi_fan(i);
			fa = &fans[i];
			if(!fa) return;
			if(t) printf("| %s = %s ", fa->name, fa->fan_state==F_ON?"on":"off");
			else {
				move(line = line + interval, 0);
				printw("%s:", fa->name);
				if(fa && fa->fan_state==F_OFF){
					attron (COLOR_PAIR (3));
					printw(" off");
					attroff (COLOR_PAIR (3));
				}
				else if(fa && fa->fan_state==F_ON){
					attron (COLOR_PAIR (1));
					printw(" on ");
					attroff (COLOR_PAIR (1));
				}
			}
		}
	} else if(fan_s != SUCCESS && t && not_f) printf("| fan = no info ");
	else if(fan_s != SUCCESS && !t && not_f){
		move(line = line + interval,0);
		printw("Fan: ");
		noinfo();
	}
}

static void
print_therm(const int therm_s, global_t *global, const int t, const int not_f){
	int i;
	thermal_t *tp;
	if(!global) return;

	if(therm_s == SUCCESS){
		for(i=0; i<global->thermal_count; i++){
			/* read current thermal zone values */
			read_acpi_zone(i, global);
			tp = &thermals[i];
			if(!tp) return;
			if(global->thermal_count == 1 && !t)
				snprintf(tp->name, MAX_NAME, "Temperature");

			if(!t) move(line = line + interval,0);
			if(tp->frequency == DISABLED){
				if(t)
					printf("| %s = %d C ",tp->name, tp->temperature);
				else{
					printw("%s: ", tp->name);
					color_change(NULL, tp->temperature);
					printw("%d ", tp->temperature);
					attrset(WA_NORMAL);
					printw("degrees C");
				}
			}
			else{
				if(t)
					printf("| %s = %d C, freq = %d", tp->name, tp->temperature, tp->frequency);
				else{
					printw("%s: ", tp->name);
					color_change(NULL, tp->temperature);
					printw("%d ", global->temperature);
					attrset(WA_NORMAL);
					printw("degrees C, frequency: %d", tp->frequency);
				}
			}
		}
	} else if(therm_s != SUCCESS && t && not_f) printf("| temp = no info ");
	else if(therm_s != SUCCESS && !t && not_f){
		move(line = line + interval, 0);
		printw("Thermal zones: ");
		noinfo();
	}
}

static void
print_charge(const charge_state_t charge, const int t){
	/* to use color_change here is a dirty hack :) */
	if(charge == C_CHARGE)
		if(t) printf(" charging ");
		else {
			color_change(NULL, 60);
			printw("charging    \n");
		}
	else if(charge == C_DISCHARGE)
		if(t) printf(" discharging ");
		else{
			color_change(NULL, 70);
			printw("discharging \n");
		}
	else if(charge == C_CHARGED)
		if(t) printf(" charged ");
		else {
			color_change(NULL, 50);
			printw("charged   \n");
		}
	else printf(" no info ");
	if(!t) attrset(WA_NORMAL);
}

static void
print_time(const battery_t *binfo, global_t *global, const int t){
	adapter_t *ac = &global->adapt;
	if(!ac) return;
	if (!ac->ac_state == P_AC){
		if(t)
			printf (" rtime %02d:%02d h ", binfo->remaining_time / 60, binfo->remaining_time % 60);
		else{
			move(++line, 0);
			printw("Remaining time: %02d:%02d h   ", binfo->remaining_time / 60, binfo->remaining_time % 60);
		}
	}
	if (ac->ac_state == P_AC && binfo->charge_time > 0){
		if(t)
			printf (" rctime %02d:%02d h ", binfo->charge_time / 60, binfo->charge_time % 60);
		else{
			move(++line, 0);
			printw("Rem. charge time: %02d:%02d h     ", binfo->charge_time / 60, binfo->charge_time % 60);
		}
	}
}


static void
print_battery(const int batt_s, global_t *global, const int t, const int not_f){
	battery_t *binfo;
	int battery_loops, bar_steps;
	int i;

	if(!global) return;
	if(batt_s != SUCCESS && not_f){
		if(t)
			printf("| no info ");
		else{
			move(line = line + interval, 0);
			printw("Battery information: ");
			noinfo();
		}
		return;
	}
	for(i = 0; i < global->batt_count; i++){
		binfo = &batteries[i];
		if(!binfo) return;
		/* read current battery values */
		read_acpi_batt(i);
		if(binfo && binfo->present){
			if(t) printf("| %s = %d%%", binfo->name, binfo->percentage);
			else if (COLS > 48){
				move(line = line + interval, 0);
				printw ("%s Capacity [", binfo->name);
				color_change (binfo, 0);
				if(COLS <= 88)
					bar_steps = binfo->percentage / 4;
				else
					bar_steps = binfo->percentage / 2;

				for (battery_loops = 1; battery_loops <= bar_steps; battery_loops++)
					printw ("|");
				attrset (WA_NORMAL);
				for (battery_loops = 1; battery_loops <= 25 - bar_steps; battery_loops++)
					printw (" ");
				printw ("]");
				printw (" %2d%%", binfo->percentage);
			} else {
				move(line = line + interval, 0);
				printw ("Battery %s on %2d%%", binfo->name, binfo->percentage);
			}
			if(!t) {
				move(line = line + interval, 0);
				printw("%s status: ", binfo->name);
			}
			print_charge(binfo->charge_state, t);
			print_time(binfo, global, t);
		} else if(binfo && !binfo->present && not_f){
			if(t) printf("| %s = not present ", binfo->name);
			else{
				move(line = line + interval, 0);
				printw("%s: not present  ", binfo->name);
			}
		}
	}
}

static void
init_ncurses(void){
	initscr();
	cbreak();
	noecho();
	nodelay(stdscr, TRUE);
	start_color();             /* initialize colors */
	use_default_colors ();
	init_pair (1, COLOR_GREEN, -1);
	init_pair (2, COLOR_YELLOW, -1);
	init_pair (3, COLOR_RED, -1);
	init_pair (4, COLOR_WHITE, -1);
}

static void
	acpi_handling(global_t *global, const int batt_s, const int therm_s, const int ac_s, const int fan_s, const unsigned int options){
		if((options & BATT_F))
			print_battery(batt_s, global, options & PTEXT_F, options & NON_F);
		if((options & AC_F))
			print_ac(ac_s, global, options & PTEXT_F, options & NON_F);
		if((options & THERM_F))
			print_therm(therm_s, global, options & PTEXT_F, options & NON_F);
		if((options & FAN_F))
			print_fan(fan_s, global, options & PTEXT_F, options & NON_F);
		if((options & GOV_F))
			print_gov(options & PTEXT_F, options & NON_F);
		if((options & CPU_F))
			print_cpu(options & PTEXT_F, options & NON_F);
		if(options & PTEXT_F) printf("\n");
		else curs_set(0);

		/* refresh the output if not enough LINES on the screen,
		 * display without empty line between the output lines */
		if(!(options & PTEXT_F) && line >= LINES - 1){
			print_window();
			interval = 1;
			acpi_handling(global, batt_s, therm_s, ac_s, fan_s, options);
		} else interval = 2;

	}

static void
user_input(global_t *global, const int batt_s, const int therm_s, const int ac_s, const int fan_s, const unsigned int options){
	char ch = '\0';

	if(!(options & PTEXT_F)) print_window();

	acpi_handling(global, batt_s, therm_s, ac_s, fan_s, options);

	if(!(options & PTEXT_F)){
		ch = getch();
		switch(ch){
			case 'q':
				endwin();
				puts("");
				exit(EXIT_SUCCESS);
				break;
			case 'h':
				help(options);
				break;
			case 'r':
				if(!(options & PTEXT_F)){
					move(2, 0);
					line = 0;
				}
				acpi_handling(global, batt_s, therm_s, ac_s, fan_s, options);
			default: 
				break;
		}
	}
}

int
main(int argc, char **argv){
	unsigned int options = 0;
	int delay = 1;
	int option, r;
	extern char *optarg;
	extern int optind, optopt;
	global_t *global = NULL;
	int ac_s, batt_s, therm_s, fan_s;
	fd_set rd;
	struct timeval tv;

	options = 0;

	/* parse command line arguments */
	while ((option = getopt(argc, argv, "fnpahvtblcgd:")) != -1) {
		if (option == EOF)
			break;
		switch (option) {
			case 'f':
				options |= FAN_F;
				break;
			case 'h':
				usage();
				break;
			case 'a':
				options |= AC_F;
				break;
			case 'v':
				puts ("YACPI " VERSION " by Nico Golde - Released: 09.08.2007\n");
				return 0;
				break;
			case 'b':
				options |= BATT_F;
				break;
			case 't':
				options |= THERM_F;
				break;
			case 'l':
				options |= LOOP_F;
				break;
			case 'n':
				options |= NON_F;
				break;
			case 'd':
				delay = strtol(optarg, NULL, 10);
				if(delay <= 0){
					fprintf(stderr, "Option -%c requires an unsigned integer (>=0)  as arguments\n", optopt);
					return -1;
				}
				break;
			case 'c':
				options |= CPU_F;
				break;
			case 'p':
				options |= PTEXT_F;
				break;
			case 'g':
				options |= GOV_F;
				break;
			case ':':
				fprintf(stderr, "Option -%c requires an operand\n", optopt);
				return -1;
				break;
			case '?':
				fprintf(stderr, "Unrecognized option: -%c\n", optopt);
				return -1;
			default:
				usage ();
				break;
		}
	}

	if (0 == (options & ~NON_F & ~PTEXT_F & ~LOOP_F))
		options |= ALL_F;

	if(check_acpi_support() == NOT_SUPPORTED){
		printf("No acpi support for your system?\n");
		return -1;
	}

	if((global = malloc (sizeof (global_t))) == NULL){
		perror("malloc");
		return -1;
	}

	/* initialize battery, thermal zones, fans and ac state */
	batt_s = init_acpi_batt(global);
	therm_s = init_acpi_thermal(global);
	fan_s = init_acpi_fan(global);
	ac_s = init_acpi_acadapt(global);

	if(!(options & PTEXT_F)){
		/* clear the screen */
		printf("\033[2J");
		init_ncurses();
	}

	do {
		if(!(options & LOOP_F) && !(options & PTEXT_F)){
			nodelay(stdscr, FALSE);
			while(1) user_input(global, batt_s, therm_s, ac_s, fan_s, options);
		}

		user_input(global, batt_s, therm_s, ac_s, fan_s, options);

		if((options & LOOP_F) && !(options & PTEXT_F)){
			FD_ZERO(&rd);
			FD_SET(0, &rd);

			tv.tv_sec = delay;
			tv.tv_usec = 0;

			r = select(1, &rd, 0, 0, &tv);
			if(r < 0 && errno != EINTR){
				perror("yacpi: error on select");
				return -1;
			}
			if(FD_ISSET(0, &rd)) user_input(global, batt_s, therm_s, ac_s, fan_s, options);
		}

		if ((options & LOOP_F) && (options & PTEXT_F))
			sleep (delay);

	} while (options & LOOP_F);

	if(!(options & PTEXT_F)){
		curs_set(1);
		endwin();
	}
	free(global);

	return 0;
}
