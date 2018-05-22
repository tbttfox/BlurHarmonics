#include "harmonicSolver.h"
#include <math.h>

# define TAU 6.28318530717958647692

Vec3 interp(const Vec3 &low, int lowKey, const Vec3 &high, int highKey, int tVal){
    // Calculate a linearly interpolated Vec3 for when the
    // frame keys are not right next to each other
    double perc = double(tVal - lowKey) / double(highKey - lowKey);

    Vec3 out;
    for (size_t i=0; i<3; ++i){
        out[i] = low[i]*perc + high[i]*(1-perc);
    }
    return out;
}

Vec3 calcAccel(const Vec3 &cur, int curKey, const Vec3 &rprev, int prevKey, const Vec3 &rpost, int postKey){
    Vec3 prev = rprev, post = rpost;

    if (prevKey != curKey-1) prev = interp(prev, prevKey, cur, curKey, curKey-1);
    if (postKey != curKey+1) post = interp(cur, curKey, post, postKey, curKey+1);

    Vec3 setter;
    for (size_t j=0; j<3; ++j)
        setter[j] = prev[j] - (2 * cur[j]) + post[j];
    return setter;
}

Vec3 calcAccel(const HarmCacheCIt &prevIt, const HarmCacheCIt &curIt, const HarmCacheCIt &postIt){
    // Calculate an acceleration given iterators to 3 harmCache values
    int prevKey = prevIt->first;
    int curKey = curIt->first;
    int postKey = postIt->first;

    Vec3 cur = std::get<1>(curIt->second);
    Vec3 prev = std::get<1>(prevIt->second);
    Vec3 post = std::get<1>(postIt->second);

    return calcAccel(cur, curKey, prev, prevKey, post, postKey);
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

void updateFirstFrame(const HarmCacheCIt &curIt, const HarmCacheCIt &nxtIt, HarmCacheMap &accel) {
	// Get the acceleration on the first frame
	// assuming that everything is at rest outside of range
	// This can be ignored later in the solver
	int curKey = curIt->first;
	int postKey = nxtIt->first;
	int prevKey = curKey - 1;

	Vec3 cur = std::get<1>(curIt->second);
	Vec3 post = std::get<1>(nxtIt->second);
	Vec3 prev = { 0.0, 0.0, 0.0 };

	Vec3 vacc = calcAccel(cur, curKey, prev, prevKey, post, postKey);
	double step = std::get<0>(curIt->second);
	accel[curKey] = std::make_tuple(step, vacc);
	return;
}

void updateAccel(const HarmCacheMap &cache, HarmCacheMap &accel, int inserted){
    // Update the acceleration values near where we just
    // inserted a new value
	auto curIt = cache.find(inserted);
	if (curIt == cache.end()) return;

	if (cache.size() < 2) {
		double step = std::get<0>(curIt->second);
		Vec3 zero = { 0.0, 0.0, 0.0 };
		accel[inserted] = std::make_tuple(step, zero);
		return;
	}

	auto nxtIt = curIt, nxt2It = curIt;
	for (size_t i = 0; i < 3; ++i) {
		// build two incremented iterators
		// If we're out of bounds, continue
		bool bad = true;
		nxtIt = curIt; nxtIt++;
		if (nxtIt != cache.end()) {
			nxt2It = nxtIt; nxt2It++;
			if (nxt2It != cache.end()) {
				bad = false;
			}
		}

		if (!bad) {
			// calculate the accel
			double step = std::get<0>(nxtIt->second);
			accel[nxtIt->first] = std::make_tuple(step, calcAccel(curIt, nxtIt, nxt2It));
		}

		// if we're going out of bounds at the start, build the first frame and return
		if (curIt == cache.begin()) {
			updateFirstFrame(curIt, nxtIt, accel);
			return;
		}

		// do the previous triplet on the next loop
		curIt = std::prev(curIt);
	}
}

Vec3 harmonicSolver(int time,
        unsigned int waves, unsigned int length, double amp, double decay,
        const Vec3 &ampAxis, bool matchVelocity, const HarmCacheMap &cache,
        bool ignoreInitialAccel
        ){

    Vec3 val = {0.0, 0.0, 0.0};

    if (cache.size() < 3) return val;

    auto minTime = cache.begin()->first;
    if (time < minTime) return val;

    double mval = (matchVelocity) ?  amp * length / TAU : amp;
    unsigned int crvLen = waves * length;
    double step = 0.0;
    double p2l = TAU / double(length);
    double dcl = decay / double(crvLen);

    for (size_t b=0; b<crvLen; ++b){
		auto it = cache.find(time - b);
        // Because I use linear interpolation to fill in missing data
        // the acceleration would be 0, so I can skip it
        if (it == cache.end()){
			step += 1.0;
			continue;
		}
		if (time - b < minTime) break;
        if (ignoreInitialAccel && (time - b == minTime)) break;

		const Vec3 &accel = std::get<1>(it->second);
        double crv = sin(step * p2l) / exp(step * dcl);

        for (size_t i=0; i<3; ++i){
            val[i] -= accel[i] * crv * mval * ampAxis[i];
        }
        step += std::get<0>(it->second);
    }
    return val;
}

