/* ============================================================================
 * File:    util.h
 * Purpose: API for util.c 
 *
 * Author:  David Voronov
 * Date:    2025-06-20
 * ========================================================================= */

#ifndef UTIL_H

typedef struct minfs_opts {
	int v; // verbosity. Set to 1 if verbose selected. default: 0  
	int sub; // subpartition number. default: -1 (no subpartition selected)
	int part; // partition number. default: 0
	int optind; // option index after parsing
} minfs_opts;

// Macro for default values for struct
#define MINFS_OPTS_DEFAULT \
    (minfs_opts){           \
        .v    = 0,          \
        .sub  = -1,         \
        .part = 0,          \
		 		.optind = 0					\
    }


minfs_opts ingest_opt(int argc, char *argv[], void (*print_help)(void)); 

#endif 
