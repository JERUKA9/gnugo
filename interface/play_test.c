/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * This is GNU GO, a Go program. Contact gnugo@gnu.org, or see       *
 * http://www.gnu.org/software/gnugo/ for more information.          *
 *                                                                   *
 * Copyright 1999, 2000, 2001, 2002 by the Free Software Foundation. *
 *                                                                   *
 * This program is free software; you can redistribute it and/or     *
 * modify it under the terms of the GNU General Public License as    *
 * published by the Free Software Foundation - version 2             *
 *                                                                   *
 * This program is distributed in the hope that it will be useful,   *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of    *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the     *
 * GNU General Public License in file COPYING for more details.      *
 *                                                                   *
 * You should have received a copy of the GNU General Public         *
 * License along with this program; if not, write to the Free        *
 * Software Foundation, Inc., 59 Temple Place - Suite 330,           *
 * Boston, MA 02111, USA.                                            *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "gnugo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "interface.h"
#include "sgftree.h"
#include "gg_utils.h"
#include "liberty.h"

static void replay_node(SGFNode *node, int color_to_test);


/* --------------------------------------------------------------*/
/* replay a game */
/* --------------------------------------------------------------*/

void
play_replay(SGFNode *sgf_head, int color_to_replay)
{
  Gameinfo  gameinfo;
  int tmpi;
  float tmpf;
  char *tmpc = NULL;

  SGFNode *node;

  /* Get the board size. */
  if (!sgfGetIntProperty(sgf_head, "SZ", &tmpi)) {
    fprintf(stderr, "Couldn't find the size (SZ) attribute!\n");
    exit(EXIT_FAILURE);
  }
  gameinfo_clear(&gameinfo, tmpi, 5.5);
   
  /* Get the number of handicap stones. */
  if (sgfGetIntProperty(sgf_head, "HA", &tmpi)) {
    /* Handicap stones should appear as AW,AB properties in the sgf file. */
    gameinfo.handicap = tmpi;
  }

  /* Get the komi. */
  if (sgfGetFloatProperty(sgf_head, "KM", &tmpf))
    komi = tmpf;

  if (!quiet) {
    if (sgfGetCharProperty(sgf_head, "RU", &tmpc))
      printf("Ruleset:      %s\n", tmpc);
    if (sgfGetCharProperty(sgf_head, "GN", &tmpc))
      printf("Game Name:    %s\n", tmpc);
    if (sgfGetCharProperty(sgf_head, "DT", &tmpc))
      printf("Game Date:    %s\n", tmpc);
    if (sgfGetCharProperty(sgf_head, "GC", &tmpc))
      printf("Game Comment: %s\n", tmpc);
    if (sgfGetCharProperty(sgf_head, "US", &tmpc))
      printf("Game User:    %s\n", tmpc);
    if (sgfGetCharProperty(sgf_head, "PB", &tmpc))
      printf("Black Player: %s\n", tmpc);
    if (sgfGetCharProperty(sgf_head, "PW", &tmpc))
      printf("White Player: %s\n", tmpc);

    gameinfo_print(&gameinfo);
  }

  sgffile_write_gameinfo(&gameinfo, "replay game");         
   
  /*
   * Now actually run through the file.  This is the interesting part.
   * We need to traverse the SGF tree, and every time we encounter a node
   * we need to check what move GNU Go would make, and see if it is OK. 
   */
  node = sgf_head;
  while (node) {
    replay_node(node, color_to_replay);
    node = node->child;
  }

   sgffile_close_file();
}



/*
 * Handle this node.
 */

static void
replay_node(SGFNode *node, int color_to_replay)
{
  SGFProperty *sgf_prop;  /* iterate over properties of the node */
  SGFProperty *move_prop = NULL; /* remember if we see a move property */
  int color; /* color of move to be made at this node. */
  
  int boardsize = gnugo_get_boardsize();
  int m, n; /* Move from file. */
  int i, j; /* Move generated by GNU Go. */

  /* Handle any AB / AW properties, and note presence
   * of move properties.
   */

  for (sgf_prop = node->props; sgf_prop; sgf_prop = sgf_prop->next) {
    switch (sgf_prop->name) {
    case SGFAB:
      /* add black */
      gnugo_add_stone(get_moveX(sgf_prop, boardsize),
		      get_moveY(sgf_prop, boardsize), BLACK);
      sgffile_put_stone(get_moveX(sgf_prop, boardsize),
			get_moveY(sgf_prop, boardsize), BLACK);
      break;
    case SGFAW:
      /* add white */
      gnugo_add_stone(get_moveX(sgf_prop, boardsize),
		      get_moveY(sgf_prop, boardsize), WHITE);
      sgffile_put_stone(get_moveX(sgf_prop, boardsize),
			get_moveY(sgf_prop, boardsize), WHITE);
      break;
    case SGFB:
    case SGFW:
      move_prop = sgf_prop;  /* remember it for later */
      break;
    }
  }

  /* Only generate moves at move nodes. */
  if (!move_prop)
    return;

  m = get_moveX(move_prop, boardsize);
  n = get_moveY(move_prop, boardsize);
  color = (move_prop->name == SGFW) ? WHITE : BLACK;

  if (color == color_to_replay || color_to_replay == GRAY) {
    /* Get a move from the engine for color. */
    gnugo_genmove(&i, &j, color);
    /* Now report on how well the computer generated the move. */
    if (i != m || j != n || !quiet) {
      mprintf("Move %d (%C): ", movenum + 1, color);
    
      mprintf("GNU Go plays %m ", i, j);
      if (!gnugo_is_pass(i, j))
	printf("(%.2f) ", potential_moves[i][j]);
      mprintf("- Game move %m ", m, n);
      if (!gnugo_is_pass(m, n) && potential_moves[m][n] > 0.0)
	printf("(%.2f) ", potential_moves[m][n]);
      printf("\n");
    }
    if (i != m || j != n) {
      char buf[127];
      gg_snprintf(buf, 127, "GNU Go plays %s(%.2f) - Game move %s(%.2f)",
	location_to_string(POS(i,j)),
	gnugo_is_pass(i, j) ? 0 : potential_moves[i][j],
	location_to_string(POS(m,n)),
        gnugo_is_pass(m, n) && potential_moves[m][n] < 0.0 ? 0 : potential_moves[m][n]);
      sgffile_write_comment(buf);
      sgffile_write_circle_mark(i,j);
    }
  }

  /* Finally, do play the move from the file. */
  gnugo_play_move(m, n, color);
  sgffile_move_made(m, n, color, 0);
}


/*
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 2
 * End:
 */
