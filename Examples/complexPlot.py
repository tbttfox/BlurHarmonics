from __future__ import division
import numpy as np
from sortedcontainers import SortedDict
from math import exp, sin, cos, pi
TAU = 2 * pi

def interp(low, lowKey, high, highKey, tVal):
	''' Linearly interpolate 2 values '''
	perc = (tVal - lowKey) / (highKey - lowKey)
	return low*perc + high*(1-perc)

def calcAccel(cur, curKey, prev, prevKey, post, postKey):
	'''Estimate an acceleration given 3 points in time'''
	if prevKey != curKey-1:
		prev = interp(prev, prevKey, cur, curKey, curKey-1)

	if postKey != curKey+1:
		post = interp(cur, curKey, post, postKey, curKey+1)
	return prev - (2 * cur) + post

def calcAccelEasy(cache, idx):
	'''Convenience function to calculate the acceleration at a particular index'''
	preFrame, preVal = cache.peekitem(idx-1)
	curFrame, curVal = cache.peekitem(idx)
	nxtFrame, nxtVal = cache.peekitem(idx+1)
	return calcAccel(curVal[1], curFrame, preVal[1], preFrame, nxtVal[1], nxtFrame)

def builAlldAccel(cache):
	''' Build the acceleration cache from the value cache '''
	accel = SortedDict()
	if len(cache) < 3:
		return accel

	for i in range(1, len(cache) - 1):
		frame, (step, _) = cache.peekitem(i)
		accel[frame] = (step, calcAccelEasy(cache, i))
	return accel

def getFirstFrameAccel(cache):
	'''A special case for inserting at the first frame where theres no previous motion'''
	# Get the acceleration on the first frame assuming that everything is at rest outside of range
	# This can be ignored later in the solver
	keys = cache.keys()
	curKey = keys[0]
	postKey = keys[1]
	prevKey = curKey - 1.0

	cur = cache[curKey][1]
	post = cache[postKey][1]
	prev = 0.0

	return calcAccel(cur, curKey, prev, prevKey, post, postKey)

def updateAccel(cache, accel, inserted):
	'''Insert a new acceleration key at inserted, and update any adjacent accelerations'''
	if inserted not in cache:
		return

	if len(cache) < 2:
		insStep, _ = cache[inserted]
		accel[inserted] = (insStep, 0.0)
		return

	idxVal = cache.keys().index(inserted)
	for idx in range(idxVal, idxVal-3, -1):
		if len(cache) - idx > 2:
			nxtFrm, (nxtStep, _) = cache.peekitem(idx+1)
			accel[nxtFrm] = (nxtStep, calcAccelEasy(cache, idx+1))

		if idx == 0:
			curFrm, (curStep, _) = cache.peekitem(idx)
			accel[curFrm] = (curStep, getFirstFrameAccel(cache))
			return


def handleEdge(cache, idx, tKey):
	''' Interpolate the step size and handle any edge cases '''
	it = cache.peekitem(idx)
	pKey = it[0]
	if idx + 1 == len(cache):
		return tKey - pKey
	elif tKey == pKey:
		return 0.0
	nit = cache.peekitem(idx + 1)
	nKey = nit[0]
	pStep = it[1][0]
	nStep = nit[1][0]

	perc = (tKey - pKey) / (nKey - pKey)
	return (pStep * (1.0 - perc) + nStep * perc) * perc

def harmonicSolver(cache, time, waves, length, decay, term, amp, axisAmp,
				   matchVelocity=True, ignoreInitialAccel=True):
	''' Solve a spring at a given time based on cached acceleration

	Parameters
	----------
	cache : SortedDict{float: (float, float)}
		A sorted dictionary keyed by the time whose value is (step, acceleration)
	time : float
		The current time value in seconds
	waves : float
		The number of waves to calculate
	decay : float
		The exponential decay value. Zero gives linear decay
	term : float
		The terminal amplitude. After the maximum number of waves
		this will be the amplitude multiplier
	amp : float
		The starting amplitude of the waves
	axisAmp : float
		The per-axis amplitude. Makes more sense when solving the full 3d vector
	matchVelocity : bool, optional
		Automatically scale the amplitude so that an amplitude of 1 will produce
		a sine wave where the tangent slope will match the input velocity slope.
		Defaults True
	ignoreInitialAccel : bool, optional
		Whether to ignore the instantaneous acceleration that happens at the first
		frame of animation. Defaults True
	'''
	val, vel, acc = 0.0, 0.0, 0.0
	idx = cache.bisect_right(time)

	if idx == 0:
		return val, vel, acc # Only happens if time is the first key
	idx -= 1
	ti = 1.0 - term
	t = term
	step = handleEdge(cache, idx, time)
	mvel = amp * length / TAU if matchVelocity else amp
	wl = waves * length
	p2l = TAU / length
	edl = ti * exp(-decay) / wl
	dl = -decay / wl


	# There's just no good way to make this readable
	while step < wl:
		if ignoreInitialAccel and idx == 0:
			break
		it = cache.peekitem(idx)
		accel = it[1][1]

		# The general idea behind this decay math:
		# Take `exp(step*decay)` and subtract `step*exp(decay)` to shift the
		# termination to `0` but leave the starting point at `1`
		# Then scale the whole thing down by `(1-term)`
		# Finally, shift it up by `term` so that the curve starts at `1` and ends at `term`
		# Then just multiply it by the sine wave

		txl = step * p2l
		edxl = ti * exp(step*dl)
		sedl = step * edl

		# Position Curve
		posC = sin(txl) * (edxl - sedl + t)
		val -= accel * mvel * axisAmp * posC

		# Velocity Curve
		velC = (edxl*dl - edl) * sin(txl) + (t + (edxl - sedl)) * cos(txl) * p2l
		vel -= accel * mvel * axisAmp * velC

		# Acceleration curve
		aa = edxl*sin(txl)*dl**2
		bb = (edxl*dl - edl)*cos(txl)*2*p2l
		cc = (t + (edxl - sedl))*sin(txl)*p2l**2
		accC = aa + bb + cc
		acc -= accel * mvel * axisAmp * accC

		step += it[1][0]
		if idx == 0: break
		idx -= 1

	return val, vel, acc


def plotTest(anim, stepVal, num=10, amp=1.0, length=20, decay=5.0, term=0.0,
			 matchVelocity=True, ignoreInitialAccel=True):
	# Add extra frames so that the wave will fully decay
	extra = int((num + .5) * length)
	anim += [anim[-1]] * extra

	# Allows for anim and stepVal to have different lengths
	stepVal += [stepVal[-1]] * len(anim)
	stepVal = stepVal[:len(anim)]

	# Get the frame numbers for everything
	trange = range(len(anim))

	# Parse the position cache into an acceleration cache
	cache = SortedDict([(t, (s, v)) for t, s, v in zip(trange, stepVal, anim)])
	accel = builAlldAccel(cache)

	vals, vels, accs = [], [], []
	for f in trange:
		val, vel, acc = harmonicSolver(
			accel, f, num, length, decay, term, amp, 1.0,
			matchVelocity=matchVelocity, ignoreInitialAccel=ignoreInitialAccel)

		vals.append(val)
		vels.append(vel)
		accs.append(acc)
	return vals, vels, accs, accel, trange





def test():
	anim = range(10)
	stepVal = [1.0] * len(anim)
	vals, vels, accs, accelCache, trange = plotTest(anim, stepVal)

	import matplotlib.pyplot as plt

	accel = [0.0, 0.0] + [x[1] for x in accelCache.values()]
	saccs = [-j-i for i,j in zip(accel, accs)]

	end = 300

	#plt.plot(trange[:end], anim[:end], 'b--')
	plt.plot(trange[:end], saccs[:end], 'g')
	plt.plot(trange[:end], vals[:end], 'r')
	plt.plot(trange[:end], vels[:end], 'y')

	plt.show()







if __name__ == "__main__":
	test()


