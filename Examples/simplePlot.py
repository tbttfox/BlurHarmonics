#pylint:disable=missing-docstring,redefined-outer-name,invalid-name
import matplotlib.pyplot as plt
from math import sin, cos, exp, pi, floor, ceil


def getAccel(anim, curTime):
	if curTime == 0:
		#return 0
		curTime += 1
	elif curTime == len(anim) - 1:
		return 0
		curTime -= 1

	last = anim[curTime - 1]
	cur = anim[curTime]
	fut = anim[curTime + 1]
	return last - 2*cur + fut

def evalCrv(c, w, d, t):
	''' c -> count, w -> wavelength, d -> decay '''
	return sin(2 * pi * t / w) / exp(t * d / (c * w))

def test(anim, num=10, amp=1.0, length=20, decay=5.0, matchVelocity=True):
	extra = int((num + .5) * length)
	anim += [anim[-1]] * extra

	stepVal = [1.0] * len(anim)
	stepVal = [1.0] * 60 + [3.5] * 60 + [1.0]


	stepVal += [stepVal[-1]] * len(anim)
	stepVal = stepVal[:len(anim)]

	# Build the response curve
	trange = range(len(anim))
	crvLen = int(num * length) + 1

	accel = [getAccel(anim, curTime) for curTime in trange]

	mvel = amp
	if matchVelocity:
		mvel = amp * length / (2 * pi)

	out = []
	for f in trange: # frame
		val = anim[f]
		s = 0 # total steps taken
		for b in range(crvLen):
			val -= accel[f-b] * evalCrv(num, length, decay, s) * mvel
			s += stepVal[f-b]
		out.append(val)

	plt.plot(trange, anim, 'b--')
	plt.plot(trange, accel, 'g')
	plt.plot(trange, out, 'r')
	plt.show()




if __name__ == "__main__":
	# linear stop
	anim = range(10)

	# Parabolic stop
	#anim = [9.0 - (((i-10) ** 2) * 9.0 / 100.0) for i in range(10)]

	# Sine motion
	#anim = [sin(pi * i / 10) for i in range(80)]

	test(anim)



