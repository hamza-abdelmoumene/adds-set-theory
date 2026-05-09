#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

/**
 *  @file user_interface.h
 *  @brief
 *  Contains the complete intro animation sequence.
 *  Blocks until the final "Press any key to exit..." prompt is acknowledged.
 *  Includes the graphical experience for the user such as menus, separators,
 *  and prompts.
 *  No hard logic here.
 *  */

/**
 * @brief Displays the application's introductory animation.
 * Clears the screen and plays a sequence of text and graphical effects.
 */
void RunAnimation(void);

/**
 * @brief Renders and handles interaction for the main application menu.
 * Contains the main interactive loop for the user interface.
 */
void RunMenu(void);
#endif /* USER_INTERFACE_H */