#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "D.h"

/*-------------------------------------------------------------
  Parallel curve generator
  -------------------------------------------------------------
  Given a polyline defined by arrays (x[i], y[i]), this routine
  builds another polyline offset to the right by a distance h.

  The algorithm proceeds segment by segment:
  - For each corner, it constructs the outward normal bisector.
  - It generates the offset points A, B, etc.
  - If two offset segments intersect, the intersection is skipped.
  - Otherwise, the routine may add one or two points depending
    on geometric criteria (length limits lmin, lmax).

  Output points (x0, y0) are stored sequentially up to nmax entries.
  If overflow occurs, the function returns 0.
-------------------------------------------------------------*/

typedef struct { double x, y; } Point;

/*-------------------------------------------------------------
  Helper: Euclidean distance between two points
-------------------------------------------------------------*/
static double distance(Point a, Point b) {
    double dx = b.x - a.x, dy = b.y - a.y;
    return sqrt(dx*dx + dy*dy);
}

/*-------------------------------------------------------------
  Helper: Check if two line segments (p1,p2) and (q1,q2) intersect
-------------------------------------------------------------*/
static int intersect(Point p1, Point p2, Point q1, Point q2) {
    double d = (p2.x - p1.x) * (q2.y - q1.y) - (p2.y - p1.y) * (q2.x - q1.x);
    if (fabs(d) < 1e-12) return 0;  // Parallel or coincident

    double t = ((q1.x - p1.x) * (q2.y - q1.y) - (q1.y - p1.y) * (q2.x - q1.x)) / d;
    double u = ((q1.x - p1.x) * (p2.y - p1.y) - (q1.y - p1.y) * (p2.x - p1.x)) / d;
    return (t >= 0 && t <= 1 && u >= 0 && u <= 1);
}

/*-------------------------------------------------------------
  Main routine: build a right-hand offset (parallel) curve
-------------------------------------------------------------*/
int build_parallel_curve(
    double *x, double *y, int n,     // Input polyline
    double h,                        // Offset distance
    double lmin, double lmax,        // Length thresholds
    double *x0, double *y0, int nmax // Output buffer and limit
) {
    if (n < 2) return 0;

    int m = 0;
    Point A, B;

    /* ---- Initial offset point ---- */
    double dx = x[1] - x[0], dy = y[1] - y[0];
    double len = sqrt(dx*dx + dy*dy);
    A.x = x[0] + h * (-dy / len);
    A.y = y[0] + h * ( dx / len);

    if (m < nmax) { x0[m] = A.x; y0[m] = A.y; m++; } else return 0;

    /* ---- Process each segment ---- */
    for (int i = 0; i < n - 1; i++) {

        Point P1 = {x[i], y[i]};
        Point P2 = {x[i+1], y[i+1]};

        /* Compute the outward normal at vertex (i+1) */
        double nx, ny;
        if (i < n - 2) {
            double dx1 = x[i+1] - x[i],     dy1 = y[i+1] - y[i];
            double dx2 = x[i+2] - x[i+1],   dy2 = y[i+2] - y[i+1];
            double l1 = sqrt(dx1*dx1 + dy1*dy1);
            double l2 = sqrt(dx2*dx2 + dy2*dy2);
            dx1 /= l1; dy1 /= l1;
            dx2 /= l2; dy2 /= l2;
            nx = -(dy1 + dy2);
            ny =  (dx1 + dx2);
        } else {
            nx = -(y[i+1] - y[i]);
            ny =  (x[i+1] - x[i]);
        }

        double nlen = sqrt(nx*nx + ny*ny);
        B.x = x[i+1] + h * nx / nlen;
        B.y = y[i+1] + h * ny / nlen;

        /* Check intersection between (P_i,B) and (P_{i+1},A) */
        int cross = intersect(P1, B, P2, A);

        if (!cross) {
            /* Segment lengths for geometric filtering */
            double l1 = (i < n - 2) ? distance(P1, (Point){x[i+2], y[i+2]}) : 1e9;
            double l2 = distance(A, B);

            if (l2 < lmin && l2 < l1) {
                /* Skip point B — too short to be significant */
                continue;
            }

            if (l2 > lmax) {
                /* Insert an intermediate point C midway between A and B,
                   adjusted so it lies at distance h from the base segment. */
                Point M = {(A.x + B.x) / 2.0, (A.y + B.y) / 2.0};
                double dxs = P2.x - P1.x, dys = P2.y - P1.y;
                double slen = sqrt(dxs*dxs + dys*dys);
                Point C = {M.x + h * (-dys / slen), M.y + h * (dxs / slen)};

                if (m < nmax) { x0[m] = C.x; y0[m] = C.y; m++; } else return 0;
            }

            /* Append point B and continue */
            if (m < nmax) { x0[m] = B.x; y0[m] = B.y; m++; } else return 0;
            A = B;

        } else {
            /* Intersection — update A but do not store B */
            A = B;
        }
    }

    return m;
}

/*-------------------------------------------------------------
  Simplified wrapper for parallel curve generation
  All parameters are pointers to type D structures
  Returns: D structure containing a list with the parallel curves x0,y0
-------------------------------------------------------------*/
D *_parallel5(D *x, D *y, D *h, D *lmin, D *lmax) {
    // Check if there were errors in upper levels
    if (DError) {
        // Free all arguments
        DLibera(x);
        DLibera(y);
        DLibera(h);
        DLibera(lmin);
        DLibera(lmax);
        return DCreaNulo();
    }

    // Check that all arguments are of type double
    if (x->t != D_TIPO_DOUBLE || y->t != D_TIPO_DOUBLE || 
        h->t != D_TIPO_DOUBLE || lmin->t != D_TIPO_DOUBLE || 
        lmax->t != D_TIPO_DOUBLE) {
        DError("parallel : bad argument type");
        // Free all arguments
        DLibera(x);
        DLibera(y);
        DLibera(h);
        DLibera(lmin);
        DLibera(lmax);
        return DCreaNulo();
    }

    // Check sizes of arguments
    if (x->n != y->n || h->n != 1 || lmin->n != 1 || lmax->n != 1) {
        DError("parallel : bad argument size");
        // Free all arguments
        DLibera(x);
        DLibera(y);
        DLibera(h);
        DLibera(lmin);
        DLibera(lmax);
        return DCreaNulo();
    }

    const int nmax = 1000;  // Maximum number of output points
    int n = x->n;          // Get number of points from x structure
    
    // Get pointers to the data arrays from the input structures
    double *xd = x->p.d;
    double *yd = y->p.d;
    
    // Allocate temporary arrays for build_parallel_curve output
    double *x0d = malloc(nmax * sizeof(double));
    double *y0d = malloc(nmax * sizeof(double));
    
    // Get the scalar values from the D structures
    double hd = h->p.d[0];
    double lmin_val = lmin->p.d[0];
    double lmax_val = lmax->p.d[0];
    
    // Call the original function with the data arrays and parameter values
    int result = build_parallel_curve(xd, yd, n, hd, lmin_val, lmax_val, x0d, y0d, nmax);

    // Create D structures for the output using the result size
    D *x0 = NULL;
    D *y0 = NULL;
    D *output = NULL;

    if (result > 0) {
        // Create D structures with the exact size needed
        x0 = DCreaDouble(result);
        y0 = DCreaDouble(result);

        // Copy the data to the D structures
        for(int i = 0; i < result; i++) {
            x0->p.d[i] = x0d[i];
            y0->p.d[i] = y0d[i];
        }

        // Create the output list and insert the structures
        output = DCreaLista();
        DInserta(output, x0);
        DInserta(output, y0);
    } else {
        // Create a null structure if no result
        output = DCreaNulo();
    }

    // Free temporary arrays
    free(x0d);
    free(y0d);
    
    // Free all input arguments
    DLibera(x);
    DLibera(y);
    DLibera(h);
    DLibera(lmin);
    DLibera(lmax);
    
    return output;
}
