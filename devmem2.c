/*
 * devmem2.c: Simple program to read/write from/to any location in memory.
 *
 *  Copyright (C) 2000, Jan-Derk Bakker (jdb@lartmaker.nl)
 *
 *
 * This software has been developed for the LART computing board
 * (http://www.lart.tudelft.nl/). The development has been sponsored by
 * the Mobile MultiMedia Communications (http://www.mmc.tudelft.nl/)
 * and Ubiquitous Communications (http://www.ubicom.tudelft.nl/)
 * projects.
 *
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/mman.h>

#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
  __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

void show_usage(char *program)
{
	fprintf(stderr, "\nUsage:\t%s { address } [ type [ data ] ]\n"
		"\taddress : memory address to act upon\n"
		"\ttype    : access operation type : [b]yte, [h]alfword, [w]ord, [l]ong\n"
		"\tdata    : data to be written\n\n",
		program);
		exit(1);
}

unsigned long read_mem(int access_type, void *virt_addr)
{
	unsigned long read_result = 0;

	switch (access_type) {
		case 'b':
			read_result = *((unsigned char *) virt_addr);
			break;
		case 'h':
			read_result = *((unsigned short *) virt_addr);
			break;
		case 'w':
			read_result = *((unsigned int *) virt_addr);
			break;
		case 'l':
			read_result = *((unsigned long *) virt_addr);
			break;
		default:
			fprintf(stderr, "Illegal data type '%c'.\n", access_type);
			exit(2);
	}

	return read_result;
}

int main(int argc, char **argv)
{
	int fd;
	void *map_base, *virt_addr;
	unsigned long read_result, writeval;
	off_t target;
	int access_type = 'w';

	if (argc < 2)
		show_usage(argv[0]);

	target = strtoul(argv[1], 0, 0);

	if (argc > 2)
		access_type = tolower(argv[2][0]);
	/* partial read/write */
	if ((fd = open("/dev/mem", argv[3] ? (O_RDWR | O_SYNC) : (O_RDONLY | O_SYNC)))
		== -1) FATAL;
	printf("/dev/mem opened.\n");
	fflush(stdout);

	/* Map one page */
	map_base = mmap(0, MAP_SIZE,
		argv[3] ? (PROT_READ | PROT_WRITE) : PROT_READ,
		MAP_SHARED, fd, target & ~MAP_MASK);
	if (map_base == MAP_FAILED) FATAL;
	printf("Memory mapped at address %p.\n", map_base);
	fflush(stdout);

	virt_addr = map_base + (target & MAP_MASK);

	/* Read memory, write memory will run into it */
	if (argc <= 3) {
		read_result = read_mem(access_type, virt_addr);
		printf("Value at address 0x%lX (%p): 0x%lX\n", target, virt_addr, read_result);
		fflush(stdout);
	}

	/* Write memory */
	if (argc > 3) {
		writeval = strtoul(argv[3], 0, 0);
		switch (access_type) {
			case 'b':
				*((unsigned char *) virt_addr) = writeval;
				break;
			case 'h':
				*((unsigned short *) virt_addr) = writeval;
				break;
			case 'w':
				*((unsigned int *) virt_addr) = writeval;
				break;
			case 'l':
				*((unsigned long *) virt_addr) = writeval;
		}
		printf("Written 0x%lX\n", writeval);
		fflush(stdout);
		/* don't read back */
#if 0
		read_result = read_mem(access_type, virt_addr);
		printf("Readback 0x%lX\n", read_result);
		fflush(stdout);
#endif
	}

	if (munmap(map_base, MAP_SIZE) == -1) FATAL;
	close(fd);
	return 0;
}
