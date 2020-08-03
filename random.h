#define COIN_FLIP     (RandomVal (2) == 0)

void          RandomInit(unsigned long seed);
float         RandomFloat();
bool          RandomRoll(int odds);
unsigned long RandomVal(int range);
unsigned long RandomVal(void);
GLvector2     RandomVector2();
