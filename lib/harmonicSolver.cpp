#include "harmonicSolver.h"
#include <math.h>

# define TAU 6.28318530717958647692

void interp(Vec3 &out, const HarmCacheMap &cache, int lowKey, int highKey, int tVal){
	auto lit = cache.find(lowKey);
	if (lit == cache.end()) return;

	auto hit = cache.find(highKey);
	if (hit == cache.end()) return;

    const Vec3 &low = std::get<1>(lit->second);
    const Vec3 &high = std::get<1>(hit->second);

    double perc = double(tVal - lowKey) / double(highKey - lowKey);

    for (size_t i=0; i<3; ++i){
        out[i] = low[i]*perc + high[i]*(1-perc);
    }
}

Vec3 calcAccel(const HarmCacheMap &cache, int prevKey, int curKey, int postKey){
    Vec3 setter = {0.0, 0.0, 0.0};
    Vec3 cur, prev, post;

    auto curIt = cache.find(curKey);
    auto prevIt = cache.find(prevKey);
    auto postIt = cache.find(postKey);

    if ( curIt == cache.end() || prevIt == cache.end() || postIt == cache.end())
        return setter;

    cur = std::get<1>(curIt->second);
    prev = std::get<1>(prevIt->second);
    post = std::get<1>(postIt->second);

    if (prevKey != curKey-1) interp(prev, cache, prevKey, curKey, curKey-1);
    if (postKey != curKey+1) interp(post, cache, curKey, postKey, curKey+1);

    for (size_t j=0; j<3; ++j) setter[j] = prev[j] - (2 * cur[j]) + post[j];

    return setter;
}

void buildAllAccel(HarmCacheMap &cache, int &minTime, int &maxTime){
    // read and sort integer keys
    // if missing a key, interpolate to the next one
    // if in the middle of interpolation, accel is zero

    // Also, grab the min/max keys because I have to loop anyway
    std::vector<int> keys;
    keys.resize(cache.size());
    minTime = cache.begin()->first;
    maxTime = cache.begin()->first;

    for (auto it=cache.begin(); it!=cache.end(); ++it){
        minTime = (it->first < minTime) ? it->first : minTime;
        maxTime = (it->first > maxTime) ? it->first : maxTime;
        keys.push_back(it->first);
    }
    std::sort(keys.begin(), keys.end());

    for (size_t i=1; i<keys.size()-1; ++i){
        std::get<2>(cache[keys[i]]) = calcAccel(cache, keys[i-1], keys[i], keys[i+1]);
    }
}

void updateAccel(HarmCacheMap &cache, std::vector<int> keys, int inserted){
    if (cache.size() < 3) return;

    // Binary search for the index
    auto lb = std::lower_bound(keys.begin(), keys.end(), inserted);
    if (lb == keys.end()) return;

    size_t idx = lb - keys.begin();

    if (idx >= 2)
        std::get<2>(cache[keys[idx-1]]) = calcAccel(cache, keys[idx-2], keys[idx-1], keys[idx]);

    if (idx >= 1 && idx <= cache.size()-2)
        std::get<2>(cache[keys[idx]]) = calcAccel(cache, keys[idx-1], keys[idx], keys[idx+1]);

    if (idx <= cache.size()-3)
        std::get<2>(cache[keys[idx+1]]) = calcAccel(cache, keys[idx], keys[idx+1], keys[idx+2]);
}

Vec3 harmonicSolver( int time, int minTime, int maxTime,
        unsigned int waves, unsigned int length, double amp, double decay,
        const Vec3 &ampAxis, bool matchVelocity, const HarmCacheMap &cache
        ){

    Vec3 val = {0.0, 0.0, 0.0};
    // min/max time need to be cached
    if (time < minTime || time > maxTime) return val;

    double mval = (matchVelocity) ?  amp * length / TAU : amp;
    unsigned int crvLen = waves * length;
    double step = 0.0;
    double p2l = TAU / double(length);
    double dcl = decay / double(crvLen);

    for (size_t b=0; b<crvLen; ++b){

		auto it = cache.find(time - b);
		const Vec3 &accel = std::get<2>(it->second);
		
        double crv = sin(step * p2l) / exp(step * dcl);

        for (size_t i=0; i<3; ++i){
            val[i] -= accel[i] * crv * mval * ampAxis[i];
        }
        step += std::get<0>(it->second);
    }
    return val;
}

