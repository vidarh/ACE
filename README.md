
ACE
===

Originally written by David Benn:

 * http://www.users.on.net/~dbenn/index.html
 * http://dbenn.wordpress.com/

This version maintained by Vidar Hokstad.

License: GNU General Public License

ACE is a recursive descent peephole optimizing compiler for a large subset of AmigaBASIC
plus a number of extensions.

It generates M68k assembler suitable for use with A68k.

Please read README_FIRST for details on the original source release.

## Changes since the last release ##

This version is based on the Aminet source code release.

The full set of changesets from the Aminet release is in Git.

The changes are mainly comprised of:

 * Making ACE compile and run on Linux (but still only generating M68k output)
 * Various minor cleanups for newer compilers

## Future ##

I won't have time to make a lot of updates but I want to do a few things:

 * Test and ensure it works properly on AmigaOS 3.x
 * Port it to AROS on x86 w/both m68k and x86 backends.
 * See if I can find someone to port it to AmigaOS 4.x and MorphOS (both PPC)
 * Rewrite the asm parts of the runtime library
 * **Maybe** do a Linux port of the runtime.
 



