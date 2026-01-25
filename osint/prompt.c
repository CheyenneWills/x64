
/*
Copyright 1987-2012 Robert B. K. Dewar and Mark Emmer.
Copyright 2012-2017 David Shields
*/

#include "port.h"
extern char *source_version;
/*    prompt() - used to give user usage info in command line versions.
 *
 */
void
prompt(void)
{

    wrterr("usage: spitbol [options] files[.sbl or .spx] [args to HOST(2)]");

    wrterr("source files are concatenated, filename '-' is standard "
           "input/output");
    wrterr("# is a decimal number.  Append \"k\" for kilobytes, \"m\" for "
           "megabytes, \"g\" for gigabytes");
    wrterr("spitbol %s", source_version);
    wrterr("options:");
    wrterr("-d# #bytes max heap            -i#[,#] #bytes initial heap size");
    wrterr("  -d* limited by system                [,#bytes increment size]");
    wrterr("-m# #bytes max object size     -s# #bytes stack size");
    wrterr("-c compiler statistics         -x execution statistics");
    wrterr("-a same as -lcx                -l normal listing");
    wrterr("-g# listing page length        -t# listing line width");
    wrterr("-p long listing format         -z standard listing options");
    wrterr("-o=file[.lst]  listing file    -h suppress version ID/date in "
           "listing");
    wrterr("-b suppress signon message     -e errors to list file only");
    wrterr("-k run with compilation error  -n suppress execution");
    wrterr("-F fold source code case (ignore source code case) -f don't fold "
           "source code");
    wrterr("-u \"string\" data passed to HOST(0)");

    wrterr("-y write save (.spx) file");

    wrterr("-r INPUT from source file following END statement");
    wrterr("-T=file  write TERMINAL output to file");
    wrterr("-#=file[options]  associate file with I/O channel #");
    wrterr("");
    wrterr("option defaults: -F -d* -i24m,4m -m4m -s8m");

    exit(0);
}
