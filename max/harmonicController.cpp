#include "harmonicController.h";
#include "harmonicSolver.h"

static HarmonicControllerClassDesc HarmonicControllerDesc;
ClassDesc2* GetHarmonicControllerDesc() { return &HarmonicControllerDesc; }

static ParamBlockDesc2 harmoniccontroller_param_blk (
	harmoniccontroller_params, _M("params"),  0, &HarmonicControllerDesc,
	P_AUTO_CONSTRUCT | P_AUTO_UI, PBLOCK_REF,

	//rollout
	IDD_PANEL, IDS_PARAMS, 0, 0, NULL,

	pb_numWaves,		_M("numWaves"),		TYPE_FLOAT,	P_ANIMATABLE,	IDS_NUMWAVES,
		p_default,		5.0f,
		p_range,		0.0f,1000.0f,
		p_ui,			TYPE_SPINNER,		EDITTYPE_FLOAT,	IDC_NUMWAVES,	IDC_NUMWAVES_SPIN, 0.01f,
		p_end,

	pb_waveLength,		_M("waveLength"),	TYPE_FLOAT,	P_ANIMATABLE,	IDS_WAVELENGTH,
		p_default,		5.0f,
		p_range,		0.0f,1000.0f,
		p_ui,			TYPE_SPINNER,		EDITTYPE_FLOAT,	IDC_WAVELENGTH,	IDC_WAVELENGTH_SPIN, 0.01f,
		p_end,

	pb_amplitude,		_M("amplitude"),	TYPE_FLOAT,	P_ANIMATABLE,	IDS_AMPLITUDE,
		p_default,		1.0f,
		p_range,		0.0f,1000.0f,
		p_ui,			TYPE_SPINNER,		EDITTYPE_FLOAT,	IDC_AMPLITUDE,	IDC_AMPLITUDE_SPIN, 0.01f,
		p_end,

	pb_ampX,			_M("ampX"),	TYPE_FLOAT,	P_ANIMATABLE,	IDS_AMPX,
		p_default,		1.0f,
		p_range,		0.0f,1000.0f,
		p_ui,			TYPE_SPINNER,		EDITTYPE_FLOAT,	IDC_AMPX,	IDC_AMPX_SPIN, 0.01f,
		p_end,
	pb_ampY,			_M("ampY"),	TYPE_FLOAT,	P_ANIMATABLE,	IDS_AMPY,
		p_default,		1.0f,
		p_range,		0.0f,1000.0f,
		p_ui,			TYPE_SPINNER,		EDITTYPE_FLOAT,	IDC_AMPY,	IDC_AMPY_SPIN, 0.01f,
		p_end,
	pb_ampZ,			_M("ampZ"),	TYPE_FLOAT,	P_ANIMATABLE,	IDS_AMPZ,
		p_default,		1.0f,
		p_range,		0.0f,1000.0f,
		p_ui,			TYPE_SPINNER,		EDITTYPE_FLOAT,	IDC_AMPZ,	IDC_AMPZ_SPIN, 0.01f,
		p_end,

	pb_decay,			_M("decay"),			TYPE_FLOAT,	P_ANIMATABLE,	IDS_DECAY,
		p_default,		3.0f,
		p_range,		0.0f,1000.0f,
		p_ui,			TYPE_SPINNER,		EDITTYPE_FLOAT,	IDC_DECAY,	IDC_DECAY_SPIN, 0.01f,
		p_end,

	pb_termination,		_M("termination"),		TYPE_FLOAT,	P_ANIMATABLE,	IDS_TERMINATION,
		p_default,		3.0f,
		p_range,		0.0f,1.0f,
		p_ui,			TYPE_SPINNER,		EDITTYPE_FLOAT,	IDC_TERMINATION,	IDC_TERMINATION_SPIN, 0.01f,
		p_end,

	pb_step,			_M("step"),			TYPE_FLOAT,	P_ANIMATABLE,	IDS_STEP,
		p_default,		1.0f,
		p_range,		0.0f,1000.0f,
		p_ui,			TYPE_SPINNER,		EDITTYPE_FLOAT,	IDC_STEP,	IDC_STEP_SPIN, 0.01f,
		p_end,

	pb_firstFrame,		_M("firstFrame"),	TYPE_INT,		IDS_FIRSTFRAME,
		p_default,		1,
		p_range,		-1000000, 1000000,
		p_ui,			TYPE_SPINNER,		EDITTYPE_FLOAT,	IDC_FIRSTFRAME,	IDC_FIRSTFRAME_SPIN, 0.01f,
		p_end,

	pb_ignoreFirst,		_M("ignoreFirst"),	TYPE_BOOL,	P_RESET_DEFAULT,	IDS_IGNOREFIRST,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX,	IDC_IGNOREFIRSTFRAME_CHECK,
		p_end,

	pb_normalize,		_M("normalize"),	TYPE_BOOL,	P_RESET_DEFAULT,	IDS_NORMALIZE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX,	IDC_NORMALIZE_CHECK,
		p_end,

	pb_update,			_M("update"),	TYPE_BOOL,	P_RESET_DEFAULT,	IDS_UPDATE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX,	IDC_UPDATE_CHECK,
		p_end,

	pb_reference,		_M("reference"), TYPE_INODE,  0,	IDS_REFERENCE_PICKNODE,
		p_ui, 			TYPE_PICKNODEBUTTON, IDC_REFERENCE_PICKNODE,
		p_end,

	p_end
);

INT_PTR HarmonicControlDlgProc::DlgProc(TimeValue t, IParamMap2 * map, HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
		case WM_INITDIALOG: {
            cont->hWnd = hWnd;
            break;
        }
		case CC_SPINNER_CHANGE: {
			cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			break;
		}
		case WM_COMMAND: {
            if (LOWORD(wParam) == IDC_OKBUTTON) {
                cont->activeDialog = FALSE;
                DestroyModelessParamMap2(cont->pDialogMap);
            }
			else {
                cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
                GetCOREInterface()->RedrawViews(GetCOREInterface()->GetTime());
            }
            break;
        }
    }
    return FALSE;
}

class RangeRestore: public RestoreObj {
    public: HarmonicController * cont;
    Interval ur,
    rr;
    RangeRestore(HarmonicController * c) {
        cont = c;
        ur = cont->range;
    }
    void Restore(int isUndo) {
        rr = cont->range;
        cont->range = ur;
        cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
    }
    void Redo() {
        cont->range = rr;
        cont->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
    }
    void EndHold() {
        cont->ClearAFlag(A_HELD);
    }
    MSTR Description() {
        return MSTR(_M("harmonic control range"));
    }
};

// make the paramblock
HarmonicController::HarmonicController() {
    HarmonicControllerDesc.MakeAutoParamBlocks(this);
    pDialogMap = NULL;
    proc = NULL;
    activeDialog = FALSE;
    stopRef = FALSE;
    rebuildCache = FALSE;
}

HarmonicController::~HarmonicController() {
	if (activeDialog) DestroyModelessParamMap2(pDialogMap);
    DeleteAllRefsFromMe();
}

void HarmonicController::HoldRange() {
    if (theHold.Holding() && !TestAFlag(A_HELD)) {
        SetAFlag(A_HELD);
        theHold.Put(new RangeRestore(this));
    }
}

void HarmonicController::EditTimeRange(Interval range, DWORD flags) {
    if (!(flags & EDITRANGE_LINKTOKEYS)) {
        HoldRange();
        this->range = range;
        NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
    }
}

void HarmonicController::MapKeys(TimeMap * map, DWORD flags) {
    if (flags & TRACK_MAPRANGE) {
        HoldRange();
        TimeValue t0 = map->map(range.Start());
        TimeValue t1 = map->map(range.End());
        range.Set(t0, t1);
        NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
    }
}

RefTargetHandle HarmonicController::Clone(RemapDir & remap) {
    // make a new harmonic controller and give it our param values.
    HarmonicController * cont = new HarmonicController;
    cont->ReplaceReference(PBLOCK_REF, pblock->Clone(remap));
    // Clone the strength controller
    CloneControl(cont, remap);
    BaseClone(this, cont, remap);
    return cont;
}

void HarmonicController::Copy(Control * from) {
    //pblock->SetController(pb_driver, 0, from);;
}

void HarmonicController::SetValueLocalTime(TimeValue t, void * val, int commit, GetSetMethod method) {}

void HarmonicController::GetValueLocalTime(TimeValue t, void * val, Interval & valid, GetSetMethod method) {
	// TODO: Find where the GetSetMethod comes from.  This controller only works if
	// we know where we're coming from (meaning method *MUST* be CTRL_RELATIVE)

	float numWaves, waveLength, amplitude, ampX, step, ampY, ampZ, decay, termination;
	int firstFrame;
	BOOL update, ignoreFirst, normalize, update;

	pblock->GetValue(pb_update, t, update, FOREVER);
	pblock->GetValue(pb_numWaves, t, update, FOREVER);
	pblock->GetValue(pb_waveLength, t, update, FOREVER);
	pblock->GetValue(pb_amplitude, t, update, FOREVER);
	pblock->GetValue(pb_ampX, t, update, FOREVER);
	pblock->GetValue(pb_ampY, t, update, FOREVER);
	pblock->GetValue(pb_ampZ, t, update, FOREVER);
	pblock->GetValue(pb_decay, t, update, FOREVER);
	pblock->GetValue(pb_termination, t, update, FOREVER);
	pblock->GetValue(pb_step, t, update, FOREVER);
	pblock->GetValue(pb_firstFrame, t, update, FOREVER);
	pblock->GetValue(pb_ignoreFirst, t, update, FOREVER);
	pblock->GetValue(pb_normalize, t, update, FOREVER);
	pblock->GetValue(pb_update, t, update, FOREVER);
	
	double frameTime = (double)t * ((double)GetFrameRate() / (double)TIME_TICKSPERSEC);
	if (update) {
		// Insert a new acceleration key at inserted, and update any adjacent accelerations
		HarmCacheMap cache, accel;
		updateAccel(cache, accel, frameTime);
	}
	// TODO: Set the proper point3 value, not a float
	Point3 f;
	
	// TODO: Get the current offset from the solver for the current time
	double waves, length, ampl, decay, term;
	Vec3 ampAxis, out;
	bool matchVelocity, ignoreInitialAccel;
	HarmCacheMap cache;
	out = harmonicSolver(frameTime, waves, length, ampl, decay,
		term, ampAxis, matchVelocity, cache, ignoreInitialAccel);

    // This controller is always changing.
    valid.SetInstant(t);
	//pblock->GetValue(pb_cache, t, f, FOREVER);
    * ((Point3 * ) val) = f;
}

void HarmonicController::Extrapolate(Interval range, TimeValue t, void * val, Interval & valid, int type) {
	// Out of range is handled by the solver
	// so just pass this back to the getter
	GetValueLocalTime(t, val, valid);
}

RefResult HarmonicController::NotifyRefChanged(const Interval& iv, RefTargetHandle hTarg,
	PartID& partID, RefMessage msg, BOOL propagate) {

    switch (msg) {
		case REFMSG_CHANGE: {
			if (hTarg == pblock) {
				ParamID changing_param = pblock->LastNotifyParamID();
				harmoniccontroller_param_blk.InvalidateUI(changing_param);
				if (activeDialog) {
					UpdateWindow(hWnd);
				}
			}
			break;
		}

		case REFMSG_OBJECT_CACHE_DUMPED: {
			return REF_STOP;
			break;
		}
    }
    return REF_SUCCEED;
}

// TODO: Save my custom cache data
IOResult HarmonicController::Save(ISave * isave) {
    return IO_OK;
}

// TODO: Load my custom cache data
IOResult HarmonicController::Load(ILoad * iload) {
    ULONG nb = 0;
    IOResult res = IO_OK;
    return IO_OK;
}

void HarmonicController::EditTrackParams(
    TimeValue t,
    ParamDimensionBase * dim,
    TCHAR * pname,
    HWND hParent,
    IObjParam * ip,
    DWORD flags) {

    if (!activeDialog) {
        activeDialog = TRUE;
        HarmonicControlDlgProc * proc = new HarmonicControlDlgProc();
        proc->SetThing(this);
        pDialogMap = CreateModelessParamMap2(
            (MapID)harmoniccontroller_params, pblock, t, hInstance, MAKEINTRESOURCE(IDD_PANEL),
            GetCOREInterface()->GetMAXHWnd(), proc);
    }
}

