/* ============================================================================
 * File:    util.c
 * Purpose: Provides utility functions that may be useful for programs
 * 					interacting with minfs. (e.g. minls, minget)
 *
 * Author:  David Voronov
 * Date:    2025-06-20
 * ========================================================================= */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "util.h"

void default_print_help(); 
/*
 * ingest_opt - parses command line arguments
 *
 * params:
 * int argc - argument count
 * char *argv[] - array of argument strings
 * void *(print_help)(void) - usage print function called if user enters
 * improper options
 *
 * return:
 * struct minfs_opts - structure containing values of options. See util.h
 *
 * NOTE: Additional options (imagefile, srcpath, dstpath) that are specific
 * to a command are not parsed here. It is up to the client to parse those.
 *
 * */
minfs_opts ingest_opt(int argc, char *argv[], void (*print_help)(void)) {
	if (!print_help) {
		print_help = &default_print_help;
	}

	minfs_opts opts = MINFS_OPTS_DEFAULT; 

  char *optstring = "hvp:s:";
	int opt;
  while ((opt = getopt(argc, argv, optstring)) != -1) {
    switch (opt) {
    case 'h':
      print_help();
      break;
    case 'v':
      opts.v = 1;
      break;
    case 'p':
      if (strncmp(optarg, "?", 1)) {
        // Read optarg into int
        char *end;
        long p_val = strtol(optarg, &end, 10);
        if (!strcmp(end, optarg)) {
          print_help();
        } else {
          opts.part = (int)p_val;
          if (opts.part < 0 || opts.part > 3) {
            fprintf(stderr, "Minix only supports partitions 0 through 3.\n");
            exit(1);
          }
        }
      }
      break;
    case 's':
      if (strncmp(optarg, "?", 1)) {
        // Read optarg into int
        char *end;
        long s_val = strtol(optarg, &end, 10);
        if (!strcmp(end, optarg)) {
          print_help();
        } else {
          opts.sub = (int)s_val;
          if (opts.sub < 0 || opts.sub > 3) {
            fprintf(stderr,
                    "Minix only supports sub-partitions 0 through 3.\n");
            exit(1);
          }
        }
      }
      break;
    }
  }
	opts.optind = optind;

  return opts;
}

/*
 * default_print_help - provided if client doesn't provide a proper help function
 * */
void default_print_help() {
	fprintf(stderr, "Improper command usage.\n");
	exit(0);
}

