#ifdef DEBUG
	#define DEBUG_PRINT(...)                      \
		do {                                  \
			fprintf(stderr, __VA_ARGS__); \
		} while (0)
#else
	#define DEBUG_PRINT(...) \
		do {             \
		} while (0)
#endif

void die(const char *format, ...);
