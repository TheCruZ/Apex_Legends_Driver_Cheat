#pragma once

//Structs
struct Fade {
	float f1;
	float f2;
	float f3;
	float f4;
	float f5;
	float f6;
	Fade(float amount) {
		f1 = amount;
		f2 = amount;
		f3 = amount;
		f4 = amount;
		f5 = amount;
		f6 = amount;
	}
};

struct Color {
	float r;
	float g;
	float b;
	Color() {
	}
	Color(float r1, float g1, float b1) {
		r = r1;
		g = g1;
		b = b1;
	}
};