//
// Created by Twister on 27/09/2018.
//

#ifndef TGF_POINT_H
#define TGF_POINT_H

#include "Types.h"
#include <algorithm>

namespace tgf {
	class Vector {
		friend class Lua;
	public:
		float x, y;

		explicit Vector(float x, float y);
		explicit Vector(float v);
		Vector();

		float dot(const Vector& o) const;
		float angle() const;
		float length() const;
		Vector normalized() const;
		Vector perp() const;
		void set(float x, float y);

		Vector operator+ (const Vector& o) const;
		Vector operator- (const Vector& o) const;
		Vector operator* (const Vector& o) const;
		Vector operator* (float o) const;
		Vector operator/ (float o) const;
		Vector operator-() const;

	protected:
		/// For lua
		Vector add(const Vector& o) const { return this->operator+(o); }
		Vector sub(const Vector& o) const { return this->operator-(o); }
		Vector mul(const Vector& o) const { return this->operator*(o); }
		Vector mulF(float o) const { return this->operator*(o); }
		Vector div(float o) const { return this->operator/(o); }
		Vector neg() const { return this->operator-(); }
		Str toString() const;
	};
}


#endif //TGF_POINT_H
