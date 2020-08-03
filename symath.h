#ifndef SYMATH_H
#define SYMATH_H

//Keep an angle between 0 and 360
float syMathAngle(float angle);
//Get an angle between two given points on a grid
float syMathAngle(float x1, float y1, float x2, float y2);
//Get distance between 2 points on a plane.
float syMathDistance(float x1, float y1, float x2, float y2);
//difference between two angles
float syMathAngleDifference(float a1, float a2);
//interpolate between two values
float syMathLerp(float val, float a, float b);
//input values from 0.0 to 1.0 and convert them to values along a 0 to 1 curve.
float syMathScalarCurve(float val);
//input values from 0.0 to 1.0 and convert them to values along a 0 to 1 to 0 curve.
float syMathScalarCurveLoop (float val);
float syMathLerpQuad(float y0, float y1, float y2, float y3, GLvector2 offset, bool left);
float syMathInterpolate (float n1, float n2, float delta);

#endif // SYMATH_H