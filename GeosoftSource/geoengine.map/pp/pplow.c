//=============================================================================
#include "stdafx.h"
//=============================================================================
//=============================================================================
//
// PPLOW.c
//
// Geosoft high-level graphics class.
//
// Lowlevel parts of the PP object that deal with clipping/vectoring.
//
//=============================================================================

#include <col.h>                // Colours
#include <mp.h>                 // MP Object
#include <pp.h>                 // PP Object
#include <dd.h>                 // DD Object
#include <lpt.h>                // LPT Object
#include "pp.ih"

//-----------------------------------------------------------------------------

void Move_PP(H_PP*    hPP,      // PP handle
             Point_PP pt)       // move to this point
{
	DEBUG_STACK;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- Reflect move in variables ---

	if (hPP->sNoDraw && (!hPP->sDoExtents))
		return;
	hPP->dloc = Trans_PP(hPP, pt);
	hPP->linacc = 0;

}


//-----------------------------------------------------------------------------

void Vect_PP(H_PP*    hPP,      // PP handle
             Point_PP pt)       // vector to this point
{
	DEBUG_STACK;

	Point_DD dpt;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- Generate the Device Location ---

	if (hPP->sNoDraw && (!hPP->sDoExtents))
		return;
	dpt = Trans_PP(hPP, pt);


	// --- Dashed or regular line ? ---

	if (hPP->CurAttrib.Lpat && hPP->pLpt)
		IDashLn_PP(hPP, hPP->dloc, dpt, hPP->pLpt, hPP->CurAttrib.Lptc, 0L);
	else
	{

		// --- Draw a simple line ---

		IVect_PP(hPP, hPP->dloc, dpt);
	}

	hPP->dloc = dpt;

}

//-----------------------------------------------------------------------------

void Circle_PP(H_PP*    hPP,     // PP Object
               long     lRadius, // Radius in microns
               double   start,   // Start segment angle
               double   end,     // End segment angle
               short    close,   // Close the segment (pie segment)
               Point_PP loc)     // Location in microns to place the circle
{
	DEBUG_STACK;

	short     sCheckErr = 0;

	double    sOptSections;
	double    dAng1, dAng2;
	double    dAng, dCos, dSin;
	double    dX, dY, dRadius;
	short     sSections;
	short     sAllin;
	short     i;
	Point_DD* pBuff = NULL;
	Point_DD  lLoc;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?
	if (lRadius <= 0L)
		return;                           // Invalid Circle

	// ---  Radius must be valid ---

	dRadius = (double) lRadius;
	lLoc = Trans_PP(hPP, loc);

	// --- Is this a full circle ? ---

	if (start == end)
	{
		// --- Full circle no closure ---

		CHECK_ERR(sEllipse_DD(hPP->hDD, &lLoc, lRadius, hPP->CurAttrib.Lcol, hPP->CurAttrib.Lthk));

		return;

	}
	else
	{

		// --- Determine number of sections ---

		if (lRadius <= 2268)
			sOptSections = 64;
		else if (lRadius >= 17872)
			sOptSections = 512;
		else
			sOptSections = (short) (DEG_TO_GRAD2 * dRadius / 1000.0);

		// --- Circle Segment ---

		dAng1 = start / DEG_TO_GRAD;
		dAng2 = end / DEG_TO_GRAD;


		// --- Make the angles positive ---

		while ((dAng1 < 0.0) || (dAng2 < 0.0))
		{
			dAng1 += PI2;
			dAng2 += PI2;
		}


		// --- Determine number of sections needed for arc ---

		if (dAng1 > dAng2)
			sSections = (short) ((sOptSections * (PI2 - dAng1 + dAng2) / PI2) + 4);
		else
			sSections = (short) ((sOptSections * (dAng2 - dAng1) / PI2) + 4);
	}


	// --- Allocate Memory on the stack for the sections ---

	pBuff = (Point_DD*) AllocTemp_MP(hPP->hMP, (sSections + 4) * sizeof (Point_DD));
	CHECK_ERR(pBuff == NULL);

	// --- Determine Angle Increments ---

	if (sSections > 4)
	{
		dAng = PI2 / sOptSections;
		dCos = cos(dAng);
		dSin = sin(dAng);
	}


	// --- Generate a Zero Centered Circle ---

	dX = dRadius* cos(dAng1);
	dY = dRadius* sin(dAng1);
	for (i = 1; i < sSections - 2; i++)
	{
		pBuff[i].x = (long) dX;
		pBuff[i].y = (long) dY;
		ICircle_PP(dSin, dCos, &dX, &dY);
	}

	// --- Complete the arc to the last point ---

	pBuff[sSections - 2].x = (long) (dRadius * cos(dAng2));
	pBuff[sSections - 2].y = (long) (dRadius * sin(dAng2));


	// --- Move the circle to the center point ---

	pBuff[0] = lLoc;
	pBuff[sSections - 1] = lLoc;
	sAllin = IVisible_PP(hPP, lLoc);
	for (i = 1; i < sSections - 1; i++)
	{
		pBuff[i].x += lLoc.x;
		pBuff[i].y += lLoc.y;
		sAllin &= IVisible_PP(hPP, pBuff[i]);
	}


	// --- Draw the circle outline ---

	if (close)
		IMVect_PP(hPP, pBuff, sSections, sAllin, 1);
	else
		IMVect_PP(hPP, pBuff + 1, sSections - 2, sAllin, 1);


GS_EXIT:

	// --- Destroy the buffer memory ---

	if (pBuff)
		Free_MP(hPP->hMP, pBuff);

	return;

}



//-----------------------------------------------------------------------------
//                              PRIVATE FUNCTIONS
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

void IVect_PP(H_PP*    hPP,     // PP Object
              Point_DD p1,      // First Point in the line
              Point_DD p2)      // Second Point in the line
{
	DEBUG_STACK;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- Clip the line ---

	if (IClipLn_PP(hPP, &p1, &p2))
	{
		// --- Actually Something to draw ---

		if (hPP->sDoExtents)
		{
			IPExt_PP(hPP, &p1, 1);
			IPExt_PP(hPP, &p2, 1);
		}
		else
		{
			Move_DD(hPP->hDD, p1);
			Vect_DD(hPP->hDD, p2);
		}
	}

}


//-----------------------------------------------------------------------------

long IDashLn_PP(H_PP*         hPP,      // PP Object
                Point_DD      p1,       // First Point
                Point_DD      p2,       // Second Point
                const lpt_ob* lpt,      // Line Patter object to use
                long          lptc,     // Line pitch to use (in microns)
                long          acc)      // Accumulator
{
	DEBUG_STACK;

	Point_DD pp1, pp2;
	short    val;
	long     a;
	double   dx, dy;


	// --- Validate Parameters ---

	ASSERT(hPP && lpt);                  // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?
	ASSERT((lptc > 0) && (acc >= 0));    // Valid Line Pitch/Accumulator


	// --- Clip the line first ---

	pp1 = p1;
	pp2 = p2;
	val = IClipLn_PP(hPP, &pp1, &pp2);

	// --- Is the line visible ? ---

	if (val == 0)
	{
		dx = (double) (pp2.x - pp1.x);
		dy = (double) (pp2.y - pp1.y);
		acc = (long) (fmod(sqrt(dx * dx + dy * dy) + (double) acc, (double) lptc));
		return (acc);             // Drop Out
	}

	// --- Determine how much of the line was clipped ---
	// --- And add it in microns to the accumulator ---

	if (val && ((pp1.x != p1.x) || (pp1.y != p1.y)))
	{
		acc += (long) (sqrt(pow((double) pp1.x - (double) p1.x, 2.0) + pow((double) pp1.y - (double) p1.y, 2.0)));
	}

	// --- Draw the clipped line using dashes ---

	a = IDashSeg_PP(hPP, pp1, pp2, lpt, lptc, acc);


	// --- Determine how much of the line was clipped ---
	// --- And add it in microns to the post accumulator ---

	if (val && ((pp2.x != p2.x) || (pp2.y != p2.y)))
	{
		a += (long) (sqrt(pow((double) pp2.x - (double) p2.x, 2.0) + pow((double) pp2.y - (double) p2.y, 2.0)));
	}

	return (a);

}


//-----------------------------------------------------------------------------

long IDashSeg_PP(H_PP*         hPP,     // PP Object
                 Point_DD      p1,      // First Point
                 Point_DD      p2,      // Second Point
                 const lpt_ob* lpt,     // Line Patter object to use
                 long          lptc,    // Line pitch to use
                 long          acc)     // Accumulator
{
	DEBUG_STACK;

	double   dx, dy;                     // Deltas (change in x,y)
	double   cx, cy;                     // Contributions of x,y compared to length
	double   px, py;                     // Current point's x,y

	double   len;                        // Length of the line
	double   pitch;                      // Pitch of the line patterns
	double   offset;                     // Where do we start
	double   slen;                       // Actual length of a segment we are drawing

	long     rlen;                       // Length to return (accumulated length)
	short    cseg;                       // Current segment we are drawing

	short    done = 0;                   // Are we done drawing ?
	Point_DD pt1, pt2;                   // Current two points for this segment



	// --- Validate Parameters ---

	ASSERT(hPP && lpt);                  // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?
	ASSERT((lptc > 0) && (acc >= 0));    // Valid Line Pitch/Accumulator


	// --- Preliminary Computations ---

	pitch = (double) lptc;       // Pitch
	dx = (double) (p2.x - p1.x); // Delta in X
	dy = (double) (p2.y - p1.y); // Delta in Y


	// --- Check for a zero length line ---

	if ((dx == 0.0) && (dy == 0.0))
		return (acc);


	// --- Other Calculations ---

	len = sqrt(dx * dx + dy * dy);       // Length of the line
	cx = dx / len;                       // Contributions in X
	cy = dy / len;                       // Contributions in Y
	px = (double) p1.x;                  // Starting X location
	py = (double) p1.y;                  // Starting Y location


	// --- Compute where we are begining relative the accumulator ---

	offset = fmod((double) acc, pitch) / pitch;
	rlen = (long) (fmod(len + (double) acc, pitch));


	// --- Cycle the segments to the proper first segment ---

	cseg = 0;
	while ((fabs(lpt->pdItems[cseg]) < offset) && (cseg < lpt->lItems))
	{
		offset -= fabs(lpt->pdItems[cseg]);
		cseg++;
	}


	// --- Determine the length of the first segment ---

	slen = (fabs(lpt->pdItems[cseg]) * pitch) - (offset * pitch);
	pt1 = p1;


	// --- Begin looping, drawing each segment as we go ---

	for (;;)
	{

		// --- Compute This Segment's information ---

		if (len < slen)
		{
			slen = len;
			done = 1;
		}

		len -= slen;              // Decrease the length of the line to draw
		px += cx * slen;          // Compute the next point's X location
		py += cy * slen;          // Compute the next point's Y location
		pt2.x = (long) px;        // The next point's X
		pt2.y = (long) py;        // The next point's Y


		// --- If this is a visible segment then draw it ---

		if (lpt->pdItems[cseg] >= 0.0)
		{
			// --- Draw the Segment ---

			if (hPP->sDoExtents)
			{
				IPExt_PP(hPP, &pt1, 1);
				IPExt_PP(hPP, &pt2, 1);
			}
			else
			{
				Move_DD(hPP->hDD, pt1);
				Vect_DD(hPP->hDD, pt2);
			}
		}
		if (done)
			return (rlen);


		// --- Remember the last point ---

		pt1 = pt2;


		// --- Cycle to the next segment ---

		cseg++;
		if (cseg == lpt->lItems)
			cseg = 0;
		slen = fabs(lpt->pdItems[cseg]) * pitch;
	}

}


//-----------------------------------------------------------------------------

short ISide_PP(Point_DD* pt,            // Point to see where
               Point_DD  clipll,        // Lower Left Point
               Point_DD  clipur)        // Upper Right Point
{
	DEBUG_STACK;

	short place = 0;


	// --- Validate Parameters ---

	ASSERT((clipll.x <= clipur.x) && (clipll.y <= clipur.y));    // Valid Clip Region


	// --- Where is the point found ? ---

	if (pt->x < clipll.x)
		place |= 1;               // Left Side
	if (pt->x > clipur.x)
		place |= 2;               // Right Side
	if (pt->y < clipll.y)
		place |= 4;               // Top Side
	if (pt->y > clipur.y)
		place |= 8;               // Bottom Side

	return (place);

}


//-----------------------------------------------------------------------------

short IClipLn_PP(H_PP*     hPP,         // PP Object
                 Point_DD* p1,          // First Point
                 Point_DD* p2)          // Second Point
{
	DEBUG_STACK;

	short    c1, c2, c, cs;
	double   dx, dy;
	Point_DD pt;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?


	// --- No Clip in Extents Mode ---

	if (hPP->sDoExtents)
		return (1);               // Fully Visible


	// --- Try to locate the points ---

	c1 = ISide_PP(p1, hPP->clipll, hPP->clipur);
	c2 = ISide_PP(p2, hPP->clipll, hPP->clipur);


	// --- Loop to try to bring the point into clip region ---

	while (c1 || c2)
	{
		if (c1 & c2)
			return (0);            // Are both points away ?

		// --- Setup variables ---

		if (c1)
		{
			c = c1;
			cs = 0;                // Working with first point
		}
		else
		{
			c = c2;
			cs = 1;                // Working with second point
		}
		dx = (double) (p2->x - p1->x);
		dy = (double) (p2->y - p1->y);
		if (dx == 0.0)
			dx = 0.000000001;


		// --- Clip ---

		if (c & 1)                // Left Side
		{
			pt.y = p1->y + (long) (dy * (double) (hPP->clipll.x - p1->x) / dx);
			pt.x = hPP->clipll.x;

		}
		else if (c & 2)
		{
			pt.y = p1->y + (long) (dy * (double) (hPP->clipur.x - p1->x) / dx);
			pt.x = hPP->clipur.x;

		}
		else if (c & 4)
		{
			pt.x = p1->x + (long) (dx * (double) (hPP->clipll.y - p1->y) / dy);
			pt.y = hPP->clipll.y;

		}
		else if (c & 8)
		{
			pt.x = p1->x + (long) (dx * (double) (hPP->clipur.y - p1->y) / dy);
			pt.y = hPP->clipur.y;
		}

		// --- Update the clipped point ---

		if (cs == 0)              // Which point were we working with ?
		{
			*p1 = pt;
			c1 = ISide_PP(p1, hPP->clipll, hPP->clipur);
		}
		else
		{
			*p2 = pt;
			c2 = ISide_PP(p2, hPP->clipll, hPP->clipur);
		}
	}

	// --- Done ---

	return (1);

}




//-----------------------------------------------------------------------------

long IClipPoly_PP(H_PP*     hPP,        // PP Object
                  long      npts,       // Number of points in the polygon
                  Point_DD* pts,        // The actual points
                  Point_DD* rpts)       // Resulting Points
{
	DEBUG_STACK;

	long n1, n2;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?
	ASSERT((npts > 0) && pts && rpts);   // Valid buffers and points


	// --- No Clip in Extents Mode ---

	if (hPP->sDoExtents)
	{
		// --- Copy the Points across ---

		memcpy(rpts, pts, sizeof (Point_DD) * npts);
		return (npts);            // All the points are visible

	}


	// --- Setup number of points ---

	n1 = npts;


	// --- Clip the sides of the polygon ---

	n2 = 0;
	IClipEdge_PP(n1, pts, &n2, rpts, hPP->clipll.x, 0 | 2);
	if (n2 == 0)
		return (0);

	n1 = 0;
	IClipEdge_PP(n2, rpts, &n1, pts, hPP->clipur.x, 0 | 0);
	if (n1 == 0)
		return (0);

	n2 = 0;
	IClipEdge_PP(n1, pts, &n2, rpts, hPP->clipur.y, 1 | 0);
	if (n2 == 0)
		return (0);

	n1 = 0;
	IClipEdge_PP(n2, rpts, &n1, pts, hPP->clipll.y, 1 | 2);
	if (n1 == 0)
		return (0);

	// --- Return number of visible points ---

	return (n1);

}



//-----------------------------------------------------------------------------

void IClipEdge_PP(long      npts,       // Number of points in polygon
                  Point_DD* pts,        // Actual polygon points
                  long*     nrpts,      // Number of clipped points
                  Point_DD* rpts,       // Actual clipped points
                  long      eval,       // Location of the edge
                  short     einfo)      // Edge corresponds to x or y as well as inside
                                   // 0 for x      1 for y
                                   // 0 for less   1 for greater    (is inside)
{
	DEBUG_STACK;

	long   in1, in2, i;
	double dx, dy, slop;
	long*  pt1, * pt2;
	long   pt[2];


	// --- Validate Parameters ---

	ASSERT(pts && rpts && (npts > 0) && (*nrpts == 0));  // Valid Buffers/Points
	ASSERT((einfo >= 0) && (einfo <= 3));                // Valid Info Directive

	// --- Initialize Variables ---

	*nrpts = 0;
	pt1 = (long*) &(pts[npts - 1]);


	// --- Deal with each point ---

	for (i = 0; i < npts; i++)
	{

		// --- Grab a point ---

		pt2 = (long*) &(pts[i]);


		// --- Determine if first/second point is inside ---

		if (einfo & 2)
		{
			in1 = (pt1[einfo & 1] >= eval);
			in2 = (pt2[einfo & 1] >= eval);
		}
		else
		{
			in1 = (pt1[einfo & 1] <= eval);
			in2 = (pt2[einfo & 1] <= eval);
		}


		// --- Are both points inside ? ---

		if (in1 && in2)
		{
			// --- Output the second point ---

			rpts[(*nrpts)++] = *((Point_DD*) pt2);

		}
		else if (in1 || in2)
		{                         // Any Point Inside

			// --- Determine the slop ---

			dx = (double) (pt2[!(einfo & 1)] - pt1[!(einfo & 1)]);
			dy = (double) (pt2[einfo & 1] - pt1[einfo & 1]);
			if (dy == 0.0)
				dy = 0.0000001;
			slop = dx / dy;


			// --- Determine Intersection ---

			pt[!(einfo & 1)] = (long) (((double) (eval - pt1[einfo & 1]) * slop)) + pt1[!(einfo & 1)];
			pt[einfo & 1] = eval;


			// --- Was the second point inside ? ---

			if (in1)
			{
				// --- First point inside, output the intersection point only ---

				rpts[(*nrpts)++] = *((Point_DD*) pt);

			}
			else if (in2)
			{

				// --- Second point inside, output intersection and second point ---

				rpts[(*nrpts)++] = *((Point_DD*) pt);
				rpts[(*nrpts)++] = *((Point_DD*) pt2);

			}

		}                         // Ignore if both points outside


		// --- Point one to be current point ---

		pt1 = pt2;
	}

}



//-----------------------------------------------------------------------------

void IMVect_PP(H_PP*     hPP,    // PP Handle
               Point_DD* pts,    // Array of points
               long      npts,   // number of points
               short     allin,  // All the points visible ?
               short     uselpt) // Are LPT's allows ?
{
	DEBUG_STACK;

	long i;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?
	ASSERT((npts > 0) && (pts != NULL)); // Valid point table passed in
	if (hPP->sNoDraw && (!hPP->sDoExtents))
		return;


	// --- Are all the points visible ? ---

	if (allin)
	{

		// --- Do dashes or vector ---

		if (uselpt && hPP->CurAttrib.Lpat && hPP->pLpt)
		{
			hPP->linacc = 0L;
			for (i = 0; i < npts - 1; i++)
				hPP->linacc = IDashSeg_PP(hPP, pts[i], pts[i + 1], hPP->pLpt, hPP->CurAttrib.Lptc, hPP->linacc);
		}
		else
		{


			// --- No line pattern so simply vector the points ---

			if (hPP->sDoExtents)
				IPExt_PP(hPP, pts, npts);
			else
			{
				Move_DD(hPP->hDD, pts[0]);
				PVect_DD(hPP->hDD, pts, npts);
			}
		}

	}
	else
	{


		// --- Dashes or Vector ? ---

		if (hPP->pLpt && uselpt && hPP->CurAttrib.Lpat)
		{

			// --- Do full clipping of Dashed lines ---

			hPP->linacc = 0L;
			for (i = 0; i < npts - 1; i++)
				hPP->linacc = IDashLn_PP(hPP, pts[i], pts[i + 1], hPP->pLpt, hPP->CurAttrib.Lptc, hPP->linacc);


		}
		else
		{

			// --- Cliped Vectoring ---

			for (i = 0; i < npts - 1; i++)
				IVect_PP(hPP, pts[i], pts[i + 1]);
		}

	}


}


//-----------------------------------------------------------------------------

void IPVect_PP(H_PP*    hPP,    // PP handle
               CON_DAT* pts,    // chain of points
               long     npts,   // number of points in the chain
               short    type,   // Type of data and conversion to do
               short    cpoint) // Do we include current point ?
// 0 No, 1 Yes
{
	DEBUG_STACK;

	Point_DD* ps;
	Point_PP  dpt;
	long      i;
	short     allin;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?
	ASSERT((npts > 0) && (pts != NULL)); // Valid point table passed in

	// --- Make memory for the points ---

	ps = (Point_DD*) AllocTemp_MP(hPP->hMP, sizeof (Point_DD) * (npts + cpoint));
	if (ps == NULL)
		return;


	// --- Convert the logical points to device coordinates ---

	if (cpoint)
	{
		ps[0] = hPP->dloc;
		allin = IVisible_PP(hPP, ps[0]);
	}
	else
		allin = 1;

	for (i = cpoint; i < npts + cpoint; i++)
	{
		switch (type)
		{
			case CONV_DOUBLE:     // --- Standard Doubles ---

				ps[i] = Trans_PP(hPP, ((Point_PP*) pts)[i - cpoint]);
				break;

			case CONV_FLOAT:      // --- Floats ---

				dpt.x = (double) ((float*) pts)[(i - cpoint) * 2];
				dpt.y = (double) ((float*) pts)[((i - cpoint) * 2) + 1];
				ps[i] = Trans_PP(hPP, dpt);
				break;

			case CONV_SHORT:      // --- Origined Shorts in .05 centimeters ---

				ps[i].x = ((long) ((short*) pts)[(i - cpoint) * 2] * 50L) - hPP->PMFOrigin;
				ps[i].y = ((long) ((short*) pts)[((i - cpoint) * 2) + 1] * 50L) - hPP->PMFOrigin;
				break;
		}
		allin &= IVisible_PP(hPP, ps[i]);
	}


	// --- Draw the vectors ---

	IMVect_PP(hPP, ps, npts + cpoint, allin, 1);


	// --- Free memory and Remeber where we are ---

	hPP->dloc = ps[npts + cpoint - 1];
	Free_MP(hPP->hMP, ps);
}



//-----------------------------------------------------------------------------

void IFill_PP(H_PP*     hPP,    // PP Object
              Point_DD* pts,    // Point buffer
              long      npts,   // Number of points
              short     allin)  // Are all the points visible ?
{
	DEBUG_STACK;

	Point_DD* pr, * pi;
	long      npt;
	long      sBuff;


	// --- Are all the points inside ? ---

	if (allin)
	{
		// --- No clip fill ---

		if (hPP->sDoExtents)
			IPExt_PP(hPP, pts, npts);
		else
		{
			Color_DD(hPP->hDD, hPP->CurAttrib.Fcol);
			Move_DD(hPP->hDD, pts[0]);
			Fill_DD(hPP->hDD, pts, npts);
			Color_DD(hPP->hDD, hPP->CurAttrib.Lcol);
		}

	}
	else
	{

		// --- Allocate Memory for second buffer ---

		pi = (Point_DD*) AllocTemp_MP(hPP->hMP, sizeof (Point_DD) * (2 * npts) * 2);
		if (pi != NULL)
		{
			sBuff = 1;
			pr = pi + (2 * npts);
			memcpy(pi, pts, sizeof (Point_DD) * npts);


			// --- Region is partial in the viewport, do proper clipping ---

			npt = IClipPoly_PP(hPP, npts, pi, pr);
			ASSERT(npt >= 0);      // Must be a valid number

		}
		else
		{

			sBuff = 0;
			npt = npts;
			pi = pts;
		}

		// --- Do the Fill ? ---

		if (npt)
		{
			// --- Fill the region of points ---

			if (hPP->sDoExtents)
				IPExt_PP(hPP, pi, npt);
			{
				Color_DD(hPP->hDD, hPP->CurAttrib.Fcol);
				Move_DD(hPP->hDD, pi[0]);
				Fill_DD(hPP->hDD, pi, npt);
				Color_DD(hPP->hDD, hPP->CurAttrib.Lcol);
			}
		}

		// --- Free the temporary buffer ---

		if (sBuff)
			Free_MP(hPP->hMP, pi);
	}

}


//-----------------------------------------------------------------------------

void VFill_PP(H_PP*    hPP,     // PP handle
              CON_DAT* pts,     // chain of points bounding area to be filled
              long     npts,    // number of points in the polyline
              short    type,    // Type of data and conversion to do
              short    border)  // Draw a border ?
{
	DEBUG_STACK;

	Point_DD* ps;
	long      i;
	short     allin;
	Point_PP  dpt;


	// --- Validate Parameters ---

	ASSERT(hPP);                         // Object passed in ?
	ASSERT(hPP->obj_id == PMF_PP_ID);    // Was it really a PP object ?

	// --- Make memory for the points ---

	ps = (Point_DD*) AllocTemp_MP(hPP->hMP, sizeof (Point_DD) * (2 * npts));
	if (ps == NULL)
		return;


	// --- Convert the logical points to device coordinates ---

	allin = 1;
	for (i = 0; i < npts; i++)
	{
		switch (type)
		{
			case CONV_DOUBLE:     // --- Standard Doubles ---

				ps[i] = Trans_PP(hPP, ((Point_PP*) pts)[i]);
				break;

			case CONV_FLOAT:      // --- Floats ---

				dpt.x = (double) ((float*) pts)[i* 2];
				dpt.y = (double) ((float*) pts)[(i * 2) + 1];
				ps[i] = Trans_PP(hPP, dpt);
				break;

			case CONV_SHORT:      // --- Origined Shorts in .05 centimeters ---

				ps[i].x = ((long) ((short*) pts)[i * 2] * 50L) - hPP->PMFOrigin;
				ps[i].y = ((long) ((short*) pts)[(i * 2) + 1] * 50L) - hPP->PMFOrigin;
				break;
		}
		allin &= IVisible_PP(hPP, ps[i]);
	}

	// --- Do the fill ---

	if (!hPP->sNoFill)
		IFill_PP(hPP, ps, npts, allin);
	if (border)
		IMVect_PP(hPP, ps, npts, allin, 1);


	// --- Deallocate the Point Storage ---

	Free_MP(hPP->hMP, ps);
}

//-----------------------------------------------------------------------------

void ICircle_PP(double  dSin,   // Sin coefficient
                double  dCos,   // Cos coefficient
                double* dX,     // Current X
                double* dY)     // Current Y
{
	DEBUG_STACK;

	double cx, sx;
	double cy, sy;


	// --- Move the X coordinate ---

	if (fabs(*dX) < 1.0e-5)
	{
		cx = 0.0;
		sx = 0.0;
	}
	else
	{
		cx = dCos * (*dX);
		sx = dSin * (*dX);
	}

	// --- Move the Y coordinate ---

	if (fabs(*dY) < 1.0e-5)
	{
		cy = 0.0;
		sy = 0.0;
	}
	else
	{
		cy = dCos * (*dY);
		sy = dSin * (*dY);
	}

	// --- Update the next point ---

	*dX = cx - sy;
	*dY = cy + sx;

}
