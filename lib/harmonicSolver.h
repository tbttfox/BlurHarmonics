#include <tuple>
#include <array>
#include <map>

// A set of typedefs for easy work with the harmonic cache
// frame: [step, worldspace, acceleration]
typedef std::array<double, 3> Vec3;
typedef std::tuple<double, Vec3> HarmCacheData;
typedef std::map<double, HarmCacheData> HarmCacheMap;
typedef HarmCacheMap::iterator HarmCacheIt;
typedef HarmCacheMap::const_iterator HarmCacheCIt;

Vec3 interp(const Vec3 &low, double lowKey, const Vec3 &high, double highKey, double tVal);
Vec3 calcAccel(const Vec3 &cur, double curKey, const Vec3 &rprev, double prevKey, const Vec3 &rpost, double postKey);
Vec3 calcAccel(const HarmCacheCIt &prevIt, const HarmCacheCIt &curIt, const HarmCacheCIt &postIt);
void buildAllAccel(const HarmCacheMap &cache, HarmCacheMap &accel);
void updateAccel(const HarmCacheMap &cache, HarmCacheMap &accel, double inserted);
double handleEdge(const HarmCacheMap &cache, const HarmCacheCIt &it, double tKey);

Vec3 harmonicSolver(double time,
        double waves, double length, double amp, double decay, double term,
        const Vec3 &ampAxis, bool matchVelocity, const HarmCacheMap &accel,
		bool ignoreInitialAccel);

