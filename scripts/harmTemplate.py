import os, time

from maya import cmds, OpenMayaUI as mui
from shiboken2 import wrapInstance, getCppPointer
from PySide2.QtCore import Qt, QPointF, QObject
from PySide2.QtWidgets import QWidget, QVBoxLayout, QLabel
from PySide2.QtGui import QPainter, QPen, QPolygonF, QColor
from math import exp, sin, pi

class HarmTemplate(QWidget):
	def __init__(self, node, parent=None):
		super(HarmTemplate, self).__init__(parent)

		self.attrs = ["decay", "termination", "numWaves", "amplitude", "waveLength"]
		self.lbl = QLabel("Wave:", self)
		self.wave = WaveDraw(self)
		self.layout = QVBoxLayout(self)
		self.layout.insertWidget(0, self.wave)
		self.layout.insertWidget(0, self.lbl)

		self.scriptJobs = None
		self._node = None
		self.setNode(node)

	def updateUI(self):
		''' Load the UI values from the node '''
		vals = [cmds.getAttr(self._node+"."+a) for a in self.attrs]
		self.wave.updateValues(*vals)

	def _buildScriptJobs(self):
		self.scriptJobs = []
		for attr in self.attrs:
			name = '{0}.{1}'.format(self._node, attr)
			self.scriptJobs.append(cmds.scriptJob(ac=[name, self.updateUI], killWithScene=1))

	def _killScriptJobs(self):
		for sj in self.scriptJobs:
			for _ in range(10):
				try:
					if cmds.scriptJob(exists=sj):
						cmds.scriptJob(kill=sj, force=True)
					break
				except RuntimeError:
					#This happens very rarely when that scriptJob is
					#being executed at the same time we try to kill it.
					cmds.warning("Got RuntimeError trying to kill scriptjob...trying again")
					time.sleep(0.1)
			else:
				cmds.warning("Killing scriptjob is taking too long...skipping")
		self.scriptJobs = None

	def setNode(self, node):
		''' This widget should now represent the same attr on a different node. '''
		oldNode = self._node
		self._node = node
		self.updateUI()

		if self.scriptJobs is None:
			self._buildScriptJobs()

		elif oldNode != self._node:
			self._killScriptJobs()
			self._buildScriptJobs()


class WaveDraw(QWidget):
	''' Draw the Falloff and wave for a harmonic '''
	bgColor = Qt.black
	limitColor = Qt.darkCyan
	waveColor = Qt.red
	gridColor = QColor(55, 55, 55)
	gridMajorColor = Qt.gray
	gridMinorColor = QColor(85, 85, 85)
	gridAxisColor = Qt.darkGray

	def __init__(self, parent):
		super(WaveDraw, self).__init__(parent)
		self.setMinimumHeight(180)
		self.decay = 3.0
		self.term = 0.0
		self.count = 5.0
		self.amp = 1.0
		self.waveLength = 20

	def updateValues(self, decay=None, term=None, count=None, amp=None, length=None):
		if decay is not None:
			self.decay = decay
		if term is not None:
			self.term = term
		if count is not None:
			self.count = count
		if length is not None:
			self.waveLength = length
		if amp is not None:
			self.amp = amp

		self.repaint()

	def paintEvent(self, event):
		painter = QPainter(self)
		rect = self.contentsRect()
		inset = 5

		self.paintBG(painter)
		rect.adjust(inset, inset, -inset, -inset)
		self.paintGrid(painter, rect)

		painter.translate(QPointF(inset, inset))
		limit, waves = self.evalCurves(rect.width(), self.decay, self.term, self.count)
		painter.setRenderHint(QPainter.Antialiasing, True)
		self.paintLimits(painter, limit, rect)
		self.paintWaves(painter, waves, rect)

	def paintBG(self, painter):
		painter.save()
		painter.setBrush(self.bgColor)
		painter.drawRect(0, 0, self.width(), self.height())
		painter.restore()

	def paintGrid(self, painter, rect):
		painter.save()
		painter.setPen(QPen(self.gridColor, 1))

		def setPainter(ptr, i):
			if i % 10 == 0:
				ptr.setPen(QPen(self.gridMajorColor, 1))
			elif i % 5 == 0:
				ptr.setPen(QPen(self.gridMinorColor, 1))
			else:
				ptr.setPen(QPen(self.gridColor, 1))

		shift = (rect.bottom() - rect.top())
		hCount = self.amp * 10.0
		if hCount == 0:
			hCount = 10
		hh = int(self.height() / 2)
		for i in range(0, int(hCount)+1, 5):
			setPainter(painter, i)
			ss = int((i/(2*hCount)) * shift)
			painter.drawLine(0, hh+ss, self.width(), hh+ss)
			painter.drawLine(0, hh-ss, self.width(), hh-ss)

		shift = (rect.right() - rect.left())
		wCount = float(self.count * self.waveLength)
		if wCount == 0:
			wCount = 20
		for i in range(int(wCount) + 1):
			setPainter(painter, i)
			ss = rect.left() + (i/wCount)*shift
			painter.drawLine(int(ss), 0, int(ss), self.height())

		painter.setPen(QPen(self.gridAxisColor, 3))
		hh = int(self.height() / 2)
		painter.drawLine(0, hh, self.width(), hh)
		painter.drawLine(rect.left(), 0, rect.left(), self.height())

		painter.restore()

	def paintLimits(self, painter, limit, rect):
		hh = int(rect.height() / 2)
		posFo = QPolygonF([QPointF(s, (f*hh)+hh+2) for s, f in enumerate(limit)])
		negFo = QPolygonF([QPointF(s, (-f*hh)+hh-2) for s, f in enumerate(limit)])

		painter.save()
		painter.setPen(QPen(self.limitColor, 2))
		painter.drawPolyline(posFo)
		painter.drawPolyline(negFo)
		painter.restore()

	def paintWaves(self, painter, waves, rect):
		hh = int(rect.height() / 2)
		curve = QPolygonF([QPointF(s, (-f*hh)+hh) for s, f in enumerate(waves)])
		painter.save()
		painter.setPen(QPen(self.waveColor, 2))
		painter.drawPolyline(curve)
		painter.restore()

	@staticmethod
	def evalCurves(width, decay, term, count):
		'''
		width -> The width of the window in pixels
		decay -> The exponential decay power
		term -> The terminating value of the decay
		count -> The number of waves per impulse
		'''
		samples = [float(i)/width for i in range(width+1)]
		falloffs = []
		waves = []
		for x in samples:
			f = (1-term)*(exp(-decay*x) - x*exp(-decay)-1) + 1
			c = sin(2 * pi * count * x) * f
			falloffs.append(f)
			waves.append(c)

		return falloffs, waves

def _getLayout(lay):
	ptr = mui.MQtUtil.findLayout(lay)
	tmpPar = wrapInstance(long(ptr), QWidget)
	tmpLay = tmpPar.layout()

	#layPtr = getCppPointer(tmpLay)
	#lay = wrapInstance(long(ptr), QVBoxLayout)
	#par = lay.parent()
	return tmpLay, tmpPar



def builder(lay, node):
	''' Build/Initialize/Install the QT GUI into the layout.

	Parameters
	----------
	lay: str
		Name of the Maya layout to add the QT GUI to
	node: str
		Name of the node to (initially) connect to the QT GUI
	'''
	lay, par = _getLayout(lay)
	widg = HarmTemplate(node, par)
	lay.addWidget(widg)

def updater(lay, node):
	''' Update the QT GUI to point to a different node

	Parameters
	----------
	lay: str
		Name of the Maya layout to where the QT GUI lives
	node: str
		Name of the new node to connect to the QT GUI
	'''
	lay, par = _getLayout(lay)

	for c in range(lay.count()):
		widg = lay.itemAt(c).widget()
		if isinstance(widg, HarmTemplate):
			#found the widget, update the node it's pointing to
			widg.setNode(node)
			break


