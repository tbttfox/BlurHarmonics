#include "harmonicsNode.h"
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnPluginData.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MMatrix.h>
#include <maya/MPoint.h>
#include <maya/MTime.h>


#include <maya/MFnDependencyNode.h>
#include <string>


#define MCHECKERROR(STAT)         \
    if (MS::kSuccess != STAT) {   \
        return MS::kFailure;      \
    }

#define MCHECKERRORMSG(STAT, MSG)   \
    if (MS::kSuccess != STAT) {     \
        cerr << MSG << endl;        \
        return MS::kFailure;        \
    }

MTypeId harmonics::id(0x00122704);

MObject harmonics::aOutput; // vector
MObject harmonics::aOutputX; // distance
MObject harmonics::aOutputY; // distance
MObject harmonics::aOutputZ; // distance

MObject harmonics::aWorldRefInverse; // Matrix
MObject harmonics::aParentInverse; // Matrix
MObject harmonics::aInput; // Matrix

MObject harmonics::aPositionCache; // harmonicMap
MObject harmonics::aAccelCache; // harmonicMap
MObject harmonics::aUpdate; // bool
MObject harmonics::aClear; // bool
MObject harmonics::aWaves; // int
MObject harmonics::aWaveLength; // int
MObject harmonics::aAmplitude; // double
MObject harmonics::aAxisAmp; // vector
MObject harmonics::aDecay; // double
MObject harmonics::aTermination; // double
MObject harmonics::aNormAmp; // bool
MObject harmonics::aIgnoreFirst; //bool
MObject harmonics::aTime; // double
MObject harmonics::aStep; // double


harmonics::harmonics() { }
harmonics::~harmonics() { }
void* harmonics::creator(){ return new harmonics(); }

MStatus harmonics::initialize(){
    MFnTypedAttribute tAttr;
    MFnMatrixAttribute mAttr;
    MFnNumericAttribute nAttr;
    MFnUnitAttribute uAttr;
    MStatus stat;

    // Output translation vector
	aOutputX = uAttr.create("outputX", "outx", MFnUnitAttribute::kDistance, 0.0, &stat);
    MCHECKERRORMSG(stat, "addAttribute: outputX");
	uAttr.setWritable(false);
	uAttr.setStorable(false);
	uAttr.setCached(false);

	aOutputY = uAttr.create("outputY", "outy", MFnUnitAttribute::kDistance, 0.0, &stat);
    MCHECKERRORMSG(stat, "addAttribute: outputY");
	uAttr.setWritable(false);
	uAttr.setStorable(false);
	uAttr.setCached(false);

	aOutputZ = uAttr.create("outputZ", "outz", MFnUnitAttribute::kDistance, 0.0, &stat);
    MCHECKERRORMSG(stat, "addAttribute: outputZ");
	uAttr.setWritable(false);
	uAttr.setStorable(false);
	uAttr.setCached(false);

	aOutput = nAttr.create("output", "output", aOutputX, aOutputY, aOutputZ, &stat);
	nAttr.setHidden(true);
	stat = addAttribute(aOutput);
    MCHECKERRORMSG(stat, "addAttribute: output");
	nAttr.setWritable(false);
	nAttr.setStorable(false);
	nAttr.setCached(false);

	// Storage of the per-frame acceleration
	aAccelCache = tAttr.create("accelCache", "accelCache", HarmCacheProxy::id, MObject::kNullObj, &stat);
	MCHECKERRORMSG(stat, "createAttribute: accelCache");
	tAttr.setHidden(true);
	stat = addAttribute(aAccelCache);
	MCHECKERRORMSG(stat, "addAttribute: accelCache");
	stat = attributeAffects(aAccelCache, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aAccelCache -> aOutput");
	stat = attributeAffects(aAccelCache, aOutputX);
	MCHECKERRORMSG(stat, "attributeAffects: aAccelCache -> aOutputX");
	stat = attributeAffects(aAccelCache, aOutputY);
	MCHECKERRORMSG(stat, "attributeAffects: aAccelCache -> aOutputY");
	stat = attributeAffects(aAccelCache, aOutputZ);
	MCHECKERRORMSG(stat, "attributeAffects: aAccelCache -> aOutputZ");

	// Storage of the per-frame world position
	aPositionCache = tAttr.create("positionCache", "positionCache", HarmCacheProxy::id, MObject::kNullObj, &stat);
	MCHECKERRORMSG(stat, "createAttribute: positionCache");
	tAttr.setHidden(true);
	stat = addAttribute(aPositionCache);
	MCHECKERRORMSG(stat, "addAttribute: positionCache");
	stat = attributeAffects(aPositionCache, aAccelCache);
	MCHECKERRORMSG(stat, "attributeAffects: aPositionCache -> aAccelCache");
	stat = attributeAffects(aPositionCache, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aPositionCache -> aOutput");
	stat = attributeAffects(aPositionCache, aOutputX);
	MCHECKERRORMSG(stat, "attributeAffects: aPositionCache -> aOutputX");
	stat = attributeAffects(aPositionCache, aOutputY);
	MCHECKERRORMSG(stat, "attributeAffects: aPositionCache -> aOutputY");
	stat = attributeAffects(aPositionCache, aOutputZ);
	MCHECKERRORMSG(stat, "attributeAffects: aPositionCache -> aOutputZ");

	// The number of waves each impulse causes
	aWaves = nAttr.create("numWaves", "numWaves", MFnNumericData::kDouble, 5.0);
	nAttr.setMin(0);
	stat = addAttribute(aWaves);
	MCHECKERRORMSG(stat, "addAttribute: numWaves");
	stat = attributeAffects(aWaves, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aWaves -> aOutput");
	stat = attributeAffects(aWaves, aOutputX);
	MCHECKERRORMSG(stat, "attributeAffects: aWaves -> aOutputX");
	stat = attributeAffects(aWaves, aOutputY);
	MCHECKERRORMSG(stat, "attributeAffects: aWaves -> aOutputY");
	stat = attributeAffects(aWaves, aOutputZ);
	MCHECKERRORMSG(stat, "attributeAffects: aWaves -> aOutputZ");

	// The length of the waves in frames
	aWaveLength = nAttr.create("waveLength", "waveLength", MFnNumericData::kDouble, 5.0);
	nAttr.setMin(0);
	stat = addAttribute(aWaveLength);
	MCHECKERRORMSG(stat, "addAttribute: waveLength");
	stat = attributeAffects(aWaveLength, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aWaveLength -> aOutput");
	stat = attributeAffects(aWaveLength, aOutputX);
	MCHECKERRORMSG(stat, "attributeAffects: aWaveLength -> aOutputX");
	stat = attributeAffects(aWaveLength, aOutputY);
	MCHECKERRORMSG(stat, "attributeAffects: aWaveLength -> aOutputY");
	stat = attributeAffects(aWaveLength, aOutputZ);
	MCHECKERRORMSG(stat, "attributeAffects: aWaveLength -> aOutputZ");

	// An overall multiplier on the wave amplitude
	aAmplitude = nAttr.create("amplitude", "amplitude", MFnNumericData::kDouble, 1.0);
	nAttr.setKeyable(true);
	stat = addAttribute(aAmplitude);
	MCHECKERRORMSG(stat, "addAttribute: amplitude");
	stat = attributeAffects(aAmplitude, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aAmplitude -> aOutput");
	stat = attributeAffects(aAmplitude, aOutputX);
	MCHECKERRORMSG(stat, "attributeAffects: aAmplitude -> aOutputX");
	stat = attributeAffects(aAmplitude, aOutputY);
	MCHECKERRORMSG(stat, "attributeAffects: aAmplitude -> aOutputY");
	stat = attributeAffects(aAmplitude, aOutputZ);
	MCHECKERRORMSG(stat, "attributeAffects: aAmplitude -> aOutputZ");

	// The per-axis (relative to the reference) amplitude multipliers
	aAxisAmp = nAttr.create("axisAmp", "axisAmp", MFnNumericData::k3Double, 1.0);
	nAttr.setKeyable(true);
	stat = addAttribute(aAxisAmp);
	MCHECKERRORMSG(stat, "addAttribute: axisAmp");
	stat = attributeAffects(aAxisAmp, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aAxisAmp -> aOutput");
	stat = attributeAffects(aAxisAmp, aOutputX);
	MCHECKERRORMSG(stat, "attributeAffects: aAxisAmp -> aOutputX");
	stat = attributeAffects(aAxisAmp, aOutputY);
	MCHECKERRORMSG(stat, "attributeAffects: aAxisAmp -> aOutputY");
	stat = attributeAffects(aAxisAmp, aOutputZ);
	MCHECKERRORMSG(stat, "attributeAffects: aAxisAmp -> aOutputZ");

    // The decay of the waves over time
    aDecay = nAttr.create("decay", "decay", MFnNumericData::kDouble, 3.0);
    nAttr.setKeyable(true);
	nAttr.setMin(0.0);
    stat = addAttribute(aDecay);
    MCHECKERRORMSG(stat, "addAttribute: decay");
	stat = attributeAffects(aDecay, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aDecay -> aOutput");
	stat = attributeAffects(aDecay, aOutputX);
	MCHECKERRORMSG(stat, "attributeAffects: aDecay -> aOutputX");
	stat = attributeAffects(aDecay, aOutputY);
	MCHECKERRORMSG(stat, "attributeAffects: aDecay -> aOutputY");
	stat = attributeAffects(aDecay, aOutputZ);
	MCHECKERRORMSG(stat, "attributeAffects: aDecay -> aOutputZ");

	// The multiplier that the waves decay to
	aTermination = nAttr.create("termination", "termination", MFnNumericData::kDouble, 0.0);
	nAttr.setKeyable(true);
	nAttr.setSoftMin(0.0);
	nAttr.setSoftMax(1.0);
	stat = addAttribute(aTermination);
	MCHECKERRORMSG(stat, "addAttribute: termination");
	stat = attributeAffects(aTermination, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aTermination -> aOutput");
	stat = attributeAffects(aTermination, aOutputX);
	MCHECKERRORMSG(stat, "attributeAffects: aTermination -> aOutputX");
	stat = attributeAffects(aTermination, aOutputY);
	MCHECKERRORMSG(stat, "attributeAffects: aTermination -> aOutputY");
	stat = attributeAffects(aTermination, aOutputZ);
	MCHECKERRORMSG(stat, "attributeAffects: aTermination -> aOutputZ");

	// Scale the amplitude so impulses conserve velocity. Uses absolute value otherwise
	aNormAmp = nAttr.create("normalizeAmplitude", "normalizeAmplitude", MFnNumericData::kBoolean, false);
	stat = addAttribute(aNormAmp);
	MCHECKERRORMSG(stat, "addAttribute: normalizeAmplitude");
	stat = attributeAffects(aNormAmp, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aNormAmp -> aOutput");
	stat = attributeAffects(aNormAmp, aOutputX);
	MCHECKERRORMSG(stat, "attributeAffects: aNormAmp -> aOutputX");
	stat = attributeAffects(aNormAmp, aOutputY);
	MCHECKERRORMSG(stat, "attributeAffects: aNormAmp -> aOutputY");
	stat = attributeAffects(aNormAmp, aOutputZ);
	MCHECKERRORMSG(stat, "attributeAffects: aNormAmp -> aOutputZ");

	// Ignore the acceleration from rest on the first considered frame
	aIgnoreFirst = nAttr.create("ignoreFirstFrame", "ignoreFirstFrame", MFnNumericData::kBoolean, true);
	stat = addAttribute(aIgnoreFirst);
	MCHECKERRORMSG(stat, "addAttribute: ignoreFirstFrame");
	stat = attributeAffects(aIgnoreFirst, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aIgnoreFirst -> aOutput");
	stat = attributeAffects(aIgnoreFirst, aOutputX);
	MCHECKERRORMSG(stat, "attributeAffects: aIgnoreFirst -> aOutputX");
	stat = attributeAffects(aIgnoreFirst, aOutputY);
	MCHECKERRORMSG(stat, "attributeAffects: aIgnoreFirst -> aOutputY");
	stat = attributeAffects(aIgnoreFirst, aOutputZ);
	MCHECKERRORMSG(stat, "attributeAffects: aIgnoreFirst -> aOutputZ");

    // The animatable step size for this frame. REQUIRES RE-SIM TO UPDATE
    aStep = nAttr.create("frequencyMult", "frequencyMult", MFnNumericData::kDouble, 1.0);
    nAttr.setKeyable(true);
    stat = addAttribute(aStep);
    MCHECKERRORMSG(stat, "addAttribute: step");
	stat = attributeAffects(aStep, aPositionCache);
	MCHECKERRORMSG(stat, "attributeAffects: aStep -> aPositionCache");
	stat = attributeAffects(aStep, aAccelCache);
	MCHECKERRORMSG(stat, "attributeAffects: aStep -> aAccelCache");
	stat = attributeAffects(aStep, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aStep -> aOutput");
	stat = attributeAffects(aStep, aOutputX);
	MCHECKERRORMSG(stat, "attributeAffects: aStep -> aOutputX");
	stat = attributeAffects(aStep, aOutputY);
	MCHECKERRORMSG(stat, "attributeAffects: aStep -> aOutputY");
	stat = attributeAffects(aStep, aOutputZ);
	MCHECKERRORMSG(stat, "attributeAffects: aStep -> aOutputZ");

	// The input time value
	aTime = uAttr.create("timeIn", "timeIn", MFnUnitAttribute::kTime);
	uAttr.setKeyable(true);
	stat = addAttribute(aTime);
	MCHECKERRORMSG(stat, "addAttribute: time");
	stat = attributeAffects(aTime, aPositionCache);
	MCHECKERRORMSG(stat, "attributeAffects: aStep -> aPositionCache");
	stat = attributeAffects(aTime, aAccelCache);
	MCHECKERRORMSG(stat, "attributeAffects: aStep -> aAccelCache");
	stat = attributeAffects(aTime, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aStep -> aOutput");
	stat = attributeAffects(aTime, aOutputX);
	MCHECKERRORMSG(stat, "attributeAffects: aStep -> aOutputX");
	stat = attributeAffects(aTime, aOutputY);
	MCHECKERRORMSG(stat, "attributeAffects: aStep -> aOutputY");
	stat = attributeAffects(aTime, aOutputZ);
	MCHECKERRORMSG(stat, "attributeAffects: aStep -> aOutputZ");

	// Flag on whether to update the stored harmonic data
	aUpdate = nAttr.create("update", "update", MFnNumericData::kBoolean, false);
	stat = addAttribute(aUpdate);
	MCHECKERRORMSG(stat, "addAttribute: update");
	stat = attributeAffects(aUpdate, aPositionCache);
	MCHECKERRORMSG(stat, "attributeAffects: aUpdate -> aPositionCache");
	stat = attributeAffects(aUpdate, aAccelCache);
	MCHECKERRORMSG(stat, "attributeAffects: aUpdate -> aAccelCache");
	stat = attributeAffects(aUpdate, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aUpdate -> aOutput");
	stat = attributeAffects(aUpdate, aOutputX);
	MCHECKERRORMSG(stat, "attributeAffects: aUpdate -> aOutputX");
	stat = attributeAffects(aUpdate, aOutputY);
	MCHECKERRORMSG(stat, "attributeAffects: aUpdate -> aOutputY");
	stat = attributeAffects(aUpdate, aOutputZ);
	MCHECKERRORMSG(stat, "attributeAffects: aUpdate -> aOutputZ");

	// Flag on whether to completely clear the cache data
	aClear = nAttr.create("clear", "clear", MFnNumericData::kBoolean, false);
	stat = addAttribute(aClear);
	MCHECKERRORMSG(stat, "addAttribute: clear");
	stat = attributeAffects(aClear, aPositionCache);
	MCHECKERRORMSG(stat, "attributeAffects: aClear -> aPositionCache");
	stat = attributeAffects(aClear, aAccelCache);
	MCHECKERRORMSG(stat, "attributeAffects: aClear -> aAccelCache");
	stat = attributeAffects(aClear, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aClear -> aOutput");
	stat = attributeAffects(aClear, aOutputX);
	MCHECKERRORMSG(stat, "attributeAffects: aClear -> aOutputX");
	stat = attributeAffects(aClear, aOutputY);
	MCHECKERRORMSG(stat, "attributeAffects: aClear -> aOutputY");
	stat = attributeAffects(aClear, aOutputZ);
	MCHECKERRORMSG(stat, "attributeAffects: aClear -> aOutputZ");

	// The worldspace matrix whose space we calculate relative to
    aWorldRefInverse = mAttr.create("worldRefInverse", "worldRefInverse", MFnMatrixAttribute::kDouble);
    stat = addAttribute(aWorldRefInverse);
    mAttr.setHidden(true);
	MCHECKERRORMSG(stat, "addAttribute: worldInverseRef");
	stat = attributeAffects(aWorldRefInverse, aPositionCache);
	MCHECKERRORMSG(stat, "attributeAffects: aWorldRefInverse -> aPositionCache");
	stat = attributeAffects(aWorldRefInverse, aAccelCache);
	MCHECKERRORMSG(stat, "attributeAffects: aWorldRefInverse -> aAccelCache");
	stat = attributeAffects(aWorldRefInverse, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aWorldRefInverse -> aOutput");
	stat = attributeAffects(aWorldRefInverse, aOutputX);
	MCHECKERRORMSG(stat, "attributeAffects: aWorldRefInverse -> aOutputX");
	stat = attributeAffects(aWorldRefInverse, aOutputY);
	MCHECKERRORMSG(stat, "attributeAffects: aWorldRefInverse -> aOutputY");
	stat = attributeAffects(aWorldRefInverse, aOutputZ);
	MCHECKERRORMSG(stat, "attributeAffects: aWorldRefInverse -> aOutputZ");

	// The world input matrix
    aParentInverse = mAttr.create("parentInverse", "parentInverse", MFnMatrixAttribute::kDouble);
    stat = addAttribute(aParentInverse);
    mAttr.setHidden(true);
    MCHECKERRORMSG(stat, "addAttribute: aParentInverse");
	stat = attributeAffects(aParentInverse, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aParentInverse -> aOutput");
	stat = attributeAffects(aParentInverse, aOutputX);
	MCHECKERRORMSG(stat, "attributeAffects: aParentInverse -> aOutputX");
	stat = attributeAffects(aParentInverse, aOutputY);
	MCHECKERRORMSG(stat, "attributeAffects: aParentInverse -> aOutputY");
	stat = attributeAffects(aParentInverse, aOutputZ);
	MCHECKERRORMSG(stat, "attributeAffects: aParentInverse -> aOutputZ");

	// The world input matrix
    aInput = mAttr.create("input", "input", MFnMatrixAttribute::kDouble);
    stat = addAttribute(aInput);
    mAttr.setHidden(true);
    MCHECKERRORMSG(stat, "addAttribute: input");
	stat = attributeAffects(aInput, aPositionCache);
	MCHECKERRORMSG(stat, "attributeAffects: aInput -> aPositionCache");
	stat = attributeAffects(aInput, aAccelCache);
	MCHECKERRORMSG(stat, "attributeAffects: aInput -> aAccelCache");
	stat = attributeAffects(aInput, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aInput -> aOutput");
	stat = attributeAffects(aInput, aOutputX);
	MCHECKERRORMSG(stat, "attributeAffects: aInput -> aOutputX");
	stat = attributeAffects(aInput, aOutputY);
	MCHECKERRORMSG(stat, "attributeAffects: aInput -> aOutputY");
	stat = attributeAffects(aInput, aOutputZ);
	MCHECKERRORMSG(stat, "attributeAffects: aInput -> aOutputZ");

    return MS::kSuccess;
}



MStatus harmonics::clearCaches(MDataBlock& data, MObject& attribute) {
	MStatus status;
	MDataHandle storageH = data.outputValue(attribute, &status);
	MCHECKERROR(status);

	MPxData *pd = storageH.asPluginData();
	HarmCacheMap inMap;
	HarmCacheProxy *inStore, *outStore;
	if (pd == nullptr)
		inStore = (HarmCacheProxy *) HarmCacheProxy::creator();
	else
		inStore = (HarmCacheProxy *) pd;

	// Build the storage output data object
	MFnPluginData fnDataCreator;
	MTypeId tmpid(HarmCacheProxy::id);
	fnDataCreator.create(tmpid, &status);
	MCHECKERROR(status);
	outStore = (HarmCacheProxy*)fnDataCreator.data(&status);
	MCHECKERROR(status);

	// Build the new data
	HarmCacheMap storage;
	outStore->getMap() = storage;
	// Set the output plug
	storageH.setMPxData(outStore);
	return status;
}



MStatus harmonics::compute( const MPlug& plug, MDataBlock& data ){
    MStatus status;

	if( plug == aPositionCache ) {
		MDataHandle updateH = data.inputValue(aUpdate, &status); // doUpdate
		bool update = updateH.asBool();
		if (!update) return status;


		// Input Data
		MDataHandle refInvH = data.inputValue(aWorldRefInverse, &status); // inverse world
		MCHECKERROR(status);
		MDataHandle inputH = data.inputValue(aInput, &status); // local space
		MCHECKERROR(status);
		MDataHandle stepH = data.inputValue(aStep, &status);
		MCHECKERROR(status);
		MDataHandle timeH = data.inputValue(aTime, &status);
		MCHECKERROR(status);

		// Output Data Handles
		MDataHandle storageH = data.outputValue(aPositionCache, &status);
		MCHECKERROR(status);

		// Get simple input data
		auto step = stepH.asDouble();

		MTime time = timeH.asTime();
		double dframe = time.value();
		int frame = (int)dframe;
		// only store integer frames, otherwise things get crazy
		if (dframe == (double)frame) {
			// Get the reference-space translation
			MMatrix mat;

			MMatrix inMat = inputH.asMatrix();
			//MMatrix parMat = parMatH.asMatrix();
			MMatrix refInvMat = refInvH.asMatrix();
			//mat.setToProduct(inMat, parMat);
			mat.setToProduct(inMat, refInvMat);

			//mat.setToProduct(inputH.asMatrix(), parInvH.asMatrix());
			//mat.setToProduct(mat, refInvH.asMatrix());

			// Get the storage input data object
			// Gotta make sure I don't define any of these things in a namespace
			MPxData *pd = storageH.asPluginData();
			HarmCacheMap inMap;
			HarmCacheProxy *inStore, *outStore;
			if (pd == nullptr)
				inStore = (HarmCacheProxy *) HarmCacheProxy::creator();
			else
				inStore = (HarmCacheProxy *) pd;

			// Build the storage output data object
			MFnPluginData fnDataCreator;
			MTypeId tmpid(HarmCacheProxy::id);
			fnDataCreator.create(tmpid, &status);
			MCHECKERROR(status);
			outStore = (HarmCacheProxy*)fnDataCreator.data(&status);
			MCHECKERROR(status);

			// Build the new data
			HarmCacheMap storage = inStore->getMap();
			Vec3 tran;
			double *mtran = mat[3]; // the matrix translation column
			tran[0] = mtran[0]; tran[1] = mtran[1]; tran[2] = mtran[2];
			storage[frame] = std::make_tuple(step, tran);

			outStore->getMap() = storage;
			// Set the output plug
			storageH.setMPxData(outStore);
		}
    } 
    else if( plug == aAccelCache ) {

		MDataHandle updateH = data.inputValue(aUpdate, &status); // doUpdate
		bool update = updateH.asBool();
		if (!update) return status;
		// If the input value is connected already, then I don't need to do anything
		if (plug.isDestination()){

			MDataHandle accelInH = data.inputValue(aAccelCache, &status);
			MDataHandle accelOutH = data.outputValue(aAccelCache, &status);
			accelOutH.copy(accelInH);

			return MS::kSuccess;
		}

		// Input Data
		MDataHandle timeH = data.inputValue(aTime, &status);
		MCHECKERROR(status);
		MTime time = timeH.asTime();
		double dframe = time.value();

		// Only store integer frames otherwise things get crazy
		int frame = (int)dframe;
		if (dframe == double(frame)) {

			// Output Data
			MDataHandle accelOutH = data.outputValue(aAccelCache, &status);
			MCHECKERROR(status);

			// Get input data
			MDataHandle storageH = data.inputValue(aPositionCache, &status);
			MCHECKERROR(status);
			auto storePD = storageH.asPluginData();
			HarmCacheProxy *hcpStore;
			if (storePD == nullptr)
				return MS::kFailure;
			hcpStore = (HarmCacheProxy *)storePD;
			HarmCacheMap &storage = hcpStore->getMap();

			// Get the old value of accel
			HarmCacheProxy *inAccel, *outAccel;
			auto accelPD = accelOutH.asPluginData();
			if (accelPD == nullptr)
				inAccel = (HarmCacheProxy *) HarmCacheProxy::creator();
			else
				inAccel = (HarmCacheProxy *) accelPD;

			// build the output object for accel
			MFnPluginData fnDataCreator;
			MTypeId tmpid(HarmCacheProxy::id);
			fnDataCreator.create(tmpid, &status);
			MCHECKERROR(status);
			outAccel = (HarmCacheProxy*)fnDataCreator.data(&status);
			MCHECKERROR(status);

			// set the data
			HarmCacheMap &accel = inAccel->getMap();
			updateAccel(storage, accel, frame);
			outAccel->getMap() = accel;
			accelOutH.setMPxData(outAccel);
		}
    } 
    else if( plug == aOutput || plug == aOutputX || plug == aOutputY || plug == aOutputZ ) {
		MDataHandle clearH = data.inputValue(aClear, &status); // doUpdate
		bool clear = clearH.asBool();
		if (clear) {
			// If there's no connection asking for it, I have to do this to
			// force computation of the position cache
			MDataHandle storageH = data.inputValue(aPositionCache, &status);
			MCHECKERROR(status);
			status = clearCaches(data, aPositionCache);
			MCHECKERROR(status);
			status = clearCaches(data, aAccelCache);
			MCHECKERROR(status);
			return status;
		}

		// bool
        MDataHandle normAmpH = data.inputValue(aNormAmp, &status);
        MCHECKERROR(status);
		MDataHandle ignoreH = data.inputValue(aIgnoreFirst, &status);
		MCHECKERROR(status);

        // int
        MDataHandle wavesH = data.inputValue(aWaves, &status);
        MCHECKERROR(status);
        MDataHandle waveLengthH = data.inputValue(aWaveLength, &status);
        MCHECKERROR(status);

        // double
        MDataHandle amplitudeH = data.inputValue(aAmplitude, &status);
        MCHECKERROR(status);
        MDataHandle decayH = data.inputValue(aDecay, &status);
        MCHECKERROR(status);
		MDataHandle termH = data.inputValue(aTermination, &status);
		MCHECKERROR(status);
        MDataHandle timeH = data.inputValue(aTime, &status);
        MCHECKERROR(status);
        MDataHandle stepH = data.inputValue(aStep, &status);
        MCHECKERROR(status);

        // k3Double
        MDataHandle axisAmpH = data.inputValue(aAxisAmp, &status);
        MCHECKERROR(status);

        // pluginData
        MDataHandle accelH = data.inputValue(aAccelCache, &status);
        MCHECKERROR(status);

		// Output data
		MDataHandle outH = data.outputValue(aOutput, &status);
		MCHECKERROR(status);

		// Space Switching
		MDataHandle parInvH = data.inputValue(aParentInverse, &status); // local space
		MCHECKERROR(status);

		auto frame = timeH.asTime().value();
		auto waves = wavesH.asDouble();
		auto length = waveLengthH.asDouble();
		auto ampl = amplitudeH.asDouble();
		auto decay = decayH.asDouble();
		auto term = termH.asDouble();
		auto ampAxisRaw = axisAmpH.asDouble3();
		auto normAmp = normAmpH.asBool();
		auto ignoreFirst = ignoreH.asBool();
		auto parInv = parInvH.asMatrix();

		// The output from the harmonic solver is a vec3 offset from the input position
		// aligned to worldspace.
		// Therefore, to put the output in its local space, we only need to handle orientation
		// so we can simply zero out the position terms
		parInv(3, 0) = 0.0; parInv(3, 1) = 0.0; parInv(3, 2) = 0.0; 

		Vec3 ampAxis;
		ampAxis[0] = ampAxisRaw[0]; ampAxis[1] = ampAxisRaw[1]; ampAxis[2] = ampAxisRaw[2];

		MPxData *pd = accelH.asPluginData();
		if (pd != nullptr) {
			auto hcp = dynamic_cast<HarmCacheProxy*>(pd);
			HarmCacheMap &accel = hcp->getMap();
			if (!accel.empty()) {
				Vec3 ret = harmonicSolver(frame, waves, length, ampl, decay, term, ampAxis, !normAmp, accel, ignoreFirst);
				MPoint tt(ret[0], ret[1], ret[2]);
				tt = tt * parInv;
				outH.set3Double(tt[0], tt[1], tt[2]);
			}
			else {
				outH.set3Double(0.0, 0.0, 0.0);
			}
		}
		data.setClean(aOutputX);
		data.setClean(aOutputY);
		data.setClean(aOutputZ);
		data.setClean(aOutput);
    } 
    else {
        return MS::kUnknownParameter;
    }
    return MS::kSuccess;
}
