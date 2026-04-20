/*******************************************************************************
 * intro.h — Public interface for the ESI intro animation library
 *
 * This file is the HEADER of the intro module. In C, a header file (.h) is
 * used to declare what a module offers to the outside world, without revealing
 * how it works internally. Any file that wants to call animation() must include
 * this header.
 *
 * Usage:
 *     #include "intro.h"
 *     ...
 *     animation();   // runs the full intro sequence
 ******************************************************************************/

/*
 * INCLUDE GUARD — prevents this header from being processed more than once
 * during a single compilation. If two files both include intro.h, without
 * this guard the compiler would see the declarations twice and throw an error.
 *
 * How it works:
 *   - First time this file is included: INTRO_H is not defined yet, so the
 *     compiler enters the block and defines INTRO_H.
 *   - Any subsequent include: INTRO_H is already defined, so the entire block
 *     is skipped. Nothing is declared twice.
 */
#ifndef INTRO_H   /* "if INTRO_H is NOT defined, enter this block" */
#define INTRO_H   /* define INTRO_H so future includes are ignored  */


/*
 * Function declaration (also called a "prototype") for animation().
 *
 * This line tells the compiler:
 *   - animation is a function
 *   - it takes no arguments (void)
 *   - it returns nothing (void)
 *
 * The actual code of animation() lives in intro.c. This declaration is just
 * a promise to the compiler that the function exists somewhere and will be
 * linked in at build time.
 *
 * Any .c file that includes intro.h can call animation() safely because the
 * compiler already knows its signature from this declaration.
 */
void animation(void);


#endif /* INTRO_H — closes the include guard opened by #ifndef above */
