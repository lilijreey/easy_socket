
#define DEF_CONCEPT(name) template <class KLASS> struct concept##name
#define IMPLEMENT_CONCEPT(name) concept##name<void>
