@echo off
rem
rem Microsoft Visual Studio C/C++ 2019+
rem
if not defined GNUWIN32 (
        set GNUWIN32=\devl\gnuwin32
)
if not defined PERL (
        set PERL=perl
)
%PERL% makelib.pl --gnuwin32=%GNUWIN32% --busybox=./support/busybox --bison=d:\Cygwin\bin\bison vc2019 %1 %2 %3 %4

