/* metermon.c */

#include <common.h>

#define SLEEP (500000)

char *cmdsink = "./IPC/SDR-1000-0-commands.fifo",
  *mtrsrc = "./IPC/SDR-1000-0-meter.fifo";

FILE *cmdfp, *mtrfp;

int label;
REAL rxm[MAXRX][RXMETERPTS];
REAL txm[TXMETERPTS];

int
main (int argc, char **argv)
{
  int i = 0, j, k, lab = getpid ();

  if (!(cmdfp = fopen (cmdsink, "r+")))
    perror (cmdsink), exit (1);
  if (!(mtrfp = fopen (mtrsrc, "r+")))
    perror (mtrsrc), exit (1);

  fprintf (stderr, "metermon OK\n");

  for (;;)
    {

      usleep (SLEEP);

      fprintf (cmdfp, "reqMeter %d\n", lab);
      fflush (cmdfp);

      if (fread ((char *) &label, sizeof (int), 1, mtrfp) != 1)
	perror ("fread meter label"), exit (1);

      if (fread ((char *) rxm, sizeof (REAL), MAXRX * RXMETERPTS, mtrfp)
	  != MAXRX * RXMETERPTS)
	perror ("fread meter"), exit (1);

      printf ("%d <%d>", i++, label);
      for (j = 0; j < MAXRX; j++)
	{
	  for (k = 0; k < RXMETERPTS; k++)
	    printf (" %8.3f", rxm[j][k]);
	  putchar ('\n');
	}

      if (fread ((char *) txm, sizeof (REAL), TXMETERPTS, mtrfp)
	  != TXMETERPTS)
	perror ("fread meter"), exit (1);

      printf ("%d\n", i++);
      for (k = 0; k < TXMETERPTS; k++)
	printf (" %8.3f", txm[k]);
      putchar ('\n');
    }

  fclose (cmdfp);
  fclose (mtrfp);

  exit (0);
}
