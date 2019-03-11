#ifndef MATHUTILS_H
#define MATHUTILS_H


namespace mu {
	float lerp(float start, float end, float percent)
	{
		return (start + percent * (end - start));
	}
}

#endif // !MATHUTILS_H