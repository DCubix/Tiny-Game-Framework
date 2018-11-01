//
// Created by Twister on 27/09/2018.
//

#include "Vector.h"

#include <cmath>

namespace tgf {

	Vector::Vector(float x, float y) : x(x), y(y) { }
	Vector::Vector(float v) : Vector(v, v) { }
	Vector::Vector() : Vector(0) { }

	float Vector::dot(const Vector& o) const {
		return x * o.x + y * o.y;
	}

	float Vector::angle() const {
		return std::atan2(y, x);
	}

	float Vector::length() const {
		return std::sqrt(dot(*this));
	}

	Vector Vector::normalized() const {
		float l = length();
		return Vector(x/l, y/l);
	}

	Vector Vector::perp() const {
		return Vector(-y, x);
	}

	Vector Vector::operator+(const Vector& o) const {
		return Vector(x + o.x, y + o.y);
	}

	Vector Vector::operator-(const Vector& o) const {
		return Vector(x - o.x, y - o.y);
	}

	Vector Vector::operator*(const Vector& o) const {
		return Vector(x * o.x, y * o.y);
	}

	Vector Vector::operator/(float o) const {
		return Vector(x / o, y / o);
	}

	Vector Vector::operator*(float o) const {
		return Vector(x * o, y * o);
	}

	Vector Vector::operator-() const {
		return Vector(-x, -y);
	}

	Str Vector::toString() const {
		return "<" + std::to_string(x) + ", " + std::to_string(y) + ">";
	}

	void Vector::set(float x, float y) {
		this->x = x;
		this->y = y;
	}


}