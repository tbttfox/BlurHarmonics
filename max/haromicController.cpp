#include "harmonicsController.h"

static HarmonicsControllerClassDesc HarmonicsControllerDesc;
ClassDesc2* GetHarmonicsControllerDesc() { return &HarmonicsControllerDesc; }

static ParamBlockDesc2 harmonicscontroller_param_blk (
	harmonicscontroller_params, _T("params"),  0, &HarmonicsControllerDesc,
	P_AUTO_CONSTRUCT | P_AUTO_UI, PBLOCK_REF,

	//rollout
	IDD_PANEL, IDS_PARAMS, 0, 0, NULL,

	pb_numWaves,		_T("numWaves"),		TYPE_FLOAT,	P_ANIMATABLE,	IDS_NUMWAVES,
		p_default,		5.0f,
		p_range,		0.0f,1000.0f,
		p_ui,			TYPE_SPINNER,		EDITTYPE_FLOAT,	IDC_NUMWAVES,	IDC_NUMWAVES_SPIN, 0.01f,
		end,

	pb_waveLength,		_T("waveLength"),	TYPE_FLOAT,	P_ANIMATABLE,	IDS_WAVELENGTH,
		p_default,		5.0f,
		p_range,		0.0f,1000.0f,
		p_ui,			TYPE_SPINNER,		EDITTYPE_FLOAT,	IDC_WAVELENGTH,	IDC_WAVELENGTH_SPIN, 0.01f,
		end,

	pb_amplitude,		_T("amplitude"),	TYPE_FLOAT,	P_ANIMATABLE,	IDS_AMPLITUDE,
		p_default,		1.0f,
		p_range,		0.0f,1000.0f,
		p_ui,			TYPE_SPINNER,		EDITTYPE_FLOAT,	IDC_AMPLITUDE,	IDC_AMPLITUDE_SPIN, 0.01f,
		end,

	pb_ampX,			_T("ampX"),	TYPE_FLOAT,	P_ANIMATABLE,	IDS_AMPX,
		p_default,		1.0f,
		p_range,		0.0f,1000.0f,
		p_ui,			TYPE_SPINNER,		EDITTYPE_FLOAT,	IDC_AMPX,	IDC_AMPX_SPIN, 0.01f,
		end,
	pb_ampY,			_T("ampY"),	TYPE_FLOAT,	P_ANIMATABLE,	IDS_AMPY,
		p_default,		1.0f,
		p_range,		0.0f,1000.0f,
		p_ui,			TYPE_SPINNER,		EDITTYPE_FLOAT,	IDC_AMPY,	IDC_AMPY_SPIN, 0.01f,
		end,
	pb_ampZ,			_T("ampZ"),	TYPE_FLOAT,	P_ANIMATABLE,	IDS_AMPZ,
		p_default,		1.0f,
		p_range,		0.0f,1000.0f,
		p_ui,			TYPE_SPINNER,		EDITTYPE_FLOAT,	IDC_AMPZ,	IDC_AMPZ_SPIN, 0.01f,
		end,

	pb_decay,		_T("decay"),			TYPE_FLOAT,	P_ANIMATABLE,	IDS_DECAY,
		p_default,		3.0f,
		p_range,		0.0f,1000.0f,
		p_ui,			TYPE_SPINNER,		EDITTYPE_FLOAT,	IDC_DECAY,	IDC_DECAY_SPIN, 0.01f,
		end,

	pb_termination,	_T("termination"),		TYPE_FLOAT,	P_ANIMATABLE,	IDS_TERMINATION,
		p_default,		3.0f,
		p_range,		0.0f,1.0f,
		p_ui,			TYPE_SPINNER,		EDITTYPE_FLOAT,	IDC_TERMINATION,	IDC_TERMINATION_SPIN, 0.01f,
		end,

	pb_step,			_T("step"),			TYPE_FLOAT,	P_ANIMATABLE,	IDS_STEP,
		p_default,		1.0f,
		p_range,		0.0f,1000.0f,
		p_ui,			TYPE_SPINNER,		EDITTYPE_FLOAT,	IDC_STEP,	IDC_STEP_SPIN, 0.01f,
		end,

	pb_firstFrame,		_T("firstFrame"),	TYPE_INT,		IDS_FIRSTFRAME,
		p_default,		1,
		p_range,		-1000000, 1000000,
		p_ui,			TYPE_SPINNER,		EDITTYPE_FLOAT,	IDC_FIRSTFRAME,	IDC_FIRSTFRAME_SPIN, 0.01f,
		end,

	pb_ignoreFirst,		_T("ignoreFirst"),	TYPE_BOOL,	P_RESET_DEFAULT,	IDS_IGNOREFIRST,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX,	IDC_IGNOREFIRST_CHECK,
		end,

	pb_normalize,		_T("normalize"),	TYPE_BOOL,	P_RESET_DEFAULT,	IDS_NORMALIZE,
		p_default,		TRUE,
		p_ui,			TYPE_SINGLECHEKBOX,	IDC_NORMALIZE_CHECK,
		end,

	pb_reference,  _T("reference"), 		TYPE_INODE,  0,					IDS_REFERENCE_PICKNODE,
		p_ui, 			TYPE_PICKNODEBUTTON, IDC_REFERENCE_PICKNODE,
		p_end,

	p_end
);

