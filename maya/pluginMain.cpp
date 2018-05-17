#include "harmonics.h"
#include <maya/MFnPlugin.h>

MStatus initializePlugin( MObject obj )
{ 
	MStatus   status;
	MFnPlugin plugin( obj, "Blur Studio", "2018", "Any");

	status = plugin.registerNode( "harmonic_maya", harmonics::id, harmonics::creator, harmonics::initialize );
	if (!status) {
		status.perror("registerNode");
		return status;
	}
	return status;
}

MStatus uninitializePlugin( MObject obj)
{
	MStatus   status;
	MFnPlugin plugin( obj );

	status = plugin.deregisterNode( harmonics::id );
	if (!status) {
		status.perror("deregisterNode");
		return status;
	}
	return status;
}
