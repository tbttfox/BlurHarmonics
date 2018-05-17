#include <tuple>
#include <array>
#include <map>

typedef std::array<double, 3> Vec3;
typedef std::tuple<double, Vec3, Vec3> HarmCacheData;
typedef std::map<int, HarmCacheData> HarmCacheMap;
typedef std::map<int, HarmCacheData>::iterator HarmCacheIt;
// frame: [step, worldspace, acceleration]

Vec3 interp(const HarmCacheIt &lowIt, const HarmCacheIt &highIt, int tVal);
Vec3 calcAccel(const HarmCacheIt &prevIt, const HarmCacheIt &curIt, const HarmCacheIt &postIt);
void buildAllAccel(HarmCacheMap &cache);
void updateAccel(HarmCacheMap &cache, int inserted);
Vec3 harmonicSolver(int time,
        unsigned int waves, unsigned int length, double amp, double decay,
        const Vec3 &ampAxis, bool matchVelocity, const HarmCacheMap &cache
        );

