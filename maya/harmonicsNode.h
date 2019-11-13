#pragma once

#include <maya/MPxNode.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MTypeId.h> 
#include <maya/MEvaluationNode.h>
#include <maya/MFnMessageAttribute.h>
#include <maya/MPlugArray.h>

#include "harmonicSolver.h"
#include "mapData.h"
 
class harmonics : public MPxNode
{
public:
	harmonics();
	virtual	~harmonics(); 

	virtual	MStatus	compute( const MPlug& plug, MDataBlock& data );
	static	void*	creator();
	static	MStatus	initialize();
	MStatus clearCaches(MDataBlock& data, MObject& attribute);

	//virtual MStatus preEvaluation(const  MDGContext& context, const MEvaluationNode& evaluationNode);

public:
	static MObject aOutput; // vector
	static MObject aOutputX; // vector
	static MObject aOutputY; // vector
	static MObject aOutputZ; // vector

	static MObject aWorldRefInverse; // Matrix
	static MObject aParentInverse; // Matrix
	static MObject aInput; // Matrix

	static MObject aPositionCache; // harmonicMap
	static MObject aAccelCache; // harmonicMap
	static MObject aUpdate; // bool
	static MObject aClear; // bool
	static MObject aWaves; // int
	static MObject aWaveLength; // int
	static MObject aAmplitude; // double
	static MObject aAxisAmp; // vector
	static MObject aDecay; // double
	static MObject aTermination; // double
	static MObject aNormAmp; // bool
	static MObject aIgnoreFirst; //bool
	static MObject aTime; // double
	static MObject aStep; // double

	static	MTypeId	id;
};

