#include "harmonicSolver.h"
#include <math.h>

# define TAU 6.28318530717958647692

Vec3 interp(const HarmCacheCIt &lowIt, const HarmCacheCIt &highIt, int tVal){
    // Calculate a linearly interpolated Vec3 for when the
    // frame keys are not right next to each other
    const Vec3 &low = std::get<1>(lowIt->second);
    const Vec3 &high = std::get<1>(highIt->second);
    int lowKey = lowIt->first;
    int highKey = highIt->first;

    double perc = double(tVal - lowKey) / double(highKey - lowKey);

    Vec3 out;
    for (size_t i=0; i<3; ++i){
        out[i] = low[i]*perc + high[i]*(1-perc);
    }
    return out;
}

Vec3 calcAccel(const HarmCacheCIt &prevIt, const HarmCacheCIt &curIt, const HarmCacheCIt &postIt){
    // Calculate an acceleration given iterators to 3 harmCache values
    int prevKey = prevIt->first;
    int curKey = curIt->first;
    int postKey = postIt->first;

    Vec3 cur = std::get<1>(curIt->second);
    Vec3 prev = std::get<1>(prevIt->second);
    Vec3 post = std::get<1>(postIt->second);

    if (prevKey != curKey-1) prev = interp(prevIt, curIt, curKey-1);
    if (postKey != curKey+1) post = interp(curIt, postIt, curKey+1);

    Vec3 setter;
    for (size_t j=0; j<3; ++j)
        setter[j] = prev[j] - (2 * cur[j]) + post[j];
    return setter;
}

void buildAllAccel(const HarmCacheMap &cache, HarmCacheMap &accel){
    // Build all acceleration values at once
    if (cache.size() < 3) return;
    HarmCacheCIt start = std::next(cache.begin());
    HarmCacheCIt end = std::prev(cache.end());
    for (auto it=start; it!=end; ++it){
        double step = std::get<0>(it->second);
        accel[it->first] = std::make_tuple(step, calcAccel(std::prev(it), it, std::next(it)));
    }
}

void updateAccel(const HarmCacheMap &cache, HarmCacheMap &accel, int inserted){
    // Update the acceleration values near where we just
    // inserted a new value

	auto curIt = cache.find(inserted);
	if (curIt == cache.end()) return;

	
	if (cache.size() < 3) {
		double step = std::get<0>(curIt->second);
		Vec3 zero = { 0.0, 0.0, 0.0 };
		accel[inserted] = std::make_tuple(step, zero);
		return;
	}

    for (size_t i = 0; i < 3; ++i){
        // build two incremented iterators
        // If we're out of bounds, continue
        auto nxtIt = curIt; nxtIt++;
        if (nxtIt == cache.end()) continue;
        auto nxt2It = nxtIt; nxt2It++;
        if (nxt2It == cache.end()) continue;

        // calculate the accel
        double step = std::get<0>(nxtIt->second);
        accel[nxtIt->first] = std::make_tuple(step, calcAccel(curIt, nxtIt, nxt2It));

        // if we're going out of bounds at the start, just return
        if (curIt == cache.begin()) return;
        // do the previous triplet on the next loop
        curIt = std::prev(curIt);
    }
}

Vec3 harmonicSolver(int time,
        unsigned int waves, unsigned int length, double amp, double decay,
        const Vec3 &ampAxis, bool matchVelocity, const HarmCacheMap &cache
        ){

    Vec3 val = {0.0, 0.0, 0.0};

    if (cache.size() < 3) return val;

    auto minTime = cache.begin()->first;
    auto maxTime = std::prev(cache.end())->first;
    if (time < minTime || time > maxTime) return val;

    double mval = (matchVelocity) ?  amp * length / TAU : amp;
    unsigned int crvLen = waves * length;
    double step = 0.0;
    double p2l = TAU / double(length);
    double dcl = decay / double(crvLen);

    for (size_t b=0; b<crvLen; ++b){
		auto it = cache.find(time - b);
        // Because I use linear interpolation to fill in missing data
        // the acceleration would be 0, so I can skip it
        if (it == cache.end()) continue;

		const Vec3 &accel = std::get<1>(it->second);
        double crv = sin(step * p2l) / exp(step * dcl);

        for (size_t i=0; i<3; ++i){
            val[i] -= accel[i] * crv * mval * ampAxis[i];
        }
        step += std::get<0>(it->second);
    }
    return val;
}

