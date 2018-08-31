%macro export_func 1
   global _%1
   _%1:
%endmacro
%define USE_SECTIONS
%define SAMPLE_RATE	44100
%define MAX_INSTRUMENTS	9
%define MAX_VOICES 2
%define HLD 1
%define BPM 120.000000
%define MAX_PATTERNS 55
%define PATTERN_SIZE_SHIFT 4
%define PATTERN_SIZE (1 << PATTERN_SIZE_SHIFT)
%define	MAX_TICKS (MAX_PATTERNS*PATTERN_SIZE)
%define	SAMPLES_PER_TICK 5512
%define DEF_LFO_NORMALIZE 0.0000453515
%define	MAX_SAMPLES	(SAMPLES_PER_TICK*MAX_TICKS)
;%define 	GO4K_USE_16BIT_OUTPUT
;%define 	GO4K_USE_GROOVE_PATTERN
;%define 	GO4K_USE_ENVELOPE_RECORDINGS
;%define 	GO4K_USE_NOTE_RECORDINGS
%define 	GO4K_CLIP_OUTPUT
%define 	GO4K_USE_DST
%define 	GO4K_USE_DLL
%define 	GO4K_USE_PAN
%define 	GO4K_USE_GLOBAL_DLL
%define 	GO4K_USE_FLD
%define 	GO4K_USE_ENV_CHECK
%define 	GO4K_USE_ENV_MOD_GM
%define 	GO4K_USE_VCO_CHECK
%define 	GO4K_USE_VCO_PHASE_OFFSET
%define 	GO4K_USE_VCO_SHAPE
%define 	GO4K_USE_VCO_MOD_TM
%define 	GO4K_USE_VCO_MOD_DM
%define 	GO4K_USE_VCO_MOD_CM
%define 	GO4K_USE_VCF_CHECK
%define 	GO4K_USE_VCF_MOD_FM
%define 	GO4K_USE_VCF_MOD_RM
%define 	GO4K_USE_VCF_HIGH
%define 	GO4K_USE_VCF_BAND
%define 	GO4K_USE_VCF_PEAK
%define 	GO4K_USE_DST_CHECK
%define		GO4K_USE_DLL_NOTE_SYNC
%define		GO4K_USE_DLL_CHORUS
%define		GO4K_USE_DLL_CHORUS_CLAMP
%define 	GO4K_USE_DLL_DAMP
%define 	GO4K_USE_DLL_DC_FILTER
%define 	GO4K_USE_FSTG_CHECK
%define 	GO4K_USE_PAN_MOD
%define		GO4K_USE_WAVESHAPER_CLIP
%define	MAX_DELAY			65536
%define MAX_UNITS			48
%define	MAX_UNIT_SLOTS	    9
%define GO4K_BEGIN_CMDDEF(def_name)
%define GO4K_END_CMDDEF db 0
%define GO4K_BEGIN_PARAMDEF(def_name)
%define GO4K_END_PARAMDEF
GO4K_ENV_ID		equ		1
%macro	GO4K_ENV 5
	db	%1
	db	%2
	db	%3
	db	%4
	db	%5
%endmacro
%define	ATTAC(val)		val	
%define	DECAY(val)		val	
%define	SUSTAIN(val)	val	
%define	RELEASE(val)	val	
%define	GAIN(val)		val	
struc	go4kENV_val
	.attac		resd	1
	.decay		resd	1
	.sustain	resd	1
	.release	resd	1
	.gain		resd	1
	.size
endstruc
struc	go4kENV_wrk
	.state		resd	1
	.level		resd	1
	.gm			resd	1
	.am			resd	1
	.dm			resd	1
	.sm			resd	1
	.rm			resd	1
	.size
endstruc
%define ENV_STATE_ATTAC		0
%define ENV_STATE_DECAY		1
%define ENV_STATE_SUSTAIN	2
%define ENV_STATE_RELEASE	3
%define ENV_STATE_OFF		4
GO4K_VCO_ID		equ		2
%macro	GO4K_VCO 8
	db	%1
	db	%2
%ifdef GO4K_USE_VCO_PHASE_OFFSET	
	db	%3
%endif	
%ifdef GO4K_USE_VCO_GATE	
	db	%4
%endif	
	db	%5
%ifdef GO4K_USE_VCO_SHAPE	
	db	%6
%endif	
	db	%7
	db	%8
%endmacro
%define	TRANSPOSE(val)	val	
%define	DETUNE(val)		val	
%define	PHASE(val)		val	
%define	GATES(val)		val	
%define	COLOR(val)		val	
%define	SHAPE(val)		val 
%define	FLAGS(val)		val	
%define SINE		0x01
%define TRISAW		0x02
%define PULSE		0x04
%define NOISE		0x08
%define LFO			0x10
%define GATE		0x20
%define	VCO_STEREO	0x40
struc	go4kVCO_val
	.transpose	resd	1
	.detune		resd	1
%ifdef GO4K_USE_VCO_PHASE_OFFSET	
	.phaseofs	resd	1
%endif	
%ifdef GO4K_USE_VCO_GATE	
	.gate		resd	1
%endif	
	.color		resd	1
%ifdef GO4K_USE_VCO_SHAPE	
	.shape		resd	1
%endif	
	.gain		resd	1
	.flags		resd	1	
	.size
endstruc
struc	go4kVCO_wrk
	.phase		resd	1
	.tm			resd	1
	.dm			resd	1
	.fm			resd	1
	.pm			resd	1
	.cm			resd	1
	.sm			resd	1
	.gm			resd	1
	.phase2		resd	1
	.size
endstruc
GO4K_VCF_ID		equ		3
%macro	GO4K_VCF 3
	db	%1
	db	%2
	db	%3
%endmacro
%define LOWPASS		0x1
%define HIGHPASS	0x2
%define BANDPASS	0x4
%define	BANDSTOP	0x3
%define ALLPASS		0x7
%define	PEAK		0x8
%define STEREO		0x10
%define	FREQUENCY(val)	val
%define	RESONANCE(val)	val
%define	VCFTYPE(val)	val
struc	go4kVCF_val
	.freq		resd	1
	.res		resd	1
	.type		resd	1
	.size
endstruc
struc	go4kVCF_wrk
	.low		resd	1
	.high		resd	1
	.band		resd	1
	.freq		resd	1
	.fm			resd	1
	.rm			resd	1
	.low2		resd	1
	.high2		resd	1
	.band2		resd	1
	.size
endstruc
GO4K_DST_ID		equ		4
%macro	GO4K_DST 3
	db	%1
%ifdef GO4K_USE_DST_SH
	db	%2
%ifdef GO4K_USE_DST_STEREO
	db	%3
%endif
%else
%ifdef GO4K_USE_DST_STEREO
	db	%3
%endif
%endif
%endmacro
%define	DRIVE(val)		val
%define	SNHFREQ(val)	val
%define	FLAGS(val)		val
struc	go4kDST_val
	.drive		resd	1
%ifdef GO4K_USE_DST_SH	
	.snhfreq	resd	1
%endif	
	.flags		resd	1
	.size
endstruc
struc	go4kDST_wrk
	.out		resd	1
	.snhphase	resd	1
	.dm			resd	1
	.sm			resd	1
	.out2		resd	1
	.size
endstruc
GO4K_DLL_ID		equ		5
%macro	GO4K_DLL 8
	db	%1
	db	%2
	db	%3
%ifdef GO4K_USE_DLL_DAMP	
	db	%4
%endif		
%ifdef GO4K_USE_DLL_CHORUS	
	db	%5
	db	%6
%endif
	db	%7
	db	%8
%endmacro
%define PREGAIN(val)	val
%define	DRY(val)		val
%define	FEEDBACK(val)	val
%define	DEPTH(val)		val
%define DAMP(val)		val
%define	DELAY(val)		val
%define	COUNT(val)		val
struc	go4kDLL_val
	.pregain	resd	1
	.dry		resd	1
	.feedback	resd	1
%ifdef GO4K_USE_DLL_DAMP	
	.damp		resd	1	
%endif
%ifdef GO4K_USE_DLL_CHORUS
	.freq		resd	1
	.depth
%endif
	.delay		resd	1
	.count		resd	1
	.size
endstruc
struc	go4kDLL_wrk
	.index		resd	1
	.store		resd	1
	.dcin		resd	1
	.dcout		resd	1
%ifdef GO4K_USE_DLL_CHORUS
	.phase		resd	1
%endif
	.buffer		resd	MAX_DELAY
	.size
endstruc
struc	go4kDLL_wrk2
 .pm			resd	1
 .fm			resd	1
 .im			resd	1
 .dm			resd	1
 .sm			resd	1
 .am			resd	1
 .size
endstruc
GO4K_FOP_ID	equ			6
%macro	GO4K_FOP 1
	db	%1
%endmacro
%define	OP(val)			val
%define FOP_POP			0x1
%define FOP_ADDP		0x2
%define FOP_MULP		0x3
%define FOP_PUSH		0x4
%define FOP_XCH			0x5
%define FOP_ADD			0x6
%define FOP_MUL			0x7
%define FOP_ADDP2		0x8
%define FOP_LOADNOTE	0x9
%define FOP_MULP2		0xa
struc	go4kFOP_val
	.flags		resd	1
	.size
endstruc
struc	go4kFOP_wrk
	.size
endstruc
GO4K_FST_ID		equ		7
%macro	GO4K_FST 2
	db	%1
	dw	%2
%endmacro
%define	AMOUNT(val)		val
%define	DEST(val)		val
%define	FST_SET			0x0000
%define	FST_ADD			0x4000
%define	FST_POP			0x8000
struc	go4kFST_val
	.amount		resd	1
	.op1		resd	1
	.size
endstruc
struc	go4kFST_wrk
	.size
endstruc
GO4K_PAN_ID		equ		8
%macro	GO4K_PAN 1
%ifdef GO4K_USE_PAN
	db	%1
%endif
%endmacro
%define	PANNING(val)	val
struc	go4kPAN_val
%ifdef GO4K_USE_PAN
	.panning	resd	1
%endif
	.size
endstruc
struc	go4kPAN_wrk
	.pm			resd	1
	.size
endstruc
GO4K_OUT_ID		equ		9
%macro	GO4K_OUT 2
	db	%1
%ifdef GO4K_USE_GLOBAL_DLL	
	db	%2
%endif	
%endmacro
%define	AUXSEND(val)	val
struc	go4kOUT_val
	.gain		resd	1
%ifdef GO4K_USE_GLOBAL_DLL	
	.auxsend	resd	1
%endif
	.size
endstruc
struc	go4kOUT_wrk
	.am			resd	1
	.gm			resd	1
	.size
endstruc
GO4K_ACC_ID		equ		10
%macro	GO4K_ACC 1
	db	%1
%endmacro
%define OUTPUT			0
%define	AUX				8
%define ACCTYPE(val)	val
struc	go4kACC_val
	.acctype	resd	1
	.size
endstruc
struc	go4kACC_wrk
	.size
endstruc
%ifdef GO4K_USE_FLD
GO4K_FLD_ID	equ		11
%macro	GO4K_FLD 1
	db	%1
%endmacro
%define	VALUE(val)	val
struc	go4kFLD_val
	.value		resd	1
	.size
endstruc
struc	go4kFLD_wrk
	.vm			resd	1
	.size
endstruc
%endif
%ifdef GO4K_USE_FSTG
GO4K_FSTG_ID	equ		12
%macro	GO4K_FSTG 2
	db	%1
	dw	%2
%endmacro
struc	go4kFSTG_val
	.amount		resd	1
	.op1		resd	1
	.size
endstruc
struc	go4kFSTG_wrk
	.size
endstruc
%endif
struc	go4k_instrument
	.release	resd	1
	.note		resd	1
	.workspace	resd	MAX_UNITS*MAX_UNIT_SLOTS
	.dlloutl	resd	1
	.dlloutr	resd	1
	.outl		resd	1
	.outr		resd	1
	.size
endstruc
struc	go4k_synth
	.instruments	resb	go4k_instrument.size * MAX_INSTRUMENTS * MAX_VOICES
	.global			resb	go4k_instrument.size * MAX_VOICES
	.size
endstruc
%ifdef USE_SECTIONS
section .g4kmuc1 data align=1
%else
section .data align=1
%endif
go4k_patterns
	db	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	61, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 0, 0, 61, HLD, 0, 0, 0, 0, 
	db	61, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 0, 0, 61, HLD, 0, 0, 61, 0, 
	db	61, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	61, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 0, 0, 61, HLD, HLD, HLD, 0, 0, 
	db	63, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 0, 0, 63, HLD, HLD, HLD, 0, 0, 
	db	66, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 0, 0, 66, HLD, HLD, HLD, 0, 0, 
	db	66, HLD, HLD, HLD, HLD, HLD, 66, HLD, 68, HLD, 66, HLD, 59, HLD, 0, 0, 
	db	0, 0, 0, 0, 0, 0, 0, 0, 96, HLD, 97, HLD, 92, HLD, 0, 0, 
	db	0, 0, 97, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	0, 0, 0, 0, 0, 0, 0, 0, 96, HLD, 97, HLD, 92, HLD, 97, HLD, 
	db	60, HLD, HLD, HLD, HLD, HLD, 60, HLD, HLD, HLD, 60, HLD, HLD, HLD, 60, HLD, 
	db	HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	0, 0, 37, HLD, 37, HLD, HLD, HLD, 37, HLD, HLD, HLD, 37, HLD, HLD, HLD, 
	db	HLD, HLD, 37, HLD, HLD, HLD, HLD, HLD, 37, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	HLD, HLD, 39, HLD, 39, HLD, HLD, HLD, 39, HLD, HLD, HLD, 39, HLD, HLD, HLD, 
	db	HLD, HLD, 39, HLD, HLD, HLD, HLD, HLD, 39, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	HLD, HLD, 42, HLD, 42, HLD, HLD, HLD, 42, HLD, HLD, HLD, 42, HLD, HLD, HLD, 
	db	HLD, HLD, 42, HLD, HLD, HLD, HLD, HLD, 42, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	HLD, HLD, 37, HLD, 37, HLD, HLD, HLD, 37, HLD, HLD, HLD, 37, HLD, HLD, HLD, 
	db	0, 0, 0, 0, 60, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 60, HLD, HLD, HLD, 
	db	HLD, HLD, HLD, HLD, 60, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 60, HLD, HLD, HLD, 
	db	HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 84, HLD, 60, HLD, 
	db	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 84, 0, 60, HLD, HLD, HLD, 
	db	HLD, HLD, HLD, HLD, 60, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 60, HLD, 108, HLD, 
	db	80, HLD, 0, 80, HLD, HLD, 87, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 89, HLD, 90, HLD, 89, HLD, 87, HLD, 
	db	HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 85, HLD, 87, HLD, 85, HLD, 79, HLD, 
	db	78, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 85, HLD, 84, HLD, 85, HLD, HLD, HLD, 
	db	77, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 
	db	HLD, HLD, HLD, HLD, HLD, HLD, HLD, HLD, 0, 0, 0, 0, 0, 0, 0, 0, 
	db	80, 85, 89, 0, 0, 92, 0, 0, 87, 0, 0, 85, 0, 0, 80, 0, 
	db	79, 82, 87, 0, 0, 92, 0, 0, 87, 0, 0, 0, 0, 0, 0, 0, 
	db	77, 80, 85, 0, 0, 82, 0, 0, 85, 0, 0, 87, 0, 0, 80, 0, 
	db	87, 0, 85, 0, 80, 0, 0, 0, 87, 0, 85, 0, 80, 0, 0, 0, 
	db	87, 0, 85, 0, 79, 0, 0, 0, 87, 0, 85, 0, 79, 0, 0, 0, 
	db	87, 0, 85, 0, 78, 0, 0, 0, 87, 0, 85, 0, 78, 0, 0, 0, 
	db	87, 0, 85, 0, 77, 0, 0, 0, 87, 0, 85, 0, 77, 0, 0, 0, 
	db	0, 0, 72, HLD, HLD, HLD, 72, HLD, HLD, HLD, 72, HLD, HLD, HLD, 72, HLD, 
	db	HLD, HLD, 72, HLD, HLD, HLD, 72, HLD, HLD, HLD, 72, HLD, HLD, HLD, 72, HLD, 
	db	0, 0, 0, 0, 72, HLD, 0, 0, 0, 0, 0, 0, 60, HLD, 0, 0, 
	db	0, 0, 0, 0, 48, HLD, 0, 0, 0, 0, 0, 0, 72, HLD, 0, 0, 
	db	0, 0, 0, 0, 48, HLD, 0, 0, 0, 0, 36, 48, 72, HLD, 96, HLD, 
	db	0, 0, 0, 0, 48, HLD, 0, 0, 0, 0, 0, 84, 72, HLD, 84, HLD, 
go4k_patterns_end
%ifdef USE_SECTIONS
section .g4kmuc2 data align=1
%else
section .data
%endif
go4k_pattern_lists
Instrument3List		db	0, 0, 0, 0, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 3, 4, 2, 5, 5, 6, 7, 4, 4, 4, 2, 5, 5, 6, 7, 4, 4, 4, 2, 5, 5, 6, 7, 4, 4, 4, 2, 5, 5, 6, 7, 4, 4, 0, 0, 0, 0, 0, 0, 0, 
Instrument4List		db	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 9, 8, 9, 8, 9, 8, 9, 10, 9, 10, 9, 0, 0, 0, 
Instrument5List		db	0, 0, 0, 0, 0, 0, 0, 0, 11, 11, 11, 11, 12, 12, 12, 12, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 0, 0, 0, 
Instrument6List		db	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 13, 14, 15, 16, 17, 18, 19, 14, 19, 14, 15, 16, 17, 18, 19, 14, 19, 14, 15, 16, 17, 18, 19, 14, 12, 12, 12, 12, 0, 0, 0, 
Instrument7List		db	0, 0, 0, 0, 0, 0, 0, 0, 20, 21, 21, 21, 12, 12, 12, 22, 21, 21, 21, 21, 21, 21, 21, 21, 0, 0, 0, 0, 0, 0, 0, 23, 21, 21, 21, 21, 21, 21, 21, 24, 21, 21, 21, 21, 21, 21, 21, 21, 12, 12, 12, 12, 0, 0, 0, 
Instrument8List		db	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 25, 26, 12, 27, 28, 29, 30, 31, 25, 26, 12, 27, 28, 29, 30, 31, 0, 0, 0, 0, 0, 0, 0, 
Instrument10List		db	32, 33, 34, 33, 32, 33, 34, 33, 32, 33, 34, 33, 0, 0, 0, 0, 35, 35, 36, 36, 37, 37, 38, 38, 35, 35, 36, 36, 37, 37, 38, 38, 35, 35, 36, 36, 37, 37, 38, 38, 35, 35, 36, 36, 37, 37, 38, 38, 38, 38, 0, 0, 0, 0, 0, 
Instrument13List		db	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 39, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 12, 12, 0, 0, 0, 0, 0, 
Instrument14List		db	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 41, 42, 41, 42, 41, 42, 41, 43, 0, 0, 0, 0, 0, 0, 0, 0, 41, 42, 41, 42, 41, 42, 41, 44, 12, 12, 12, 12, 0, 0, 0, 
go4k_pattern_lists_end
%ifdef USE_SECTIONS
section .g4kmuc3 data align=1
%else
section .data
%endif
go4k_synth_instructions
GO4K_BEGIN_CMDDEF(Instrument3)
	db GO4K_ENV_ID
	db GO4K_FOP_ID
	db GO4K_ENV_ID
	db GO4K_VCO_ID
	db GO4K_FOP_ID
	db GO4K_FOP_ID
	db GO4K_VCO_ID
	db GO4K_FOP_ID
	db GO4K_FOP_ID
	db GO4K_FST_ID
	db GO4K_PAN_ID
	db GO4K_OUT_ID
GO4K_END_CMDDEF
GO4K_BEGIN_CMDDEF(Instrument4)
	db GO4K_ENV_ID
	db GO4K_FOP_ID
	db GO4K_VCO_ID
	db GO4K_FOP_ID
	db GO4K_VCO_ID
	db GO4K_FOP_ID
	db GO4K_FOP_ID
	db GO4K_VCF_ID
	db GO4K_FST_ID
	db GO4K_PAN_ID
	db GO4K_OUT_ID
GO4K_END_CMDDEF
GO4K_BEGIN_CMDDEF(Instrument5)
	db GO4K_ENV_ID
	db GO4K_FST_ID
	db GO4K_ENV_ID
	db GO4K_DST_ID
	db GO4K_FST_ID
	db GO4K_FOP_ID
	db GO4K_VCO_ID
	db GO4K_FOP_ID
	db GO4K_PAN_ID
	db GO4K_OUT_ID
GO4K_END_CMDDEF
GO4K_BEGIN_CMDDEF(Instrument6)
	db GO4K_ENV_ID
	db GO4K_VCO_ID
	db GO4K_VCO_ID
	db GO4K_FOP_ID
	db GO4K_FOP_ID
	db GO4K_FOP_ID
	db GO4K_VCO_ID
	db GO4K_FST_ID
	db GO4K_FLD_ID
	db GO4K_FOP_ID
	db GO4K_FOP_ID
	db GO4K_VCF_ID
	db GO4K_DLL_ID
	db GO4K_VCO_ID
	db GO4K_FST_ID
	db GO4K_VCO_ID
	db GO4K_FST_ID
	db GO4K_PAN_ID
	db GO4K_OUT_ID
GO4K_END_CMDDEF
GO4K_BEGIN_CMDDEF(Instrument7)
	db GO4K_ENV_ID
	db GO4K_FST_ID
	db GO4K_ENV_ID
	db GO4K_FST_ID
	db GO4K_FST_ID
	db GO4K_FOP_ID
	db GO4K_VCO_ID
	db GO4K_VCO_ID
	db GO4K_VCO_ID
	db GO4K_VCF_ID
	db GO4K_FOP_ID
	db GO4K_FOP_ID
	db GO4K_FOP_ID
	db GO4K_VCF_ID
	db GO4K_VCF_ID
	db GO4K_VCF_ID
	db GO4K_DLL_ID
	db GO4K_PAN_ID
	db GO4K_OUT_ID
GO4K_END_CMDDEF
GO4K_BEGIN_CMDDEF(Instrument8)
	db GO4K_ENV_ID
	db GO4K_FST_ID
	db GO4K_VCO_ID
	db GO4K_VCO_ID
	db GO4K_VCO_ID
	db GO4K_FST_ID
	db GO4K_FST_ID
	db GO4K_FOP_ID
	db GO4K_FOP_ID
	db GO4K_FOP_ID
	db GO4K_VCF_ID
	db GO4K_VCF_ID
	db GO4K_DST_ID
	db GO4K_PAN_ID
	db GO4K_DLL_ID
	db GO4K_FOP_ID
	db GO4K_DLL_ID
	db GO4K_FOP_ID
	db GO4K_VCF_ID
	db GO4K_OUT_ID
GO4K_END_CMDDEF
GO4K_BEGIN_CMDDEF(Instrument10)
	db GO4K_ENV_ID
	db GO4K_VCO_ID
	db GO4K_FOP_ID
	db GO4K_VCF_ID
	db GO4K_ENV_ID
	db GO4K_FST_ID
	db GO4K_ENV_ID
	db GO4K_FOP_ID
	db GO4K_FST_ID
	db GO4K_VCO_ID
	db GO4K_FST_ID
	db GO4K_FST_ID
	db GO4K_FOP_ID
	db GO4K_VCF_ID
	db GO4K_FST_ID
	db GO4K_DLL_ID
	db GO4K_PAN_ID
	db GO4K_OUT_ID
GO4K_END_CMDDEF
GO4K_BEGIN_CMDDEF(Instrument13)
	db GO4K_ENV_ID
	db GO4K_VCO_ID
	db GO4K_FOP_ID
	db GO4K_VCF_ID
	db GO4K_VCF_ID
	db GO4K_PAN_ID
	db GO4K_DLL_ID
	db GO4K_FOP_ID
	db GO4K_DLL_ID
	db GO4K_FOP_ID
	db GO4K_OUT_ID
GO4K_END_CMDDEF
GO4K_BEGIN_CMDDEF(Instrument14)
	db GO4K_ENV_ID
	db GO4K_VCO_ID
	db GO4K_FOP_ID
	db GO4K_ENV_ID
	db GO4K_VCO_ID
	db GO4K_VCF_ID
	db GO4K_FOP_ID
	db GO4K_FOP_ID
	db GO4K_VCF_ID
	db GO4K_ENV_ID
	db GO4K_FST_ID
	db GO4K_ENV_ID
	db GO4K_FST_ID
	db GO4K_DST_ID
	db GO4K_VCF_ID
	db GO4K_DLL_ID
	db GO4K_ENV_ID
	db GO4K_FOP_ID
	db GO4K_PAN_ID
	db GO4K_OUT_ID
GO4K_END_CMDDEF
GO4K_BEGIN_CMDDEF(Global)
	db GO4K_ACC_ID
	db GO4K_DLL_ID
	db GO4K_FOP_ID
	db GO4K_DLL_ID
	db GO4K_FOP_ID
	db GO4K_ACC_ID
	db GO4K_FOP_ID
	db GO4K_OUT_ID
GO4K_END_CMDDEF
go4k_synth_instructions_end
%ifdef USE_SECTIONS
section .g4kmuc4 data align=1
%else
section .data
%endif
go4k_synth_parameter_values
GO4K_BEGIN_PARAMDEF(Instrument3)
	GO4K_ENV	ATTAC(30),DECAY(80),SUSTAIN(128),RELEASE(64),GAIN(128)
	GO4K_FOP	OP(FOP_PUSH)
	GO4K_ENV	ATTAC(11),DECAY(80),SUSTAIN(128),RELEASE(64),GAIN(128)
	GO4K_VCO	TRANSPOSE(52),DETUNE(64),PHASE(30),GATES(0),COLOR(64),SHAPE(64),GAIN(128),FLAGS(TRISAW)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_FOP	OP(FOP_ADDP)
	GO4K_VCO	TRANSPOSE(52),DETUNE(69),PHASE(0),GATES(0),COLOR(128),SHAPE(64),GAIN(128),FLAGS(SINE)
	GO4K_FOP	OP(FOP_ADDP)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_FST	AMOUNT(58),DEST(10*MAX_UNIT_SLOTS+0+FST_SET)
	GO4K_PAN	PANNING(94)
	GO4K_OUT	GAIN(68), AUXSEND(16)
GO4K_END_PARAMDEF
GO4K_BEGIN_PARAMDEF(Instrument4)
	GO4K_ENV	ATTAC(18),DECAY(80),SUSTAIN(128),RELEASE(64),GAIN(128)
	GO4K_FOP	OP(FOP_PUSH)
	GO4K_VCO	TRANSPOSE(52),DETUNE(64),PHASE(30),GATES(0),COLOR(99),SHAPE(64),GAIN(128),FLAGS(TRISAW)
	GO4K_FOP	OP(FOP_ADDP)
	GO4K_VCO	TRANSPOSE(52),DETUNE(67),PHASE(0),GATES(0),COLOR(128),SHAPE(64),GAIN(128),FLAGS(SINE)
	GO4K_FOP	OP(FOP_ADDP)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_VCF	FREQUENCY(64),RESONANCE(64),VCFTYPE(LOWPASS)
	GO4K_FST	AMOUNT(58),DEST(9*MAX_UNIT_SLOTS+0+FST_SET)
	GO4K_PAN	PANNING(64)
	GO4K_OUT	GAIN(54), AUXSEND(30)
GO4K_END_PARAMDEF
GO4K_BEGIN_PARAMDEF(Instrument5)
	GO4K_ENV	ATTAC(0),DECAY(80),SUSTAIN(0),RELEASE(56),GAIN(27)
	GO4K_FST	AMOUNT(128),DEST(0*MAX_UNIT_SLOTS+2+FST_SET)
	GO4K_ENV	ATTAC(0),DECAY(64),SUSTAIN(41),RELEASE(106),GAIN(128)
	GO4K_DST	DRIVE(29), SNHFREQ(128), FLAGS(0)
	GO4K_FST	AMOUNT(77),DEST(6*MAX_UNIT_SLOTS+1+FST_SET)
	GO4K_FOP	OP(FOP_POP)
	GO4K_VCO	TRANSPOSE(45),DETUNE(64),PHASE(32),GATES(85),COLOR(128),SHAPE(66),GAIN(128),FLAGS(SINE)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_PAN	PANNING(64)
	GO4K_OUT	GAIN(60), AUXSEND(0)
GO4K_END_PARAMDEF
GO4K_BEGIN_PARAMDEF(Instrument6)
	GO4K_ENV	ATTAC(0),DECAY(74),SUSTAIN(0),RELEASE(74),GAIN(49)
	GO4K_VCO	TRANSPOSE(64),DETUNE(60),PHASE(96),GATES(85),COLOR(128),SHAPE(127),GAIN(128),FLAGS(TRISAW)
	GO4K_VCO	TRANSPOSE(64),DETUNE(81),PHASE(64),GATES(85),COLOR(124),SHAPE(96),GAIN(128),FLAGS(TRISAW)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_FOP	OP(FOP_PUSH)
	GO4K_VCO	TRANSPOSE(64),DETUNE(64),PHASE(0),GATES(0),COLOR(128),SHAPE(64),GAIN(128),FLAGS(TRISAW|LFO)
	GO4K_FST	AMOUNT(79),DEST(2*MAX_UNIT_SLOTS+2+FST_SET+FST_POP)
	GO4K_FLD	VALUE(128)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_FOP	OP(FOP_ADDP)
	GO4K_VCF	FREQUENCY(48),RESONANCE(28),VCFTYPE(LOWPASS)
	GO4K_DLL	PREGAIN(75),DRY(115),FEEDBACK(68),DAMP(109),FREQUENCY(8),DEPTH(1),DELAY(17),COUNT(1)
	GO4K_VCO	TRANSPOSE(37),DETUNE(64),PHASE(0),GATES(0),COLOR(128),SHAPE(64),GAIN(128),FLAGS(SINE|LFO)
	GO4K_FST	AMOUNT(52),DEST(11*MAX_UNIT_SLOTS+4+FST_SET+FST_POP)
	GO4K_VCO	TRANSPOSE(50),DETUNE(64),PHASE(0),GATES(0),COLOR(128),SHAPE(64),GAIN(128),FLAGS(TRISAW|LFO)
	GO4K_FST	AMOUNT(91),DEST(13*MAX_UNIT_SLOTS+1+FST_SET+FST_POP)
	GO4K_PAN	PANNING(64)
	GO4K_OUT	GAIN(100), AUXSEND(50)
GO4K_END_PARAMDEF
GO4K_BEGIN_PARAMDEF(Instrument7)
	GO4K_ENV	ATTAC(0),DECAY(85),SUSTAIN(0),RELEASE(72),GAIN(128)
	GO4K_FST	AMOUNT(128),DEST(0*MAX_UNIT_SLOTS+2+FST_SET)
	GO4K_ENV	ATTAC(0),DECAY(56),SUSTAIN(0),RELEASE(0),GAIN(128)
	GO4K_FST	AMOUNT(108),DEST(6*MAX_UNIT_SLOTS+1+FST_SET)
	GO4K_FST	AMOUNT(72),DEST(7*MAX_UNIT_SLOTS+1+FST_SET)
	GO4K_FOP	OP(FOP_POP)
	GO4K_VCO	TRANSPOSE(64),DETUNE(64),PHASE(0),GATES(85),COLOR(128),SHAPE(32),GAIN(64),FLAGS(SINE)
	GO4K_VCO	TRANSPOSE(64),DETUNE(64),PHASE(0),GATES(85),COLOR(128),SHAPE(80),GAIN(64),FLAGS(SINE)
	GO4K_VCO	TRANSPOSE(64),DETUNE(64),PHASE(0),GATES(85),COLOR(64),SHAPE(64),GAIN(64),FLAGS(NOISE)
	GO4K_VCF	FREQUENCY(85),RESONANCE(128),VCFTYPE(LOWPASS)
	GO4K_FOP	OP(FOP_ADDP)
	GO4K_FOP	OP(FOP_ADDP)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_VCF	FREQUENCY(23),RESONANCE(32),VCFTYPE(HIGHPASS)
	GO4K_VCF	FREQUENCY(80),RESONANCE(54),VCFTYPE(HIGHPASS)
	GO4K_VCF	FREQUENCY(56),RESONANCE(3),VCFTYPE(ALLPASS)
	GO4K_DLL	PREGAIN(45),DRY(128),FEEDBACK(60),DAMP(64),FREQUENCY(0),DEPTH(0),DELAY(-1),COUNT(1)
	GO4K_PAN	PANNING(64)
	GO4K_OUT	GAIN(3), AUXSEND(0)
GO4K_END_PARAMDEF
GO4K_BEGIN_PARAMDEF(Instrument8)
	GO4K_ENV	ATTAC(10),DECAY(96),SUSTAIN(96),RELEASE(36),GAIN(90)
	GO4K_FST	AMOUNT(64),DEST(0*MAX_UNIT_SLOTS+2+FST_SET)
	GO4K_VCO	TRANSPOSE(64),DETUNE(60),PHASE(32),GATES(85),COLOR(106),SHAPE(64),GAIN(128),FLAGS(PULSE)
	GO4K_VCO	TRANSPOSE(64),DETUNE(72),PHASE(32),GATES(85),COLOR(96),SHAPE(64),GAIN(128),FLAGS(TRISAW)
	GO4K_VCO	TRANSPOSE(44),DETUNE(64),PHASE(0),GATES(85),COLOR(128),SHAPE(90),GAIN(128),FLAGS(SINE|LFO)
	GO4K_FST	AMOUNT(68),DEST(2*MAX_UNIT_SLOTS+2+FST_SET)
	GO4K_FST	AMOUNT(61),DEST(3*MAX_UNIT_SLOTS+2+FST_SET)
	GO4K_FOP	OP(FOP_POP)
	GO4K_FOP	OP(FOP_ADDP)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_VCF	FREQUENCY(26),RESONANCE(128),VCFTYPE(PEAK)
	GO4K_VCF	FREQUENCY(64),RESONANCE(64),VCFTYPE(LOWPASS)
	GO4K_DST	DRIVE(105), SNHFREQ(128), FLAGS(0)
	GO4K_PAN	PANNING(21)
	GO4K_DLL	PREGAIN(96),DRY(128),FEEDBACK(96),DAMP(64),FREQUENCY(0),DEPTH(0),DELAY(-1),COUNT(1)
	GO4K_FOP	OP(FOP_XCH)
	GO4K_DLL	PREGAIN(108),DRY(128),FEEDBACK(64),DAMP(96),FREQUENCY(0),DEPTH(0),DELAY(17),COUNT(1)
	GO4K_FOP	OP(FOP_XCH)
	GO4K_VCF	FREQUENCY(70),RESONANCE(77),VCFTYPE(LOWPASS)
	GO4K_OUT	GAIN(31), AUXSEND(2)
GO4K_END_PARAMDEF
GO4K_BEGIN_PARAMDEF(Instrument10)
	GO4K_ENV	ATTAC(3),DECAY(92),SUSTAIN(3),RELEASE(82),GAIN(128)
	GO4K_VCO	TRANSPOSE(64),DETUNE(58),PHASE(0),GATES(0),COLOR(75),SHAPE(64),GAIN(128),FLAGS(TRISAW)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_VCF	FREQUENCY(44),RESONANCE(128),VCFTYPE(LOWPASS)
	GO4K_ENV	ATTAC(64),DECAY(64),SUSTAIN(0),RELEASE(64),GAIN(128)
	GO4K_FST	AMOUNT(78),DEST(3*MAX_UNIT_SLOTS+4+FST_SET+FST_POP)
	GO4K_ENV	ATTAC(80),DECAY(78),SUSTAIN(12),RELEASE(82),GAIN(128)
	GO4K_FOP	OP(FOP_ADDP)
	GO4K_FST	AMOUNT(23),DEST(1*MAX_UNIT_SLOTS+5+FST_SET)
	GO4K_VCO	TRANSPOSE(68),DETUNE(64),PHASE(0),GATES(0),COLOR(128),SHAPE(64),GAIN(128),FLAGS(SINE|LFO)
	GO4K_FST	AMOUNT(69),DEST(1*MAX_UNIT_SLOTS+2+FST_SET)
	GO4K_FST	AMOUNT(42),DEST(3*MAX_UNIT_SLOTS+5+FST_SET)
	GO4K_FOP	OP(FOP_POP)
	GO4K_VCF	FREQUENCY(51),RESONANCE(64),VCFTYPE(BANDPASS)
	GO4K_FST	AMOUNT(54),DEST(16*MAX_UNIT_SLOTS+0+FST_SET)
	GO4K_DLL	PREGAIN(64),DRY(128),FEEDBACK(64),DAMP(64),FREQUENCY(0),DEPTH(0),DELAY(17),COUNT(1)
	GO4K_PAN	PANNING(64)
	GO4K_OUT	GAIN(34), AUXSEND(111)
GO4K_END_PARAMDEF
GO4K_BEGIN_PARAMDEF(Instrument13)
	GO4K_ENV	ATTAC(0),DECAY(64),SUSTAIN(0),RELEASE(0),GAIN(128)
	GO4K_VCO	TRANSPOSE(64),DETUNE(64),PHASE(64),GATES(85),COLOR(64),SHAPE(64),GAIN(128),FLAGS(NOISE)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_VCF	FREQUENCY(128),RESONANCE(128),VCFTYPE(BANDPASS)
	GO4K_VCF	FREQUENCY(110),RESONANCE(121),VCFTYPE(HIGHPASS)
	GO4K_PAN	PANNING(64)
	GO4K_DLL	PREGAIN(47),DRY(128),FEEDBACK(96),DAMP(64),FREQUENCY(0),DEPTH(0),DELAY(29),COUNT(1) ; ERROR
	GO4K_FOP	OP(FOP_XCH)
	GO4K_DLL	PREGAIN(101),DRY(92),FEEDBACK(25),DAMP(128),FREQUENCY(0),DEPTH(48),DELAY(30),COUNT(1) ; ERROR
	GO4K_FOP	OP(FOP_XCH)
	GO4K_OUT	GAIN(64), AUXSEND(0)
GO4K_END_PARAMDEF
GO4K_BEGIN_PARAMDEF(Instrument14)
	GO4K_ENV	ATTAC(0),DECAY(74),SUSTAIN(0),RELEASE(64),GAIN(128)
	GO4K_VCO	TRANSPOSE(64),DETUNE(64),PHASE(0),GATES(0),COLOR(64),SHAPE(64),GAIN(128),FLAGS(NOISE)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_ENV	ATTAC(0),DECAY(80),SUSTAIN(0),RELEASE(20),GAIN(128)
	GO4K_VCO	TRANSPOSE(46),DETUNE(64),PHASE(0),GATES(0),COLOR(128),SHAPE(64),GAIN(128),FLAGS(SINE)
	GO4K_VCF	FREQUENCY(27),RESONANCE(4),VCFTYPE(ALLPASS)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_FOP	OP(FOP_ADDP)
	GO4K_VCF	FREQUENCY(86),RESONANCE(31),VCFTYPE(LOWPASS)
	GO4K_ENV	ATTAC(74),DECAY(79),SUSTAIN(0),RELEASE(61),GAIN(128)
	GO4K_FST	AMOUNT(45),DEST(8*MAX_UNIT_SLOTS+4+FST_SET+FST_POP)
	GO4K_ENV	ATTAC(0),DECAY(57),SUSTAIN(0),RELEASE(36),GAIN(128)
	GO4K_FST	AMOUNT(86),DEST(4*MAX_UNIT_SLOTS+1+FST_SET+FST_POP)
	GO4K_DST	DRIVE(67), SNHFREQ(128), FLAGS(0)
	GO4K_VCF	FREQUENCY(71),RESONANCE(15),VCFTYPE(ALLPASS)
	GO4K_DLL	PREGAIN(120),DRY(57),FEEDBACK(121),DAMP(29),FREQUENCY(27),DEPTH(47),DELAY(0),COUNT(1)
	GO4K_ENV	ATTAC(25),DECAY(73),SUSTAIN(11),RELEASE(94),GAIN(128)
	GO4K_FOP	OP(FOP_MULP)
	GO4K_PAN	PANNING(64)
	GO4K_OUT	GAIN(52), AUXSEND(7)
GO4K_END_PARAMDEF
GO4K_BEGIN_PARAMDEF(Global)
	GO4K_ACC	ACCTYPE(AUX)
	GO4K_DLL	PREGAIN(40),DRY(128),FEEDBACK(125),DAMP(64),FREQUENCY(0),DEPTH(0),DELAY(1),COUNT(8)
	GO4K_FOP	OP(FOP_XCH)
	GO4K_DLL	PREGAIN(40),DRY(128),FEEDBACK(125),DAMP(64),FREQUENCY(0),DEPTH(0),DELAY(9),COUNT(8)
	GO4K_FOP	OP(FOP_XCH)
	GO4K_ACC	ACCTYPE(OUTPUT)
	GO4K_FOP	OP(FOP_ADDP2)
	GO4K_OUT	GAIN(64), AUXSEND(0)
GO4K_END_PARAMDEF
go4k_synth_parameter_values_end
%ifdef USE_SECTIONS
section .g4kmuc5 data align=1
%else
section .data
%endif
%ifdef GO4K_USE_DLL
global _go4k_delay_times
_go4k_delay_times
	dw 0
	dw 1116
	dw 1188
	dw 1276
	dw 1356
	dw 1422
	dw 1492
	dw 1556
	dw 1618
	dw 1140
	dw 1212
	dw 1300
	dw 1380
	dw 1446
	dw 1516
	dw 1580
	dw 1642
	dw 22050
%endif
