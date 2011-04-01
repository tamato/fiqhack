/*	SCCS Id: @(#)dlb_main.c 3.4	1998/08/16	*/
/* Copyright (c) Kenneth Lorber, Bethesda, Maryland, 1993. */
/* NetHack may be freely redistributed.  See license for details. */

/* data librarian; only useful if you are making the library version, DLBLIB */

#include "config.h"
#include "dlb.h"
#if !defined(O_WRONLY)
#include <fcntl.h>
#endif

static void FDECL(xexit, (int));

#ifdef DLB

#define DLB_DIRECTORY "Directory"	/* name of lib directory */
#define LIBLISTFILE "dlb.lst"		/* default list file */

/* library functions (from dlb.c) */
extern boolean FDECL(open_library,(const char *,library *));
extern void FDECL(close_library,(library *));

char *FDECL(eos, (char *));	/* also used by dlb.c */
FILE *FDECL(fopen_datafile, (const char *,const char *));

static void FDECL(Write, (int,char *,long));
static void NDECL(usage);
static void NDECL(verbose_help);
static void FDECL(write_dlb_directory, (int,int,libdir *,long,long,long));

static char default_progname[] = "dlb";
static char *progname = default_progname;

/* fixed library and list file names - can be overridden if necessary */
static const char *library_file = DLBFILE;
static const char *list_file = LIBLISTFILE;

#ifndef O_BINARY
#define O_BINARY 0
#endif

#define MAX_DLB_FILES 200	/* max # of files we'll handle */
#define DLB_VERS 1		/* version of dlb file we will write */

/*
 * How the file is encoded within the library.  Don't use a space
 * because (at least) the  SunOS 4.1.3 C library will eat the white
 * space instead of preserving it like the man page says it should.
 */
#define ENC_NORMAL 'n'	/* normal: not compressed in any way */


/*
 * If you know tar, you have a small clue how to use this (note: - does
 * NOT mean stdin/stdout).
 *
 * dlb COMMANDoptions arg... files...
 * commands:
 *  dlb x	extract all files
 *  dlb c	build the archive
 *  dlb t	list the archive
 * options:
 *  v		verbose
 *  f file	specify archive file (default DLBFILE)
 *  I file	specify file for list of files (default LIBLISTFILE)
 *  C dir	chdir to dir (used ONCE, not like tar's -C)
 */

static void
usage()
{
    (void) printf("Usage: %s [ctxCIfv] arguments... [files...]\n", progname);
    (void) printf("  default library is %s\n", library_file);
    (void) printf("  default list file is %s\n", list_file);
    xexit(EXIT_FAILURE);
}

static void
verbose_help()
{
    static const char *long_help[] = {
	"",
	"dlb COMMANDoptions args... files...",
	"  commands:",
	"    dlb ?   print this text",
	"    dlb h   ditto",
	"    dlb x   extract all files",
	"    dlb c   create the archive",
	"    dlb t   list table of contents",
	"  options:",
	"    v       verbose operation",
	"    f file  specify archive file name",
	"    I file  specify file for list of file names",
	"    C dir   change directory before processing any files",
	"",
	(char *)0
    };
    const char **str;

    for (str = long_help; *str; str++)
	(void) printf("%s\n", *str);
    usage();
}

static void
Write(out,buf,len)
    int out;
    char *buf;
    long len;
{
    if (write(out,buf,len) != len) {
	printf("Write Error in '%s'\n",library_file);
	xexit(EXIT_FAILURE);
    }
}


char *
eos(s)
    char *s;
{
    while (*s) s++;
    return s;
}


#ifdef VMS	/* essential to have punctuation, to avoid logical names */
static FILE *
vms_fopen(filename, mode)
const char *filename, *mode;
{
    char tmp[BUFSIZ];

    if (!index(filename, '.') && !index(filename, ';'))
	filename = strcat(strcpy(tmp, filename), ";0");
    return fopen(filename, mode, "mbc=16");
}
#define fopen vms_fopen
#endif	/* VMS */

/* open_library(dlb.c) needs this (which normally comes from src/files.c) */
FILE *
fopen_datafile(filename, mode)
const char *filename, *mode;
{
    return fopen(filename, mode);
}

#endif	/* DLB */

int
main(argc, argv)
    int argc;
    char **argv;
{
#ifdef DLB
    int i, r;
    int ap=2;				/* argument pointer */
    int cp;				/* command pointer */
    int iseen=0, fseen=0, verbose=0;	/* flags */
    char action=' ';
    library lib;

    if (argc > 0 && argv[0] && *argv[0]) progname = argv[0];
#ifdef VMS
    progname = vms_basename(progname);
#endif

    if (argc<2) {
	usage();
	/* doesn't return */
    }

    for(cp=0;argv[1][cp];cp++){
	switch(argv[1][cp]){
	    default:
		usage();	/* doesn't return */
	    case '-':	/* silently ignore */
		break;
	    case '?':
	    case 'h':
		verbose_help();
		break;
	    case 'I':
		if (ap == argc) usage();
		list_file=argv[ap++];
		if(iseen)
		    printf("Warning: multiple I options.  Previous ignored.\n");
		iseen=1;
		break;
	    case 'f':
		if (ap == argc) usage();
		library_file=argv[ap++];
		if(fseen)
		    printf("Warning: multiple f options.  Previous ignored.\n");
		fseen=1;
		break;
	    case 'C':
		if (ap == argc) usage();
		if(chdir(argv[ap++])){
		    printf("Can't chdir to %s\n",argv[--ap]);
		    xexit(EXIT_FAILURE);
		}
		break;
	    case 'v':
		verbose=1;
		break;
	    case 't':
	    case 'c':
	    case 'x':
		if(action != ' '){
		    printf("Only one of t,x,c may be specified.\n");
		    usage();
		}
		action=argv[1][cp];
		break;
	}
    }

    if(argv[ap] && iseen){
	printf("Too many arguments.\n");
	xexit(EXIT_FAILURE);
    }

    switch(action){
    default:
	printf("Internal error - action.\n");
	xexit(EXIT_FAILURE);
	break;
    case 't':			/* list archive */
	if (!open_library(library_file, &lib)) {
	    printf("Can't open dlb file\n");
	    xexit(EXIT_FAILURE);
	}

	for (i = 0; i < lib.nentries; i++) {
	    if (verbose)
		printf("%-14s %6ld %6ld\n",
		    lib.dir[i].fname, lib.dir[i].foffset, lib.dir[i].fsize);
	    else
		printf("%s\n", lib.dir[i].fname);
	}

	if (verbose)
	    printf("Revision:%ld  File count:%ld  String size:%ld\n",
		lib.rev, lib.nentries, lib.strsize);

	close_library(&lib);
	xexit(EXIT_SUCCESS);

    case 'x': {			/* extract archive contents */
	int f, n;
	long remainder, total_read;
	char buf[BUFSIZ];

	if (!open_library(library_file, &lib)) {
	    printf("Can't open dlb file\n");
	    xexit(EXIT_FAILURE);
	}

	for (i = 0; i < lib.nentries; i++) {
	    if (argv[ap]) {
		/* if files are listed, see if current is wanted */
		int c;
		for (c = ap; c < argc; c++)
		    if (!FILENAME_CMP(lib.dir[i].fname, argv[c])) break;
		if (c == argc) continue;	/* skip */
	    } else if (!FILENAME_CMP(lib.dir[i].fname, DLB_DIRECTORY)) {
		/*
		 * Don't extract the directory unless the user
		 * specifically asks for it.
		 *
		 * Perhaps we should never extract the directory???
		 */
		continue;
	    }
	    fseek(lib.fdata, lib.dir[i].foffset, SEEK_SET);

	    f = open(lib.dir[i].fname, O_WRONLY|O_TRUNC|O_BINARY|O_CREAT, 0640);
	    if (f < 0) {
		printf("Can't create '%s'\n", lib.dir[i].fname);
		xexit(EXIT_FAILURE);
	    }

	    /* read chunks from library and write them out */
	    total_read = 0;
	    do {
		remainder = lib.dir[i].fsize - total_read;
		if (remainder > (long) sizeof(buf))
		    r = (int) sizeof(buf);
		else
		    r = remainder;

		n = fread(buf, 1, r, lib.fdata);
		if (n != r) {
		    printf("Read Error in '%s'\n", lib.dir[i].fname);
		    xexit(EXIT_FAILURE);
		}
		if (write(f, buf, n) != n) {
		    printf("Write Error in '%s'\n", lib.dir[i].fname);
		    xexit(EXIT_FAILURE);
		}

		total_read += n;
	    } while (total_read != lib.dir[i].fsize);

	    (void) close(f);

	    if (verbose) printf("x %s\n", lib.dir[i].fname);
	}

	close_library(&lib);
	xexit(EXIT_SUCCESS);
	}

    case 'c':			/* create archive */
	{
	libdir ld[MAX_DLB_FILES];
	char buf[BUFSIZ];
	int fd, out, nfiles = 0;
	long dir_size, slen, flen, fsiz;
	boolean rewrite_directory = FALSE;

	/*
	 * Get names from either/both an argv list and a file
	 * list.  This does not do any duplicate checking
	 */

	/* get file name in argv list */
	if (argv[ap]) {
	    for ( ; ap < argc; ap++, nfiles++) {
		if (nfiles >= MAX_DLB_FILES) {
		    printf("Too many dlb files!  Stopping at %d.\n",
								MAX_DLB_FILES);
		    break;
		}
		ld[nfiles].fname = (char *) alloc(strlen(argv[ap]) + 1);
		Strcpy(ld[nfiles].fname, argv[ap]);
	    }
	}

	if (iseen) {
	    /* want to do a list file */
	    FILE *list = fopen(list_file, "r");
	    if (!list) {
		printf("Can't open %s\n",list_file);
		xexit(EXIT_FAILURE);
	    }

	    /* get file names, one per line */
	    for ( ; fgets(buf, sizeof(buf), list); nfiles++) {
		if (nfiles >= MAX_DLB_FILES) {
		    printf("Too many dlb files!  Stopping at %d.\n",
								MAX_DLB_FILES);
		    break;
		}
		*(eos(buf)-1) = '\0';	/* strip newline */
		ld[nfiles].fname = (char *) alloc(strlen(buf) + 1);
		Strcpy(ld[nfiles].fname, buf);
	    }
	    fclose(list);
	}

	if (nfiles == 0) {
	    printf("No files to archive\n");
	    xexit(EXIT_FAILURE);
	}


	/*
	 * Get file sizes and name string length.  Don't include
	 * the directory information yet.
	 */
	for (i = 0, slen = 0, flen = 0; i < nfiles; i++) {
	    fd = open(ld[i].fname, O_RDONLY|O_BINARY, 0);
	    if (fd < 0) {
		printf("Can't open %s\n", ld[i].fname);
		xexit(EXIT_FAILURE);
	    }
	    ld[i].fsize = lseek(fd, 0, SEEK_END);
	    ld[i].foffset = flen;

	    slen += strlen(ld[i].fname);	/* don't add null (yet) */
	    flen += ld[i].fsize;
	    close(fd);
	}

	/* open output file */
	out = open(library_file, O_RDWR|O_TRUNC|O_BINARY|O_CREAT, FCMASK);
	if (out < 0) {
	    printf("Can't open %s for output\n", library_file);
	    xexit(EXIT_FAILURE);
	}

	/* caculate directory size */
	dir_size = 40			/* header line (see below) */
		   + ((nfiles+1)*11)	/* handling+file offset+SP+newline */
		   + slen+strlen(DLB_DIRECTORY); /* file names */

	/* write directory */
	write_dlb_directory(out, nfiles, ld, slen, dir_size, flen);

	flen = 0L;
	/* write each file */
	for (i = 0; i < nfiles; i++) {
	    fd = open(ld[i].fname, O_RDONLY|O_BINARY, 0);
	    if (fd < 0) {
		printf("Can't open input file '%s'\n", ld[i].fname);
		xexit(EXIT_FAILURE);
	    }
	    if (verbose) printf("%s\n",ld[i].fname);

	    fsiz = 0L;
	    while ((r = read(fd, buf, sizeof buf)) != 0) {
		if (r == -1) {
		    printf("Read Error in '%s'\n", ld[i].fname);
		    xexit(EXIT_FAILURE);
		}
		if (write(out, buf, r) != r) {
		    printf("Write Error in '%s'\n", ld[i].fname);
		    xexit(EXIT_FAILURE);
		}
		fsiz += r;
	    }
	    (void) close(fd);
	    if (fsiz != ld[i].fsize) rewrite_directory = TRUE;
	    /* in case directory rewrite is needed */
	    ld[i].fsize = fsiz;
	    ld[i].foffset = flen;
	    flen += fsiz;
	}

	if (rewrite_directory) {
	    if (verbose) printf("(rewriting dlb directory info)\n");
	    (void) lseek(out, 0, SEEK_SET);	/* rewind */
	    write_dlb_directory(out, nfiles, ld, slen, dir_size, flen);
	}

	for (i = 0; i < nfiles; i++)
	    free((genericptr_t) ld[i].fname),  ld[i].fname = 0;

	(void) close(out);
	xexit(EXIT_SUCCESS);
	}
    }
#endif	/* DLB */

    xexit(EXIT_SUCCESS);
    /*NOTREACHED*/
    return 0;
}

#ifdef DLB

static void
write_dlb_directory(out, nfiles, ld, slen, dir_size, flen)
int out, nfiles;
libdir *ld;
long slen, dir_size, flen;
{
    char buf[BUFSIZ];
    int i;

    sprintf(buf,"%3ld %8ld %8ld %8ld %8ld\n",
	    (long) DLB_VERS,	    /* version of dlb file */
	    (long) nfiles+1,	    /* # of entries (includes directory) */
				    /* string length + room for nulls */
	    (long) slen+strlen(DLB_DIRECTORY)+nfiles+1,
	    (long) dir_size,	    /* start of first file */
	    (long) flen+dir_size);  /* total file size */
    Write(out, buf, strlen(buf));

    /* write each file entry */
#define ENTRY_FORMAT "%c%s %8ld\n"
    sprintf(buf, ENTRY_FORMAT, ENC_NORMAL, DLB_DIRECTORY, (long) 0);
    Write(out, buf, strlen(buf));
    for (i = 0; i < nfiles; i++) {
	sprintf(buf, ENTRY_FORMAT,
		ENC_NORMAL,		    /* encoding */
		ld[i].fname,		    /* name */
		ld[i].foffset + dir_size);  /* offset */
	Write(out, buf, strlen(buf));
    }
}

#endif	/* DLB */

static void
xexit(retcd)
    int retcd;
{
    exit(retcd);
}

/*dlb_main.c*/
