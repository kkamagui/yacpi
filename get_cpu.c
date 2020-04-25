/* get_cpu.c gets current cpu information 
 * Copyright (C) 2005 Nico Golde <nico@ngolde.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <string.h>

int
get_cpu_cur() {
	FILE *cpuinfo;
	int freq = 0;
	if((cpuinfo=fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq","r"))==NULL){
		return freq;
	}
	fscanf(cpuinfo,"%d",&freq);
	fclose(cpuinfo);
	return freq;
}

int
get_cpu_max() {
	FILE *cpuinfo;
	int freq = 0;
	if((cpuinfo=fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq","r"))==NULL){
		return freq;
	}
	fscanf(cpuinfo,"%d",&freq);
	fclose(cpuinfo);
	return freq;
}

char *
get_cpu_gov() {
	FILE *cpuinfo;
	char buffer[128];
	if((cpuinfo=fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor","r"))==NULL){
		return strdup("not supported"); 
	}
	fgets(buffer,sizeof(buffer),cpuinfo);
	buffer[strlen(buffer) - 1]='\0';
	fclose(cpuinfo);
	return strdup(buffer);
}

