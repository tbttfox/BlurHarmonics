#include "harmonics.h"
#include <maya/MPlug.h>
#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MFnPluginData.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MMatrix.h>


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

MObject harmonics::aStorage; // harmonicMap
MObject harmonics::aAccel; // harmonicMap
//MObject harmonics::aUpdate; // bool
MObject harmonics::aWaves; // int
MObject harmonics::aWaveLength; // int
MObject harmonics::aAmplitude; // double
MObject harmonics::aAxisAmp; // vector
MObject harmonics::aDecay; // double
MObject harmonics::aMatch; // bool
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
    MStatus stat;

    // Output translation vector
    aOutput = nAttr.create("output", "output", MFnNumericData::k3Double, 0.0);
    nAttr.setWritable(false);
    nAttr.setHidden(true);
    stat = addAttribute(aOutput);
    MCHECKERRORMSG(stat, "addAttribute: output");

	// The custom storage data for this harmonic
	aAccel = tAttr.create("accelStorage", "accelStorage", HarmCacheProxy::id, MObject::kNullObj, &stat);
	MCHECKERRORMSG(stat, "createAttribute: accelStorage");
	tAttr.setCached(false);
	tAttr.setHidden(true);
	stat = addAttribute(aAccel);
	MCHECKERRORMSG(stat, "addAttribute: accelStorage");
	stat = attributeAffects(aAccel, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: accelStorage -> output");

	// The custom storage data for this harmonic
	aStorage = tAttr.create("harmStorage", "harmStorage", HarmCacheProxy::id, MObject::kNullObj, &stat);
	MCHECKERRORMSG(stat, "createAttribute: harmStorage");
	tAttr.setCached(false);
	tAttr.setHidden(true);
	stat = addAttribute(aStorage);
	MCHECKERRORMSG(stat, "addAttribute: harmStorage");
	stat = attributeAffects(aStorage, aAccel);
	MCHECKERRORMSG(stat, "attributeAffects: harmStorage -> accel");

	// The number of waves each impulse causes
	aWaves = nAttr.create("numWaves", "numWaves", MFnNumericData::kInt, 5);
	nAttr.setMin(0);
	stat = addAttribute(aWaves);
	MCHECKERRORMSG(stat, "addAttribute: numWaves");
	attributeAffects(aWaves, aOutput);

	// The length of the waves
	aWaveLength = nAttr.create("waveLength", "waveLength", MFnNumericData::kInt, 5);
	nAttr.setMin(0);
	stat = addAttribute(aWaveLength);
	MCHECKERRORMSG(stat, "addAttribute: waveLength");
	attributeAffects(aWaveLength, aOutput);

	// An overall multiplier on the wave amplitude
	aAmplitude = nAttr.create("amplitude", "amplitude", MFnNumericData::kDouble, 1.0);
	nAttr.setKeyable(true);
	stat = addAttribute(aAmplitude);
	MCHECKERRORMSG(stat, "addAttribute: amplitude");
	stat = attributeAffects(aAmplitude, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: amplitude -> output");

	// The per-axis (relative to the reference) amplitude multipliers
	aAxisAmp = nAttr.create("axisAmp", "axisAmp", MFnNumericData::k3Double, 1.0);
	nAttr.setKeyable(true);
	stat = addAttribute(aAxisAmp);
	MCHECKERRORMSG(stat, "addAttribute: axisAmp");
	stat = attributeAffects(aAxisAmp, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: axisAmp -> output");
	
	// Scale the amplitude so impulses retain velocity. Uses absolute value otherwise
	aMatch = nAttr.create("matchVelocity", "matchVelocity", MFnNumericData::kBoolean, true);
	stat = addAttribute(aMatch);
	MCHECKERRORMSG(stat, "addAttribute: matchVelocity");
	attributeAffects(aMatch, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: matchVelocity -> output");

	// Scale the amplitude so impulses retain velocity. Uses absolute value otherwise
	aIgnoreFirst = nAttr.create("ignoreFirstFrame", "ignoreFirstFrame", MFnNumericData::kBoolean, true);
	stat = addAttribute(aIgnoreFirst);
	MCHECKERRORMSG(stat, "addAttribute: matchVelocity");
	attributeAffects(aIgnoreFirst, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: ignoreFirstFrame -> output");

    // The decay of the waves over time
    aDecay = nAttr.create("decay", "decay", MFnNumericData::kDouble, 5.0);
    nAttr.setKeyable(true);
    stat = addAttribute(aDecay);
    MCHECKERRORMSG(stat, "addAttribute: decay");
    stat = attributeAffects(aDecay, aOutput);
    MCHECKERRORMSG(stat, "attributeAffects: decay -> output");

    // The step size for this frame. REQUIRES RE-SIM TO UPDATE
    aStep = nAttr.create("step", "step", MFnNumericData::kDouble, 1.0);
    nAttr.setKeyable(true);
    stat = addAttribute(aStep);
    MCHECKERRORMSG(stat, "addAttribute: step");
    stat = attributeAffects(aStep, aStorage);
    MCHECKERRORMSG(stat, "attributeAffects: step -> storage");
    stat = attributeAffects(aStep, aOutput);
    MCHECKERRORMSG(stat, "attributeAffects: step -> output");
	
	// The input time value
	aTime = nAttr.create("timeIn", "timeIn", MFnNumericData::kInt, 0);
	nAttr.setKeyable(true);
	nAttr.setCached(false);
	stat = addAttribute(aTime);
	MCHECKERRORMSG(stat, "addAttribute: time");
	stat = attributeAffects(aTime, aStorage);
	MCHECKERRORMSG(stat, "attributeAffects: time -> storage");
	stat = attributeAffects(aTime, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: time -> output");

	/*
	// Flag on whether to update the stored harmonic data
	aUpdate = nAttr.create("update", "update", MFnNumericData::kBoolean, false);
	nAttr.setCached(false);
	stat = addAttribute(aUpdate);
	MCHECKERRORMSG(stat, "addAttribute: update");
	attributeAffects(aUpdate, aStorage);
	MCHECKERRORMSG(stat, "attributeAffects: update -> storage");
	attributeAffects(aUpdate, aOutput);
	MCHECKERRORMSG(stat, "attributeAffects: update -> output");
	*/

    // The worldspace matrix whose space we calculate relative to
    aReference = mAttr.create("reference", "reference", MFnMatrixAttribute::kDouble);
    stat = addAttribute(aReference);
    mAttr.setHidden(true);
	MCHECKERRORMSG(stat, "addAttribute: reference");
	stat = attributeAffects(aReference, aStorage);
	MCHECKERRORMSG(stat, "attributeAffects: reference -> storage");
    stat = attributeAffects(aReference, aOutput);
    MCHECKERRORMSG(stat, "attributeAffects: reference -> output");

	// The local input matrix
    aInput = mAttr.create("input", "input", MFnMatrixAttribute::kDouble);
    stat = addAttribute(aInput);
    mAttr.setHidden(true);
    MCHECKERRORMSG(stat, "addAttribute: input");
	stat = attributeAffects(aInput, aStorage);
	MCHECKERRORMSG(stat, "attributeAffects: input -> storage");
    stat = attributeAffects(aInput, aOutput);
    MCHECKERRORMSG(stat, "attributeAffects: input -> output");

    // The world parent inverse matrix so the output can be in local space
    aParent = mAttr.create("parent", "parent", MFnMatrixAttribute::kDouble);
    stat = addAttribute(aParent);
    mAttr.setHidden(true);
    MCHECKERRORMSG(stat, "addAttribute: parent");
	stat = attributeAffects(aParent, aStorage);
	MCHECKERRORMSG(stat, "attributeAffects: parent -> storage");
    stat = attributeAffects(aParent, aOutput);
    MCHECKERRORMSG(stat, "attributeAffects: parent -> output");

    return MS::kSuccess;
}

MStatus harmonics::compute( const MPlug& plug, MDataBlock& data ){
    MStatus status;

    if( plug == aStorage ) {
		//MDataHandle updateH = data.inputValue(aUpdate, &status);
		//MCHECKERROR(status);
		//auto update = updateH.asBool();
		bool update = true;
		if (update) {
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
			MDataHandle storageH = data.outputValue(aStorage, &status);
			MCHECKERROR(status);

			// Get simple input data
			auto step = stepH.asDouble();
			auto frame = timeH.asInt();

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

    else if( plug == aAccel ) {
		// Input Data
		MDataHandle timeH = data.inputValue(aTime, &status);
		MCHECKERROR(status);
		MDataHandle storageH = data.inputValue(aStorage, &status);
		MCHECKERROR(status);
		// Output Data
		MDataHandle accelH = data.outputValue(aAccel, &status);
		MCHECKERROR(status);

		auto frame = timeH.asInt();

		// Get input data
		auto storePD = storageH.asPluginData();
		HarmCacheProxy *hcpStore;
		if (storePD == nullptr)
			return MS::kFailure;
		hcpStore = (HarmCacheProxy *)storePD;
		HarmCacheMap &storage = hcpStore->getMap();

		// Get the old value of accel
		HarmCacheProxy *inAccel, *outAccel;
		auto accelPD = accelH.asPluginData();
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
		accelH.setMPxData(outAccel);

    } 
    else if( plug == aOutput ) {
		// bool
        MDataHandle matchH = data.inputValue(aMatch, &status);
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
        MDataHandle timeH = data.inputValue(aTime, &status);
        MCHECKERROR(status);
        MDataHandle stepH = data.inputValue(aStep, &status);
        MCHECKERROR(status);

        // k3Double
        MDataHandle axisAmpH = data.inputValue(aAxisAmp, &status);
        MCHECKERROR(status);

        // pluginData
        MDataHandle accelH = data.inputValue(aAccel, &status);
        MCHECKERROR(status);

		// Output data
		MDataHandle outH = data.outputValue(aOutput, &status);
		MCHECKERROR(status);

		auto frame = timeH.asInt();
		auto waves = wavesH.asInt();
		auto length = waveLengthH.asInt();
		auto ampl = amplitudeH.asDouble();
		auto decay = decayH.asDouble();
		auto ampAxisRaw = axisAmpH.asDouble3();
		auto matchVelocity = matchH.asBool();
		auto ignoreFirst = ignoreH.asBool();

		Vec3 ampAxis;
		ampAxis[0] = ampAxisRaw[0];
		ampAxis[1] = ampAxisRaw[1];
		ampAxis[2] = ampAxisRaw[2];

		MPxData *pd = accelH.asPluginData();
		if (pd == nullptr) {
			cerr << "Storage Handle Not initialized" << endl;
			return MS::kFailure;
		}
		auto hcp = dynamic_cast<HarmCacheProxy*>(pd);
		HarmCacheMap &accel = hcp->getMap();
		if (!accel.empty()) {
			Vec3 ret = harmonicSolver(frame, waves, length, ampl, decay, ampAxis, matchVelocity, accel, ignoreFirst);
			outH.set3Double(ret[0], ret[1], ret[2]);
		}
    } 
    else {
        return MS::kUnknownParameter;
    }
    return MS::kSuccess;
}


