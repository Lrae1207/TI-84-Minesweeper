#include "algorithms.h"

struct RandState {
	int state;
};

void initRand(RandState* state) {
	state = time(NULL);
}

int randomInt(RandState* state) {
	int x = state->state;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return state->state = x;
}

bool testChance(RandState* state, int chance) {
	float max = INT_MAX;
	float fRand = randomInt(state);

	return max / fRand <= chance;
}