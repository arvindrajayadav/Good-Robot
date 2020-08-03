/*-----------------------------------------------------------------------------

  Math.cpp

  2012 Shamus Young

  -------------------------------------------------------------------------------

  Various useful math functions.

  -----------------------------------------------------------------------------*/

#include "master.h"
#include <math.h>

#include "math.h"

/*-----------------------------------------------------------------------------
Keep an angle between 0 and 360
-----------------------------------------------------------------------------*/

float syMathAngle(float angle)
{
	if (angle < 0.0f)
		angle = 360.0f - (float)fmod(fabs(angle), 360.0f);
	else
		angle = (float)fmod(angle, 360.0f);
	return angle;
}

/*-----------------------------------------------------------------------------
Get an angle between two given points on a grid
-----------------------------------------------------------------------------*/

float syMathAngle(float x1, float y1, float x2, float y2)
{
	float   x_delta;
	float   z_delta;
	float   angle;

	z_delta = (y1 - y2);
	x_delta = (x1 - x2);
	if (x_delta == 0) {
		if (z_delta > 0)
			return 0.0f;
		else
			return 180.0f;
	}
	if (fabs(x_delta) < fabs(z_delta)) {
		angle = 90 - (float)atan(z_delta / x_delta) * RADIANS_TO_DEGREES;
		if (x_delta < 0)
			angle -= 180.0f;
	}
	else {
		angle = (float)atan(x_delta / z_delta) * RADIANS_TO_DEGREES;
		if (z_delta < 0.0f)
			angle += 180.0f;
	}
	if (angle < 0.0f)
		angle += 360.0f;
	return angle;
}

/*-----------------------------------------------------------------------------
Get distance (squared) between 2 points on a plane
-----------------------------------------------------------------------------*/

float syMathDistance2(float x1, float y1, float x2, float y2)
{
	float     dx;
	float     dy;

	dx = x1 - x2;
	dy = y1 - y2;
	return dx * dx + dy * dy;
}

/*-----------------------------------------------------------------------------
Get distance between 2 points on a plane. This is slightly slower than
MathDistance2 ()
-----------------------------------------------------------------------------*/

float syMathDistance(float x1, float y1, float x2, float y2)
{
	float     dx;
	float     dy;

	dx = x1 - x2;
	dy = y1 - y2;
	return (float)sqrt(dx * dx + dy * dy);
}

/*-----------------------------------------------------------------------------
difference between two angles
-----------------------------------------------------------------------------*/

float syMathAngleDifference(float a1, float a2)

{
	float         result;

	result = (float)fmod(a1 - a2, 360.0f);
	if (result > 180.0)
		return result - 360.0F;
	if (result < -180.0)
		return result + 360.0F;
	return result;
}

/*-----------------------------------------------------------------------------
interpolate between two values
-----------------------------------------------------------------------------*/

float syMathInterpolate(float n1, float n2, float delta)
{
	return n1 * (1.0f - delta) + n2 * delta;
}

/*-----------------------------------------------------------------------------
return a scalar of 0.0 to 1.0, based an the given values position within a range
-----------------------------------------------------------------------------*/

float syMathLerp(float val, float a, float b)
{
	if (b == a)
		return 0.0f;
	val -= a;
	val /= (b - a);
	return clamp(val, 0.0f, 1.0f);
}

/*-----------------------------------------------------------------------------
  This will take linear input values from 0.0 to 1.0 and convert them to
  values along a curve.  This could also be acomplished with sin (), but this
  way avoids converting to radians and back.
-----------------------------------------------------------------------------*/

float syMathScalarCurve (float val)
{
	float   sign;

	val = (val - 0.5f) * 2.0f;
	if (val < 0.0f)
		sign = -1.0f;
	else
		sign = 1.0f;
	if (val < 0.0f)
		val = -val;
	val = 1.0f - val;
	val *= val;
	val = 1.0f - val;
	val *= sign;
	val = (val + 1.0f) / 2.0f;
	return val;
}

/*-----------------------------------------------------------------------------
This will take linear input values from 0.0 to 1.0 and convert them to
values along a curve. Unlike above, this one ends where it started, producing a 
looping animation.
-----------------------------------------------------------------------------*/

float syMathScalarCurveLoop (float val)
{
  val *= 2.0f;
  if (val > 1.0f)
    val = 2.0f - val;
  return syMathScalarCurve (val);
}

/*-----------------------------------------------------------------------------

  This forms a theoretical quad with the four elevation values.  Given the
  offset from the upper-left corner, it determines what the elevation
  should be at that point in the center area.  left" determines if the
  quad is cut from y2 to y1, or from y0 to y3.

  y0-----y1
  |     |
  |     |
  y2-----y3

  -----------------------------------------------------------------------------*/

float syMathLerpQuad(float y0, float y1, float y2, float y3, GLvector2 offset, bool left)
{
	float   a;
	float   b;
	float   c;

	if (left) {
		if (offset.x + offset.y < 1) {
			c = y2 - y0;
			b = y1 - y0;
			a = y0;
		}
		else {
			c = y3 - y1;
			b = y3 - y2;
			a = y3 - (b + c);
		}
	}
	else { //right
		if (offset.x < offset.y) {
			c = y2 - y0;
			b = y3 - y2;
			a = y0;
		}
		else {
			c = y3 - y1;
			b = y1 - y0;
			a = y0;
		}
	}
	return (a + b * offset.x + c * offset.y);
}