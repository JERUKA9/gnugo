/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * This is GNU GO, a Go program. Contact gnugo@gnu.org, or see   *
 * http://www.gnu.org/software/gnugo/ for more information.      *
 *                                                               *
 * Copyright 1999, 2000, 2001 by the Free Software Foundation.   *
 *                                                               *
 * This program is free software; you can redistribute it and/or *
 * modify it under the terms of the GNU General Public License   *
 * as published by the Free Software Foundation - version 2.     *
 *                                                               *
 * This program is distributed in the hope that it will be       *
 * useful, but WITHOUT ANY WARRANTY; without even the implied    *
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR       *
 * PURPOSE.  See the GNU General Public License in file COPYING  *
 * for more details.                                             *
 *                                                               *
 * You should have received a copy of the GNU General Public     *
 * License along with this program; if not, write to the Free    *
 * Software Foundation, Inc., 59 Temple Place - Suite 330,       *
 * Boston, MA 02111, USA.                                        *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* Compile the eye database. This produces eyes.c. */

/* see also eyes.db, eyes.h and engine/optics.c */


#define MAX_BOARD 19
#define MAXLINE 80
#define MAXDIMEN 20
#define MAXSIZE 20
#define MAXPATNO 300

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>


#define DEBUG(x)  /* printf x */

int fatal_errors = 0;


int
main(void)
{
  char line[MAXLINE];
  int patno = 0;
  int p;
  char vertex[MAXDIMEN][MAXDIMEN];
  int marginal[MAXDIMEN][MAXDIMEN];
  int edge[MAXDIMEN][MAXDIMEN];
  int neighbors[MAXSIZE];
  int i, k, l, h;
  int m = 0, n = 0;
  int vi[MAXSIZE];
  int vj[MAXSIZE];
  int eye_number[MAXPATNO];
  int esize[MAXPATNO];
  int msize[MAXPATNO];
  int max[MAXPATNO];
  int min[MAXPATNO];
  int ends[MAXPATNO];
  int two_neighbors[MAXPATNO];
  int three_neighbors[MAXPATNO];
  int num_attacks = 0;
  int num_defenses = 0;
  int debug = 0;
  
  printf("\
/* This file is automatically generated by mkeyes. Do not\n\
 * edit it directly. Instead, edit the eye shape database.\n\
 */\n\n\
\
#include <stdio.h> /* for NULL */\n\
#include \"eyes.h\"\n\n");

  memset(ends, 0, sizeof(ends));
  memset(two_neighbors, 0, sizeof(two_neighbors));
  memset(three_neighbors, 0, sizeof(three_neighbors));
  memset(esize, 0, sizeof(esize));

  while (fgets(line, MAXLINE, stdin) && !fatal_errors) {

    if (line[strlen(line)-1] != '\n') {
      fprintf(stderr, "mkeyes: line truncated: %s\n", line);
      return 1;
    }

    /* remove trailing whitespace */
    i = strlen(line)-2;	/* start removing backwards just before newline */
    while(i >= 0 && (line[i] == ' ' || line[i] == '\t' || line[i] == '\r')) {
      line[i]   = '\n';
      line[i+1] = '\0';
      i--;
    }

    /* New pattern. */
    if (sscanf(line, "Pattern %d", &p)) {
      eye_number[patno] = p;
      if (patno > 0 && eye_number[patno] <= eye_number[patno-1]) {
	fprintf(stderr, "mkeyes: Pattern %d out of sequence\n",
		eye_number[patno]);
	return 1;
      }
      if (debug)
	fprintf(stderr, "parsing pattern %d\n", eye_number[patno]);
      memset(vertex, 0, sizeof(vertex));
      memset(marginal, 0, sizeof(marginal));
      memset(edge, 0, sizeof(edge));
      m = 0;
      esize[patno] = 0;
      msize[patno] = 0;
      num_attacks = 0;
      num_defenses = 0;
      continue;
    }
    
    /* Empty line or comment line, skip. */
    if (strncmp("#", line, 1) == 0 || strncmp("\n", line, 1) == 0)
      continue;
    
    if (strncmp(":", line, 1) != 0) {
      /* diagram line. */
      for (n = 0; n < MAXDIMEN && strncmp("\n", line + n, 1); n++) {
	/* space, harmless CR, or corner symbol */
	if (line[n] == ' ' || line[n] == '\r' || line[n] == '+')
	  continue;

	/* vertical edge */
	if (line[n] == '|') {
	  if (n == 0)
	    edge[m][n+1]++;
	  else
	    edge[m][n-1]++;
	  continue;
	}

	/* horizontal edge */
	if (line[n] == '-') {
	  if (m == 0)
	    edge[m+1][n]++;
	  else
	    edge[m-1][n]++;
	  continue;
	}

	/* All other symbols. */
	vi[esize[patno]] = m;
	vj[esize[patno]] = n;
	vertex[m][n] = line[n];
	if (debug)
	  fprintf(stderr, "%c", line[n]);
	switch (line[n]) 
	{
	  case '.':
	    marginal[m][n] = 0;
	    break;
	    
	  case '!':
	    msize[patno]++;
	    marginal[m][n] = 1;
	    break;
	    
	  case '@':
	    msize[patno]++;
	    marginal[m][n] = 1;
	    num_attacks++;
	    num_defenses++;
	    break;
	    
	  case '$':
	    msize[patno]++;
	    marginal[m][n] = 1;
	    break;
	    
	  case '(':
	    msize[patno]++;
	    marginal[m][n] = 1;
	    num_attacks++;
	    break;
	    
	  case ')':
	    msize[patno]++;
	    marginal[m][n] = 1;
	    num_defenses++;
	    break;
	    
	  case 'x':
	    marginal[m][n] = 0;
	    break;
	    
	  case '*':
	    marginal[m][n] = 0;
	    num_attacks++;
	    num_defenses++;
	    break;
	    
	  case '<':
	    marginal[m][n] = 0;
	    num_attacks++;
	    break;

	  case '>':
	    marginal[m][n] = 0;
	    num_defenses++;
	    break;
	    
	  case 'X':
	    marginal[m][n] = 0;
	    break;
	    
	  default:
	    fprintf(stderr, 
		    "mkeyes: invalid character %c in pattern %d\n",
		    line[n], eye_number[patno]);
	    fatal_errors++;
	    break;
	}
	esize[patno]++;
      }
      m++;
      if (debug)
	fprintf(stderr, "\n");
    }
    else {
      /* Colon line. */
      sscanf(line, ":%d,%d", &(max[patno]), &(min[patno]));
      if (debug)
	fprintf(stderr, "max=%d, min=%d\n", max[patno], min[patno]);

      if (max[patno] != min[patno]) {
	if (num_attacks == 0 || num_defenses == 0) {
	  fprintf(stderr,
		  "mkeyes: missing attack or defense point in pattern %d\n",
		  eye_number[patno]);
	  fatal_errors++;
	}
      }
      
      if (max[patno] == min[patno]) {
	if (num_attacks > 0 || num_defenses > 0) {
	  fprintf(stderr,
		  "mkeyes: attack or defense point in settled pattern %d\n",
		  eye_number[patno]);
	  fatal_errors++;
	}
      }
      
      printf("static struct eye_vertex eye%d[] = {\n", eye_number[patno]);
      for (l = 0; l < esize[patno]; l++) {
	
	int ni[4];
	int nj[4];
	int nb[4];
	int mx[MAXDIMEN][MAXDIMEN];
	int count = 0;
	
	memset(mx, -1, sizeof(mx));
	
	neighbors[l] = 0;
	
	for (h = 0; h < 4; h++) {
	  ni[h] = -1;
	  nj[h] = -1;
	  nb[h] = -1;
	}
	
	mx[vi[l]][vj[l]] = 0;
	
	if (vi[l] > 0 && vertex[vi[l]-1][vj[l]]) {
	  ni[neighbors[l]] = vi[l]-1;
	  nj[neighbors[l]] = vj[l];
	  neighbors[l]++;
	  count++;
	  mx[vi[l]-1][vj[l]] = l;
	}
	
	if (vi[l] < MAXDIMEN-1 && vertex[vi[l]+1][vj[l]]) {
	  ni[neighbors[l]] = vi[l]+1;
	  nj[neighbors[l]] = vj[l];
	  neighbors[l]++;
	  count++;
	  mx[vi[l]+1][vj[l]] = l;
	}
	
	if (vj[l] > 0 && vertex[vi[l]][vj[l]-1]) {
	  ni[neighbors[l]] = vi[l];
	  nj[neighbors[l]] = vj[l]-1;
	  neighbors[l]++;
	  mx[vi[l]][vj[l]-1] = l;
	}
	
	if (vi[l] < MAXDIMEN-1 && vertex[vi[l]][vj[l]+1]) {
	  ni[neighbors[l]] = vi[l];
	  nj[neighbors[l]] = vj[l]+1;
	  neighbors[l]++;
	  mx[vi[l]][vj[l]+1] = l;
	}
	
	
	if (neighbors[l] == 1)
	  ends[patno]++;
	else if (neighbors[l] == 2)
	  two_neighbors[patno]++;
	else if (neighbors[l] == 3)
	  three_neighbors[patno]++;
	
	for (h = 0; h < esize[patno]; h++) {
	  
	  for (k = 0; k < 4; k++)
	    if (ni[k] != -1 && vi[h] == ni[k] && vj[h] == nj[k])
	      nb[k] = h;
	}
	
	
	printf("   {%d, %d, \'%c\', %d, %d, %d, %d, %d, %d}",
	       vi[l], vj[l], vertex[vi[l]][vj[l]], 
	       neighbors[l], nb[0], nb[1], nb[2], nb[3],
	       edge[vi[l]][vj[l]]);
	
	if (l < esize[patno]-1)
	  printf(",\n");
	else
	  printf("\n};\n\n");
      }
      
      patno++;
      if (patno >= MAXPATNO) {
	fprintf(stderr,
		"mkeyes: Too many eye patterns. Increase MAXPATNO in mkeyes.c\n");
	fatal_errors++;
      }
    }
  }

  
  printf("\nstruct eye_graph graphs[] = {\n");
  for (l = 0; l < patno; l++) {

    printf("   {eye%d, \"%d\", %d, %d, %d, %d, %d, %d, %d}",
	   eye_number[l], eye_number[l], esize[l], msize[l], ends[l],
	   two_neighbors[l], three_neighbors[l], max[l], min[l]);
    if (l < patno-1)
      printf(",\n");
    else
      printf(",\n{NULL, 0, 0, 0, 0, 0, 0, 0, 0}\n};\n");
  }

  if (fatal_errors) {
    printf("\n\n#error in eye database.  Rebuild.\n\n");
  }

  return fatal_errors;
}

/*
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 2
 * End:
 */
