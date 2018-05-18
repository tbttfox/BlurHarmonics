#include "harmonics.h"
#include <maya/MFnPlugin.h>

MStatus initializePlugin( MObject obj )
{ 
	MStatus   status;
	MFnPlugin plugin(obj, "BlurStudio", "2018", "Any");

	status = plugin.registerData("harmonicCacheData", HarmCacheProxy::id, HarmCacheProxy::creator);
	if (!status) {
		status.perror("registerData");
		return status;
	}

 	status = plugin.registerNode("harmonics", harmonics::id, harmonics::creator, harmonics::initialize);
	if (!status) {
		status.perror("registerNode");
		return status;
	}
	return status;
}

MStatus uninitializePlugin( MObject obj)
{
	MStatus   nodeStat, dataStat;
	MFnPlugin plugin(obj);

	dataStat = plugin.deregisterData(HarmCacheProxy::id);
	if (!dataStat)
		dataStat.perror("deregisterData");

	nodeStat = plugin.deregisterNode(harmonics::id);
	if (!nodeStat)
		nodeStat.perror("deregisterNode");
	
	if (!dataStat)
		return dataStat;

	return nodeStat;
}
