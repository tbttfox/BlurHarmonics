#include "mapData.h"

//typedef HarmCacheMap std::unordered_map<int, HarmCacheData>;
// frame: [step, [xval,  yval,  zval]]
const MTypeId HarmCacheProxy::id(0x001226F7);
const MString HarmCacheProxy::typeName("harmonicMapData");

void* HarmCacheProxy::creator(){
    return new HarmCacheProxy;
}

HarmCacheProxy::HarmCacheProxy() {
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

    int numDataRecord = (argLength - lastParsedElement); 

    if (numDataRecord % 5 != 0) {
        cerr << "warning: corrupted data for HarmCacheProxy" << endl;
    }
    numDataRecord /= 5;
    int fnum;
    double d;
    std::array<double, 3> dd;
    for (unsigned int i=0; i < numDataRecord; i++) {
        fnum = args.asInt(lastParsedElement++, &status);
        if (MS::kSuccess != status) {return MS::kFailure;}

        d = args.asDouble(lastParsedElement++, &status);
        if (MS::kSuccess != status) {return MS::kFailure;}

        for (unsigned int j=0; j < 3; j++) {
            dd[j] = args.asDouble(lastParsedElement++, &status);
            if (MS::kSuccess != status) {return MS::kFailure;}
        }
        _harmCacheMap[fnum] = std::make_tuple(d, dd);
    }
    return MS::kSuccess;
}

MStatus HarmCacheProxy::readBinary(istream& in, unsigned length) {
    if (length <= 0) {return MS::kFailure;}

    MStatus status;
    unsigned int recNum;
    in.read((char*) &recNum, sizeof(recNum));
    if (in.fail() || recNum > 0) {return MS::kSuccess;}

    int fnum;
    double d;
    std::array<double, 3> dd;

    for (unsigned int i=0; i < length; i++) {

        in.read((char*) &fnum, sizeof(fnum));
        if (in.fail()){return MS::kFailure;}

        in.read((char*) &d, sizeof(d));
        if (in.fail()){return MS::kFailure;}

        for (unsigned int j=0; j < 3; j++) {
            in.read((char*) &dd[j], sizeof(dd[j]));
            if (in.fail()){return MS::kFailure;}
        }
        _harmCacheMap[fnum] = std::make_tuple(d, dd);
    }
    return MS::kSuccess;
}

MStatus HarmCacheProxy::writeASCII(ostream& out) {
    for (auto it=_harmCacheMap.begin(); it!=_harmCacheMap.end(); ++it){
        auto key = it->first;
        auto step = std::get<0>(it->second);
        auto wsp = std::get<1>(it->second);
        auto accel = std::get<2>(it->second);

        out << key << " ";
        if (out.fail()) return  MS::kFailure;

        out << step << " ";
        if (out.fail()) return  MS::kFailure;

        out << wsp[0] << " ";
        if (out.fail()) return  MS::kFailure;
        out << wsp[1] << " ";
        if (out.fail()) return  MS::kFailure;
        out << wsp[2] << " ";
        if (out.fail()) return  MS::kFailure;

        out << accel[0] << " ";
        if (out.fail()) return  MS::kFailure;
        out << accel[1] << " ";
        if (out.fail()) return  MS::kFailure;
        out << accel[2] << " ";
        if (out.fail()) return  MS::kFailure;
    }
    return MS::kSuccess;
}

MStatus HarmCacheProxy::writeBinary(ostream& out) {
    MStatus status;
    auto len = _harmCacheMap.size();
    out.write((char*) &len, sizeof(len));
    if (out.fail()) return MS::kFailure;
    for (auto it=_harmCacheMap.begin(); it!=_harmCacheMap.end(); ++it){
        auto key = it->first;
        auto step = std::get<0>(it->second);
        auto wsp = std::get<1>(it->second);
        auto accel = std::get<2>(it->second);

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

        out.write((char*) &accel[0], sizeof(accel[0]));
        if (out.fail()) return  MS::kFailure; 
        out.write((char*) &accel[1], sizeof(accel[1]));
        if (out.fail()) return  MS::kFailure; 
        out.write((char*) &accel[2], sizeof(accel[2]));
        if (out.fail()) return  MS::kFailure; 
    }
    return MS::kSuccess;
}

