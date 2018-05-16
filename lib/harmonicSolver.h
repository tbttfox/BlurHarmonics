#include <tuple>
#include <array>
#include <unordered_map>

typedef std::array<double, 3> Vec3;
typedef std::tuple<double, Vec3, Vec3> HarmCacheData;
typedef std::unordered_map<int, HarmCacheData> HarmCacheMap;
// frame: [step, worldspace, acceleration]

void interp(Vec3 &out, const HarmCacheMap &cache, int lowKey, int highKey, int tVal);
Vec3 calcAccel(const HarmCacheMap &cache, int prevKey, int curKey, int postKey);
void buildAllAccel(HarmCacheMap &cache, int &minTime, int &maxTime);
void updateAccel(HarmCacheMap &cache, std::vector<int> keys, int inserted);
Vec3 harmonicSolver( int time, int minTime, int maxTime,
        unsigned int waves, unsigned int length, double amp, double decay,
        const Vec3 &ampAxis, bool matchVelocity, const HarmCacheMap &cache
        );

