#pragma once

#include "Vector.h"
#include <GD_Graphics/DrawCall.h>

namespace gde {
	class Object {
	private:
		bool isDestroyed = false;
	public:
		Object();

		Vector3 position;
		Vector3 scale;
		Vector3 rotation;

		//Rendering stuff
		DrawCall* mDrawCall;
		void UpdateDrawCall();

		//Rigidbody stuff
		Vector3 velocity;
		Vector3 acceleration;
		float damping;

		void Destroy();
		bool get_isDestroyed();
	};
}