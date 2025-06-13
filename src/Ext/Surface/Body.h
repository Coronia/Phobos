#pragma once

// Forward declarations
struct RectangleStruct;

// Required includes
#include <Surface.h>

class SurfaceExt : public Surface
{
public:
	void BlurRect(const RectangleStruct& rect, float blurSize);

};
