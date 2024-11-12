#include <time.h>
#include <limits.h>

typedef struct {
	int state;
}RandState;

void initRand(RandState* state);
int randomInt(RandState* state);

bool testChance(RandState* state, int chance);