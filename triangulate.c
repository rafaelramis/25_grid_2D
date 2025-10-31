#include <stdio.h>
#include <math.h>

/* ===========================================================
   Helper functions
   =========================================================== */

// Compute oriented area (twice the triangle area, signed)
// Positive if counterclockwise (CCW)
static double area(double xa, double ya, double xb, double yb, double xc, double yc)
{
    return (xb - xa) * (yc - ya) - (yb - ya) * (xc - xa);
}

// Squared distance between two points (no sqrt needed)
static double dist2(double xa, double ya, double xb, double yb)
{
    double dx = xb - xa, dy = yb - ya;
    return dx * dx + dy * dy;
}

/* ===========================================================
   Main triangulation routine
   =========================================================== */

/*
 * Fill the space between two polygonal curves with triangles.
 * The triangles are oriented CCW (counterclockwise).
 *
 * Inputs:
 *   x, y  : coordinate arrays
 *   ia, na: indices of the first curve
 *   ib, nb: indices of the second curve
 *
 * Output:
 *   tri   : integer array [3*(na+nb-2)] with vertex indices
 *
 * Return:
 *   Number of triangles generated (should be na + nb - 2)
 */
int fill_between(
    double *x, double *y,
    int *ia, int na,
    int *ib, int nb,
    int *tri)
{
    int i = 0, j = 0, nt = 0;
    int sel;
    double SABC, SABD, SADC, SCBD;
    double DBC, DAC;

    while (i < na - 1 && j < nb - 1) {
        int A = ia[i];
        int B = ib[j];
        int C = ia[i+1];
        int D = ib[j+1];

        // Compute oriented areas
        SABC = area(x[A], y[A], x[B], y[B], x[C], y[C]);
        SABD = area(x[A], y[A], x[B], y[B], x[D], y[D]);

        if (SABC < 0.0) {
            sel = 1;   // advance along ib
        } else if (SABD < 0.0) {
            sel = 0;   // advance along ia
        } else {
            // Both triangles OK, check next configurations
            SCBD = area(x[C], y[C], x[B], y[B], x[D], y[D]);
            SADC = area(x[A], y[A], x[D], y[D], x[C], y[C]);

            if (SCBD < 0.0) sel = 0;
            else if (SADC < 0.0) sel = 1;
            else {
                // Compare distances between BC and AD
                DBC = dist2(x[B], y[B], x[C], y[C]);
                DAC = dist2(x[A], y[A], x[D], y[D]);
                sel = (DBC < DAC) ? 0 : 1;
            }
        }

        // Build triangle and advance
        if (sel == 0) { // advance along ia
            tri[3*nt+0] = ia[i];
            tri[3*nt+1] = ib[j];
            tri[3*nt+2] = ia[i+1];
            i++;
        } else {        // advance along ib
            tri[3*nt+0] = ia[i];
            tri[3*nt+1] = ib[j];
            tri[3*nt+2] = ib[j+1];
            j++;
        }
        nt++;
    }

    // Complete the remaining strip
    while (i < na - 1) {
        tri[3*nt+0] = ia[i];
        tri[3*nt+1] = ib[nb-1];
        tri[3*nt+2] = ia[i+1];
        i++;
        nt++;
    }
    while (j < nb - 1) {
        tri[3*nt+0] = ia[na-1];
        tri[3*nt+1] = ib[j];
        tri[3*nt+2] = ib[j+1];
        j++;
        nt++;
    }

    return nt;
}

/* ===========================================================
   Simple test program
   =========================================================== */

int main(void)
{
    // Example: two polygonal curves (simple waves)
    double x[10], y[10];
    int ia[5], ib[5];
    int tri[3*(5+5-2)];
    int na = 5, nb = 5;

    // First curve (bottom)
    for (int i = 0; i < na; i++) {
        ia[i] = i;
        x[i] = (double)i;
        y[i] = sin(0.5 * i);
    }

    // Second curve (top)
    for (int j = 0; j < nb; j++) {
        ib[j] = na + j;
        x[na + j] = (double)j;
        y[na + j] = 2.0 + 0.5 * sin(0.5 * j + 0.3);
    }

    int nt = fill_between(x, y, ia, na, ib, nb, tri);

    printf("Generated %d triangles:\n", nt);
    for (int k = 0; k < nt; k++) {
        printf("  %3d: (%d, %d, %d)\n", k, tri[3*k], tri[3*k+1], tri[3*k+2]);
    }

    printf("\nTotal vertices: %d\n", na + nb);
    for (int k = 0; k < na + nb; k++)
        printf("  %2d: (%.3f, %.3f)\n", k, x[k], y[k]);

    return 0;
}
