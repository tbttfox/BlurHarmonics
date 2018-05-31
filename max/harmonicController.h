#ifndef __HARMONICCONTROLLER__H
#define __HARMONICCONTROLLER__H

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"

extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

#define HARMONICCONTROLLER_CLASS_ID	Class_ID(0x6ffd6475, 0x7ccd1b7d)
#define PBLOCK_REF	0

enum { harmoniccontroller_params };

enum { pb_mass, pb_strength, pb_dampening, pb_samples, pb_driver, pb_cache, pb_enable, pb_manualsolve };

class HarmonicControlDlgProc;

class HarmonicController : public StdControl {
public:
    // Parameter block
    IParamBlock2	*pblock;	//ref 0
    IParamMap2		*pDialogMap;

    // Loading/Saving
    IOResult Load(ILoad *iload);
    IOResult Save(ISave *isave);

    //From Animatable
    Class_ID ClassID() {return HARMONICCONTROLLER_CLASS_ID;}		
    SClass_ID SuperClassID() { return CTRL_FLOAT_CLASS_ID; }
    void GetClassName(TSTR& s) {s = GetString(IDS_CLASS_NAME);}

    RefTargetHandle Clone( RemapDir &remap );
    RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
        PartID& partID,  RefMessage message);

    int NumSubs() { return 1; }
    TSTR SubAnimName(int i) { return GetString(IDS_PARAMS); }				
    Animatable* SubAnim(int i) { return pblock; }

    // TODO: Maintain the number or references here
    int NumRefs() { return 1; }
    RefTargetHandle GetReference(int i) { return pblock; }
    void SetReference(int i, RefTargetHandle rtarg) { pblock=(IParamBlock2*)rtarg; }

    // return number of ParamBlocks in this instance
    int	NumParamBlocks() { return 1; }
    // return i'th ParamBlock
    IParamBlock2* GetParamBlock(int i) { return pblock; }
    // return id'd ParamBlock
    IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock->ID() == id) ? pblock : NULL; }

    void DeleteThis() { delete this; }		
    //Constructor/Destructor

    HarmonicController();
    ~HarmonicController();		

    // Animatable methods		
    int IsKeyable() {return 1;}		
    BOOL IsAnimated() {return TRUE;}

    void EditTrackParams(
        TimeValue t,
        ParamDimensionBase *dim,
        TCHAR *pname,
        HWND hParent,
        IObjParam *ip,
        DWORD flags);

    int TrackParamsType() {return TRACKPARAMS_WHOLE;}

    // Control methods				
    void Copy(Control *from);
    BOOL IsLeaf() {return TRUE;}
    void CommitValue(TimeValue t) {}
    void RestoreValue(TimeValue t) {}

    void HoldRange();
    Interval GetTimeRange(DWORD flags) {return range;}
    void EditTimeRange(Interval range,DWORD flags);
    void MapKeys(TimeMap *map,DWORD flags );

    Interval range;

    HarmonicControlDlgProc *proc;
    BOOL activeDialog;

    HWND hWnd;

    // Control methods

    // StdControl methods
    void GetValueLocalTime(TimeValue t, void *val, Interval &valid, GetSetMethod method=CTRL_ABSOLUTE);
    void SetValueLocalTime(TimeValue t, void *val, int commit, GetSetMethod method);		
    void Extrapolate(Interval range,TimeValue t,void *val,Interval &valid,int type);		
    void *CreateTempValue() {return new float;}
    void DeleteTempValue(void *val) {delete (float*)val;}
    void ApplyValue(void *val, void *delta) {*((float*)val) += *((float*)delta);}
    void MultiplyValue(void *val, float m) {*((float*)val) *= m;}

    BOOL stopRef;
    BOOL rebuildCache;
};

class HarmonicControllerClassDesc:public ClassDesc2 {
public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return new HarmonicController(); }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME); }
	SClass_ID		SuperClassID() { return CTRL_FLOAT_CLASS_ID; }
	Class_ID		ClassID() { return HARMONICCONTROLLER_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	// returns fixed parsable name (scripter-visible name)
	const TCHAR*	InternalName() { return _T("HarmonicController"); }
	// returns owning module handle
	HINSTANCE		HInstance() { return hInstance; }
};

class HarmonicControlDlgProc: public ParamMap2UserDlgProc {
public:
	HarmonicController *cont;
	BOOL DlgProc(TimeValue t, IParamMap2 *map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void DeleteThis()  {delete this;}
	void SetThing(ReferenceTarget *m) { cont = (HarmonicController *)m;}
};

#endif // __HARMONICCONTROLLER__H
