# Berkeley Pascal

Berkeley Pascal was developed at the University of California, Berkeley, initially by Kenneth L. Thompson, then by William N. Joy, Susan L. Graham and Charles B. Haley. Berkeley Pascal was designed for interactive instructional use on the PDP-11 and VAX-11 computers by Kenneth L. Thompson, and further developed with extensive modifications and additions by William N. Joy, Charles B. Haley and Susan L. Graham. The interpeter was later rewritten for the VAX-11/780 by Marshall K. McKusick, and then converted into C, using `libpc` by Marshall K. McKusick, later in 1981.

Interpretive P-code is produced, providing fast translation at the expense of slower execution speed. The interpreter runs at a fraction of the speed of equivalent compiled C code, with this fraction varying from 1/5 to 1/15.

This project is an attempt to port Berkeley Pascal to Android and Linux.

This source was forked from [adamyg](https://github.com/adamyg/berkeley_pascal), which targets Windows.

## Source Roadmap:

    pi -          Pascal interpreter.

    px -          Pascal byte-code compiler.

    pdx -         Pascal debugger.

    doc -         Reference and design documents.

    src -         Command source for px, pi and pdx.

    libpc -       Pascal run-time library.

    pmerge -      Pascal file merger.

    libcompat -   Compatibility library, implements omitted functionality assumed to be available.

    tstpx -       Test cases and associated expected results.

    byacc -       Berkeley yacc, is a LALR(1) parser generator.

    eyacc -       Modified yacc allowing much improved error recovery; needed by the Pascal parser.

    ex -          Stream editor.

    contrib/flex - Flex (Fast Lexical analyzer generator) 2.5.2.

    contrib/libregex - Henry Spencer's regex package; required by Flex.

## Source Reference:

    pxref -       Cross-reference generator

    pxp -         Pascal execution profiler.

    pix -         Pascal interpreter and executor.

    pc -          Pascal compiler; requires the Berkeley Portable C Compiler (pcc) infrastructure.

    pcextern -    External symbol stabs generator.

    px_header -   libexec loader.
