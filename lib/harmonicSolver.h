#include <tuple>
#include <array>
#include <map>

typedef std::array<double, 3> Vec3;
typedef std::tuple<double, Vec3> HarmCacheData;
typedef std::map<int, HarmCacheData> HarmCacheMap;
typedef HarmCacheMap::iterator HarmCacheIt;
typedef HarmCacheMap::const_iterator HarmCacheCIt;
// frame: [step, worldspace, acceleration]

Vec3 interp(const HarmCacheCIt &lowIt, const HarmCacheCIt &highIt, int tVal);
Vec3 calcAccel(const Vec3 &cur, int curKey, const Vec3 &rprev, int prevKey, const Vec3 &rpost, int postKey);
Vec3 calcAccel(const HarmCacheCIt &prevIt, const HarmCacheCIt &curIt, const HarmCacheCIt &postIt);
void buildAllAccel(const HarmCacheMap &cache, HarmCacheMap &accel);
void updateAccel(const HarmCacheMap &cache, HarmCacheMap &accel, int inserted);
Vec3 harmonicSolver(int time,
        unsigned int waves, unsigned int length, double amp, double decay,
        const Vec3 &ampAxis, bool matchVelocity, const HarmCacheMap &accel,
		bool ignoreInitialAccel);
