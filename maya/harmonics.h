#pragma once

#include <maya/MPxNode.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MTypeId.h> 
#include <maya/MEvaluationNode.h>
#include <maya/MFnMessageAttribute.h>

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

public:
	static MObject aOutput; // vector

	static MObject aReference; // Matrix
	static MObject aParent; // Matrix
	static MObject aInput; // Matrix

	static MObject aStorage; // harmonicMap
	static MObject aUpdate; // bool
	static MObject aWaves; // int
	static MObject aWaveLength; // int
	static MObject aAmplitude; // double
	static MObject aAxisAmp; // vector
	static MObject aDecay; // double
	static MObject aMatch; // bool
	static MObject aTime; // double
	static MObject aStep; // double

	static	MTypeId	id;
};

