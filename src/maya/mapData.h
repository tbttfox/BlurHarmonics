#pragma once

#include <maya/MIOStream.h>
#include <maya/MString.h>
#include <maya/MArgList.h>

#include <maya/MPxData.h>
#include <maya/MTypeId.h>

#include <array>
#include <tuple>
#include <unordered_map>

#include "harmonicSolver.h" // for HarmCacheMap and HarmCacheData

class HarmCacheProxy : public MPxData {
public:
    HarmCacheProxy();
    virtual ~HarmCacheProxy();

    // Override methods in MPxData.
    virtual MStatus readASCII(const MArgList&, unsigned& lastElement);
    virtual MStatus readBinary(istream& in, unsigned length);
    virtual MStatus writeASCII(ostream& out);
    virtual MStatus writeBinary(ostream& out);
	virtual void copy(const MPxData& other);

    // Data access
    const HarmCacheMap& getMap() const;
    HarmCacheMap& getMap();

    // static methods and data.
    MTypeId typeId() const; 
    MString name() const;
    static const MString typeName;
    static const MTypeId id;
    static void* creator();

private:
    HarmCacheMap _harmCacheMap;
};

