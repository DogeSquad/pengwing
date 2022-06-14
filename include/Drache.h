#pragma once
#include "Object.h"

class Drache : public Object{
public:
	Drache(Shader shader, geometry object_geometry, const char* name);
	void Drache::update(int frame) override;
};