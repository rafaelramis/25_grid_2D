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

        // If both areas are negative, return 0 (invalid configuration)
        if (SABC < 0.0 && SABD < 0.0) {
            return 0;
        }

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
/*
 * Wrapper for r94: _fill_between4
 *
 * Inputs (D*):
 *   ia, ib : integer arrays containing indices of the first and second point sets
 *   x, y   : double arrays containing coordinates of all points
 *
 * Output (D*):
 *   tri    : integer array containing generated triangles (3 indices per triangle)
 *            Returns DCreaNulo() on error
 *
 * Checks performed:
 *   1. DRun flag: if false, return DCreaNulo() silently
 *   2. Argument types: ia, ib must be integers; x, y must be doubles
 *   3. Argument sizes: x and y must have the same length
 *   4. Memory management: all input D* are freed before returning
 */
D *_fill_between4(D *ia, D *ib, D *x, D *y) {
    // Check if previous error occurred (DRun is false)
    if(!DRun) {
        // Free all input arguments
        DLibera(ia);
        DLibera(ib);
        DLibera(x);
        DLibera(y);
        // Return a null D object silently
        return DCreaNulo();
    }

    // Check argument types
    if(ia->t != D_TIPO_INT || ib->t != D_TIPO_INT || x->t != D_TIPO_DOUBLE || y->t != D_TIPO_DOUBLE) {
        DError("fill_between : bad argument type");
        // Free all input arguments before returning
        DLibera(ia);
        DLibera(ib);
        DLibera(x);
        DLibera(y);
        return DCreaNulo();
    }

    // Check that x and y have the same number of elements
    if(x->n != y->n) {
        DError("fill_between : bad argument size");
        // Free all input arguments before returning
        DLibera(ia);
        DLibera(ib);
        DLibera(x);
        DLibera(y);
        return DCreaNulo();
    }

    int na = ia->n;  // number of indices in first set
    int nb = ib->n;  // number of indices in second set

    // Allocate output D* array to hold triangle indices
    // Maximum possible size: 3*(na + nb + 2)
    D *tri = DCreaInt(3*(na+nb+2));  

    // Call the original fill_between routine
    int ntri = fill_between(
        x->p.d,       // x coordinates
        y->p.d,       // y coordinates
        ia->p.i, na,  // first index set
        ib->p.i, nb,  // second index set
        tri->p.i      // output triangles
    );

    // Free all input arguments
    DLibera(ia);
    DLibera(ib);
    DLibera(x);
    DLibera(y);

    // Check if the routine generated triangles
    if(ntri <= 0) {
        DLibera(tri);       // free output memory
        return DCreaNulo(); // return null on failure
    }

    // Success: return the D* containing triangle indices
    return tri;
}



