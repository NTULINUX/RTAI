/*
COPYRIGHT (C) 2003  Lorenzo Dozio (dozio@aero.polimi.it)
COPYRIGHT (C) 2003  Paolo Mantegazza (mantegazza@aero.polimi.it)
COPYRIGHT (C) 2003  Roberto Bucher (roberto.bucher@supsi.ch)
COPYRIGHT (C) 2003  Peter Brier (pbrier@dds.nl)

COPYRIGHT (C) 2009  Henrik Slotholt (rtai@slotholt.net)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, see <http://www.gnu.org/licenses/>.

*/

#ifndef _XRTAILAB_H_
#define _XRTAILAB_H_

#define XRTAILAB_VERSION	"3.6.1"

#define MAX_NHOSTS		100
#define MAX_NAMES_SIZE		256
#define MAX_DATA_SIZE		30
#define MAX_RTAI_SCOPES		20
#define MAX_RTAI_LOGS		20
#define MAX_RTAI_METERS		20
#define MAX_RTAI_LEDS		20
#define MAX_RTAI_SYNCHS		20
#define MAX_BATCH_PARAMS	1000
#define MAX_TRACES_PER_SCOPE	10

#define FLTK_EVENTS_TICK        0.05
#define MAX_MSG_LEN             (MAX_MSG_SIZE - 100)
#define REFRESH_RATE            0.05

#define msleep(t)       do { poll(0, 0, t); } while (0)

typedef struct Target_Parameters_Struct Target_Parameters_T;
typedef struct Target_Blocks_Struct Target_Blocks_T;
typedef struct Target_Scopes_Struct Target_Scopes_T;
typedef struct Target_Logs_Struct Target_Logs_T;
typedef struct Target_ALogs_Struct Target_ALogs_T;
typedef struct Target_Leds_Struct Target_Leds_T;
typedef struct Target_Meters_Struct Target_Meters_T;
typedef struct Target_Synchs_Struct Target_Synchs_T;
typedef struct Batch_Parameters_Struct Batch_Parameters_T;

typedef struct Preferences_Struct Preferences_T;
typedef struct Profile_Struct Profile_T;
typedef struct W_Geometry_Struct W_Geometry_T;
typedef struct RGB_Color_Struct RGB_Color_T;

typedef struct Args_Struct Args_T;
typedef struct Args_Struct_ALog Alog_T;             //aggiunta 5/5 per nomi alog

typedef struct s_idx_T {
	int scope_idx;
	int trace_idx;
};

typedef struct p_idx_T {
	int block_idx;
	int param_idx;
	int val_idx;
};

typedef struct Param_Input_Block_T {
	Fl_Float_Input **inputW;
};

struct Args_Struct
{
	const char *mbx_id;
	int index;
	int x, y, w, h;
};
struct Args_Struct_ALog
{
	const char *mbx_id;
	int index;
	int x, y, w, h;
	const char *alog_name;
};
struct Target_Parameters_Struct
{
	char model_name[MAX_NAMES_SIZE];
	char block_name[MAX_NAMES_SIZE];
	char param_name[MAX_NAMES_SIZE];
	unsigned int n_rows;
	unsigned int n_cols;
	unsigned int data_type;
	unsigned int data_class;
	double data_value[MAX_DATA_SIZE];
};

struct Target_Blocks_Struct
{
	char name[MAX_NAMES_SIZE];
	int offset;
};

struct Target_Scopes_Struct
{
	char *name;
	char **traceName;
	int ntraces;
	int visible;
	float dt;
};

struct Target_Logs_Struct
{
	char name[MAX_NAMES_SIZE];
	int nrow;
	int ncol;
	float dt;
};

struct Target_ALogs_Struct
{
	char name[MAX_NAMES_SIZE];
	int nrow;
	int ncol;
	float dt;
};

struct Target_Leds_Struct
{
	char name[MAX_NAMES_SIZE];
	int n_leds;
	int visible;
	float dt;
};

struct Target_Meters_Struct
{
	char name[MAX_NAMES_SIZE];
	int visible;
	float dt;
};

struct Target_Synchs_Struct
{
	char name[MAX_NAMES_SIZE];
	int visible;
	float dt;
};

struct Batch_Parameters_Struct
{
	int index;
	int mat_index;
	double value;
};

struct Preferences_Struct
{
	char *Target_IP;
	char *Target_Interface_Task_Name;
	char *Target_Scope_Mbx_ID;
	char *Target_Log_Mbx_ID;
	char *Target_ALog_Mbx_ID;
	char *Target_Led_Mbx_ID;
	char *Target_Meter_Mbx_ID;
	char *Target_Synch_Mbx_ID;
	int MW_w, MW_h, MW_x, MW_y;
	char *Current_Profile;
	int Num_Profiles;
	Fl_String_List Profiles;
};

struct W_Geometry_Struct
{
	int x, y, w, h;
	int visible;
};

struct RGB_Color_Struct
{
	float r, g, b;
};

struct Profile_Struct
{
	char *Target_IP;
	char *Target_Interface_Task_Name;
	char *Target_Scope_Mbx_ID;
	char *Target_Log_Mbx_ID;
	char *Target_ALog_Mbx_ID;
	char *Target_Led_Mbx_ID;
	char *Target_Meter_Mbx_ID;
	char *Target_Synch_Mbx_ID;
	char *Target_Name;

	int n_params, n_blocks;
	int n_scopes, n_logs, n_alogs;           //aggiunto 4/5
	int n_meters, n_leds;
	int n_synchs;

	W_Geometry_T P_Mgr_W;
	int P_Mgr_Batch;

	W_Geometry_T S_Mgr_W;			/* scopes manager geometry */
	int S_Mgr_Show[MAX_RTAI_SCOPES];	/* scopes manager show-hide buttons */
	int S_Mgr_Grid[MAX_RTAI_SCOPES];	/* scopes manager grid on-off buttons */
	int S_Mgr_PT[MAX_RTAI_SCOPES];		/* scopes manager points-time buttons */
	W_Geometry_T S_W[MAX_RTAI_SCOPES];	/* scope geometry */
	RGB_Color_T S_Bg_C[MAX_RTAI_SCOPES];	/* scope background color */
	RGB_Color_T S_Grid_C[MAX_RTAI_SCOPES];	/* scope grid color */
	float S_Mgr_SecDiv[MAX_RTAI_SCOPES];	/* scope seconds per division value */
	int S_Mgr_PSave[MAX_RTAI_SCOPES];	/* scope n points to save */
	float S_Mgr_TSave[MAX_RTAI_SCOPES];	/* scope n seconds to save */
	char *S_Mgr_File[MAX_RTAI_SCOPES];	/* scopes manager save file name */
	int S_Mgr_Flags[MAX_RTAI_SCOPES];	/* scopes Flags  */
	int S_Mgr_Trigger[MAX_RTAI_SCOPES];	/* scopes Trigger mode  */
	int S_Mgr_T_Show[MAX_TRACES_PER_SCOPE][MAX_RTAI_SCOPES];	/* scopes manager trace show-hide buttons */
	float S_Mgr_T_UnitDiv[MAX_TRACES_PER_SCOPE][MAX_RTAI_SCOPES];	/* scope trace units per division value */
	RGB_Color_T S_Trace_C[MAX_TRACES_PER_SCOPE][MAX_RTAI_SCOPES];	/* scope trace color */
	float S_Mgr_T_Width[MAX_TRACES_PER_SCOPE][MAX_RTAI_SCOPES];	/* scope trace width */
	float S_Mgr_T_Offset[MAX_TRACES_PER_SCOPE][MAX_RTAI_SCOPES];	/* scope trace offset */
	int S_Mgr_T_Options[MAX_TRACES_PER_SCOPE][MAX_RTAI_SCOPES];	/* scope trace options */

	int n_traces[MAX_RTAI_SCOPES];

	W_Geometry_T Log_Mgr_W;			/* logs manager geometry */
	int Log_Mgr_PT[MAX_RTAI_LOGS];		/* logs manager points-time buttons */
	int Log_Mgr_PSave[MAX_RTAI_LOGS];	/* logs manager n points to save */
	float Log_Mgr_TSave[MAX_RTAI_LOGS];	/* logs manager n seconds to save */
	char *Log_Mgr_File[MAX_RTAI_LOGS];	/* logs manager save file name */
	
	W_Geometry_T ALog_Mgr_W;		/* logs manager geometry */			//added 4/5
	char *ALog_Mgr_File[MAX_RTAI_LOGS];	/* logs manager save file name */

	W_Geometry_T Led_Mgr_W;			/* leds manager geometry */
	int Led_Mgr_Show[MAX_RTAI_LEDS];	/* leds manager show-hide buttons */
	Fl_String Led_Mgr_Color[MAX_RTAI_LEDS];	/* led color */
	W_Geometry_T Led_W[MAX_RTAI_LEDS];	/* led geometry */

	W_Geometry_T M_Mgr_W;			/* meters manager geometry */
	int M_Mgr_Show[MAX_RTAI_METERS];	/* meters manager show-hide buttons */
	float M_Mgr_Maxv[MAX_RTAI_METERS];	/* meters manager max values */
	float M_Mgr_Minv[MAX_RTAI_METERS];	/* meters manager min values */
	RGB_Color_T M_Bg_C[MAX_RTAI_METERS];	/* meter background color */
	RGB_Color_T M_Arrow_C[MAX_RTAI_METERS];	/* meter arrow color */
	RGB_Color_T M_Grid_C[MAX_RTAI_METERS];	/* meter grid color */
	W_Geometry_T M_W[MAX_RTAI_METERS];	/* meter geometry */

	W_Geometry_T Sy_Mgr_W;			/* synchs manager geometry */
	int Sy_Mgr_Show[MAX_RTAI_SYNCHS];	/* synchs manager show-hide buttons */
	W_Geometry_T Sy_W[MAX_RTAI_SYNCHS];	/* synch geometry */

	char *Text;
};

typedef enum {
	R_COLOR,
	G_COLOR,
	B_COLOR
} RGB_Colors;

typedef enum {
	rt_SCALAR,
	rt_VECTOR,
	rt_MATRIX_ROW_MAJOR,
	rt_MATRIX_COL_MAJOR,
	rt_MATRIX_COL_MAJOR_ND
} Param_Class;

typedef enum {
	CONNECT_TO_TARGET,
	CONNECT_TO_TARGET_WITH_PROFILE,
	DISCONNECT_FROM_TARGET,
	START_TARGET,
	STOP_TARGET,
	UPDATE_PARAM,
	GET_TARGET_TIME,
	BATCH_DOWNLOAD,
	GET_PARAMS,
	CLOSE
} Commands;

typedef enum {
	GEOMETRY_PREF,
	CONNECT_PREF,
	PROFILES_PREF,
	PROFILE_PREF
} Pref_Type;

typedef enum {
	PARAMS_MANAGER,
	SCOPES_MANAGER,
	LOGS_MANAGER,
	ALOGS_MANAGER,
	LEDS_MANAGER,
	METERS_MANAGER,
	SYNCHS_MANAGER
} Manager_Type;

void rlg_quit_cb(Fl_Widget*, void*);
void rlg_connect_cb(Fl_Widget *, void *);
void rlg_disconnect_cb(Fl_Widget *, void *);
void rlg_connect_wprofile_cb(Fl_Widget *, void *);
void rlg_save_profile_cb(Fl_Widget *, void *);
void rlg_delete_profile_cb(Fl_Widget *, void *);
void rlg_start_stop_cb(Fl_Widget *, void *);
void rlg_params_mgr_cb(Fl_Widget *, void *);
void rlg_scopes_mgr_cb(Fl_Widget *, void *);
void rlg_logs_mgr_cb(Fl_Widget *, void *);
void rlg_alogs_mgr_cb(Fl_Widget *, void *); //aggiunto 4/5
void rlg_leds_mgr_cb(Fl_Widget *, void *);
void rlg_meters_mgr_cb(Fl_Widget *, void *);
void rlg_synchs_mgr_cb(Fl_Widget *, void *);
void rlg_update_parameters_cb(Fl_Widget *, void *);
void rlg_batch_update_parameters_cb(Fl_Widget *, void *);
void rlg_upload_parameters_cb(Fl_Widget *, void *);
double get_parameter(Target_Parameters_T, int, int, int *);

Fl_Widget *add_paper(Fl_Group* parent, const char* name, Fl_Image* image);

#endif
