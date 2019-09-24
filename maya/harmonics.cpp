#include "harmonics.h"
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnPluginData.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnUnitAttribute.h>
#include <maya/MMatrix.h>
#include <maya/MTime.h>


#define MCHECKERROR(STAT)         \
    if (MS::kSuccess != STAT) {   \
        return MS::kFailure;      \
    }

#define MCHECKERRORMSG(STAT, MSG)   \
    if (MS::kSuccess != STAT) {     \
        cerr << MSG << endl;        \
        return MS::kFailure;        \
    }

MTypeId harmonics::id(0x001226F8);

MObject harmonics::aOutput; // vector

MObject harmonics::aReference; // Matrix
MObject harmonics::aParent; // Matrix
MObject harmonics::aInput; // Matrix

MObject harmonics::aPositionCache; // harmonicMap
MObject harmonics::aAccelCache; // harmonicMap
MObject harmonics::aChainCache; // harmonicMap
MObject harmonics::aUpdate; // bool
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
    aOutput = nAttr.create("output", "output", MFnNumericData::k3Double, 0.0);
    nAttr.setWritable(false);
    nAttr.setHidden(true);
    stat = addAttribute(aOutput);
    MCHECKERRORMSG(stat, "addAttribute: output");

	// The custom storage data for this harmonic
	aAccelCache = tAttr.create("accelCache", "accelCache", HarmCacheProxy::id, MObject::kNullObj, &stat);
	MCHECKERRORMSG(stat, "createAttribute: accelCache");
	tAttr.setHidden(true);
	stat = addAttribute(aAccelCache);
	MCHECKERRORMSG(stat, "addAttribute: accelCache");
	stat = attributeAffects(aAccelCache, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aAccel -> aOutput");
	stat = attributeAffects(aAccelCache, aChainCache);
	MCHECKERRORMSG(stat, "attributeAffects: aAccelCache -> aChainCache");

	// The custom storage data for this harmonic
	aPositionCache = tAttr.create("positionCache", "positionCache", HarmCacheProxy::id, MObject::kNullObj, &stat);
	MCHECKERRORMSG(stat, "createAttribute: positionCache");
	tAttr.setHidden(true);
	stat = addAttribute(aPositionCache);
	MCHECKERRORMSG(stat, "addAttribute: positionCache");
	stat = attributeAffects(aPositionCache, aAccelCache);
	MCHECKERRORMSG(stat, "attributeAffects: aStep -> aAccel");
	stat = attributeAffects(aPositionCache, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aStorage -> aOutput");
	stat = attributeAffects(aPositionCache, aChainCache);
	MCHECKERRORMSG(stat, "attributeAffects: aPositionCache -> aChainCache");


	// The custom storage data for this harmonic
	aChainCache = tAttr.create("chainCache", "chainCache", HarmCacheProxy::id, MObject::kNullObj, &stat);
	MCHECKERRORMSG(stat, "createAttribute: chainCache");
	tAttr.setHidden(true);
	stat = addAttribute(aChainCache);
	MCHECKERRORMSG(stat, "addAttribute: chainCache");

	// The number of waves each impulse causes
	aWaves = nAttr.create("numWaves", "numWaves", MFnNumericData::kDouble, 5.0);
	nAttr.setMin(0);
	stat = addAttribute(aWaves);
	MCHECKERRORMSG(stat, "addAttribute: numWaves");
	stat = attributeAffects(aWaves, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aWaves -> aOutput");
	stat = attributeAffects(aWaves, aChainCache);
	MCHECKERRORMSG(stat, "attributeAffects: aWaves -> aChainCache");

	// The length of the waves
	aWaveLength = nAttr.create("waveLength", "waveLength", MFnNumericData::kDouble, 5.0);
	nAttr.setMin(0);
	stat = addAttribute(aWaveLength);
	MCHECKERRORMSG(stat, "addAttribute: waveLength");
	stat = attributeAffects(aWaveLength, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aWaves -> aOutput");
	stat = attributeAffects(aWaveLength, aChainCache);
	MCHECKERRORMSG(stat, "attributeAffects: aWaveLength -> aChainCache");

	// An overall multiplier on the wave amplitude
	aAmplitude = nAttr.create("amplitude", "amplitude", MFnNumericData::kDouble, 1.0);
	nAttr.setKeyable(true);
	stat = addAttribute(aAmplitude);
	MCHECKERRORMSG(stat, "addAttribute: amplitude");
	stat = attributeAffects(aAmplitude, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aAmplitude -> aOutput");
	stat = attributeAffects(aAmplitude, aChainCache);
	MCHECKERRORMSG(stat, "attributeAffects: aAmplitude -> aChainCache");

	// The per-axis (relative to the reference) amplitude multipliers
	aAxisAmp = nAttr.create("axisAmp", "axisAmp", MFnNumericData::k3Double, 1.0);
	nAttr.setKeyable(true);
	stat = addAttribute(aAxisAmp);
	MCHECKERRORMSG(stat, "addAttribute: axisAmp");
	stat = attributeAffects(aAxisAmp, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aAxisAmp -> aOutput");
	stat = attributeAffects(aAxisAmp, aChainCache);
	MCHECKERRORMSG(stat, "attributeAffects: aAxisAmp -> aChainCache");

    // The decay of the waves over time
    aDecay = nAttr.create("decay", "decay", MFnNumericData::kDouble, 3.0);
    nAttr.setKeyable(true);
	nAttr.setMin(0.0);
    stat = addAttribute(aDecay);
    MCHECKERRORMSG(stat, "addAttribute: decay");
	stat = attributeAffects(aDecay, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aDecay -> aOutput");
	stat = attributeAffects(aDecay, aChainCache);
	MCHECKERRORMSG(stat, "attributeAffects: aDecay -> aChainCache");

	// The decay of the waves over time
	aTermination = nAttr.create("termination", "termination", MFnNumericData::kDouble, 0.0);
	nAttr.setKeyable(true);
	nAttr.setSoftMin(0.0);
	nAttr.setSoftMax(1.0);
	stat = addAttribute(aTermination);
	MCHECKERRORMSG(stat, "addAttribute: termination");
	stat = attributeAffects(aTermination, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aTermination -> aOutput");
	stat = attributeAffects(aTermination, aChainCache);
	MCHECKERRORMSG(stat, "attributeAffects: aTermination -> aChainCache");

	// Scale the amplitude so impulses retain velocity. Uses absolute value otherwise
	aNormAmp = nAttr.create("normalizeAmplitude", "normalizeAmplitude", MFnNumericData::kBoolean, false);
	stat = addAttribute(aNormAmp);
	MCHECKERRORMSG(stat, "addAttribute: normalizeAmplitude");
	stat = attributeAffects(aNormAmp, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aAxisAmp -> aOutput");
	stat = attributeAffects(aNormAmp, aChainCache);
	MCHECKERRORMSG(stat, "attributeAffects: aNormAmp -> aChainCache");

	// Scale the amplitude so impulses retain velocity. Uses absolute value otherwise
	aIgnoreFirst = nAttr.create("ignoreFirstFrame", "ignoreFirstFrame", MFnNumericData::kBoolean, true);
	stat = addAttribute(aIgnoreFirst);
	MCHECKERRORMSG(stat, "addAttribute: ignoreFirstFrame");
	stat = attributeAffects(aIgnoreFirst, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aIgnoreFirst -> aOutput");
	stat = attributeAffects(aIgnoreFirst, aChainCache);
	MCHECKERRORMSG(stat, "attributeAffects: aIgnoreFirst -> aChainCache");

    // The step size for this frame. REQUIRES RE-SIM TO UPDATE
    aStep = nAttr.create("step", "step", MFnNumericData::kDouble, 1.0);
    nAttr.setKeyable(true);
    stat = addAttribute(aStep);
    MCHECKERRORMSG(stat, "addAttribute: step");
	stat = attributeAffects(aStep, aPositionCache);
	MCHECKERRORMSG(stat, "attributeAffects: aStep -> aStorage");
	stat = attributeAffects(aStep, aAccelCache);
	MCHECKERRORMSG(stat, "attributeAffects: aStep -> aAccel");
	stat = attributeAffects(aStep, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aStep -> aOutput");
	stat = attributeAffects(aStep, aChainCache);
	MCHECKERRORMSG(stat, "attributeAffects: aStep -> aChainCache");

	// The input time value
	aTime = uAttr.create("timeIn", "timeIn", MFnUnitAttribute::kTime);
	uAttr.setKeyable(true);
	stat = addAttribute(aTime);
	MCHECKERRORMSG(stat, "addAttribute: time");
	stat = attributeAffects(aTime, aPositionCache);
	MCHECKERRORMSG(stat, "attributeAffects: aStep -> aStorage");
	stat = attributeAffects(aTime, aAccelCache);
	MCHECKERRORMSG(stat, "attributeAffects: aStep -> aAccel");
	stat = attributeAffects(aTime, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aStep -> aOutput");
	stat = attributeAffects(aTime, aChainCache);
	MCHECKERRORMSG(stat, "attributeAffects: aTime -> aChainCache");

	// Flag on whether to update the stored harmonic data
	aUpdate = nAttr.create("update", "update", MFnNumericData::kBoolean, false);
	stat = addAttribute(aUpdate);
	MCHECKERRORMSG(stat, "addAttribute: update");
	stat = attributeAffects(aUpdate, aPositionCache);
	MCHECKERRORMSG(stat, "attributeAffects: aStep -> aStorage");
	stat = attributeAffects(aUpdate, aAccelCache);
	MCHECKERRORMSG(stat, "attributeAffects: aStep -> aAccel");
	stat = attributeAffects(aUpdate, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aUpdate -> aOutput");
	stat = attributeAffects(aUpdate, aChainCache);
	MCHECKERRORMSG(stat, "attributeAffects: aUpdate -> aChainCache");

    // The worldspace matrix whose space we calculate relative to
    aReference = mAttr.create("reference", "reference", MFnMatrixAttribute::kDouble);
    stat = addAttribute(aReference);
    mAttr.setHidden(true);
	MCHECKERRORMSG(stat, "addAttribute: reference");
	stat = attributeAffects(aReference, aPositionCache);
	MCHECKERRORMSG(stat, "attributeAffects: aStep -> aStorage");
	stat = attributeAffects(aReference, aAccelCache);
	MCHECKERRORMSG(stat, "attributeAffects: aStep -> aAccel");
	stat = attributeAffects(aReference, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aUpdate -> aOutput");
	stat = attributeAffects(aReference, aChainCache);
	MCHECKERRORMSG(stat, "attributeAffects: aReference -> aChainCache");

	// The local input matrix
    aInput = mAttr.create("input", "input", MFnMatrixAttribute::kDouble);
    stat = addAttribute(aInput);
    mAttr.setHidden(true);
    MCHECKERRORMSG(stat, "addAttribute: input");
	stat = attributeAffects(aInput, aPositionCache);
	MCHECKERRORMSG(stat, "attributeAffects: aStep -> aStorage");
	stat = attributeAffects(aInput, aAccelCache);
	MCHECKERRORMSG(stat, "attributeAffects: aStep -> aAccel");
	stat = attributeAffects(aInput, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aInput -> aOutput");
	stat = attributeAffects(aInput, aChainCache);
	MCHECKERRORMSG(stat, "attributeAffects: aInput -> aChainCache");

    // The world parent inverse matrix so the output can be in local space
    aParent = mAttr.create("parent", "parent", MFnMatrixAttribute::kDouble);
    stat = addAttribute(aParent);
    mAttr.setHidden(true);
    MCHECKERRORMSG(stat, "addAttribute: parent");
	stat = attributeAffects(aParent, aPositionCache);
	MCHECKERRORMSG(stat, "attributeAffects: aStep -> aStorage");
	stat = attributeAffects(aParent, aAccelCache);
	MCHECKERRORMSG(stat, "attributeAffects: aStep -> aAccel");
	stat = attributeAffects(aParent, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: aParent -> aOutput");
	stat = attributeAffects(aParent, aChainCache);
	MCHECKERRORMSG(stat, "attributeAffects: aParent -> aChainCache");

    return MS::kSuccess;
}

MStatus harmonics::compute( const MPlug& plug, MDataBlock& data ){
    MStatus status;

    if( plug == aPositionCache ) {
		// Input Data
		MDataHandle referenceH = data.inputValue(aReference, &status); // inverse world
		MCHECKERROR(status);
		MDataHandle parentH = data.inputValue(aParent, &status); // world space
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
			mat.setToProduct(inputH.asMatrix(), parentH.asMatrix());
			mat.setToProduct(mat, referenceH.asMatrix());

			// Get the storage input data object
			// Gotta make sure I don't define any of these things in a namespace
			auto pd = storageH.asPluginData();
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
			auto storage = inStore->getMap();
			Vec3 tran;
			auto mtran = mat[3]; // the matrix translation column
			tran[0] = mtran[0]; tran[1] = mtran[1]; tran[2] = mtran[2];
			storage[frame] = std::make_tuple(step, tran);

			outStore->getMap() = storage;
			// Set the output plug
			storageH.setMPxData(outStore);
		}
    } 
    else if( plug == aAccelCache ) {
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
    else if( plug == aOutput ) {
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
        MDataHandle accelH = data.outputValue(aAccelCache, &status);
        MCHECKERROR(status);

		// Output data
		MDataHandle outH = data.outputValue(aOutput, &status);
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

		Vec3 ampAxis;
		ampAxis[0] = ampAxisRaw[0];
		ampAxis[1] = ampAxisRaw[1];
		ampAxis[2] = ampAxisRaw[2];

		MPxData *pd = accelH.asPluginData();
		if (pd != nullptr) {
			auto hcp = dynamic_cast<HarmCacheProxy*>(pd);
			HarmCacheMap &accel = hcp->getMap();
			if (!accel.empty()) {
				Vec3 ret = harmonicSolver(frame, waves, length, ampl, decay, term, ampAxis, !normAmp, accel, ignoreFirst);
				outH.set3Double(ret[0], ret[1], ret[2]);
			}
		}
    } 
    else {
        return MS::kUnknownParameter;
    }
    return MS::kSuccess;
}

MStatus harmonics::setDependentsDirty(const MPlug& plug, MPlugArray& plugArray) {
	// Only dirty the cache stuff if update is True
	MPlug upPlug(thisMObject(), aUpdate);
	if (upPlug.asBool()){
		if (plug == aPositionCache){
			plugArray.append(MPlug(thisMObject(), aAccelCache));
		}

		if ( plug == aStep || plug == aTime || plug == aUpdate ||
				plug == aReference || plug == aInput || plug == aParent){

			plugArray.append(MPlug(thisMObject(), aAccelCache));
			plugArray.append(MPlug(thisMObject(), aPositionCache));
		}
	}
	return MPxNode::setDependentsDirty(plug, plugArray);
}
