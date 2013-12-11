/*********************************************************************************
Locale.h stores file names, directory names, and related constants for the local computer.
If the program is to be used on another computer, only entires in this file need
to be changed.
*********************************************************************************/
  
#ifndef LOCALE_H
#define LOCALE_H


/*********************************************************************************
These are the defines for files and directories used in Dlg.[h,cpp]

There are several Modula-2 source files: FullExpr.mod TestExpr.mod sieve.mod
*********************************************************************************/

#define MOD_SRC  "bubl.mod"
#define MOD_DIR  "c:\\Compiler\\wannpiler\\Compiled Programs\\bubl"
#define TOK_DUMP "Tokens.txt"
#define SYM_DUMP "Symbols.txt"
#define SYM_DRIVER_DUMP "Driver.txt"
#define RES_DUMP "Keywords.txt"


/*********************************************************************************
These are the defines for Parse.[h,cpp]

This is the name of the "main" procedure
*********************************************************************************/

#define MAIN_PROC " BKW_main"


/*********************************************************************************
These are the defines for files and directories used in Emitter.[h,cpp]
The first is the location of io.mac and io.obj. The second in the bin file
   for the assebler and linker (MASM, ML, NMAKE, NMAKER, and LINK)
*********************************************************************************/

#define IO_MAC_PATH   "C:\\programs\\MASM611\\BIN\\"
#define MASM_BIN_PATH "c:\\programs\\masm611\\bin\\"

#endif /* LOCALE_H */



