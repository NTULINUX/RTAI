/*
COPYRIGHT (C) 2003  Lorenzo Dozio (dozio@aero.polimi.it)
COPYRIGHT (C) 2003  Paolo Mantegazza (mantegazza@aero.polimi.it)
COPYRIGHT (C) 2003  Roberto Bucher (roberto.bucher@supsi.ch)
COPYRIGHT (C) 2003  Peter Brier (pbrier@dds.nl)
COPYRIGHT (C) 2003  Alberto Sechi (albertosechi@libero.it)
COPYRIGHT (C) 2003  Rob Dye (rdye@telos-systems.com)

COPYRIGHT (C) 2009  Robert Dye (rdye@telos-systems.com)
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

#include <stdio.h>
#include <pthread.h>
#include <sys/poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/io.h>
#include <sys/poll.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <efltk/Fl.h>
#include <efltk/Fl_Item.h>
#include <efltk/Fl_Window.h>
#include <efltk/Fl_Browser.h>
#include <efltk/Fl_Menu_Bar.h>
#include <efltk/Fl_Main_Window.h>
#include <efltk/Fl_Workspace.h>
#include <efltk/Fl_MDI_Bar.h>
#include <efltk/Fl_Config.h>
#include <efltk/Fl_Dialog.h>
#include <efltk/fl_ask.h>
#include <efltk/Fl_Text_Editor.h>

#include <rtai_netrpc.h>
#include <rtai_msg.h>
#include <rtai_mbx.h>

#include <Fl_Scope.h>
#include <Fl_Scope_Window.h>
#include <Fl_Led.h>
#include <Fl_Led_Window.h>
#include <Fl_Meter.h>
#include <Fl_Meter_Window.h>
#include <Fl_Synch.h>
#include <Fl_Synch_Window.h>
#include <Fl_Params_Manager.h>
#include <Fl_Scopes_Manager.h>
#include <Fl_Logs_Manager.h>
#include <Fl_ALogs_Manager.h>
#include <Fl_Leds_Manager.h>
#include <Fl_Meters_Manager.h>
#include <Fl_Synchs_Manager.h>
#include <xrtailab.h>
#include <icons/start_icon.xpm>
#include <icons/stop_icon.xpm>
#include <icons/parameters_icon.xpm>
#include <icons/log_icon.xpm>
#include <icons/auto_log_icon.xpm>
#include <icons/connect_icon.xpm>
#include <icons/connect_wprofile_icon.xpm>
#include <icons/disconnect_icon.xpm>
#include <icons/profile_icon.xpm>
//#include <icons/synchronoscope_icon.xpm>
#include <icons/save_profile_icon.xpm>
#include <icons/del_profile_icon.xpm>
#include <icons/exit_icon.xpm>

static RT_TASK *Target_Interface_Task;
static RT_TASK *RLG_Main_Task;
static pthread_t Target_Interface_Thread;
static pthread_t *Get_Scope_Data_Thread;
static pthread_t *Get_Log_Data_Thread;
static pthread_t *Get_ALog_Data_Thread;
static pthread_t *Get_Led_Data_Thread;
static pthread_t *Get_Meter_Data_Thread;
static pthread_t *Get_Synch_Data_Thread;

static int Direct_Profile_Idx = -1;
static char *Direct_Profile = NULL;
static int Verbose = 0;

static int Is_Target_Connected;
static unsigned int Is_Target_Running;
static int End_App;
static int End_All;
static long Target_Node;

int Num_Tunable_Parameters;
int Num_Tunable_Blocks;
int Num_Scopes;
int Num_Logs;
int Num_ALogs;
int Num_Leds;
int Num_Meters;
int Num_Synchs;

Target_Parameters_T *Tunable_Parameters;
Target_Blocks_T *Tunable_Blocks;
Target_Scopes_T *Scopes;
Target_Logs_T *Logs;
Target_ALogs_T *ALogs;
Target_Leds_T *Leds;
Target_Meters_T *Meters;
Target_Synchs_T *Synchs;

//Target_Synch_T Synchronoscope;
Batch_Parameters_T Batch_Parameters[MAX_BATCH_PARAMS];
Preferences_T Preferences;
Profile_T *Profile;

Fl_Parameters_Manager *Parameters_Manager;
Fl_Scopes_Manager *Scopes_Manager;
Fl_Logs_Manager *Logs_Manager;
Fl_ALogs_Manager *ALogs_Manager;
Fl_Leds_Manager *Leds_Manager;
Fl_Meters_Manager *Meters_Manager;
Fl_Synchs_Manager *Synchs_Manager;

//Fl_Synch_Window *Synchronoscope_Win;

Fl_Main_Window *RLG_Main_Window;
Fl_Menu_Bar    *RLG_Main_Menu;
Fl_Workspace   *RLG_Main_Workspace;
Fl_Widget      *RLG_Main_Status;
Fl_Tool_Button *RLG_Connect_Button;
Fl_Tool_Button *RLG_Disconnect_Button;
Fl_Tool_Button *RLG_Connect_wProfile_Button;
Fl_Tool_Button *RLG_Save_Profile_Button;
Fl_Tool_Button *RLG_Delete_Profile_Button;
Fl_Tool_Button *RLG_Start_Button;
Fl_Tool_Button *RLG_Stop_Button;
Fl_Tool_Button *RLG_Params_Mgr_Button;
Fl_Tool_Button *RLG_Scopes_Mgr_Button;
Fl_Tool_Button *RLG_Logs_Mgr_Button;
Fl_Tool_Button *RLG_ALogs_Mgr_Button;
Fl_Tool_Button *RLG_Leds_Mgr_Button;
Fl_Tool_Button *RLG_Meters_Mgr_Button;
Fl_Tool_Button *RLG_Synchs_Mgr_Button;
Fl_Tool_Button *RLG_Text_Button;
Fl_Tool_Button *RLG_Exit_Button;

Fl_Window *RLG_Text_Window;
Fl_Text_Buffer *RLG_Text_Buffer;

Fl_Dialog *RLG_Connect_Dialog;

Fl_Input *RLG_Target_IP_Address;
Fl_Input *RLG_Target_Task_ID;
Fl_Input *RLG_Target_Scope_ID;
Fl_Input *RLG_Target_Log_ID;
Fl_Input *RLG_Target_ALog_ID;
Fl_Input *RLG_Target_Led_ID;
Fl_Input *RLG_Target_Meter_ID;
Fl_Input *RLG_Target_Synch_ID;

Fl_Dialog *RLG_Save_Profile_Dialog;
Fl_Input *RLG_Save_Profile_Input;

Fl_Dialog *RLG_Delete_Profile_Dialog;
Fl_Browser *RLG_Delete_Profile_Tree;

Fl_Dialog *RLG_Connect_wProfile_Dialog;
Fl_Browser *RLG_Connect_wProfile_Tree;
Fl_Browser *RLG_Profile_Contents;
const char *RLG_Target_Name;

Fl_Menu_Item RLG_Main_Menu_Table[] = {
	{" &File", FL_ALT+'f', 0, 0, FL_SUBMENU},
		{" Connect... ",                FL_ALT+'c', rlg_connect_cb,          0},
		{" Disconnect ",                FL_ALT+'d', rlg_disconnect_cb,       0, FL_MENU_INACTIVE},
		{" Connect with Profiles... ",  FL_ALT+'i', rlg_connect_wprofile_cb, 0, FL_MENU_DIVIDER},
		{" Save Profile... ",           FL_ALT+'v', rlg_save_profile_cb,     0, FL_MENU_INACTIVE},
		{" Delete Profile... ",         FL_ALT+'t', rlg_delete_profile_cb,   0, FL_MENU_DIVIDER},
		{" Quit ",                      FL_ALT+'q', rlg_quit_cb,             0, 0},
		{0},
	{" Vie&w", FL_ALT+'w', 0, 0, FL_SUBMENU},
		{" Parameters ",     FL_ALT+'p', rlg_params_mgr_cb, 0, FL_MENU_INACTIVE|FL_MENU_TOGGLE},
		{" Scopes ",         FL_ALT+'s', rlg_scopes_mgr_cb, 0, FL_MENU_INACTIVE|FL_MENU_TOGGLE},
		{" Logs ",           FL_ALT+'l', rlg_logs_mgr_cb,   0, FL_MENU_INACTIVE|FL_MENU_TOGGLE},
		{" Auto Logs ",      FL_ALT+'a', rlg_alogs_mgr_cb,  0, FL_MENU_INACTIVE|FL_MENU_TOGGLE}, //aggiunto 4/5
		{" Leds ",           FL_ALT+'e', rlg_leds_mgr_cb,   0, FL_MENU_INACTIVE|FL_MENU_TOGGLE},
		{" Meters ",         FL_ALT+'m', rlg_meters_mgr_cb, 0, FL_MENU_INACTIVE|FL_MENU_TOGGLE},
		{" Synchs ",         FL_ALT+'y', rlg_synchs_mgr_cb, 0, FL_MENU_INACTIVE|FL_MENU_TOGGLE|FL_MENU_DIVIDER},
		{" Close All ",      0,          0,                 0, FL_MENU_INACTIVE},
		{0},
	{" &Help", FL_ALT+'h', 0, 0, FL_SUBMENU},
		{" Help contents ",  0, 0},
		{" About RTAI-Lab ", 0, 0},
		{0},
	{0}
};

static volatile int GlobalRet[16];

static inline void RT_RPC(RT_TASK *task, unsigned int msg, unsigned int *reply)
{
	GlobalRet[msg & 0xf] = 0;
	rt_send(task, msg);
	while (!GlobalRet[msg & 0xf]) {
		Fl::wait(FLTK_EVENTS_TICK);
	}
}

static inline void RT_RETURN(RT_TASK *task, unsigned int reply)
{
	GlobalRet[reply] = 1;
}

Fl_Widget* add_paper(Fl_Group* parent, const char* name, Fl_Image* image)
{
	parent->begin();
	Fl_Item* o = new Fl_Item(name);
	o->image(image);
	return o;
}

unsigned long get_an_id(const char *root)
{
	int i;
	char name[7];
	for (i = 0; i < MAX_NHOSTS; i++) {
		sprintf(name, "%s%d", root, i);
		if (!rt_get_adr(nam2num(name))) {
			return nam2num(name);
		}
	}
	return 0;
}

void rlg_read_pref(int p_type, const char *c_file, int p_idx)
{
	Fl_Config App("RTAI-Lab", c_file, 1);

	switch (p_type) {
		case GEOMETRY_PREF:
			App.set_section("Main Window Geometry");
			App.read("w", Preferences.MW_w, (int)(Fl::w()/1.8));
			App.read("h", Preferences.MW_h, (int)(Fl::h()/1.3));
			App.read("x", Preferences.MW_x, 20);
			App.read("y", Preferences.MW_y, 20);
			break;
		case CONNECT_PREF:
			App.set_section("Target Settings");
			App.read("ip_address",   Preferences.Target_IP, "127.0.0.1");
			App.read("task_id",      Preferences.Target_Interface_Task_Name, "IFTASK");
			App.read("scope_mbx_id", Preferences.Target_Scope_Mbx_ID, "RTS");
			App.read("log_mbx_id",   Preferences.Target_Log_Mbx_ID, "RTL");
			App.read("auto_log_mbx_id",Preferences.Target_ALog_Mbx_ID, "RAL"); //aggiunto 4/5
			App.read("led_mbx_id",   Preferences.Target_Led_Mbx_ID, "RTE");
			App.read("meter_mbx_id", Preferences.Target_Meter_Mbx_ID, "RTM");
			App.read("synch_mbx_id", Preferences.Target_Synch_Mbx_ID, "RTY");
			RLG_Target_IP_Address->value(Preferences.Target_IP);
			RLG_Target_Task_ID->value(Preferences.Target_Interface_Task_Name);
			RLG_Target_Scope_ID->value(Preferences.Target_Scope_Mbx_ID);
			RLG_Target_Log_ID->value(Preferences.Target_Log_Mbx_ID);
			RLG_Target_ALog_ID->value(Preferences.Target_ALog_Mbx_ID);
			RLG_Target_Led_ID->value(Preferences.Target_Led_Mbx_ID);
			RLG_Target_Meter_ID->value(Preferences.Target_Meter_Mbx_ID);
			RLG_Target_Synch_ID->value(Preferences.Target_Synch_Mbx_ID);
			break;
		case PROFILES_PREF:
			App.set_section("Profiles");
			App.read("last_profile", Preferences.Current_Profile, ".my_profile");
			App.read("number_of_profiles", Preferences.Num_Profiles, 0);
			for (int i = 0; i < Preferences.Num_Profiles; i++) {
				char buf[100], *p;
				sprintf(buf, "profile_name_%d", i + 1);
				App.read(buf, p, ".my_profile");
				Preferences.Profiles.append(p);
			}
			break;
		case PROFILE_PREF:
/*
			App.set_section("Text");
			App.read("text", Profile[p_idx].Text, 0);
*/
			App.set_section("Target");
			App.read("name",	 Profile[p_idx].Target_Name, "target");
			App.read("ip_address",   Profile[p_idx].Target_IP, "127.0.0.1");
			App.read("task_id",      Profile[p_idx].Target_Interface_Task_Name, "IFTASK");
			App.read("scope_mbx_id", Profile[p_idx].Target_Scope_Mbx_ID, "RTS");
			App.read("log_mbx_id",   Profile[p_idx].Target_Log_Mbx_ID, "RTL");
			App.read("auto_log_mbx_id",Profile[p_idx].Target_ALog_Mbx_ID, "RAL");
			App.read("led_mbx_id",   Profile[p_idx].Target_Led_Mbx_ID, "RTE");
			App.read("meter_mbx_id", Profile[p_idx].Target_Meter_Mbx_ID, "RTM");
			App.read("synch_mbx_id", Profile[p_idx].Target_Synch_Mbx_ID, "RTY");
			App.read("n_params",	 Profile[p_idx].n_params, 0);
			App.read("n_blocks",	 Profile[p_idx].n_blocks, 0);
			App.read("n_scopes",	 Profile[p_idx].n_scopes, 0);
			App.read("n_logs",	 Profile[p_idx].n_logs, 0);
			App.read("n_alogs",	 Profile[p_idx].n_alogs, 0);
			App.read("n_leds",	 Profile[p_idx].n_leds, 0);
			App.read("n_meters",	 Profile[p_idx].n_meters, 0);
			App.read("n_synchs",	 Profile[p_idx].n_synchs, 0);
			App.set_section("Parameters Manager");
			App.read("x", Profile[p_idx].P_Mgr_W.x, 0);
			App.read("y", Profile[p_idx].P_Mgr_W.y, 0);
			App.read("w", Profile[p_idx].P_Mgr_W.w, 430);
			App.read("h", Profile[p_idx].P_Mgr_W.h, 260);
			App.read("visible", Profile[p_idx].P_Mgr_W.visible, 0);
			App.read("batch download", Profile[p_idx].P_Mgr_Batch, 0);
			App.set_section("Scopes Manager");
			App.read("x", Profile[p_idx].S_Mgr_W.x, 0);
			App.read("y", Profile[p_idx].S_Mgr_W.y, 290);
			App.read("w", Profile[p_idx].S_Mgr_W.w, 480);
			App.read("h", Profile[p_idx].S_Mgr_W.h, 300);
			App.read("visible", Profile[p_idx].S_Mgr_W.visible, 0);
			for (int i = 0; i < Profile[p_idx].n_scopes; i++) {
				char buf[100];
				sprintf(buf, "scope widget n.%d traces", i + 1);
				App.read(buf, Profile[p_idx].n_traces[i], 1);
				for (int j = 0; j < Profile[p_idx].n_traces[i]; j++) {
					sprintf(buf, "scope widget n.%d trace %d show", i + 1, j + 1);
					App.read(buf, Profile[p_idx].S_Mgr_T_Show[j][i], 1);
					sprintf(buf, "scope widget n.%d trace %d unitdiv", i + 1, j + 1);
					App.read(buf, Profile[p_idx].S_Mgr_T_UnitDiv[j][i], 2.5);
					sprintf(buf, "scope widget n.%d trace %d color r", i + 1, j + 1);
					App.read(buf, Profile[p_idx].S_Trace_C[j][i].r, 1.0);
					sprintf(buf, "scope widget n.%d trace %d color g", i + 1, j + 1);
					App.read(buf, Profile[p_idx].S_Trace_C[j][i].g, 1.0);
					sprintf(buf, "scope widget n.%d trace %d color b", i + 1, j + 1);
					App.read(buf, Profile[p_idx].S_Trace_C[j][i].b, 1.0);
					sprintf(buf, "scope widget n.%d trace %d offset", i + 1, j + 1);
					App.read(buf, Profile[p_idx].S_Mgr_T_Offset[j][i], 1.0);
					sprintf(buf, "scope widget n.%d trace %d width", i + 1, j + 1);
					App.read(buf, Profile[p_idx].S_Mgr_T_Width[j][i], 0.1);
					sprintf(buf, "scope widget n.%d trace %d options", i + 1, j + 1);
					App.read(buf, Profile[p_idx].S_Mgr_T_Options[j][i], 0);

				}
				sprintf(buf, "scope widget n.%d show", i + 1);
				App.read(buf, Profile[p_idx].S_Mgr_Show[i], 0);
				sprintf(buf, "scope widget n.%d grid", i + 1);
				App.read(buf, Profile[p_idx].S_Mgr_Grid[i], 1);
				sprintf(buf, "scope widget n.%d pt", i + 1);
				App.read(buf, Profile[p_idx].S_Mgr_PT[i], 1);
				sprintf(buf, "scope widget n.%d filename", i + 1);
				App.read(buf, Profile[p_idx].S_Mgr_File[i], "filename");
				sprintf(buf, "scope widget n.%d x", i + 1);
				App.read(buf, Profile[p_idx].S_W[i].x, 500);
				sprintf(buf, "scope widget n.%d y", i + 1);
				App.read(buf, Profile[p_idx].S_W[i].y, 290);
				sprintf(buf, "scope widget n.%d w", i + 1);
				App.read(buf, Profile[p_idx].S_W[i].w, 250);
				sprintf(buf, "scope widget n.%d h", i + 1);
				App.read(buf, Profile[p_idx].S_W[i].h, 250);
				sprintf(buf, "scope widget n.%d bg r", i + 1);
				App.read(buf, Profile[p_idx].S_Bg_C[i].r, 0.388);
				sprintf(buf, "scope widget n.%d bg g", i + 1);
				App.read(buf, Profile[p_idx].S_Bg_C[i].g, 0.451);
				sprintf(buf, "scope widget n.%d bg b", i + 1);
				App.read(buf, Profile[p_idx].S_Bg_C[i].b, 0.604);
				sprintf(buf, "scope widget n.%d grid r", i + 1);
				App.read(buf, Profile[p_idx].S_Grid_C[i].r, 0.65);
				sprintf(buf, "scope widget n.%d grid g", i + 1);
				App.read(buf, Profile[p_idx].S_Grid_C[i].g, 0.65);
				sprintf(buf, "scope widget n.%d grid b", i + 1);
				App.read(buf, Profile[p_idx].S_Grid_C[i].b, 0.65);
				sprintf(buf, "scope widget n.%d secdiv", i + 1);
				App.read(buf, Profile[p_idx].S_Mgr_SecDiv[i], 0.1);
				sprintf(buf, "scope widget n.%d save points", i + 1);
				App.read(buf, Profile[p_idx].S_Mgr_PSave[i], 1000);
				sprintf(buf, "scope widget n.%d save time", i + 1);
				App.read(buf, Profile[p_idx].S_Mgr_TSave[i], 1.0);
				sprintf(buf, "scope widget n.%d trigger", i + 1);
				App.read(buf, Profile[p_idx].S_Mgr_Trigger[i], 1);
				sprintf(buf, "scope widget n.%d flags", i + 1);
				App.read(buf, Profile[p_idx].S_Mgr_Flags[i], 1);

			}
			App.set_section("Logs Manager");
			App.read("x", Profile[p_idx].Log_Mgr_W.x, 480);
			App.read("y", Profile[p_idx].Log_Mgr_W.y, 0);
			App.read("w", Profile[p_idx].Log_Mgr_W.w, 380);
			App.read("h", Profile[p_idx].Log_Mgr_W.h, 250);
			App.read("visible", Profile[p_idx].Log_Mgr_W.visible, 0);
			for (int i = 0; i < Profile[p_idx].n_logs; i++) {
				char buf[100];
				sprintf(buf, "log widget n.%d pt", i + 1);
				App.read(buf, Profile[p_idx].Log_Mgr_PT[i], 1);
				sprintf(buf, "log widget n.%d save points", i + 1);
				App.read(buf, Profile[p_idx].Log_Mgr_PSave[i], 1000);
				sprintf(buf, "log widget n.%d save time", i + 1);
				App.read(buf, Profile[p_idx].Log_Mgr_TSave[i], 1.0);
				sprintf(buf, "log widget n.%d filename", i + 1);
				App.read(buf, Profile[p_idx].Log_Mgr_File[i], "filename");
			}
			App.set_section("Auto Logs Manager");
			App.read("x", Profile[p_idx].ALog_Mgr_W.x, 480);
			App.read("y", Profile[p_idx].ALog_Mgr_W.y, 0);
			App.read("w", Profile[p_idx].ALog_Mgr_W.w, 380);
			App.read("h", Profile[p_idx].ALog_Mgr_W.h, 250);
			App.read("visible", Profile[p_idx].ALog_Mgr_W.visible, 0);
			for (int i = 0; i < Profile[p_idx].n_alogs; i++) {
				char buf[100];
				sprintf(buf, "auto log widget n.%d filename", i + 1);
				App.read(buf, Profile[p_idx].ALog_Mgr_File[i], "filename");
			}
			App.set_section("Leds Manager");
			App.read("x", Profile[p_idx].Led_Mgr_W.x, 500);
			App.read("y", Profile[p_idx].Led_Mgr_W.y, 290);
			App.read("w", Profile[p_idx].Led_Mgr_W.w, 320);
			App.read("h", Profile[p_idx].Led_Mgr_W.h, 250);
			App.read("visible", Profile[p_idx].Led_Mgr_W.visible, 0);
			for (int i = 0; i < Profile[p_idx].n_leds; i++) {
				char buf[100];
				sprintf(buf, "led widget n.%d show", i + 1);
				App.read(buf, Profile[p_idx].Led_Mgr_Show[i], 0);
				sprintf(buf, "led widget n.%d color", i + 1);
				App.read(buf, Profile[p_idx].Led_Mgr_Color[i], "green");
				sprintf(buf, "led widget n.%d x", i + 1);
				App.read(buf, Profile[p_idx].Led_W[i].x, 500);
				sprintf(buf, "led widget n.%d y", i + 1);
				App.read(buf, Profile[p_idx].Led_W[i].y, 290);
				sprintf(buf, "led widget n.%d w", i + 1);
				App.read(buf, Profile[p_idx].Led_W[i].w, 250);
				sprintf(buf, "led widget n.%d h", i + 1);
				App.read(buf, Profile[p_idx].Led_W[i].h, 250);
			}
			App.set_section("Meters Manager");
			App.read("x", Profile[p_idx].M_Mgr_W.x, 530);
			App.read("y", Profile[p_idx].M_Mgr_W.y, 320);
			App.read("w", Profile[p_idx].M_Mgr_W.w, 320);
			App.read("h", Profile[p_idx].M_Mgr_W.h, 250);
			App.read("visible", Profile[p_idx].M_Mgr_W.visible, 0);
			for (int i = 0; i < Profile[p_idx].n_meters; i++) {
				char buf[100];
				sprintf(buf, "meter widget n.%d show", i + 1);
				App.read(buf, Profile[p_idx].M_Mgr_Show[i], 0);
				sprintf(buf, "meter widget n.%d max", i + 1);
				App.read(buf, Profile[p_idx].M_Mgr_Maxv[i], 1.2);
				sprintf(buf, "meter widget n.%d min", i + 1);
				App.read(buf, Profile[p_idx].M_Mgr_Minv[i], -1.2);
				sprintf(buf, "meter widget n.%d bg r", i + 1);
				App.read(buf, Profile[p_idx].M_Bg_C[i].r, 1.0);
				sprintf(buf, "meter widget n.%d bg g", i + 1);
				App.read(buf, Profile[p_idx].M_Bg_C[i].g, 1.0);
				sprintf(buf, "meter widget n.%d bg b", i + 1);
				App.read(buf, Profile[p_idx].M_Bg_C[i].b, 1.0);
				sprintf(buf, "meter widget n.%d arrow r", i + 1);
				App.read(buf, Profile[p_idx].M_Arrow_C[i].r, 0.0);
				sprintf(buf, "meter widget n.%d arrow g", i + 1);
				App.read(buf, Profile[p_idx].M_Arrow_C[i].g, 0.0);
				sprintf(buf, "meter widget n.%d arrow b", i + 1);
				App.read(buf, Profile[p_idx].M_Arrow_C[i].b, 0.0);
				sprintf(buf, "meter widget n.%d grid r", i + 1);
				App.read(buf, Profile[p_idx].M_Grid_C[i].r, 0.65);
				sprintf(buf, "meter widget n.%d grid g", i + 1);
				App.read(buf, Profile[p_idx].M_Grid_C[i].g, 0.65);
				sprintf(buf, "meter widget n.%d grid b", i + 1);
				App.read(buf, Profile[p_idx].M_Grid_C[i].b, 0.65);
				sprintf(buf, "meter widget n.%d x", i + 1);
				App.read(buf, Profile[p_idx].M_W[i].x, 0);
				sprintf(buf, "meter widget n.%d y", i + 1);
				App.read(buf, Profile[p_idx].M_W[i].y, 0);
				sprintf(buf, "meter widget n.%d w", i + 1);
				App.read(buf, Profile[p_idx].M_W[i].w, 300);
				sprintf(buf, "meter widget n.%d h", i + 1);
				App.read(buf, Profile[p_idx].M_W[i].h, 200);
			}
			App.set_section("Synchs Manager");
			App.read("x", Profile[p_idx].Sy_Mgr_W.x, 0);
			App.read("y", Profile[p_idx].Sy_Mgr_W.y, 0);
			App.read("w", Profile[p_idx].Sy_Mgr_W.w, 260);
			App.read("h", Profile[p_idx].Sy_Mgr_W.h, 200);
			App.read("visible", Profile[p_idx].Sy_Mgr_W.visible, 0);
			for (int i = 0; i < Profile[p_idx].n_synchs; i++) {
				char buf[100];
				sprintf(buf, "synch widget n.%d show", i + 1);
				App.read(buf, Profile[p_idx].Sy_Mgr_Show[i], 0);
				sprintf(buf, "synch widget n.%d x", i + 1);
				App.read(buf, Profile[p_idx].Sy_W[i].x, 0);
				sprintf(buf, "synch widget n.%d y", i + 1);
				App.read(buf, Profile[p_idx].Sy_W[i].y, 0);
				sprintf(buf, "synch widget n.%d w", i + 1);
				App.read(buf, Profile[p_idx].Sy_W[i].w, 260);
				sprintf(buf, "synch widget n.%d h", i + 1);
				App.read(buf, Profile[p_idx].Sy_W[i].h, 200);
			}
			break;
		default:
			break;
	}
}

void rlg_write_pref(int p_type, const char *c_file)
{
	Fl_Config App("RTAI-Lab", c_file, 1);

	switch (p_type) {
		case GEOMETRY_PREF:
			App.set_section("Main Window Geometry");
			App.write("w", (int)RLG_Main_Window->w());
			App.write("h", (int)RLG_Main_Window->h());
			App.write("x", (int)RLG_Main_Window->x());
			App.write("y", (int)RLG_Main_Window->y());
			break;
		case CONNECT_PREF:
			App.set_section("Target Settings");
			App.write("ip_address",   RLG_Target_IP_Address->value());
			App.write("task_id",      RLG_Target_Task_ID->value());
			App.write("scope_mbx_id", RLG_Target_Scope_ID->value());
			App.write("log_mbx_id",   RLG_Target_Log_ID->value());
			App.write("alog_mbx_id",  RLG_Target_ALog_ID->value());
			App.write("led_mbx_id",   RLG_Target_Led_ID->value());
			App.write("meter_mbx_id", RLG_Target_Meter_ID->value());
			App.write("synch_mbx_id", RLG_Target_Synch_ID->value());
			break;
		case PROFILES_PREF:
			App.remove_sec("Profiles");
			App.set_section("Profiles");
			App.write("last_profile", Preferences.Current_Profile);
			App.write("number_of_profiles", Preferences.Num_Profiles);
			for (int i = 0; i < Preferences.Num_Profiles; i++) {
				char buf[100];
				sprintf(buf, "profile_name_%d", i + 1);
				App.write(buf, Preferences.Profiles.item(i).c_str());
			}
			break;
		case PROFILE_PREF:
/*
			App.set_section("Text");
			App.write("text", RLG_Text_Buffer->text());
*/
			App.set_section("Target");
			App.write("name",	  RLG_Target_Name);
			App.write("ip_address",   RLG_Target_IP_Address->value());
			App.write("task_id",      RLG_Target_Task_ID->value());
			App.write("scope_mbx_id", RLG_Target_Scope_ID->value());
			App.write("log_mbx_id",   RLG_Target_Log_ID->value());
			App.write("alog_mbx_id",  RLG_Target_ALog_ID->value());
			App.write("led_mbx_id",   RLG_Target_Led_ID->value());
			App.write("meter_mbx_id", RLG_Target_Meter_ID->value());
			App.write("synch_mbx_id", RLG_Target_Synch_ID->value());
			App.write("n_params",	  Num_Tunable_Parameters);
			App.write("n_blocks",	  Num_Tunable_Blocks);
			App.write("n_scopes",	  Num_Scopes);
			App.write("n_logs",	  Num_Logs);
			App.write("n_alogs",	  Num_ALogs);
			App.write("n_leds",	  Num_Leds);
			App.write("n_meters",	  Num_Meters);
			App.write("n_synchs",	  Num_Synchs);
			if (Parameters_Manager) {
				App.set_section("Parameters Manager");
				App.write("x", Parameters_Manager->x());
				App.write("y", Parameters_Manager->y());
				App.write("w", Parameters_Manager->w());
				App.write("h", Parameters_Manager->h());
				App.write("visible", Parameters_Manager->visible());
				App.write("batch download", Parameters_Manager->batch_download());
			}
			if (Scopes_Manager) {
				App.set_section("Scopes Manager");
				App.write("x", Scopes_Manager->x());
				App.write("y", Scopes_Manager->y());
				App.write("w", Scopes_Manager->w());
				App.write("h", Scopes_Manager->h());
				App.write("visible", Scopes_Manager->visible());
				for (int i = 0; i < Num_Scopes; i++) {
					char buf[100];
					sprintf(buf, "scope widget n.%d traces", i + 1);
					App.write(buf, Scopes[i].ntraces);
					for (int j = 0; j < Scopes[i].ntraces; j++) {
						sprintf(buf, "scope widget n.%d trace %d show", i + 1, j + 1);
						App.write(buf, Scopes_Manager->trace_show_hide(i, j));
						sprintf(buf, "scope widget n.%d trace %d unitdiv", i + 1, j + 1);
						App.write(buf, Scopes_Manager->trace_unit_div(i, j));
						sprintf(buf, "scope widget n.%d trace %d color r", i + 1, j + 1);
						App.write(buf, Scopes_Manager->t_color(i, j, R_COLOR));
						sprintf(buf, "scope widget n.%d trace %d color g", i + 1, j + 1);
						App.write(buf, Scopes_Manager->t_color(i, j, G_COLOR));
						sprintf(buf, "scope widget n.%d trace %d color b", i + 1, j + 1);
						App.write(buf, Scopes_Manager->t_color(i, j, B_COLOR));
						sprintf(buf, "scope widget n.%d trace %d offset", i + 1, j + 1);
						App.write(buf, Scopes_Manager->trace_offset(i, j));
						sprintf(buf, "scope widget n.%d trace %d width", i + 1, j + 1);
						App.write(buf, Scopes_Manager->trace_width(i, j));
						sprintf(buf, "scope widget n.%d trace %d options", i + 1, j + 1);
						App.write(buf, Scopes_Manager->Scope_Windows[i]->Plot->trace_flags(j) );

					}
					sprintf(buf, "scope widget n.%d show", i + 1);
					App.write(buf, Scopes_Manager->show_hide(i));
					sprintf(buf, "scope widget n.%d grid", i + 1);
					App.write(buf, Scopes_Manager->grid_on_off(i));
					sprintf(buf, "scope widget n.%d pt", i + 1);
					App.write(buf, Scopes_Manager->points_time(i));
					sprintf(buf, "scope widget n.%d filename", i + 1);
					App.write(buf, Scopes_Manager->file_name(i));
					sprintf(buf, "scope widget n.%d x", i + 1);
					App.write(buf, Scopes_Manager->sw_x(i));
					sprintf(buf, "scope widget n.%d y", i + 1);
					App.write(buf, Scopes_Manager->sw_y(i));
					sprintf(buf, "scope widget n.%d w", i + 1);
					App.write(buf, Scopes_Manager->sw_w(i));
					sprintf(buf, "scope widget n.%d h", i + 1);
					App.write(buf, Scopes_Manager->sw_h(i));
					sprintf(buf, "scope widget n.%d bg r", i + 1);
					App.write(buf, Scopes_Manager->b_color(i, R_COLOR));
					sprintf(buf, "scope widget n.%d bg g", i + 1);
					App.write(buf, Scopes_Manager->b_color(i, G_COLOR));
					sprintf(buf, "scope widget n.%d bg b", i + 1);
					App.write(buf, Scopes_Manager->b_color(i, B_COLOR));
					sprintf(buf, "scope widget n.%d grid r", i + 1);
					App.write(buf, Scopes_Manager->g_color(i, R_COLOR));
					sprintf(buf, "scope widget n.%d grid g", i + 1);
					App.write(buf, Scopes_Manager->g_color(i, G_COLOR));
					sprintf(buf, "scope widget n.%d grid b", i + 1);
					App.write(buf, Scopes_Manager->g_color(i, B_COLOR));
					sprintf(buf, "scope widget n.%d secdiv", i + 1);
					App.write(buf, Scopes_Manager->sec_div(i));
					sprintf(buf, "scope widget n.%d save points", i + 1);
					App.write(buf, Scopes_Manager->p_save(i));
					sprintf(buf, "scope widget n.%d save time", i + 1);
					App.write(buf, Scopes_Manager->t_save(i));
					sprintf(buf, "scope widget n.%d flags", i + 1);
					App.write(buf, Scopes_Manager->Scope_Windows[i]->Plot->scope_flags() );
					sprintf(buf, "scope widget n.%d trigger", i + 1);
					App.write(buf, Scopes_Manager->Scope_Windows[i]->Plot->trigger_mode() );
				}
			}
			if (Logs_Manager) {
				App.set_section("Logs Manager");
				App.write("x", Logs_Manager->x());
				App.write("y", Logs_Manager->y());
				App.write("w", Logs_Manager->w());
				App.write("h", Logs_Manager->h());
				App.write("visible", Logs_Manager->visible());
				for (int i = 0; i < Num_Logs; i++) {
					char buf[100];
					sprintf(buf, "log widget n.%d pt", i + 1);
					App.write(buf, Logs_Manager->points_time(i));
					sprintf(buf, "log widget n.%d save points", i + 1);
					App.write(buf, Logs_Manager->p_save(i));
					sprintf(buf, "log widget n.%d save time", i + 1);
					App.write(buf, Logs_Manager->t_save(i));
					sprintf(buf, "log widget n.%d filename", i + 1);
					App.write(buf, Logs_Manager->file_name(i));
				}
			}
			if (ALogs_Manager) {
				App.set_section("Auto Logs Manager");
				App.write("x", ALogs_Manager->x());
				App.write("y", ALogs_Manager->y());
				App.write("w", ALogs_Manager->w());
				App.write("h", ALogs_Manager->h());
				App.write("visible", ALogs_Manager->visible());
				for (int i = 0; i < Num_ALogs; i++) {
					char buf[100];
					sprintf(buf, "auto log widget n.%d filename", i + 1);
					App.write(buf, ALogs_Manager->file_name(i));
				}
			}
			if (Leds_Manager) {
				App.set_section("Leds Manager");
				App.write("x", Leds_Manager->x());
				App.write("y", Leds_Manager->y());
				App.write("w", Leds_Manager->w());
				App.write("h", Leds_Manager->h());
				App.write("visible", Leds_Manager->visible());
				for (int i = 0; i < Num_Leds; i++) {
					char buf[100];
					sprintf(buf, "led widget n.%d show", i + 1);
					App.write(buf, Leds_Manager->show_hide(i));
					sprintf(buf, "led widget n.%d color", i + 1);
					App.write(buf, Leds_Manager->color(i));
					sprintf(buf, "led widget n.%d x", i + 1);
					App.write(buf, Leds_Manager->lw_x(i));
					sprintf(buf, "led widget n.%d y", i + 1);
					App.write(buf, Leds_Manager->lw_y(i));
					sprintf(buf, "led widget n.%d w", i + 1);
					App.write(buf, Leds_Manager->lw_w(i));
					sprintf(buf, "led widget n.%d h", i + 1);
					App.write(buf, Leds_Manager->lw_h(i));
				}
			}
			if (Meters_Manager) {
				App.set_section("Meters Manager");
				App.write("x", Meters_Manager->x());
				App.write("y", Meters_Manager->y());
				App.write("w", Meters_Manager->w());
				App.write("h", Meters_Manager->h());
				App.write("visible", Meters_Manager->visible());
				for (int i = 0; i < Num_Meters; i++) {
					char buf[100];
					sprintf(buf, "meter widget n.%d show", i + 1);
					App.write(buf, Meters_Manager->show_hide(i));
					sprintf(buf, "meter widget n.%d max", i + 1);
					App.write(buf, Meters_Manager->maxv(i));
					sprintf(buf, "meter widget n.%d min", i + 1);
					App.write(buf, Meters_Manager->minv(i));
					sprintf(buf, "meter widget n.%d bg r", i + 1);
					App.write(buf, Meters_Manager->b_color(i, R_COLOR));
					sprintf(buf, "meter widget n.%d bg g", i + 1);
					App.write(buf, Meters_Manager->b_color(i, G_COLOR));
					sprintf(buf, "meter widget n.%d bg b", i + 1);
					App.write(buf, Meters_Manager->b_color(i, B_COLOR));
					sprintf(buf, "meter widget n.%d arrow r", i + 1);
					App.write(buf, Meters_Manager->a_color(i, R_COLOR));
					sprintf(buf, "meter widget n.%d arrow g", i + 1);
					App.write(buf, Meters_Manager->a_color(i, G_COLOR));
					sprintf(buf, "meter widget n.%d arrow b", i + 1);
					App.write(buf, Meters_Manager->a_color(i, B_COLOR));
					sprintf(buf, "meter widget n.%d grid r", i + 1);
					App.write(buf, Meters_Manager->g_color(i, R_COLOR));
					sprintf(buf, "meter widget n.%d grid g", i + 1);
					App.write(buf, Meters_Manager->g_color(i, G_COLOR));
					sprintf(buf, "meter widget n.%d grid b", i + 1);
					App.write(buf, Meters_Manager->g_color(i, B_COLOR));
					sprintf(buf, "meter widget n.%d x", i + 1);
					App.write(buf, Meters_Manager->mw_x(i));
					sprintf(buf, "meter widget n.%d y", i + 1);
					App.write(buf, Meters_Manager->mw_y(i));
					sprintf(buf, "meter widget n.%d w", i + 1);
					App.write(buf, Meters_Manager->mw_w(i));
					sprintf(buf, "meter widget n.%d h", i + 1);
					App.write(buf, Meters_Manager->mw_h(i));
				}
			}
			if (Synchs_Manager) {
				App.set_section("Synchs Manager");
				App.write("x", Synchs_Manager->x());
				App.write("y", Synchs_Manager->y());
				App.write("w", Synchs_Manager->w());
				App.write("h", Synchs_Manager->h());
				App.write("visible", Synchs_Manager->visible());
				for (int i = 0; i < Num_Synchs; i++) {
					char buf[100];
					sprintf(buf, "synch widget n.%d show", i + 1);
					App.write(buf, Synchs_Manager->show_hide(i));
					sprintf(buf, "synch widget n.%d x", i + 1);
					App.write(buf, Synchs_Manager->sw_x(i));
					sprintf(buf, "synch widget n.%d y", i + 1);
					App.write(buf, Synchs_Manager->sw_y(i));
					sprintf(buf, "synch widget n.%d w", i + 1);
					App.write(buf, Synchs_Manager->sw_w(i));
					sprintf(buf, "synch widget n.%d h", i + 1);
					App.write(buf, Synchs_Manager->sw_h(i));
				}
			}
			break;
		default:
			break;
	}
}

void rlg_quit_cb(Fl_Widget*, void*)
{
	if (Is_Target_Connected) {
		RT_RPC(Target_Interface_Task, DISCONNECT_FROM_TARGET, 0);
	}
	End_App = 1;
	for (int n = 0; n < Num_Scopes; n++) {
		pthread_join(Get_Scope_Data_Thread[n], NULL);
	}
	for (int n = 0; n < Num_Logs; n++) {
		pthread_join(Get_Log_Data_Thread[n], NULL);
	}
	for (int n = 0; n < Num_ALogs; n++) {
		pthread_join(Get_ALog_Data_Thread[n], NULL);
	}
	for (int n = 0; n < Num_Leds; n++) {
		pthread_join(Get_Led_Data_Thread[n], NULL);
	}
	for (int n = 0; n < Num_Meters; n++) {
		pthread_join(Get_Meter_Data_Thread[n], NULL);
	}
	for (int n = 0; n < Num_Synchs; n++) {
		pthread_join(Get_Synch_Data_Thread[n], NULL);
	}
	if (Parameters_Manager) Parameters_Manager->hide();
	if (Scopes_Manager) Scopes_Manager->hide();
	if (Logs_Manager) Logs_Manager->hide();
	if (ALogs_Manager) ALogs_Manager->hide();
	if (Leds_Manager) Leds_Manager->hide();
	if (Meters_Manager) Meters_Manager->hide();
	if (Synchs_Manager) Synchs_Manager->hide();
	rt_send(Target_Interface_Task, CLOSE);
	pthread_join(Target_Interface_Thread, NULL);
	rlg_write_pref(GEOMETRY_PREF, "rtailab");
	End_All = 1;
	return;
}

void rlg_start_stop_cb(Fl_Widget *, void *)
{
	if (Is_Target_Running) {
		if (!fl_ask("Are you sure you want to stop the real time control?")) {
			return;
		}
		RT_RPC(Target_Interface_Task, STOP_TARGET, 0);
	} else {
		RT_RPC(Target_Interface_Task, START_TARGET, 0);
	}
}

void rlg_batch_update_parameters_cb(Fl_Widget *o, void *v)
{
	int i, n;

	for (i = n = 0; i < Parameters_Manager->batch_counter(); i++) {
		n += Parameters_Manager->update_parameter(Batch_Parameters[i].index, Batch_Parameters[i].mat_index, Batch_Parameters[i].value);
	}
	if (Parameters_Manager->batch_counter() > 0 && n == Parameters_Manager->batch_counter()) {
		RT_RPC(Target_Interface_Task, BATCH_DOWNLOAD, 0);
		Parameters_Manager->batch_counter( 0);
	}
}

void rlg_update_parameters_cb(Fl_Widget *o, void *v)
{
	p_idx_T *idx = (p_idx_T *)v;
	int blk = idx->block_idx;
	int prm = idx->param_idx;
	int ind = idx->val_idx;
	int map_offset = Tunable_Blocks[blk].offset + prm;
	double value = atof(((Fl_Float_Input*)o)->value());

	if (Parameters_Manager->batch_download() && Parameters_Manager->batch_counter() < MAX_BATCH_PARAMS) {
		Batch_Parameters[Parameters_Manager->batch_counter()].index = map_offset;
		Batch_Parameters[Parameters_Manager->batch_counter()].value = value;
		Batch_Parameters[Parameters_Manager->increment_batch_counter()].mat_index = ind;
	} else {
		if (Parameters_Manager->update_parameter(map_offset, ind, value)) {
			RT_RPC(Target_Interface_Task, (ind << 20) | (map_offset << 4) | UPDATE_PARAM, 0);
//			RT_RPC(Target_Interface_Task, (map_offset << 4) | UPDATE_PARAM, 0);
//			RT_RPC(Target_Interface_Task, (map_offset << 16) | UPDATE_PARAM, 0);
		}
	}
}

void rlg_upload_parameters_cb(Fl_Widget *o, void *v)
{
	RT_RPC(Target_Interface_Task, GET_PARAMS, 0);
}

void rlg_synchs_mgr_cb(Fl_Widget *, void *)
{
	if (RLG_Main_Menu_Table[15].checked()) {
		if (Synchs_Manager) Synchs_Manager->hide();
		RLG_Main_Menu_Table[15].clear();
		RLG_Synchs_Mgr_Button->clear();
	} else {
		if (Synchs_Manager) Synchs_Manager->show();
		RLG_Main_Menu_Table[15].set();
		RLG_Synchs_Mgr_Button->set();
	}
	RLG_Main_Menu->menu(RLG_Main_Menu_Table);
	RLG_Main_Menu->redraw();
	RLG_Main_Window->redraw();
}

void rlg_meters_mgr_cb(Fl_Widget *, void *)
{
	if (RLG_Main_Menu_Table[14].checked()) {
		if (Meters_Manager) Meters_Manager->hide();
		RLG_Main_Menu_Table[14].clear();
		RLG_Meters_Mgr_Button->clear();
	} else {
		if (Meters_Manager) Meters_Manager->show();
		RLG_Main_Menu_Table[14].set();
		RLG_Meters_Mgr_Button->set();
	}
	RLG_Main_Menu->menu(RLG_Main_Menu_Table);
	RLG_Main_Menu->redraw();
	RLG_Main_Window->redraw();
}

void rlg_leds_mgr_cb(Fl_Widget *, void *)
{
	if (RLG_Main_Menu_Table[13].checked()) {
		if (Leds_Manager) Leds_Manager->hide();
		RLG_Main_Menu_Table[13].clear();
		RLG_Leds_Mgr_Button->clear();
	} else {
		if (Leds_Manager) Leds_Manager->show();
		RLG_Main_Menu_Table[13].set();
		RLG_Leds_Mgr_Button->set();
	}
	RLG_Main_Menu->menu(RLG_Main_Menu_Table);
	RLG_Main_Menu->redraw();
	RLG_Main_Window->redraw();
}

void rlg_logs_mgr_cb(Fl_Widget *, void *)
{
	if (RLG_Main_Menu_Table[11].checked()) {
		if (Logs_Manager) Logs_Manager->hide();
		RLG_Main_Menu_Table[11].clear();
		RLG_Logs_Mgr_Button->clear();
	} else {
		if (Logs_Manager) Logs_Manager->show();
		RLG_Main_Menu_Table[11].set();
		RLG_Logs_Mgr_Button->set();
	}
	RLG_Main_Menu->menu(RLG_Main_Menu_Table);
	RLG_Main_Menu->redraw();
	RLG_Main_Window->redraw();
}
void rlg_alogs_mgr_cb(Fl_Widget *, void *)                     
{
	if (RLG_Main_Menu_Table[12].checked()) {
		if (ALogs_Manager) ALogs_Manager->hide();
		RLG_Main_Menu_Table[12].clear();
		RLG_ALogs_Mgr_Button->clear();
	} else {
		if (ALogs_Manager) ALogs_Manager->show();
		RLG_Main_Menu_Table[12].set();
		RLG_ALogs_Mgr_Button->set();
	}
	RLG_Main_Menu->menu(RLG_Main_Menu_Table);
	RLG_Main_Menu->redraw();
	RLG_Main_Window->redraw();
}
void rlg_scopes_mgr_cb(Fl_Widget *, void *)
{
	if (RLG_Main_Menu_Table[10].checked()) {
		if (Scopes_Manager) Scopes_Manager->hide();
		RLG_Main_Menu_Table[10].clear();
		RLG_Scopes_Mgr_Button->clear();
	} else {
		if (Scopes_Manager) Scopes_Manager->show();
		RLG_Main_Menu_Table[10].set();
		RLG_Scopes_Mgr_Button->set();
	}
	RLG_Main_Menu->menu(RLG_Main_Menu_Table);
	RLG_Main_Menu->redraw();
	RLG_Main_Window->redraw();
}

void rlg_params_mgr_cb(Fl_Widget *, void *)
{
	if (RLG_Main_Menu_Table[9].checked()) {
		if (Parameters_Manager) Parameters_Manager->hide();
		RLG_Main_Menu_Table[9].clear();
		RLG_Params_Mgr_Button->clear();
	} else {
		if (Parameters_Manager) Parameters_Manager->show();
		RLG_Main_Menu_Table[9].set();
		RLG_Params_Mgr_Button->set();
	}
	RLG_Main_Menu->menu(RLG_Main_Menu_Table);
	RLG_Main_Menu->redraw();
	RLG_Main_Window->redraw();
}

void rlg_connect_wprofile_cb(Fl_Widget *, void *)
{
	Fl_Dialog& dialog = *RLG_Connect_wProfile_Dialog;

	dialog.x(RLG_Main_Window->x() + (int)((RLG_Main_Window->w()-dialog.w())/2));
	dialog.y(RLG_Main_Window->y() + (int)((RLG_Main_Window->h()-dialog.h())/2));

	RLG_Connect_wProfile_Tree->clear();
	for (int i = 0; i < Preferences.Num_Profiles; i++) {
		add_paper(RLG_Connect_wProfile_Tree, Preferences.Profiles.item(i).c_str(), Fl_Image::read_xpm(0, profile_icon));
	}
	RLG_Connect_wProfile_Tree->redraw();
	RLG_Connect_wProfile_Tree->relayout();

	switch (dialog.show_modal()) {
		case FL_DLG_OK:
			if (Preferences.Num_Profiles > 0) {
				RT_RPC(Target_Interface_Task, CONNECT_TO_TARGET_WITH_PROFILE, 0);
			}
			break;
		case FL_DLG_CANCEL:
			break;
	}
}

void rlg_delete_profile_cb(Fl_Widget *, void *)
{
	Fl_Dialog& dialog = *RLG_Delete_Profile_Dialog;

	dialog.x(RLG_Main_Window->x() + (int)((RLG_Main_Window->w()-dialog.w())/2));
	dialog.y(RLG_Main_Window->y() + (int)((RLG_Main_Window->h()-dialog.h())/2));

	switch (dialog.show_modal()) {
		case FL_DLG_OK:
			if (Preferences.Num_Profiles > 0) {
				Fl_Browser* tree = (Fl_Browser*) RLG_Delete_Profile_Tree;
				Fl_Widget* w = tree->goto_focus();
				Preferences.Num_Profiles--;
				Preferences.Profiles.remove(w->label().c_str());
				if (!strcmp(Preferences.Current_Profile, w->label().c_str())) {
					Preferences.Current_Profile = strdup(Preferences.Profiles.item(0).c_str());
				}
				if (w) {
					Fl_Group* g = w->parent();
					g->remove(w);
					delete w;
					g->relayout();
				}
				rlg_write_pref(PROFILES_PREF, "rtailab");
			}
			RLG_Delete_Profile_Dialog->redraw();
			RLG_Main_Window->redraw();
			break;
        	case FL_DLG_CANCEL:
			break;
	}
}

void rlg_save_profile_cb(Fl_Widget *, void *)
{
	Fl_Dialog& dialog = *RLG_Save_Profile_Dialog;

	dialog.x(RLG_Main_Window->x() + (int)((RLG_Main_Window->w()-dialog.w())/2));
	dialog.y(RLG_Main_Window->y() + (int)((RLG_Main_Window->h()-dialog.h())/2));

	RLG_Save_Profile_Input->value(Preferences.Current_Profile);

	switch (dialog.show_modal()) {
		case FL_DLG_OK:
			if (strlen(strdup(RLG_Save_Profile_Input->value())) > 0) {
				if (Preferences.Num_Profiles == 0) {
					Preferences.Num_Profiles++;
					Preferences.Current_Profile = strdup(RLG_Save_Profile_Input->value());
					Preferences.Profiles.append(strdup(RLG_Save_Profile_Input->value()));
					add_paper(RLG_Connect_wProfile_Tree, RLG_Save_Profile_Input->value(), Fl_Image::read_xpm(0, profile_icon));
					add_paper(RLG_Delete_Profile_Tree, RLG_Save_Profile_Input->value(), Fl_Image::read_xpm(0, profile_icon));
					rlg_write_pref(PROFILES_PREF, "rtailab");
					rlg_write_pref(PROFILE_PREF, strdup(RLG_Save_Profile_Input->value()));
					Profile = (Profile_T *)realloc(Profile, Preferences.Num_Profiles*sizeof(Profile_T));
					rlg_read_pref(PROFILE_PREF, strdup(RLG_Save_Profile_Input->value()), Preferences.Num_Profiles - 1);
				} else {
					for (int i = 0; i < Preferences.Num_Profiles; i++) {
						if (!strcmp(strdup(RLG_Save_Profile_Input->value()), Preferences.Profiles.item(i).c_str())) {
							Preferences.Current_Profile = strdup(RLG_Save_Profile_Input->value());
							rlg_write_pref(PROFILES_PREF, "rtailab");
							rlg_write_pref(PROFILE_PREF, strdup(RLG_Save_Profile_Input->value()));
							goto nothing_else_to_do;
						}
					}
					Preferences.Num_Profiles++;
					Preferences.Current_Profile = strdup(RLG_Save_Profile_Input->value());
					Preferences.Profiles.append(strdup(RLG_Save_Profile_Input->value()));
					add_paper(RLG_Connect_wProfile_Tree, RLG_Save_Profile_Input->value(), Fl_Image::read_xpm(0, profile_icon));
					add_paper(RLG_Delete_Profile_Tree, RLG_Save_Profile_Input->value(), Fl_Image::read_xpm(0, profile_icon));
					rlg_write_pref(PROFILES_PREF, "rtailab");
					rlg_write_pref(PROFILE_PREF, strdup(RLG_Save_Profile_Input->value()));
					Profile = (Profile_T *)realloc(Profile, Preferences.Num_Profiles*sizeof(Profile_T));
					rlg_read_pref(PROFILE_PREF, strdup(RLG_Save_Profile_Input->value()), Preferences.Num_Profiles - 1);
				}
			}
nothing_else_to_do:
			RLG_Main_Window->redraw();
			break;
        	case FL_DLG_CANCEL:
			break;
	}
}

void rlg_text_cb(Fl_Widget *w, void *v)
{
	static int i = 0;

	i = 1 - i;

	if (i) {
		RLG_Text_Button->set();
		RLG_Text_Window->show();
	} else {
		RLG_Text_Button->clear();
		RLG_Text_Window->hide();
	}
	RLG_Main_Window->redraw();
}

void rlg_connect_cb(Fl_Widget *, void *)
{
	Fl_Dialog& dialog = *RLG_Connect_Dialog;

	dialog.x(RLG_Main_Window->x() + (int)((RLG_Main_Window->w()-dialog.w())/2));
	dialog.y(RLG_Main_Window->y() + (int)((RLG_Main_Window->h()-dialog.h())/2));

	rlg_read_pref(CONNECT_PREF, "rtailab", 0);

	switch (dialog.show_modal()) {
		case FL_DLG_OK:
			rlg_write_pref(CONNECT_PREF, "rtailab");
			RT_RPC(Target_Interface_Task, CONNECT_TO_TARGET, 0);
			break;
		case FL_DLG_CANCEL:
			break;
	}
}

void rlg_disconnect_cb(Fl_Widget *, void *)
{
	if (!fl_ask("Do you want to disconnect?")) {
		return;
	}
	RT_RPC(Target_Interface_Task, DISCONNECT_FROM_TARGET, 0);
}

static void *rt_get_synch_data(void *arg)
{
	RT_TASK *GetSynchronoscopeDataTask;
	MBX *GetSynchronoscopeDataMbx;
	char GetSynchronoscopeDataMbxName[7];
	long GetSynchronoscopeDataPort;
	int MsgData = 0, MsgLen, MaxMsgLen, DataBytes;
	float MsgBuf[MAX_MSG_LEN/sizeof(float)];
	int n;
	int index = ((Args_T *)arg)->index;
	char *mbx_id = strdup(((Args_T *)arg)->mbx_id);
	int x = ((Args_T *)arg)->x;
	int y = ((Args_T *)arg)->y;
	int w = ((Args_T *)arg)->w;
	int h = ((Args_T *)arg)->h;

	rt_allow_nonroot_hrt();
	if (!(GetSynchronoscopeDataTask = rt_task_init_schmod(get_an_id("HGY"), 99, 0, 0, SCHED_RR, 0xFF))) {
		printf("Cannot init Host GetSynchronoscopeData Task\n");
		return (void *)1;
	}
	if(Target_Node==0) GetSynchronoscopeDataPort=0;
	else
	  GetSynchronoscopeDataPort = rt_request_port(Target_Node);
	sprintf(GetSynchronoscopeDataMbxName, "%s%d", mbx_id, index);
	if (!(GetSynchronoscopeDataMbx = (MBX *)RT_get_adr(Target_Node, GetSynchronoscopeDataPort, GetSynchronoscopeDataMbxName))) {
		printf("Error in getting %s mailbox address\n", GetSynchronoscopeDataMbxName);
		exit(1);
	}
	DataBytes = sizeof(float);
	MaxMsgLen = (MAX_MSG_LEN/DataBytes)*DataBytes;
	MsgLen = (((int)(DataBytes*REFRESH_RATE*(1./Synchs[index].dt)))/DataBytes)*DataBytes;
	if (MsgLen < DataBytes) MsgLen = DataBytes;
	if (MsgLen > MaxMsgLen) MsgLen = MaxMsgLen;
	MsgData = MsgLen/DataBytes;

	Fl_Synch_Window *Synch_Win = new Fl_Synch_Window(x, y, w, h, RLG_Main_Workspace->viewport(), Synchs[index].name);
	Synchs_Manager->Synch_Windows[index] = Synch_Win;

	rt_send(Target_Interface_Task, 0);
	mlockall(MCL_CURRENT | MCL_FUTURE);

	while (true) {
		if (End_App || !Is_Target_Connected) break;
		while (RT_mbx_receive_if(Target_Node, GetSynchronoscopeDataPort, GetSynchronoscopeDataMbx, &MsgBuf, MsgLen)) {
			if (End_App || !Is_Target_Connected) goto end;
			if (!Synchs_Manager->visible()) {
				Fl::lock();
				RLG_Synchs_Mgr_Button->activate();
				Fl::unlock();
			}
			if (Synchs[index].visible) {
				Fl::lock();
				Synch_Win->show();
				Fl::unlock();
			} else {
				Fl::lock();
				Synch_Win->hide();
				Fl::unlock();
			}
			msleep(10);
		}
		Fl::lock();
		for (n = 0; n < MsgData; n++) {
			Synch_Win->Synch->value(MsgBuf[n]);
			Synch_Win->Synch->redraw();
		}
		Fl::unlock();
	}
end:
	if (Verbose) {
		printf("Deleting synch thread number...%d\n", index);
	}
	Synch_Win->hide();
	rt_release_port(Target_Node, GetSynchronoscopeDataPort);
	rt_task_delete(GetSynchronoscopeDataTask);

	return 0;
}

static void *rt_get_meter_data(void *arg)
{
	RT_TASK *GetMeterDataTask;
	MBX *GetMeterDataMbx;
	char GetMeterDataMbxName[7];
	long GetMeterDataPort;
	int MsgData = 0, MsgLen, MaxMsgLen, DataBytes;
	float MsgBuf[MAX_MSG_LEN/sizeof(float)];
	int n;
	int index = ((Args_T *)arg)->index;
	char *mbx_id = strdup(((Args_T *)arg)->mbx_id);
	int x = ((Args_T *)arg)->x;
	int y = ((Args_T *)arg)->y;
	int w = ((Args_T *)arg)->w;
	int h = ((Args_T *)arg)->h;

	rt_allow_nonroot_hrt();
	if (!(GetMeterDataTask = rt_task_init_schmod(get_an_id("HGM"), 99, 0, 0, SCHED_RR, 0xFF))) {
		printf("Cannot init Host GetMeterData Task\n");
		return (void *)1;
	}
	if(Target_Node == 0) GetMeterDataPort=0;
	else GetMeterDataPort = rt_request_port(Target_Node);

	sprintf(GetMeterDataMbxName, "%s%d", mbx_id, index);
	if (!(GetMeterDataMbx = (MBX *)RT_get_adr(Target_Node, GetMeterDataPort, GetMeterDataMbxName))) {
		printf("Error in getting %s mailbox address\n", GetMeterDataMbxName);
		exit(1);
	}
	DataBytes = sizeof(float);
	MaxMsgLen = (MAX_MSG_LEN/DataBytes)*DataBytes;
	MsgLen = (((int)(DataBytes*REFRESH_RATE*(1./Meters[index].dt)))/DataBytes)*DataBytes;
	if (MsgLen < DataBytes) MsgLen = DataBytes;
	if (MsgLen > MaxMsgLen) MsgLen = MaxMsgLen;
	MsgData = MsgLen/DataBytes;

	Fl_Meter_Window *Meter_Win = new Fl_Meter_Window(x, y, w, h, RLG_Main_Workspace->viewport(), Meters[index].name);
	Meters_Manager->Meter_Windows[index] = Meter_Win;

	rt_send(Target_Interface_Task, 0);
	mlockall(MCL_CURRENT | MCL_FUTURE);

	while (true) {
		if (End_App || !Is_Target_Connected) break;
		while (RT_mbx_receive_if(Target_Node, GetMeterDataPort, GetMeterDataMbx, &MsgBuf, MsgLen)) {
			if (End_App || !Is_Target_Connected) goto end;
			if (!Meters_Manager->visible()) {
				Fl::lock();
				RLG_Meters_Mgr_Button->activate();
				Fl::unlock();
			}
			if (Meters[index].visible) {
				Fl::lock();
				Meter_Win->show();
				Fl::unlock();
			} else {
				Fl::lock();
				Meter_Win->hide();
				Fl::unlock();
			}
			msleep(10);
		}
		Fl::lock();
		for (n = 0; n < MsgData; n++) {
			Meter_Win->Meter->value(MsgBuf[n]);
			Meter_Win->Meter->redraw();
		}
		Fl::unlock();
	}
end:
	if (Verbose) {
		printf("Deleting meter thread number...%d\n", index);
	}
	Meter_Win->hide();
	rt_release_port(Target_Node, GetMeterDataPort);
	rt_task_delete(GetMeterDataTask);

	return 0;
}

static void *rt_get_led_data(void *arg)
{
	RT_TASK *GetLedDataTask;
	MBX *GetLedDataMbx;
	char GetLedDataMbxName[7];
	long GetLedDataPort;
	int MsgData = 0, MsgLen, MaxMsgLen, DataBytes;
	unsigned int MsgBuf[MAX_MSG_LEN/sizeof(unsigned int)];
	int n;
	int index = ((Args_T *)arg)->index;
	char *mbx_id = strdup(((Args_T *)arg)->mbx_id);
	int x = ((Args_T *)arg)->x;
	int y = ((Args_T *)arg)->y;
	int w = ((Args_T *)arg)->w;
	int h = ((Args_T *)arg)->h;
	unsigned int Led_Mask = 0;

	rt_allow_nonroot_hrt();
	if (!(GetLedDataTask = rt_task_init_schmod(get_an_id("HGE"), 99, 0, 0, SCHED_RR, 0xFF))) {
		printf("Cannot init Host GetLedData Task\n");
		return (void *)1;
	}
	if(Target_Node == 0) GetLedDataPort=0;
	else GetLedDataPort = rt_request_port(Target_Node);
	sprintf(GetLedDataMbxName, "%s%d", mbx_id, index);
	if (!(GetLedDataMbx = (MBX *)RT_get_adr(Target_Node, GetLedDataPort, GetLedDataMbxName))) {
		printf("Error in getting %s mailbox address\n", GetLedDataMbxName);
		exit(1);
	}
	DataBytes = sizeof(unsigned int);
	MaxMsgLen = (MAX_MSG_LEN/DataBytes)*DataBytes;
	MsgLen = (((int)(DataBytes*REFRESH_RATE*(1./Leds[index].dt)))/DataBytes)*DataBytes;
	if (MsgLen < DataBytes) MsgLen = DataBytes;
	if (MsgLen > MaxMsgLen) MsgLen = MaxMsgLen;
	MsgData = MsgLen/DataBytes;

	Fl_Led_Window *Led_Win = new Fl_Led_Window(x, y, w, h, RLG_Main_Workspace->viewport(), Leds[index].name, Leds[index].n_leds);
	Leds_Manager->Led_Windows[index] = Led_Win;

	rt_send(Target_Interface_Task, 0);
	mlockall(MCL_CURRENT | MCL_FUTURE);

	while (true) {
		if (End_App || !Is_Target_Connected) break;
		while (RT_mbx_receive_if(Target_Node, GetLedDataPort, GetLedDataMbx, &MsgBuf, MsgLen)) {
			if (End_App || !Is_Target_Connected) goto end;
			if (!Leds_Manager->visible()) {
				Fl::lock();
				RLG_Leds_Mgr_Button->activate();
				Fl::unlock();
			}
			if (Leds[index].visible) {
				Fl::lock();
				Led_Win->show();
				Fl::unlock();
			} else {
				Fl::lock();
				Led_Win->hide();
				Fl::unlock();
			}
			msleep(10);
		}
		Fl::lock();
		for (n = 0; n < MsgData; n++) {
			Led_Mask = MsgBuf[n];
			Led_Win->led_mask(Led_Mask);
			Led_Win->led_on_off();
			Led_Win->update();
		}
		Fl::unlock();
	}
end:
	if (Verbose) {
		printf("Deleting led thread number...%d\n", index);
	}
	Led_Win->hide();
	rt_release_port(Target_Node, GetLedDataPort);
	rt_task_delete(GetLedDataTask);

	return 0;
}

static void *rt_get_log_data(void *arg)
{
	RT_TASK *GetLogDataTask;
	MBX *GetLogDataMbx;
	char GetLogDataMbxName[7];
	long GetLogDataPort;
	int MsgData = 0, MsgLen, MaxMsgLen, DataBytes;
	float MsgBuf[MAX_MSG_LEN/sizeof(float)];
	int n, i, j, k, DataCnt = 0;
	int index = ((Args_T *)arg)->index;
	char *mbx_id = strdup(((Args_T *)arg)->mbx_id);

	rt_allow_nonroot_hrt();
	if (!(GetLogDataTask = rt_task_init_schmod(get_an_id("HGL"), 99, 0, 0, SCHED_RR, 0xFF))) {
		printf("Cannot init Host GetLogData Task\n");
		return (void *)1;
	}
	if (Target_Node == 0) GetLogDataPort = 0;
	else GetLogDataPort = rt_request_port(Target_Node);
	sprintf(GetLogDataMbxName, "%s%d", mbx_id, index);
	if (!(GetLogDataMbx = (MBX *)RT_get_adr(Target_Node, GetLogDataPort, GetLogDataMbxName))) {
		printf("Error in getting %s mailbox address\n", GetLogDataMbxName);
		exit(1);
	}
	DataBytes = (Logs[index].nrow*Logs[index].ncol)*sizeof(float);
	MaxMsgLen = (MAX_MSG_LEN/DataBytes)*DataBytes;
	MsgLen = (((int)(DataBytes*REFRESH_RATE*(1./Logs[index].dt)))/DataBytes)*DataBytes;
	if (MsgLen < DataBytes) MsgLen = DataBytes;
	if (MsgLen > MaxMsgLen) MsgLen = MaxMsgLen;
	MsgData = MsgLen/DataBytes;

	rt_send(Target_Interface_Task, 0);
	mlockall(MCL_CURRENT | MCL_FUTURE);

	while (true) {
		if (End_App || !Is_Target_Connected) break;
		while (RT_mbx_receive_if(Target_Node, GetLogDataPort, GetLogDataMbx, &MsgBuf, MsgLen)) {
			if (End_App || !Is_Target_Connected) goto end;
			if (!Logs_Manager->visible()) {
				Fl::lock();
				RLG_Logs_Mgr_Button->activate();
				Fl::unlock();
			}
			msleep(10);
		}
		if (Logs_Manager->start_saving(index)) {
			for (n = 0; n < MsgData; n++) {
				++DataCnt;
//				fprintf(Logs_Manager->save_file(index), "Data # %d\n", ++DataCnt);
				for (i = 0; i < Logs[index].nrow; i++) {
					j = n*Logs[index].nrow*Logs[index].ncol + i;
					for (k = 0; k < Logs[index].ncol; k++) {
						fprintf(Logs_Manager->save_file(index), "%1.5f ", MsgBuf[j]);
						j += Logs[index].nrow;
					}
					fprintf(Logs_Manager->save_file(index), "\n");
				}
				if (DataCnt == Logs_Manager->n_points_to_save(index)) {
					Logs_Manager->stop_saving(index);
					DataCnt = 0;
					break;
				}
			}
		}
	}
end:
	if (Verbose) {
		printf("Deleting log thread number...%d\n", index);
	}
	rt_release_port(Target_Node, GetLogDataPort);
	rt_task_delete(GetLogDataTask);

	return 0;
}
static void *rt_get_alog_data(void *arg)
{
	RT_TASK *GetALogDataTask;				
	MBX *GetALogDataMbx;
	char GetALogDataMbxName[7];
	long GetALogDataPort;
	int MsgData = 0, MsgLen, MaxMsgLen, DataBytes;
	float MsgBuf[MAX_MSG_LEN/sizeof(float)];
	int n, i, j, k;
	int index = ((Alog_T *)arg)->index;
	char *mbx_id = strdup(((Alog_T *)arg)->mbx_id);
	char *alog_file_name = strdup(((Alog_T *)arg)->alog_name);   //read alog block name and set it to file name
	FILE *saving;
	long size_counter = 0;
	long logging = 0;
	
	
	if((saving = fopen(alog_file_name, "a+")) == NULL){
		printf("Error opening auto log file %s\n", alog_file_name);
		}
	
	rt_allow_nonroot_hrt();
	
	if (!(GetALogDataTask = rt_task_init_schmod(get_an_id("HGA"), 99, 0, 0, SCHED_RR, 0xFF))) {
		printf("Cannot init Host GetALogData Task\n");
		return (void *)1;
	}

	if (Target_Node == 0) GetALogDataPort = 0;
	else GetALogDataPort = rt_request_port(Target_Node);
	sprintf(GetALogDataMbxName, "%s%d", mbx_id, index);

	if (!(GetALogDataMbx = (MBX *)RT_get_adr(Target_Node, GetALogDataPort, GetALogDataMbxName))) {
		printf("Error in getting %s mailbox address\n", GetALogDataMbxName);
		exit(1);
	}
	DataBytes = (ALogs[index].nrow*ALogs[index].ncol)*sizeof(float)+sizeof(float);
	MaxMsgLen = (MAX_MSG_LEN/DataBytes)*DataBytes;
	MsgLen = (((int)(DataBytes*REFRESH_RATE*(1./ALogs[index].dt)))/DataBytes)*DataBytes;
	if (MsgLen < DataBytes) MsgLen = DataBytes;
	if (MsgLen > MaxMsgLen) MsgLen = MaxMsgLen;
	MsgData = MsgLen/DataBytes;
	
	//printf("MsgData %d MsgLen %d MaxMsgLen %d DataBytes %d DimBuf= %d\n", MsgData, MsgLen, MaxMsgLen, DataBytes,MAX_MSG_LEN/sizeof(float));
	
	rt_send(Target_Interface_Task, 0);
	mlockall(MCL_CURRENT | MCL_FUTURE);
	
	while (true) {
		if (End_App || !Is_Target_Connected) break;
		while (RT_mbx_receive_if(Target_Node, GetALogDataPort, GetALogDataMbx, &MsgBuf, MsgLen)) {
			if (End_App || !Is_Target_Connected) goto end;
			if (!ALogs_Manager->visible()) {
				Fl::lock();
				RLG_ALogs_Mgr_Button->activate();
				Fl::unlock();
			}
			msleep(10);
		}
			for (n = 0; n < MsgData; n++) {
				size_counter=ftell(saving);    			//get file dimension in bytes
				//printf("Size counter: %d\n", size_counter);
				if(((int)MsgBuf[(((n+1)*ALogs[index].nrow*ALogs[index].ncol + (n+1))-1)]) &&
				size_counter<=1000000){
					for (i = 0; i < ALogs[index].nrow; i++) {
						j = n*ALogs[index].nrow*ALogs[index].ncol + i;
						for (k = 0; k < ALogs[index].ncol; k++) {
							fprintf(saving,"%1.5f ",MsgBuf[j]); 
							j += ALogs[index].nrow;
						}
						fprintf(saving, "\n");
						j++;
					}
				}
				/*if (size_counter > 100000){
					fclose(saving);
					system("tar -czvf logged.tgz RTAI_ALOG");
					if((saving = fopen(alog_file_name, "a+")) == NULL){
						printf("Error opening auto log file %s\n", alog_file_name);
						}
				}*/		
			}
			
	}
end:
	if (Verbose) {
		printf("Deleting auto log thread number...%d\n", index);
	}
	fclose(saving);
	rt_release_port(Target_Node, GetALogDataPort);
	rt_task_delete(GetALogDataTask);

	return 0;
}
static void *rt_get_scope_data(void *arg)
{
	RT_TASK *GetScopeDataTask;
	MBX *GetScopeDataMbx;
	char GetScopeDataMbxName[7];
	long GetScopeDataPort;
	int MsgData = 0, MsgLen, MaxMsgLen, TracesBytes;
	double MsgBuf[MAX_MSG_LEN/sizeof(double)];
	int n, nn, js, jl;
	int index = ((Args_T *)arg)->index;
	char *mbx_id = strdup(((Args_T *)arg)->mbx_id);
	int x = ((Args_T *)arg)->x;
	int y = ((Args_T *)arg)->y;
	int w = ((Args_T *)arg)->w;
	int h = ((Args_T *)arg)->h;
	int stop_draw = false;
	int save_idx = 0;

	rt_allow_nonroot_hrt();
	if (!(GetScopeDataTask = rt_task_init_schmod(get_an_id("HGS"), 99, 0, 0, SCHED_RR, 0xFF))) {
		printf("Cannot init Host GetScopeData Task\n");
		return (void *)1;
	}
	if(Target_Node == 0) GetScopeDataPort = 0;
	else GetScopeDataPort = rt_request_port(Target_Node);
	sprintf(GetScopeDataMbxName, "%s%d", mbx_id, index);
	if (!(GetScopeDataMbx = (MBX *)RT_get_adr(Target_Node, GetScopeDataPort, GetScopeDataMbxName))) {
		printf("Error in getting %s mailbox address\n", GetScopeDataMbxName);
		return (void *)1;
	}
	TracesBytes = (Scopes[index].ntraces + 1)*sizeof(double);
	MaxMsgLen = (MAX_MSG_LEN/TracesBytes)*TracesBytes;
	MsgLen = (((int)(TracesBytes*REFRESH_RATE*(1./Scopes[index].dt)))/TracesBytes)*TracesBytes;
	if (MsgLen < TracesBytes) MsgLen = TracesBytes;
	if (MsgLen > MaxMsgLen) MsgLen = MaxMsgLen;
	MsgData = MsgLen/TracesBytes;

	Fl_Scope_Window *Scope_Win = new Fl_Scope_Window(x, y, w, h, RLG_Main_Workspace->viewport(), Scopes[index].name, Scopes[index].ntraces, Scopes[index].dt);
	Scopes_Manager->Scope_Windows[index] = Scope_Win;

	rt_send(Target_Interface_Task, 0);
	mlockall(MCL_CURRENT | MCL_FUTURE);

	while (true) {
		if (End_App || !Is_Target_Connected) break;
		while (RT_mbx_receive_if(Target_Node, GetScopeDataPort, GetScopeDataMbx, &MsgBuf, MsgLen)) {
			if (End_App || !Is_Target_Connected) goto end;
			if (!Scopes_Manager->visible()) {
				Fl::lock();
				RLG_Scopes_Mgr_Button->activate();
				Fl::unlock();
			}
			if (Scopes[index].visible) {
				Fl::lock();
				Scope_Win->show();
				Fl::unlock();
			} else {
				Fl::lock();
				Scope_Win->hide();
				Fl::unlock();
			}
			msleep(10);
		}
		Fl::lock();
		js = 1; // Drop sampletime
		for (n = 0; n < MsgData; n++) {
			for (nn = 0; nn < Scopes[index].ntraces; nn++) {
				Scope_Win->Plot->add_to_trace(nn, MsgBuf[js++]);
			}
			js++;
		}
		if (Scope_Win->is_visible() && (!stop_draw)) {
			Scope_Win->Plot->redraw();
		}
		if (Scopes_Manager->start_saving(index)) {
			jl = 0;
			for (n = 0; n < MsgData; n++) {
				for (nn = 0; nn < Scopes[index].ntraces + 1; nn++) {
					fprintf(Scopes_Manager->save_file(index), "%1.5f ", MsgBuf[jl++]);
				}
				fprintf(Scopes_Manager->save_file(index), "\n");
				save_idx++;
				if (save_idx == Scopes_Manager->n_points_to_save(index)) {
					Scopes_Manager->stop_saving(index);
					save_idx = 0;
					break;
				}
			}
		}
		Fl::unlock();
	}

end:
	if (Verbose) {
		printf("Deleting scope thread number...%d\n", index);
	}
	Scope_Win->hide();
	rt_release_port(Target_Node, GetScopeDataPort);
	rt_task_delete(GetScopeDataTask);

	return 0;
}

double get_parameter(Target_Parameters_T p, int nr, int nc, int *val_idx)
{
	switch (p.data_class) {
		case rt_SCALAR:
			*val_idx = 0;
			return (p.data_value[0]);
		case rt_VECTOR:
			*val_idx = nc;
			return (p.data_value[nc]);
		case rt_MATRIX_ROW_MAJOR:
			*val_idx = nr*p.n_cols+nc;
			return (p.data_value[nr*p.n_cols+nc]);
		case rt_MATRIX_COL_MAJOR:
			*val_idx = nc*p.n_rows+nr;
			return (p.data_value[nc*p.n_rows+nr]);
		default:
			return (0.0);
	}
}

static long try_to_connect(const char *IP)
{
	int counter = 0;
	int counter_max = 5;
	char buf[100];
	long port;
	struct sockaddr_in addr;

	sprintf(buf, "Trying to connect to %s", IP);
	RLG_Main_Status->label(buf);
	RLG_Connect_Button->deactivate();
	RLG_Main_Window->redraw();
	if (Verbose) {
		printf("%s...", buf);
		fflush(stdout);
	}
	inet_aton(IP, &addr.sin_addr);
	Target_Node = addr.sin_addr.s_addr;
	while ((port = rt_request_port(Target_Node)) <= 0 && counter++ <= counter_max) {
		msleep(100);
	}

	return port;
}

static int get_parameters_info(long port, RT_TASK *task)
{
	unsigned int req = 'c';
	char c_req = 'i';
	int blk_index = 0;
	int n_params = 0;

	RT_rpc(Target_Node, port, task, req, &Is_Target_Running);
	n_params = Is_Target_Running & 0xffff;
	Is_Target_Running >>= 16;
	if (n_params > 0) Tunable_Parameters = new Target_Parameters_T [n_params];
	else               Tunable_Parameters = new Target_Parameters_T [1];

	RT_rpcx(Target_Node, port, task, &c_req, &Tunable_Parameters[0], sizeof(char), sizeof(Target_Parameters_T));
        RLG_Target_Name = strdup(Tunable_Parameters[0].model_name);

	for (int n = 0; n < n_params; n++) {
		RT_rpcx(Target_Node, port, task, &c_req, &Tunable_Parameters[n], sizeof(char), sizeof(Target_Parameters_T));
		if (n > 0) {
			if (strcmp(Tunable_Parameters[n-1].block_name, Tunable_Parameters[n].block_name)) {
				Num_Tunable_Blocks++;
			}
		} else {
			Num_Tunable_Blocks = 1;
		}
	}
	if (Num_Tunable_Blocks > 0) Tunable_Blocks = new Target_Blocks_T [Num_Tunable_Blocks];
	blk_index = 0;
	for (int n = 0; n < n_params; n++) {
		if (n > 0) {
			if (strcmp(Tunable_Parameters[n-1].block_name, Tunable_Parameters[n].block_name)) {
				blk_index++;
				strncpy(Tunable_Blocks[blk_index].name, Tunable_Parameters[n].block_name + strlen(Tunable_Parameters[0].model_name) + 1, MAX_NAMES_SIZE);
				Tunable_Blocks[blk_index].offset = n;
			}
		} else {
			strncpy(Tunable_Blocks[0].name, Tunable_Parameters[0].block_name + strlen(Tunable_Parameters[0].model_name) + 1, MAX_NAMES_SIZE);
			Tunable_Blocks[0].offset = 0;
		}
	}

	return n_params;

}

static void upload_parameters_info(long port, RT_TASK *task)
{
	unsigned int req = 'g';
	char c_req = 'i';
	int n;

	RT_rpc(Target_Node, port, task, req, &Is_Target_Running);
	if (Verbose) {
		printf("Upload parameters...\n");
	}
	for (n = 0; n < Num_Tunable_Parameters; n++) {
		RT_rpcx(Target_Node, port, task, &c_req, &Tunable_Parameters[n], sizeof(char), sizeof(Target_Parameters_T));
	}
}

static int get_scope_blocks_info(long port, RT_TASK *task, const char *mbx_id)
{
	int n_scopes = 0;
	int req = -1, msg;

	for (int n = 0; n < MAX_RTAI_SCOPES; n++) {
		char mbx_name[7];
		sprintf(mbx_name, "%s%d", mbx_id, n);
		if (!RT_get_adr(Target_Node, port, mbx_name)) {
			n_scopes = n;
			break;
		}
	}
	if (n_scopes > 0) Scopes = new Target_Scopes_T [n_scopes];
	for (int n = 0; n < n_scopes; n++) {
		char name[MAX_NAMES_SIZE];
		Scopes[n].visible = false;
		RT_rpcx(Target_Node, port, task, &n, &Scopes[n].ntraces, sizeof(int), sizeof(int));
		RT_rpcx(Target_Node, port, task, &n, &name, sizeof(int), sizeof(name));
		Scopes[n].name = (char*)malloc(strlen(name)+1);
		strcpy(Scopes[n].name, name);
		Scopes[n].traceName = (char**)malloc(sizeof(char*)*Scopes[n].ntraces);
		int j;
		for(j=0; j<Scopes[n].ntraces; j++) {
			RT_rpcx(Target_Node, port, task, &j, &name, sizeof(int), sizeof(name));
			Scopes[n].traceName[j] = (char*)malloc(strlen(name)+1);
			strcpy(Scopes[n].traceName[j], name);
		}
		j=-1;
		RT_rpcx(Target_Node, port, task, &j, &Scopes[n].dt, sizeof(int), sizeof(float));
	}
	RT_rpcx(Target_Node, port, task, &req, &msg, sizeof(int), sizeof(int));

	return n_scopes;
}

static int get_log_blocks_info(long port, RT_TASK *task, const char *mbx_id)
{
	int n_logs = 0;
	int req = -1, msg;

	for (int n = 0; n < MAX_RTAI_LOGS; n++) {
		char mbx_name[7];
		sprintf(mbx_name, "%s%d", mbx_id, n);
		if (!RT_get_adr(Target_Node, port, mbx_name)) {
			n_logs = n;
			break;
		}
	}
	if (n_logs > 0) Logs = new Target_Logs_T [n_logs];
	for (int n = 0; n < n_logs; n++) {
		char log_name[MAX_NAMES_SIZE];
		RT_rpcx(Target_Node, port, task, &n, &Logs[n].nrow, sizeof(int), sizeof(int));
		RT_rpcx(Target_Node, port, task, &n, &Logs[n].ncol, sizeof(int), sizeof(int));
		RT_rpcx(Target_Node, port, task, &n, &log_name, sizeof(int), sizeof(log_name));
		strncpy(Logs[n].name, log_name, MAX_NAMES_SIZE);
		RT_rpcx(Target_Node, port, task, &n, &Logs[n].dt, sizeof(int), sizeof(float));
	}
	RT_rpcx(Target_Node, port, task, &req, &msg, sizeof(int), sizeof(int));

	return n_logs;
}

static int get_alog_blocks_info(long port, RT_TASK *task, const char *mbx_id)            //added il 4/5/2005
{
	int n_alogs = 0;
	int req = -1, msg;
	for (int n = 0; n < MAX_RTAI_LOGS; n++) {
		char mbx_name[7];
		sprintf(mbx_name, "%s%d", mbx_id, n);
		if (!RT_get_adr(Target_Node, port, mbx_name)) {
			n_alogs = n;
			break;
		}
	}
	if (n_alogs > 0) ALogs = new Target_ALogs_T [n_alogs];
	for (int n = 0; n < n_alogs; n++) {
		char alog_name[MAX_NAMES_SIZE];
		RT_rpcx(Target_Node, port, task, &n, &ALogs[n].nrow, sizeof(int), sizeof(int));
		RT_rpcx(Target_Node, port, task, &n, &ALogs[n].ncol, sizeof(int), sizeof(int));
		RT_rpcx(Target_Node, port, task, &n, &alog_name, sizeof(int), sizeof(alog_name));
		strncpy(ALogs[n].name, alog_name, MAX_NAMES_SIZE);
		RT_rpcx(Target_Node, port, task, &n, &ALogs[n].dt, sizeof(int), sizeof(float));
	}
	RT_rpcx(Target_Node, port, task, &req, &msg, sizeof(int), sizeof(int));

	return n_alogs;
}

static int get_led_blocks_info(long port, RT_TASK *task, const char *mbx_id)
{
	int n_leds = 0;
	int req = -1, msg;

	for (int n = 0; n < MAX_RTAI_LEDS; n++) {
		char mbx_name[7];
		sprintf(mbx_name, "%s%d", mbx_id, n);
		if (!RT_get_adr(Target_Node, port, mbx_name)) {
			n_leds = n;
			break;
		}
	}
	if (n_leds > 0) Leds = new Target_Leds_T [n_leds];
	for (int n = 0; n < n_leds; n++) {
		char led_name[MAX_NAMES_SIZE];
		Leds[n].visible = false;
		RT_rpcx(Target_Node, port, task, &n, &Leds[n].n_leds, sizeof(int), sizeof(int));
		RT_rpcx(Target_Node, port, task, &n, &led_name, sizeof(int), sizeof(led_name));
		strncpy(Leds[n].name, led_name, MAX_NAMES_SIZE);
		RT_rpcx(Target_Node, port, task, &n, &Leds[n].dt, sizeof(int), sizeof(float));
	}
	RT_rpcx(Target_Node, port, task, &req, &msg, sizeof(int), sizeof(int));

	return n_leds;
}

static int get_meter_blocks_info(long port, RT_TASK *task, const char *mbx_id)
{
	int n_meters = 0;
	int req = -1, msg;

	for (int n = 0; n < MAX_RTAI_METERS; n++) {
		char mbx_name[7];
		sprintf(mbx_name, "%s%d", mbx_id, n);
		if (!RT_get_adr(Target_Node, port, mbx_name)) {
			n_meters = n;
			break;
		}
	}
	if (n_meters > 0) Meters = new Target_Meters_T [n_meters];
	for (int n = 0; n < n_meters; n++) {
		char meter_name[MAX_NAMES_SIZE];
		Meters[n].visible = false;
		RT_rpcx(Target_Node, port, task, &n, &meter_name, sizeof(int), sizeof(meter_name));
		strncpy(Meters[n].name, meter_name, MAX_NAMES_SIZE);
		RT_rpcx(Target_Node, port, task, &n, &Meters[n].dt, sizeof(int), sizeof(float));
	}
	RT_rpcx(Target_Node, port, task, &req, &msg, sizeof(int), sizeof(int));

	return n_meters;
}

static int get_synch_blocks_info(long port, RT_TASK *task, const char *mbx_id)
{
	int n_synchs = 0;
	int req = -1, msg;

	for (int n = 0; n < MAX_RTAI_SYNCHS; n++) {
		char mbx_name[7];
		sprintf(mbx_name, "%s%d", mbx_id, n);
		if (!RT_get_adr(Target_Node, port, mbx_name)) {
			n_synchs = n;
			break;
		}
	}
	if (n_synchs > 0) Synchs = new Target_Synchs_T [n_synchs];
	for (int n = 0; n < n_synchs; n++) {
		char synch_name[MAX_NAMES_SIZE];
		Synchs[n].visible = false;
		RT_rpcx(Target_Node, port, task, &n, &synch_name, sizeof(int), sizeof(synch_name));
		strncpy(Synchs[n].name, synch_name, MAX_NAMES_SIZE);
		RT_rpcx(Target_Node, port, task, &n, &Synchs[n].dt, sizeof(int), sizeof(float));
	}
	RT_rpcx(Target_Node, port, task, &req, &msg, sizeof(int), sizeof(int));

	return n_synchs;
}

static void rlg_manager_window(int n_elems, int type, int view_flag, int x, int y, int w, int h)
{
	Fl_MDI_Viewport *v = RLG_Main_Workspace->viewport();

	if (n_elems > 0) {
		switch (type) {
			case PARAMS_MANAGER:
				Parameters_Manager = new Fl_Parameters_Manager(x, y, w, h, v, "Parameters Manager");
				Parameters_Manager->show();
				if (!view_flag) {
					Parameters_Manager->hide();
				} else {
					RLG_Main_Menu_Table[9].set();
					RLG_Params_Mgr_Button->set();
				}
				break;
			case SCOPES_MANAGER:
				Scopes_Manager = new Fl_Scopes_Manager(x, y, w, h, v, "Scopes Manager");
				Scopes_Manager->show();
				if (!view_flag) {
					Scopes_Manager->hide();
				} else {
					RLG_Main_Menu_Table[10].set();
					RLG_Scopes_Mgr_Button->set();
				}
				break;
			case LOGS_MANAGER:
				Logs_Manager = new Fl_Logs_Manager(x, y, w, h, v, "Logs Manager");
				Logs_Manager->show();
				if (!view_flag) {
					Logs_Manager->hide();
				} else {
					RLG_Main_Menu_Table[11].set();
					RLG_Logs_Mgr_Button->set();
				}
				break;
			case ALOGS_MANAGER:
				ALogs_Manager = new Fl_ALogs_Manager(x, y, w, h, v, "ALogs Manager");
				ALogs_Manager->show();
				if (!view_flag) {
					ALogs_Manager->hide();
				} else {
					RLG_Main_Menu_Table[12].set();
					RLG_ALogs_Mgr_Button->set();
				}
				break;	
			case LEDS_MANAGER:
				Leds_Manager = new Fl_Leds_Manager(x, y, w, h, v, "Leds Manager");
				Leds_Manager->show();
				if (!view_flag) {
					Leds_Manager->hide();
				} else {
					RLG_Main_Menu_Table[13].set();
					RLG_Leds_Mgr_Button->set();
				}
				break;
			case METERS_MANAGER:
				Meters_Manager = new Fl_Meters_Manager(x, y, w, h, v, "Meters Manager");
				Meters_Manager->show();
				if (!view_flag) {
					Meters_Manager->hide();
				} else {
					RLG_Main_Menu_Table[14].set();
					RLG_Meters_Mgr_Button->set();
				}
				break;
			case SYNCHS_MANAGER:
				Synchs_Manager = new Fl_Synchs_Manager(x, y, w, h, v, "Synchs Manager");
				Synchs_Manager->show();
				if (!view_flag) {
					Synchs_Manager->hide();
				} else {
					RLG_Main_Menu_Table[15].set();
					RLG_Synchs_Mgr_Button->set();
				}
				break;
			default:
				break;
		}
	}
}

static void rlg_update_after_connect(void)
{
	char buf[128];

	Fl::lock();
	RLG_Save_Profile_Button->activate();
	RLG_Delete_Profile_Button->deactivate();
	RLG_Scopes_Mgr_Button->activate();
	RLG_Logs_Mgr_Button->activate();
	RLG_ALogs_Mgr_Button->activate();
	RLG_Leds_Mgr_Button->activate();
	RLG_Meters_Mgr_Button->activate();
	RLG_Synchs_Mgr_Button->activate();
	Is_Target_Running ? RLG_Stop_Button->activate() : RLG_Start_Button->activate();
	RLG_Connect_Button->deactivate();
	RLG_Connect_wProfile_Button->deactivate();
	RLG_Disconnect_Button->activate();
	RLG_Main_Menu_Table[1].deactivate();
	RLG_Main_Menu_Table[2].activate();
	RLG_Main_Menu_Table[3].deactivate();
	RLG_Main_Menu_Table[4].activate();
	RLG_Main_Menu_Table[5].deactivate();
	for (int i = 9; i <= 14; i++) RLG_Main_Menu_Table[i].activate();
        if(Num_Tunable_Parameters!=0)
	  RLG_Params_Mgr_Button->activate();
	sprintf(buf, "Target: %s.", RLG_Target_Name);
	RLG_Main_Status->label(buf);
	RLG_Main_Menu->menu(RLG_Main_Menu_Table);
	RLG_Main_Menu->redraw();
	RLG_Main_Window->redraw();
	Fl::unlock();
}


static Commands abort_connection(long Target_Port) {
	RLG_Connect_Button->deactivate();
	rt_release_port(Target_Node, Target_Port);
	exit(1);
}

static void *rt_target_interface(void *args)
{
	unsigned int code, U_Request;
	long Target_Port = 0;
	RT_TASK *If_Task = NULL, *task;

	rt_allow_nonroot_hrt();
	if (!(Target_Interface_Task = rt_task_init_schmod(get_an_id("HTI"), 98, 0, 0, SCHED_FIFO, 0xFF))) {
		printf("Cannot init Target_Interface_Task\n");
		exit(1);
	}
	rt_send(RLG_Main_Task, 0);

	while (!End_App) {
		if (!(task = rt_receive(0, &code))) continue;
		if (Verbose) {
			printf("Received code %d from task %p\n", code, task);
		}
		switch (code & 0xf) {

			case CONNECT_TO_TARGET:

				if (Verbose) {
					printf("Reading target settings\n");
				}
				Fl::lock();
				rlg_read_pref(CONNECT_PREF, "rtailab", 0);
				Fl::unlock();
				if (!strcmp(Preferences.Target_IP, "0")) {
					Target_Node = 0;
				} else {
					Target_Port = try_to_connect(Preferences.Target_IP);
					Fl::lock();
					RLG_Connect_Button->activate();
					Fl::unlock();
					if (Target_Port <= 0) {
					        Fl::lock();
						RLG_Main_Status->label("Sorry, no route to target");
						RLG_Main_Window->redraw();
						Fl::unlock();
						if (Verbose) {
							printf(" Sorry, no route to target\n");
						}
						RT_RETURN(task, CONNECT_TO_TARGET);
						break;
					}
					if (Verbose) {
						printf(" Ok\n");
					}
				}
				if (!(If_Task = (RT_TASK *)RT_get_adr(Target_Node, Target_Port, Preferences.Target_Interface_Task_Name))) {
				        Fl::lock();
				        RLG_Main_Status->label("No target or bad interface task identifier");
					RLG_Main_Window->redraw();
					Fl::unlock();
					if (Verbose) {
						printf("No target or bad interface task identifier\n");
					}
					RT_RETURN(task, CONNECT_TO_TARGET);
					break;
				}
				Num_Tunable_Parameters = get_parameters_info(Target_Port, If_Task);
				if (Verbose) {
					printf("Target is running...%s\n", Is_Target_Running ? "yes" : "no");
					printf("Number of target tunable parameters...%d\n", Num_Tunable_Parameters);
					for (int n = 0; n < Num_Tunable_Parameters; n++) {
						printf("Block: %s\n", Tunable_Parameters[n].block_name);
						printf(" Parameter: %s\n", Tunable_Parameters[n].param_name);
						printf(" Number of rows: %d\n", Tunable_Parameters[n].n_rows);
						printf(" Number of cols: %d\n", Tunable_Parameters[n].n_cols);
						for (unsigned int nr = 0; nr < Tunable_Parameters[n].n_rows; nr++) {
							for (unsigned int nc = 0; nc < Tunable_Parameters[n].n_cols; nc++) {
								printf(" Value    : %f\n", Tunable_Parameters[n].data_value[nr*Tunable_Parameters[n].n_cols+nc]);
							}
						}
					}
				}
				Num_Scopes = get_scope_blocks_info(Target_Port, If_Task, Preferences.Target_Scope_Mbx_ID);
				if (Verbose) {
					printf("Number of target real time scopes: %d\n", Num_Scopes);
					for (int n = 0; n < Num_Scopes; n++) {
						printf("Scope: %s\n", Scopes[n].name);
						printf(" Number of traces...%d\n", Scopes[n].ntraces);
						for(int j = 0; j < Scopes[n].ntraces; j++) {
							printf("  %s\n", Scopes[n].traceName[j]);
						}
						printf(" Sampling time...%f\n", Scopes[n].dt);
						if (Scopes[n].dt <= 0.) {
							printf("Fatal Error, Scope %s sampling time is equal to %f,\n", Scopes[n].name, Scopes[n].dt);
							printf("while Rtai-lab needs a finite, positive sampling time\n");
							printf("This error often occurs when the sampling time is inherited\n");
							printf("from so-called time-continous simulink blocks\n");
							abort_connection(Target_Port);
						}
					}
				}
				Num_Logs = get_log_blocks_info(Target_Port, If_Task, Preferences.Target_Log_Mbx_ID);
				if (Verbose) {
					printf("Number of target real time logs: %d\n", Num_Logs);
					for (int n = 0; n < Num_Logs; n++) {
						printf("Log: %s\n", Logs[n].name);
						printf(" Number of rows...%d\n", Logs[n].nrow);
						printf(" Number of cols...%d\n", Logs[n].ncol);
						printf(" Sampling time...%f\n", Logs[n].dt);
						if (Logs[n].dt <= 0.) {
							printf("Fatal Error, Log %s sampling time is equal to %f,\n", Logs[n].name, Logs[n].dt);
							printf("while Rtai-lab needs a finite, positive sampling time\n");
							printf("This error often occurs when the sampling time is inherited\n");
							printf("from so-called time-continous simulink blocks\n");
							abort_connection(Target_Port);
						}
					}
				}
				Num_ALogs = get_alog_blocks_info(Target_Port, If_Task, Preferences.Target_ALog_Mbx_ID);
				if (Verbose) {
					printf("Number of target real time automatic logs: %d\n", Num_ALogs);
					for (int n = 0; n < Num_ALogs; n++) {
						printf("Log: %s\n", ALogs[n].name);
						printf(" Number of rows...%d\n", ALogs[n].nrow);
						printf(" Number of cols...%d\n", ALogs[n].ncol);
						printf(" Sampling time...%f\n", ALogs[n].dt);
						if (ALogs[n].dt <= 0.) {
							printf("Fatal Error, Log %s sampling time is equal to %f,\n", ALogs[n].name, ALogs[n].dt);
							printf("while Rtai-lab needs a finite, positive sampling time\n");
							printf("This error often occurs when the sampling time is inherited\n");
							printf("from so-called time-continous simulink blocks\n");
							abort_connection(Target_Port);
						}
					}
				}
				Num_Leds = get_led_blocks_info(Target_Port, If_Task, Preferences.Target_Led_Mbx_ID);
				if (Verbose) {
					printf("Number of target real time leds: %d\n", Num_Leds);
					for (int n = 0; n < Num_Leds; n++) {
						printf("Led: %s\n", Leds[n].name);
						printf(" Number of leds...%d\n", Leds[n].n_leds);
						printf(" Sampling time...%f\n", Leds[n].dt);
						if (Leds[n].dt <= 0.) {
							printf("Fatal Error, Led %s samplig time is equal to %f,\n", Leds[n].name, Leds[n].dt);
							printf("while Rtai-lab needs a finite, positive sampling time\n");
							printf("This error often occurs when the sampling time is inherited\n");
							printf("from so-called time-continous simulink blocks\n");
							abort_connection(Target_Port);
						}
					}
				}
				Num_Meters = get_meter_blocks_info(Target_Port, If_Task, Preferences.Target_Meter_Mbx_ID);
				if (Verbose) {
					printf("Number of target real time meters: %d\n", Num_Meters);
					for (int n = 0; n < Num_Meters; n++) {
						printf("Meter: %s\n", Meters[n].name);
						printf(" Sampling time...%f\n", Meters[n].dt);
						if (Meters[n].dt <= 0.) {
							printf("Fatal Error, Meter %s samplig time is equal to %f,\n", Meters[n].name, Meters[n].dt);
							printf("while Rtai-lab needs a finite, positive sampling time\n");
							printf("This error often occurs when the sampling time is inherited\n");
							printf("from so-called time-continous simulink blocks\n");
							abort_connection(Target_Port);
						}
					}
				}
				Num_Synchs = get_synch_blocks_info(Target_Port, If_Task, Preferences.Target_Synch_Mbx_ID);
				if (Verbose) {
					printf("Number of target real time synchronoscopes: %d\n", Num_Synchs);
					for (int n = 0; n < Num_Synchs; n++) {
						printf("Synchronoscope: %s\n", Synchs[n].name);
						printf(" Sampling time...%f\n", Synchs[n].dt);
						if (Synchs[n].dt <= 0.) {
							printf("Fatal Error, Synchronoscope %s samplig time is equal to %f,\n", Synchs[n].name, Synchs[n].dt);
							printf("while Rtai-lab needs a finite, positive sampling time\n");
							printf("This error often occurs when the sampling time is inherited\n");
							printf("from so-called time-continous simulink blocks\n");
							abort_connection(Target_Port);
						}
					}
				}
				Fl::lock();
				Is_Target_Connected = 1;
				rlg_manager_window(Num_Tunable_Parameters, PARAMS_MANAGER, false, 0, 0, 430, 260);
				rlg_manager_window(Num_Scopes, SCOPES_MANAGER, false, 0, 290, 480, 300);
				rlg_manager_window(Num_Logs, LOGS_MANAGER, false, 440, 0, 380, 250);
				rlg_manager_window(Num_ALogs, ALOGS_MANAGER, false, 460, 0, 380, 250);
				rlg_manager_window(Num_Leds, LEDS_MANAGER, false, 500, 290, 320, 250);
				rlg_manager_window(Num_Meters, METERS_MANAGER, false, 530, 320, 320, 250);
				rlg_manager_window(Num_Synchs, SYNCHS_MANAGER, false, 530, 320, 320, 250);
				Fl::unlock();
				if (Verbose) {
					printf("Target %s is correctly connected\n", RLG_Target_Name);
				}
				if (Num_Scopes > 0) Get_Scope_Data_Thread = new pthread_t [Num_Scopes];
				for (int n = 0; n < Num_Scopes; n++) {
					unsigned int msg;
					Args_T thr_args;
					thr_args.index = n;
					thr_args.mbx_id = strdup(Preferences.Target_Scope_Mbx_ID);
					thr_args.x = 500; 
					thr_args.y = 290;
					thr_args.w = 250;
					thr_args.h = 250;
					pthread_create(&Get_Scope_Data_Thread[n], NULL, rt_get_scope_data, &thr_args);
					rt_receive(0, &msg);
				}
				if (Num_Logs > 0) Get_Log_Data_Thread = new pthread_t [Num_Logs];
				for (int n = 0; n < Num_Logs; n++) {
					unsigned int msg;
					Args_T thr_args;
					thr_args.index = n;
					thr_args.mbx_id = strdup(Preferences.Target_Log_Mbx_ID);
					pthread_create(&Get_Log_Data_Thread[n], NULL, rt_get_log_data, &thr_args);
					rt_receive(0, &msg);
				}
				if (Num_ALogs > 0) Get_ALog_Data_Thread = new pthread_t [Num_ALogs];
				for (int n = 0; n < Num_ALogs; n++) {
					unsigned int msg;
					Alog_T thr_args;
					thr_args.index = n;
					thr_args.mbx_id = strdup(Preferences.Target_ALog_Mbx_ID);
					thr_args.alog_name = strdup(ALogs[n].name);
					printf("%s alog name\n", ALogs[n].name);
					pthread_create(&Get_ALog_Data_Thread[n], NULL, rt_get_alog_data, &thr_args);
					rt_receive(0, &msg);
				}
				if (Num_Leds > 0) Get_Led_Data_Thread = new pthread_t [Num_Leds];
				for (int n = 0; n < Num_Leds; n++) {
					unsigned int msg;
					Args_T thr_args;
					thr_args.index = n;
					thr_args.mbx_id = strdup(Preferences.Target_Led_Mbx_ID);
					thr_args.x = 500; 
					thr_args.y = 290;
					thr_args.w = 250;
					thr_args.h = 250;
					pthread_create(&Get_Led_Data_Thread[n], NULL, rt_get_led_data, &thr_args);
					rt_receive(0, &msg);
				}
				if (Num_Meters > 0) Get_Meter_Data_Thread = new pthread_t [Num_Meters];
				for (int n = 0; n < Num_Meters; n++) {
					unsigned int msg;
					Args_T thr_args;
					thr_args.index = n;
					thr_args.mbx_id = strdup(Preferences.Target_Meter_Mbx_ID);
					thr_args.x = 0; 
					thr_args.y = 0;
					thr_args.w = 300;
					thr_args.h = 200;
					pthread_create(&Get_Meter_Data_Thread[n], NULL, rt_get_meter_data, &thr_args);
					rt_receive(0, &msg);
				}
				if (Num_Synchs > 0) Get_Synch_Data_Thread = new pthread_t [Num_Synchs];
				for (int n = 0; n < Num_Synchs; n++) {
					unsigned int msg;
					Args_T thr_args;
					thr_args.index = n;
					thr_args.mbx_id = strdup(Preferences.Target_Synch_Mbx_ID);
					thr_args.x = 0; 
					thr_args.y = 0;
					thr_args.w = 300;
					thr_args.h = 200;
					pthread_create(&Get_Synch_Data_Thread[n], NULL, rt_get_synch_data, &thr_args);
					rt_receive(0, &msg);
				}
				Fl::lock();
				rlg_update_after_connect();
				Fl::unlock();
				RT_RETURN(task, CONNECT_TO_TARGET);
				break;

			case CONNECT_TO_TARGET_WITH_PROFILE: {

				Fl_Browser *p_tree;
				Fl_Widget *p;
				int p_idx;
				const char *p_file;

				// Probably should have a lock of some sort around this, but not the Fl lock.
				if (!Direct_Profile) {
					p_tree = (Fl_Browser *)RLG_Connect_wProfile_Tree;
					p = p_tree->goto_focus();
					p_idx = p_tree->focus_index()[0];
					p_file = strdup(p->label().c_str());
				} else {
					p_idx = Direct_Profile_Idx;
					p_file = strdup(Direct_Profile);
				}	

				if (Verbose) {
					printf("Reading profile %s settings\n", p_file);
				}
				Fl::lock();
				rlg_read_pref(PROFILE_PREF, p_file, p_idx);
				Fl::unlock();
				if (!strcmp(Profile[p_idx].Target_IP, "0")) {
					Target_Node = 0;
				} else {
					Target_Port = try_to_connect(Profile[p_idx].Target_IP);
					Fl::lock();
					RLG_Connect_wProfile_Button->activate();
					Fl::unlock();
					if (Target_Port <= 0) {
					        Fl::lock();
						RLG_Main_Status->label("Sorry, no route to the specified target");
						RLG_Main_Window->redraw();
						Fl::unlock();
						if (Verbose) {
							printf(" Sorry, no route to the specified target\n");
						}
						RT_RETURN(task, CONNECT_TO_TARGET_WITH_PROFILE);
						break;
					}
					if (Verbose) {
						printf(" Ok\n");
					}
				}
				if (!(If_Task = (RT_TASK *)RT_get_adr(Target_Node, Target_Port, Profile[p_idx].Target_Interface_Task_Name))) {
				        Fl::lock();
					RLG_Main_Status->label("No target with the specified interface task name");
					RLG_Main_Window->redraw();
					Fl::unlock();
					if (Verbose) {
						printf("No target with the specified interface task name\n");
					}
					RT_RETURN(task, CONNECT_TO_TARGET_WITH_PROFILE);
					break;
				}
				Num_Tunable_Parameters = get_parameters_info(Target_Port, If_Task);
				Num_Scopes = get_scope_blocks_info(Target_Port, If_Task, Profile[p_idx].Target_Scope_Mbx_ID);
				Num_Logs = get_log_blocks_info(Target_Port, If_Task, Profile[p_idx].Target_Log_Mbx_ID);
				Num_ALogs = get_alog_blocks_info(Target_Port, If_Task, Profile[p_idx].Target_ALog_Mbx_ID);
				Num_Leds = get_led_blocks_info(Target_Port, If_Task, Profile[p_idx].Target_Led_Mbx_ID);
				Num_Meters = get_meter_blocks_info(Target_Port, If_Task, Profile[p_idx].Target_Meter_Mbx_ID);
				Num_Synchs = get_synch_blocks_info(Target_Port, If_Task, Profile[p_idx].Target_Synch_Mbx_ID);
				Fl::lock();
				Is_Target_Connected = 1;
				rlg_manager_window(Num_Tunable_Parameters, PARAMS_MANAGER,
						   Profile[p_idx].P_Mgr_W.visible, Profile[p_idx].P_Mgr_W.x,
						   Profile[p_idx].P_Mgr_W.y, Profile[p_idx].P_Mgr_W.w, Profile[p_idx].P_Mgr_W.h);
				if (Profile[p_idx].P_Mgr_Batch) {
					Parameters_Manager->batch_download(true);
					Parameters_Manager->batch_counter(0);
				}
				rlg_manager_window(Num_Scopes, SCOPES_MANAGER, Profile[p_idx].S_Mgr_W.visible,
						   Profile[p_idx].S_Mgr_W.x, Profile[p_idx].S_Mgr_W.y,
						   Profile[p_idx].S_Mgr_W.w, Profile[p_idx].S_Mgr_W.h);
				for (int i = 0; i < Num_Scopes; i++) {
					if (Profile[p_idx].S_Mgr_Show[i]) {
						Scopes_Manager->show_hide(i, true);
					}
				}
				rlg_manager_window(Num_Logs, LOGS_MANAGER, Profile[p_idx].Log_Mgr_W.visible,
						   Profile[p_idx].Log_Mgr_W.x, Profile[p_idx].Log_Mgr_W.y,
						   Profile[p_idx].Log_Mgr_W.w, Profile[p_idx].Log_Mgr_W.h);
				rlg_manager_window(Num_ALogs, ALOGS_MANAGER, Profile[p_idx].ALog_Mgr_W.visible,
						   Profile[p_idx].ALog_Mgr_W.x, Profile[p_idx].ALog_Mgr_W.y,
						   Profile[p_idx].ALog_Mgr_W.w, Profile[p_idx].ALog_Mgr_W.h);
				rlg_manager_window(Num_Leds, LEDS_MANAGER, Profile[p_idx].Led_Mgr_W.visible,
						   Profile[p_idx].Led_Mgr_W.x, Profile[p_idx].Led_Mgr_W.y,
						   Profile[p_idx].Led_Mgr_W.w, Profile[p_idx].Led_Mgr_W.h);
				for (int i = 0; i < Num_Leds; i++) {
					if (Profile[p_idx].Led_Mgr_Show[i]) {
						Leds_Manager->show_hide(i, true);
					}
				}
				rlg_manager_window(Num_Meters, METERS_MANAGER, Profile[p_idx].M_Mgr_W.visible,
						   Profile[p_idx].M_Mgr_W.x, Profile[p_idx].M_Mgr_W.y,
						   Profile[p_idx].M_Mgr_W.w, Profile[p_idx].M_Mgr_W.h);
				for (int i = 0; i < Num_Meters; i++) {
					if (Profile[p_idx].M_Mgr_Show[i]) {
						Meters_Manager->show_hide(i, true);
					}
				}
				rlg_manager_window(Num_Synchs, SYNCHS_MANAGER, Profile[p_idx].Sy_Mgr_W.visible,
						   Profile[p_idx].Sy_Mgr_W.x, Profile[p_idx].Sy_Mgr_W.y,
						   Profile[p_idx].Sy_Mgr_W.w, Profile[p_idx].Sy_Mgr_W.h);
				for (int i = 0; i < Num_Synchs; i++) {
					if (Profile[p_idx].Sy_Mgr_Show[i]) {
						Synchs_Manager->show_hide(i, true);
					}
				}
				Fl::unlock();
				if (Verbose) {
					printf("Target is correctly connected\n");
				}
				if (Num_Scopes > 0) Get_Scope_Data_Thread = new pthread_t [Num_Scopes];
				for (int n = 0; n < Num_Scopes; n++) {
					unsigned int msg;
					Args_T thr_args;
					thr_args.index = n;
					thr_args.mbx_id = strdup(Profile[p_idx].Target_Scope_Mbx_ID);
					thr_args.x = Profile[p_idx].S_W[n].x;
					thr_args.y = Profile[p_idx].S_W[n].y;
					thr_args.w = Profile[p_idx].S_W[n].w;
					thr_args.h = Profile[p_idx].S_W[n].h;
					pthread_create(&Get_Scope_Data_Thread[n], NULL, rt_get_scope_data, &thr_args);
					rt_receive(0, &msg);
					Fl::lock();
					Scopes_Manager->b_color(n, Profile[p_idx].S_Bg_C[n]);
					Scopes_Manager->g_color(n, Profile[p_idx].S_Grid_C[n]);
					if (!Profile[p_idx].S_Mgr_Grid[n]) {
						Scopes_Manager->grid_on_off(n, false);
					} 
					if (!Profile[p_idx].S_Mgr_PT[n]) {
						Scopes_Manager->points_time(n, false);
					} 
					Scopes_Manager->sec_div(n, Profile[p_idx].S_Mgr_SecDiv[n]);
					Scopes_Manager->p_save(n, Profile[p_idx].S_Mgr_PSave[n]);
					Scopes_Manager->t_save(n, Profile[p_idx].S_Mgr_TSave[n]);
					Scopes_Manager->file_name(n, Profile[p_idx].S_Mgr_File[n]);
					Scopes_Manager->Scope_Windows[n]->Plot->trigger_mode( Profile[p_idx].S_Mgr_Trigger[n]);
					Scopes_Manager->Scope_Windows[n]->Plot->scope_flags( Profile[p_idx].S_Mgr_Flags[n]);

					for (int t = 0; t < Scopes[n].ntraces; t++) {
						if (!Profile[p_idx].S_Mgr_T_Show[t][n]) {
							Scopes_Manager->trace_show_hide(n, t, false);
						}
						Scopes_Manager->trace_unit_div(n, t, Profile[p_idx].S_Mgr_T_UnitDiv[t][n]);
						Scopes_Manager->t_color(n, t, Profile[p_idx].S_Trace_C[t][n]);
						Scopes_Manager->trace_offset(n, t, Profile[p_idx].S_Mgr_T_Offset[t][n]);
						Scopes_Manager->trace_width(n, t, Profile[p_idx].S_Mgr_T_Width[t][n]);
 						Scopes_Manager->trace_flags(n, t, Profile[p_idx].S_Mgr_T_Options[t][n]);
					}
					Fl::unlock();
				}
				if (Num_Logs > 0) Get_Log_Data_Thread = new pthread_t [Num_Logs];
				for (int n = 0; n < Num_Logs; n++) {
					unsigned int msg;
					Args_T thr_args;
					thr_args.index = n;
					thr_args.mbx_id = strdup(Profile[p_idx].Target_Log_Mbx_ID);
					pthread_create(&Get_Log_Data_Thread[n], NULL, rt_get_log_data, &thr_args);
					rt_receive(0, &msg);
					Fl::lock();
					if (!Profile[p_idx].Log_Mgr_PT[n]) {
						Logs_Manager->points_time(n, false);
					} 
					Logs_Manager->p_save(n, Profile[p_idx].Log_Mgr_PSave[n]);
					Logs_Manager->t_save(n, Profile[p_idx].Log_Mgr_TSave[n]);
					Logs_Manager->file_name(n, Profile[p_idx].Log_Mgr_File[n]);
					Fl::unlock();
				}
				if (Num_ALogs > 0) Get_ALog_Data_Thread = new pthread_t [Num_ALogs];
				for (int n = 0; n < Num_ALogs; n++) {
					unsigned int msg;
					Alog_T thr_args;
					thr_args.alog_name = strdup(ALogs[n].name);
					printf("%s alog name\n", ALogs[n].name);
					thr_args.index = n;
					thr_args.mbx_id = strdup(Profile[p_idx].Target_ALog_Mbx_ID);
					pthread_create(&Get_ALog_Data_Thread[n], NULL, rt_get_alog_data, &thr_args);
					rt_receive(0, &msg);
					Fl::lock();
					ALogs_Manager->file_name(n, Profile[p_idx].ALog_Mgr_File[n]);
					Fl::unlock();
				}
				if (Num_Leds > 0) Get_Led_Data_Thread = new pthread_t [Num_Leds];
				for (int n = 0; n < Num_Leds; n++) {
					unsigned int msg;
					Args_T thr_args;
					thr_args.index = n;
					thr_args.mbx_id = strdup(Profile[p_idx].Target_Led_Mbx_ID);
					thr_args.x = Profile[p_idx].Led_W[n].x;
					thr_args.y = Profile[p_idx].Led_W[n].y;
					thr_args.w = Profile[p_idx].Led_W[n].w;
					thr_args.h = Profile[p_idx].Led_W[n].h;
					pthread_create(&Get_Led_Data_Thread[n], NULL, rt_get_led_data, &thr_args);
					rt_receive(0, &msg);
					Fl::lock();
					Leds_Manager->color(n, Profile[p_idx].Led_Mgr_Color[n]);
					Fl::unlock();
				}
				if (Num_Meters > 0) Get_Meter_Data_Thread = new pthread_t [Num_Meters];
				for (int n = 0; n < Num_Meters; n++) {
					unsigned int msg;
					Args_T thr_args;
					thr_args.index = n;
					thr_args.mbx_id = strdup(Profile[p_idx].Target_Meter_Mbx_ID);
					thr_args.x = Profile[p_idx].M_W[n].x;
					thr_args.y = Profile[p_idx].M_W[n].y;
					thr_args.w = Profile[p_idx].M_W[n].w;
					thr_args.h = Profile[p_idx].M_W[n].h;
					pthread_create(&Get_Meter_Data_Thread[n], NULL, rt_get_meter_data, &thr_args);
					rt_receive(0, &msg);
					Fl::lock();
					Meters_Manager->minv(n, Profile[p_idx].M_Mgr_Minv[n]);
					Meters_Manager->maxv(n, Profile[p_idx].M_Mgr_Maxv[n]);
					Meters_Manager->b_color(n, Profile[p_idx].M_Bg_C[n]);
					Meters_Manager->a_color(n, Profile[p_idx].M_Arrow_C[n]);
					Meters_Manager->g_color(n, Profile[p_idx].M_Grid_C[n]);
					Fl::unlock();
				}
				if (Num_Synchs > 0) Get_Synch_Data_Thread = new pthread_t [Num_Synchs];
				for (int n = 0; n < Num_Synchs; n++) {
					unsigned int msg;
					Args_T thr_args;
					thr_args.index = n;
					thr_args.mbx_id = strdup(Profile[p_idx].Target_Synch_Mbx_ID);
					thr_args.x = Profile[p_idx].Sy_W[n].x;
					thr_args.y = Profile[p_idx].Sy_W[n].y;
					thr_args.w = Profile[p_idx].Sy_W[n].w;
					thr_args.h = Profile[p_idx].Sy_W[n].h;
					pthread_create(&Get_Synch_Data_Thread[n], NULL, rt_get_synch_data, &thr_args);
					rt_receive(0, &msg);
				}
				Fl::lock();
				rlg_update_after_connect();
				Fl::unlock();

				RT_RETURN(task, CONNECT_TO_TARGET_WITH_PROFILE);
				break;
			}

			case DISCONNECT_FROM_TARGET:

				if (Verbose) {
					printf("Disconnecting from target %s\n", Tunable_Parameters[0].model_name);
				}
				Is_Target_Connected = 0;
				for (int n = 0; n < Num_Scopes; n++) {
					pthread_join(Get_Scope_Data_Thread[n], NULL);
				}
				for (int n = 0; n < Num_Logs; n++) {
					pthread_join(Get_Log_Data_Thread[n], NULL);
				}
				for (int n = 0; n < Num_ALogs; n++) {				//modifiche 29/06/05
					pthread_join(Get_ALog_Data_Thread[n], NULL);
				}
				for (int n = 0; n < Num_Leds; n++) {
					pthread_join(Get_Led_Data_Thread[n], NULL);
				}
				for (int n = 0; n < Num_Meters; n++) {
					pthread_join(Get_Meter_Data_Thread[n], NULL);
				}
				for (int n = 0; n < Num_Synchs; n++) {
					pthread_join(Get_Synch_Data_Thread[n], NULL);
				}
//				pthread_join(GetTargetTimeThread, NULL);
				Fl::lock();
				if (Parameters_Manager) Parameters_Manager->hide();
				if (Scopes_Manager) Scopes_Manager->hide();
				if (Logs_Manager) Logs_Manager->hide();
				if (ALogs_Manager) ALogs_Manager->hide();
				if (Leds_Manager) Leds_Manager->hide();
				if (Meters_Manager) Meters_Manager->hide();
				if (Synchs_Manager) Synchs_Manager->hide();
				RLG_Main_Window->redraw();
				RLG_Connect_Button->activate();
				RLG_Connect_wProfile_Button->activate();
				RLG_Disconnect_Button->deactivate();
				RLG_Main_Menu_Table[1].activate();
				RLG_Main_Menu_Table[2].deactivate();
				RLG_Main_Menu_Table[3].activate();
				RLG_Main_Menu_Table[4].deactivate();
				RLG_Main_Menu_Table[5].activate();
				for (int i = 9; i <= 15; i++) RLG_Main_Menu_Table[i].deactivate();
				RLG_Main_Menu->menu(RLG_Main_Menu_Table);
				RLG_Main_Menu->redraw();
				RLG_Start_Button->deactivate();
				RLG_Stop_Button->deactivate();
				RLG_Save_Profile_Button->deactivate();
				RLG_Delete_Profile_Button->activate();
				RLG_Params_Mgr_Button->deactivate();
				RLG_Scopes_Mgr_Button->deactivate();
				RLG_Logs_Mgr_Button->deactivate();
				RLG_ALogs_Mgr_Button->deactivate();
				RLG_Leds_Mgr_Button->deactivate();
				RLG_Meters_Mgr_Button->deactivate();
				RLG_Synchs_Mgr_Button->deactivate();
				Fl::unlock();
				rt_release_port(Target_Node, Target_Port);
				Target_Port = 0;
				free(Tunable_Parameters);
				for(int n = 0; n < Num_Scopes; n++) { // Free memory used by names in scopes
					free(Scopes[n].name);
					for(int j = 0; j < Scopes[n].ntraces; j++) {
						free(Scopes[n].traceName[j]);
					}
					free(Scopes[n].traceName);
				}
				Fl::lock();
				RLG_Main_Status->label("Ready...");
				RLG_Main_Window->redraw();
				Fl::unlock();
				if (Verbose) {
					printf("Disconnected succesfully.\n");
				}
				RT_RETURN(task, DISCONNECT_FROM_TARGET);
				break;

			case START_TARGET:

				if (!Is_Target_Running) {
					if (Verbose) {
						printf("Starting real time code...");
					}
					U_Request = 's';
					RT_rpc(Target_Node, Target_Port, If_Task, U_Request, &Is_Target_Running);
				        Fl::lock();
					RLG_Start_Button->deactivate();
					RLG_Stop_Button->activate();
					Fl::unlock();
					if (Verbose) {
						printf("ok\n");
					}
				}
				RT_RETURN(task, START_TARGET);
				break;

			case STOP_TARGET:

				if (Is_Target_Running) {
					U_Request = 't';
					Is_Target_Connected = 0;
					for (int n = 0; n < Num_Scopes; n++) {
						pthread_join(Get_Scope_Data_Thread[n], NULL);
					}
					for (int n = 0; n < Num_Logs; n++) {
						pthread_join(Get_Log_Data_Thread[n], NULL);
					}
					for (int n = 0; n < Num_ALogs; n++) {			//modifiche 29/06/05
						pthread_join(Get_ALog_Data_Thread[n], NULL);
					}
					for (int n = 0; n < Num_Leds; n++) {
						pthread_join(Get_Led_Data_Thread[n], NULL);
					}
					for (int n = 0; n < Num_Meters; n++) {
						pthread_join(Get_Meter_Data_Thread[n], NULL);
					}
					for (int n = 0; n < Num_Synchs; n++) {
						pthread_join(Get_Synch_Data_Thread[n], NULL);
					}
//					pthread_join(GetTargetTimeThread, NULL);
					if (Verbose) {
						printf("Stopping real time code...");
					}
					RT_rpc(Target_Node, Target_Port, If_Task, U_Request, &Is_Target_Running);
					rt_release_port(Target_Node, Target_Port);
					Target_Node = 0;
					Target_Port = 0;	
					Fl::lock();
					if (Parameters_Manager) Parameters_Manager->hide();
					if (Scopes_Manager) Scopes_Manager->hide();
					if (Logs_Manager) Logs_Manager->hide();
					if (Leds_Manager) Leds_Manager->hide();
					if (Meters_Manager) Meters_Manager->hide();
					if (Synchs_Manager) Synchs_Manager->hide();
					RLG_Connect_Button->activate();
					RLG_Connect_wProfile_Button->activate();
					RLG_Disconnect_Button->deactivate();
					RLG_Main_Menu_Table[1].activate();
					RLG_Main_Menu_Table[2].deactivate();
					RLG_Main_Menu_Table[4].deactivate();
					RLG_Main_Menu_Table[5].activate();
					for (int i = 9; i <= 15; i++) RLG_Main_Menu_Table[i].deactivate();
					RLG_Main_Menu->menu(RLG_Main_Menu_Table);
					RLG_Main_Menu->redraw();
					RLG_Start_Button->deactivate();
					RLG_Stop_Button->deactivate();
					RLG_Save_Profile_Button->deactivate();
					RLG_Delete_Profile_Button->activate();
					RLG_Params_Mgr_Button->deactivate();
					RLG_Scopes_Mgr_Button->deactivate();
					RLG_Logs_Mgr_Button->deactivate();
					RLG_ALogs_Mgr_Button->deactivate();
					RLG_Leds_Mgr_Button->deactivate();
					RLG_Meters_Mgr_Button->deactivate();
					RLG_Synchs_Mgr_Button->deactivate();
					RLG_Main_Status->label("Ready...");
					RLG_Main_Window->redraw();
					Fl::unlock();
					if (Verbose) {
						printf("ok\n");
					}
				}
				RT_RETURN(task, STOP_TARGET);
				break;

			case UPDATE_PARAM: {

				U_Request = 'p';
				int Map_Offset = (code >> 4) & 0xffff;
				int Mat_Idx = (code >> 20) & 0xfff;
//				int Map_Offset = code >> 4;
//				int Map_Offset = code >> 16;
				int Is_Param_Updated;
				double new_value = (double)Tunable_Parameters[Map_Offset].data_value[Mat_Idx];
				RT_rpc(Target_Node, Target_Port, If_Task, U_Request, &Is_Target_Running);
				RT_rpcx(Target_Node, Target_Port, If_Task, &Map_Offset, &Is_Param_Updated, sizeof(int), sizeof(int));
				RT_rpcx(Target_Node, Target_Port, If_Task, &new_value, &Is_Param_Updated, sizeof(double), sizeof(int));
				RT_rpcx(Target_Node, Target_Port, If_Task, &Mat_Idx, &Is_Param_Updated, sizeof(int), sizeof(int));
				RT_RETURN(task, UPDATE_PARAM);
				break;
			}

			case BATCH_DOWNLOAD: {

				U_Request = 'd';
				int Is_Param_Updated;
				int Counter = Parameters_Manager->batch_counter();
				RT_rpc(Target_Node, Target_Port, If_Task, U_Request, &Is_Target_Running);
				RT_rpcx(Target_Node, Target_Port, If_Task, &Counter, &Is_Param_Updated, sizeof(int), sizeof(int));
				RT_rpcx(Target_Node, Target_Port, If_Task, &Batch_Parameters, &Is_Param_Updated, sizeof(Batch_Parameters_T)*Counter, sizeof(int));
				RT_RETURN(task, BATCH_DOWNLOAD);
				break;
			}

			case GET_PARAMS: {

				upload_parameters_info(Target_Port, If_Task);
				if (Verbose) {
					printf("Target is running...%s\n", Is_Target_Running ? "yes" : "no");
					printf("Number of target tunable parameters...%d\n", Num_Tunable_Parameters);
					for (int n = 0; n < Num_Tunable_Parameters; n++) {
						printf("Block: %s\n", Tunable_Parameters[n].block_name);
						printf(" Parameter: %s\n", Tunable_Parameters[n].param_name);
						printf(" Number of rows: %d\n", Tunable_Parameters[n].n_rows);
						printf(" Number of cols: %d\n", Tunable_Parameters[n].n_cols);
						for (unsigned int nr = 0; nr < Tunable_Parameters[n].n_rows; nr++) {
							for (unsigned int nc = 0; nc < Tunable_Parameters[n].n_cols; nc++) {
								printf(" Value    : %f\n", Tunable_Parameters[n].data_value[nr*Tunable_Parameters[n].n_cols+nc]);
							}
						}
					}
				}
				// Do we need an Fl:lock() here? I'm not sure
				Parameters_Manager->reload_params();
				RT_RETURN(task, GET_PARAMS);
				break;
			}

/*
			case GET_TARGET_TIME: {
				unsigned int Request = 'm';
				int Msg;
				RT_rpc(TargetNode, TargetPort, If_Task, Request, &IsTargetRunning);
				RT_rpcx(TargetNode, TargetPort, If_Task, &Msg, &TargetTime, sizeof(int), sizeof(float));
				break;
			}
*/
			case CLOSE:

				rt_task_delete(Target_Interface_Task);
				return 0;

			default:
				break;
		}
	}

	rt_task_delete(Target_Interface_Task);

	return 0;
}

Fl_Dialog *rlg_connect_wprofile_dialog(int w, int h)
{
	Fl_Dialog *RLG_CWPD;

	RLG_CWPD = new Fl_Dialog(w, h, "Connect to Target with Profiles");
	RLG_CWPD->new_group("Connect Using Profiles");
	{ Fl_Browser *o = RLG_Connect_wProfile_Tree = new Fl_Browser(5, 5, 150, 240);
	  o->indented(1);
	  for (int i = 0; i < Preferences.Num_Profiles; i++) {
		add_paper(o, Preferences.Profiles.item(i).c_str(), Fl_Image::read_xpm(0, profile_icon));
		if (Direct_Profile) {
			if (strcmp(Direct_Profile, Preferences.Profiles.item(i).c_str()) == 0) {
				Direct_Profile_Idx = i;
			}
		}
	  }
	  o->end();
	}
	{ Fl_Browser *o = RLG_Profile_Contents = new Fl_Browser(160, 5, w-165, 240);
	  o->end();
	}
	RLG_CWPD->end();
	RLG_CWPD->buttons(FL_DLG_OK|FL_DLG_CANCEL,FL_DLG_OK);

	return RLG_CWPD;
}

Fl_Dialog *rlg_delete_profile_dialog(int w, int h)
{
	Fl_Dialog *RLG_DPD;

	RLG_DPD = new Fl_Dialog(w, h, "Delete Profile");
	RLG_DPD->new_group("Delete Profile");
	{ Fl_Browser *o = RLG_Delete_Profile_Tree = new Fl_Browser(10, 10, w-20, h-60);
	  o->indented(1);
	  for (int i = 0; i < Preferences.Num_Profiles; i++) {
		add_paper(o, Preferences.Profiles.item(i).c_str(), Fl_Image::read_xpm(0, profile_icon));
	  }
	}
	RLG_DPD->end();
	RLG_DPD->buttons(FL_DLG_OK|FL_DLG_CANCEL,FL_DLG_OK);

	return RLG_DPD;
}

Fl_Dialog *rlg_save_profile_dialog(int w, int h)
{
	Fl_Dialog *RLG_SPD;

	RLG_SPD = new Fl_Dialog(w, h, "Save Profile");
	RLG_SPD->new_group("Save Profile");
	{ Fl_Input *o = RLG_Save_Profile_Input = new Fl_Input(10, 20, 120, 20, "Profile Name");
	  o->align(FL_ALIGN_RIGHT);
	}
	RLG_SPD->end();
	RLG_SPD->buttons(FL_DLG_OK|FL_DLG_CANCEL,FL_DLG_OK);

	return RLG_SPD;
}

Fl_Dialog *rlg_connect_dialog(int w, int h)
{
	Fl_Dialog *RLG_CD;

	RLG_CD = new Fl_Dialog(w, h, "Connect to Target");
	RLG_CD->new_group("Connect");
	{ Fl_Input *o = RLG_Target_IP_Address = new Fl_Input(10, 20, 120, 20, " IP Address");
	  o->align(FL_ALIGN_RIGHT);
	  o->maximum_size(15);
	}
	{ Fl_Input *o = RLG_Target_Task_ID = new Fl_Input(10, 20 + 27, 70, 20, " Task Identifier");
	  o->align(FL_ALIGN_RIGHT);
	  o->maximum_size(6);
	}
	{ Fl_Input *o = RLG_Target_Scope_ID = new Fl_Input(10, 20 + 27*2, 70, 20, " Scope Identifier");
	  o->align(FL_ALIGN_RIGHT);
	  o->maximum_size(3);
	}
	{ Fl_Input *o = RLG_Target_Log_ID = new Fl_Input(10, 20 + 27*3, 70, 20, " Log Identifier");
	  o->align(FL_ALIGN_RIGHT);
	  o->maximum_size(3);
	}
	{ Fl_Input *o = RLG_Target_ALog_ID = new Fl_Input(10, 20 + 27*4, 70, 20, " ALog Identifier");
	  o->align(FL_ALIGN_RIGHT);
	  o->maximum_size(3);
	}
	{ Fl_Input *o = RLG_Target_Led_ID = new Fl_Input(10, 20 + 27*5, 70, 20, " Led Identifier");
	  o->align(FL_ALIGN_RIGHT);
	  o->maximum_size(3);
	}
	{ Fl_Input *o = RLG_Target_Meter_ID = new Fl_Input(10, 20 + 27*6, 70, 20, " Meter Identifier");
	  o->align(FL_ALIGN_RIGHT);
	  o->maximum_size(3);
	}
	{ Fl_Input *o = RLG_Target_Synch_ID = new Fl_Input(10, 20 + 27*7, 70, 20, " Synch Identifier");
	  o->align(FL_ALIGN_RIGHT);
	  o->maximum_size(3);
	}
	RLG_CD->end();
	RLG_CD->buttons(FL_DLG_OK|FL_DLG_CANCEL,FL_DLG_OK);

	return RLG_CD;
}

void rlg_main_toolbar(Fl_Tool_Bar *RLG_MT)
{
/* RTAI-Lab GUI (RLG) tool bar buttons */

/* Quit */
	RLG_Exit_Button = RLG_MT->add_button(Fl_Image::read_xpm(0, exit_icon), 0,
						      "Quit", "Quit xrtailab");
	RLG_Exit_Button->callback((Fl_Callback *)rlg_quit_cb);

	RLG_MT->add_divider();

/* Connect to target with profiles */
	RLG_Connect_wProfile_Button = RLG_MT->add_button(Fl_Image::read_xpm(0, connect_wprofile_icon), 0,
						      "Edit Profiles", "Connect to target with profiles...");
	RLG_Connect_wProfile_Button->callback((Fl_Callback *)rlg_connect_wprofile_cb);

/* Connect to target without profiles */
	RLG_Connect_Button = RLG_MT->add_button(Fl_Image::read_xpm(0, connect_icon), 0,
						"Connect", "Connect to target without profiles...");
	RLG_Connect_Button->callback((Fl_Callback *)rlg_connect_cb);

/* Disconnect from target */
	RLG_Disconnect_Button = RLG_MT->add_button(Fl_Image::read_xpm(0, disconnect_icon), 0,
						   "Disconnect", "Disconnect from target...");
	RLG_Disconnect_Button->callback((Fl_Callback *)rlg_disconnect_cb);
	RLG_Disconnect_Button->deactivate();

	RLG_MT->add_divider();

/* Save profile */
	RLG_Save_Profile_Button = RLG_MT->add_button(Fl_Image::read_xpm(0, save_profile_icon), 0,
						    "Save Profile", "Save Profile...");
	RLG_Save_Profile_Button->callback((Fl_Callback *)rlg_save_profile_cb);
	RLG_Save_Profile_Button->deactivate();

/* Delete profile */
	RLG_Delete_Profile_Button = RLG_MT->add_button(Fl_Image::read_xpm(0, del_profile_icon), 0,
						       "Delete Profile", "Delete Profile...");
	RLG_Delete_Profile_Button->callback((Fl_Callback *)rlg_delete_profile_cb);
//	RLG_Delete_Profile_Button->deactivate();

	RLG_MT->add_divider();

/* Start the real time code */
	RLG_Start_Button = RLG_MT->add_button(Fl_Image::read_xpm(0, start_icon), 0,
					      "Start", "Start real time code");
	RLG_Start_Button->callback((Fl_Callback *)rlg_start_stop_cb);
	RLG_Start_Button->deactivate();

/* Stop the real time code */
	RLG_Stop_Button = RLG_MT->add_button(Fl_Image::read_xpm(0, stop_icon), 0,
					     "Stop", "Stop real time code");
	RLG_Stop_Button->callback((Fl_Callback *)rlg_start_stop_cb);
	RLG_Stop_Button->deactivate();

	RLG_MT->add_divider();

/* Target tunable parameters manager */
	RLG_Params_Mgr_Button = RLG_MT->add_toggle(Fl_Image::read_xpm(0, parameters_icon), 0,
						   "Parameters", "Open/close parameters manager");
	RLG_Params_Mgr_Button->callback((Fl_Callback *)rlg_params_mgr_cb);
	RLG_Params_Mgr_Button->deactivate();

/* Target real time scopes manager */
	RLG_Scopes_Mgr_Button = RLG_MT->add_toggle(Fl_Image::read_xpm(0, scope_icon), 0,
						   "Scopes", "Open/close scope manager");
	RLG_Scopes_Mgr_Button->callback((Fl_Callback *)rlg_scopes_mgr_cb);
	RLG_Scopes_Mgr_Button->deactivate();

/* Target real time logging blocks manager */
	RLG_Logs_Mgr_Button = RLG_MT->add_toggle(Fl_Image::read_xpm(0, log_icon), 0,
						 "Logs", "Open/close log manager");
	RLG_Logs_Mgr_Button->callback((Fl_Callback *)rlg_logs_mgr_cb);
	RLG_Logs_Mgr_Button->deactivate();
	
/* Target automatic real time logging blocks manager */						//aggiunto 3/5
	RLG_ALogs_Mgr_Button = RLG_MT->add_toggle(Fl_Image::read_xpm(0, auto_log_icon_xpm), 0,
						 "Logs", "Open/close automatic log manager");
	RLG_ALogs_Mgr_Button->callback((Fl_Callback *)rlg_alogs_mgr_cb);
	RLG_ALogs_Mgr_Button->deactivate();
	

/* Target real time leds manager */
	RLG_Leds_Mgr_Button = RLG_MT->add_toggle(Fl_Image::read_xpm(0, led_icon), 0,
						 "Leds", "Open/close led manager");
	RLG_Leds_Mgr_Button->callback((Fl_Callback *)rlg_leds_mgr_cb);
	RLG_Leds_Mgr_Button->deactivate();

/* Target real time analog meters manager */
	RLG_Meters_Mgr_Button = RLG_MT->add_toggle(Fl_Image::read_xpm(0, meter_icon), 0,
						   "Meters", "Open/close meter manager");
	RLG_Meters_Mgr_Button->callback((Fl_Callback *)rlg_meters_mgr_cb);
	RLG_Meters_Mgr_Button->deactivate();

/* Target real time synchronoscope */
	RLG_Synchs_Mgr_Button = RLG_MT->add_button(Fl_Image::read_xpm(0, synchronoscope_icon), 0,
					      "Synchronoscope", "Open/close synch manager");
	RLG_Synchs_Mgr_Button->callback((Fl_Callback *)rlg_synchs_mgr_cb);
	RLG_Synchs_Mgr_Button->deactivate();

	RLG_MT->add_divider();

	RLG_Text_Button = RLG_MT->add_button(Fl_Image::read_xpm(0, save_profile_icon), 0,
					     "Text", "Open/close text editor");
	RLG_Text_Button->callback((Fl_Callback *)rlg_text_cb);
	RLG_Text_Button->deactivate();
}

Fl_Window *rlg_text_window(int w, int h)
{
	Fl_Window *RLG_TW = new Fl_Window(w, h, "Text");
	Fl_Text_Buffer *buf = new Fl_Text_Buffer();
	Fl_Text_Editor *editor = new Fl_Text_Editor(10, 10, 200, 100);
	buf->text(Profile[0].Text);
	editor->layout_align(FL_ALIGN_CLIENT);
	editor->buffer(buf);
	RLG_TW->end();
	RLG_TW->resizable(RLG_TW);
	RLG_Text_Buffer = buf;
	return RLG_TW;
}

Fl_Main_Window *rlg_main_window(int x, int y, int w, int h)
{
	Fl_Main_Window *RLG_MW;
	Fl_Tool_Bar    *RLG_MT;

/* RTAI-Lab GUI (RLG) main window */
	RLG_MW = new Fl_Main_Window(x, y, w, h, "RTAI-Lab Graphical User Interface");

/* RTAI-Lab GUI (RLG) main menu bar */
	RLG_Main_Menu = RLG_MW->menu();
	RLG_Main_Menu->box(FL_THIN_UP_BOX);
	RLG_Main_Menu->menu(RLG_Main_Menu_Table);

/* RTAI-Lab GUI (RLG) main tool bar */
	RLG_MT = RLG_MW->toolbar();
	RLG_MT->box(FL_THIN_UP_BOX);
	rlg_main_toolbar(RLG_MT);

/* RTAI-Lab GUI (RLG) workspace */
	RLG_Main_Workspace = new Fl_Workspace(0, 0, (int)(Fl::w()), (int)(Fl::h()));
	RLG_MW->view(RLG_Main_Workspace);

/* RTAI-Lab GUI (RLG) main status bar */
	RLG_Main_Status = RLG_MW->status();
	RLG_Main_Status->box(FL_THIN_UP_BOX);
	RLG_Main_Status->label("Ready...");

	RLG_MW->end();

	return RLG_MW;
}

struct option options[] = {
	{ "help",       0, 0, 'h' },
	{ "verbose",    0, 0, 'v' },
	{ "version",    0, 0, 'V' },
	{ "profile",    1, 0, 'p' }
};

void print_usage(void)
{
	fputs(
("\nUsage:  xrtailab [OPTIONS]\n"
"\n"
"OPTIONS:\n"
"  -h, --help\n"
"      print usage\n"
"  -v, --verbose\n"
"      verbose output\n"
"  -V, --version\n"
"      print XRTAI-Lab version\n"
"  -p <profile_name>, --profile <profile_name>\n"
"      direct connection to target with the specified profile\n"
"\n")
		,stderr);
	exit(0);
}

int main(int argc, char **argv)
{
	char *lang_env;
	int c, option_index = 0;
	unsigned int msg;

	while (1) {
		c = getopt_long(argc, argv, "hvVp:", options, &option_index);
		if (c == -1)
			break;
		switch (c) {
			case 'v':
				Verbose = 1;
				break;
			case 'V':
				fputs("XRTAI-Lab version " XRTAILAB_VERSION "\n", stderr);
				exit(0);
			case 'p':
				Direct_Profile = strdup(optarg);
				break;
			case 'h':
				print_usage();
				exit(0);
			default:
				break;
		}
	}

	lang_env = getenv("LANG");
	setenv("LANG", "en_US", 1);

	rt_allow_nonroot_hrt();
	if (!(RLG_Main_Task = rt_task_init_schmod(get_an_id("RLGM"), 99, 0, 0, SCHED_FIFO, 0xFF))) {
		printf("Cannot init RTAI-Lab GUI main task\n");
		return 1;
	}

	/* Quothe the fltk doc's: "The main thread must call lock() once
	 * before <i>any</i> call to fltk to initial the thread system."
	 * So, I moved this call up above the first rlg_*() call.
	 */
	Fl::lock();

	rlg_read_pref(GEOMETRY_PREF, "rtailab", 0);
	rlg_read_pref(PROFILES_PREF, "rtailab", 0);
	Profile = (Profile_T *)calloc(Preferences.Num_Profiles, sizeof(Profile_T));
	for (int i = 0; i < Preferences.Num_Profiles; i++) {
		rlg_read_pref(PROFILE_PREF, Preferences.Profiles.item(i).c_str(), i);
	}

//	RLG_Main_Window = rlg_main_window(0, 0, Fl::w(), Fl::h());
	RLG_Main_Window = rlg_main_window(Preferences.MW_x, Preferences.MW_y, Preferences.MW_w, Preferences.MW_h);
	RLG_Connect_Dialog = rlg_connect_dialog(250, 290);
	RLG_Connect_wProfile_Dialog = rlg_connect_wprofile_dialog(450, 300);
	if (Direct_Profile && Direct_Profile_Idx < 0) {
		printf("No profile with that name.\n");
		rt_task_delete(RLG_Main_Task);
		exit(1);
	}
	RLG_Save_Profile_Dialog = rlg_save_profile_dialog(250, 100);
	RLG_Delete_Profile_Dialog = rlg_delete_profile_dialog(250, 200);
//	RLG_Text_Window = rlg_text_window(200, 200);
	RLG_Main_Window->show();
//	RLG_Main_Window->show(argc, argv);

	pthread_create(&Target_Interface_Thread, NULL, rt_target_interface, NULL);
	rt_receive(0, &msg); // Wait for thread to start (it does an rt_send).

	if (Direct_Profile) {
		RT_RPC(Target_Interface_Task, CONNECT_TO_TARGET_WITH_PROFILE, 0);
	}

	while (!End_All)  Fl::wait(FLTK_EVENTS_TICK);

	rt_task_delete(RLG_Main_Task);


	setenv("LANG", lang_env, 1);

//	printf("%s\n", RLG_Text_Buffer->text());

	return 0;
}
