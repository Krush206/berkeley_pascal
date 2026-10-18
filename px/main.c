/* -*- mode: c; tabs: 8; hard-tabs: yes; -*- */

/*
 * px - interpreter for Berkeley Pascal
 * Version 3.0 Winter 1979
 */
#include "whoami.h"
#include "px.h"

int
main(argc, argv)
	int argc;
	char **argv;
{
	px_main(argc, argv);
	return (0);
}
