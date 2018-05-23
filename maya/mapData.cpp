#include "mapData.h"
#include <sstream>
#include <string>

//typedef HarmCacheMap std::unordered_map<int, HarmCacheData>;
// frame: [step, [xval,  yval,  zval]]
const MTypeId HarmCacheProxy::id(0x001226F7);
const MString HarmCacheProxy::typeName("harmonicMapData");

void* HarmCacheProxy::creator(){
    return new HarmCacheProxy;
}

HarmCacheProxy::HarmCacheProxy() {
	_harmCacheMap = HarmCacheMap();
}

HarmCacheProxy::~HarmCacheProxy() {
}

void HarmCacheProxy::copy(const MPxData& other) {
	if (other.typeId() == HarmCacheProxy::id) {
		const HarmCacheProxy* otherData = (const HarmCacheProxy*) & other;
		_harmCacheMap = otherData->getMap();
	}
	else {
		//  we need to convert to the other type based on its iff Tag
		cerr << "wrong data format!" << endl;
	}
	return;
}


const HarmCacheMap& HarmCacheProxy::getMap() const {
    return _harmCacheMap;
}

HarmCacheMap& HarmCacheProxy::getMap() {
    return _harmCacheMap;
}

MTypeId HarmCacheProxy::typeId() const {
    return HarmCacheProxy::id;
}

MString HarmCacheProxy::name() const { 
    return HarmCacheProxy::typeName; 
}

MStatus HarmCacheProxy::readASCII(const MArgList& args, unsigned& lastParsedElement) {
    MStatus status;
    int argLength = args.length();
    if(argLength <= 0) {return MS::kFailure;} 

    unsigned int numDataRecord = (argLength - lastParsedElement); 

    if (numDataRecord % 5 != 0) {
        cerr << "warning: corrupted data for HarmCacheProxy" << endl;
    }

    numDataRecord /= 5;
    int fnum;
    double d;
	Vec3 ws;

    for (unsigned int i=0; i < numDataRecord; i++) {
        fnum = args.asInt(lastParsedElement++, &status);
        if (MS::kSuccess != status) {return MS::kFailure;}

        d = args.asDouble(lastParsedElement++, &status);
        if (MS::kSuccess != status) {return MS::kFailure;}

        for (unsigned int j=0; j < 3; j++) {
            ws[j] = args.asDouble(lastParsedElement++, &status);
            if (MS::kSuccess != status) {return MS::kFailure;}
        }

        _harmCacheMap[fnum] = std::make_tuple(d, ws);
    }

    return MS::kSuccess;
}

MStatus HarmCacheProxy::readBinary(istream& in, unsigned length) {
    if (length <= 0) {return MS::kFailure;}

    MStatus status;
    unsigned recNum, rc=0;
    in.read((char*) &recNum, sizeof(recNum));
	rc += sizeof(recNum);
    if (in.fail() || recNum == 0) {return MS::kSuccess;}

	int fnum;
    double d;
	Vec3 ws;

    for (unsigned i=0; i < recNum; i++) {

        in.read((char*) &fnum, sizeof(fnum));
		rc += sizeof(fnum);
        if (in.fail()){
			return MS::kFailure;}

        in.read((char*) &d, sizeof(d));
		rc += sizeof(d);
        if (in.fail()){
			return MS::kFailure;}

        for (unsigned j=0; j < 3; j++) {
            in.read((char*) &ws[j], sizeof(ws[j]));
			rc += sizeof(ws[j]);
            if (in.fail()){
				return MS::kFailure;}
        }

        _harmCacheMap[fnum] = std::make_tuple(d, ws);
    }

    return MS::kSuccess;
}

MStatus HarmCacheProxy::writeASCII(ostream& out) {
	// For some reason, maya was crashing if I just
	// wrote directly to out. Writing to myOut, and
	// writing that to out worked though. Whatever
	std::ostringstream myOut;
    for (auto it=_harmCacheMap.begin(); it!=_harmCacheMap.end(); ++it){
        double key = it->first;
        double step = std::get<0>(it->second);
        Vec3 wsp = std::get<1>(it->second);
		double a = wsp[0], b = wsp[1], c = wsp[2];

        myOut << key << " " << step << " " << a << " " << b << " " << c << " ";
        if (myOut.fail()) return MS::kFailure;
    }
	std::string ss = myOut.str();
	out << ss;
	if (out.fail()) return MS::kFailure;
    return MS::kSuccess;
}

MStatus HarmCacheProxy::writeBinary(ostream& out) {
    MStatus status;
    unsigned len = _harmCacheMap.size();
    out.write((char*) &len, sizeof(len));
    if (out.fail()) return MS::kFailure;
    for (auto it=_harmCacheMap.begin(); it!=_harmCacheMap.end(); ++it){
        auto key = it->first;
        auto step = std::get<0>(it->second);
        auto wsp = std::get<1>(it->second);

        out.write((char*) &key, sizeof(key));
        if (out.fail()) return  MS::kFailure; 

        out.write((char*) &step, sizeof(step));
        if (out.fail()) return  MS::kFailure; 

        out.write((char*) &wsp[0], sizeof(wsp[0]));
        if (out.fail()) return  MS::kFailure; 
        out.write((char*) &wsp[1], sizeof(wsp[1]));
        if (out.fail()) return  MS::kFailure; 
        out.write((char*) &wsp[2], sizeof(wsp[2]));
        if (out.fail()) return  MS::kFailure; 
    }
    return MS::kSuccess;
}

