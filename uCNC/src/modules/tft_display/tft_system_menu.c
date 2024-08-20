/*
	Name: tft_system_menu.c
	Description: TFT display for µCNC using lgvl. Integration functions with system menu

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 20-08-2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "src/cnc.h"
#include "src/modules/system_menu.h"
#include "src/modules/softspi.h"
#include "lvgl/lvgl.h"
#include "lvgl/src/drivers/display/st7796/lv_st7796.h"
#include "ui/ui.h"
#include "src/modules/touch_screen/touch_screen.h"

#ifndef MAX_TFT_LINES
#define MAX_TFT_LINES 6
#endif

/**
 * Helper functions
 */
static void io_states_str(char *buff)
{
	uint8_t controls = io_get_controls();
	uint8_t limits = io_get_limits();
	uint8_t probe = io_get_probe();
	uint8_t i = 0;
	if (CHECKFLAG(controls, (ESTOP_MASK | SAFETY_DOOR_MASK | FHOLD_MASK)) || CHECKFLAG(limits, LIMITS_MASK) || probe)
	{
		if (CHECKFLAG(controls, ESTOP_MASK))
		{
			buff[i++] = 'R';
		}

		if (CHECKFLAG(controls, SAFETY_DOOR_MASK))
		{
			buff[i++] = 'D';
		}

		if (CHECKFLAG(controls, FHOLD_MASK))
		{
			buff[i++] = 'H';
		}

		if (probe)
		{
			buff[i++] = 'P';
		}

		if (CHECKFLAG(limits, LINACT0_LIMIT_MASK))
		{
			buff[i++] = 'X';
		}

		if (CHECKFLAG(limits, LINACT1_LIMIT_MASK))
		{

#if ((AXIS_COUNT == 2) && defined(USE_Y_AS_Z_ALIAS))
			buff[i++] = 'Z';
#else
			buff[i++] = 'Y';
#endif
		}

		if (CHECKFLAG(limits, LINACT2_LIMIT_MASK))
		{
			buff[i++] = 'Z';
		}

		if (CHECKFLAG(limits, LINACT3_LIMIT_MASK))
		{
			buff[i++] = 'A';
		}

		if (CHECKFLAG(limits, LINACT4_LIMIT_MASK))
		{
			buff[i++] = 'B';
		}

		if (CHECKFLAG(limits, LINACT5_LIMIT_MASK))
		{
			buff[i++] = 'C';
		}
	}
}

static void render_status(char *buff)
{
	uint8_t state = cnc_get_exec_state(0xFF);
	uint8_t filter = 0x80;
	while (!(state & filter) && filter)
	{
		filter >>= 1;
	}

	state &= filter;
	if (cnc_has_alarm())
	{
		rom_strcpy(buff, MSG_STATUS_ALARM);
		ui_object_set_themeable_style_property(ui_idle_container_statusinfo, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_COLOR,
																					 _ui_theme_color_alarm);
	}
	else if (mc_get_checkmode())
	{
		rom_strcpy(buff, MSG_STATUS_CHECK);
	}
	else
	{
		switch (state)
		{
		case EXEC_DOOR:
			rom_strcpy(buff, MSG_STATUS_DOOR);
			ui_object_set_themeable_style_property(ui_idle_container_statusinfo, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_COLOR,
																						 _ui_theme_color_hold);
			break;
		case EXEC_KILL:
		case EXEC_UNHOMED:
			rom_strcpy(buff, MSG_STATUS_ALARM);
			ui_object_set_themeable_style_property(ui_idle_container_statusinfo, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_COLOR,
																						 _ui_theme_color_alarm);
			break;
		case EXEC_HOLD:
			rom_strcpy(buff, MSG_STATUS_HOLD);
			ui_object_set_themeable_style_property(ui_idle_container_statusinfo, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_COLOR,
																						 _ui_theme_color_hold);
			break;
		case EXEC_HOMING:
			rom_strcpy(buff, MSG_STATUS_HOME);
			ui_object_set_themeable_style_property(ui_idle_container_statusinfo, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_COLOR,
																						 _ui_theme_color_button);
			break;
		case EXEC_JOG:
			rom_strcpy(buff, MSG_STATUS_JOG);
			ui_object_set_themeable_style_property(ui_idle_container_statusinfo, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_COLOR,
																						 _ui_theme_color_button);
			break;
		case EXEC_RUN:
			rom_strcpy(buff, MSG_STATUS_RUN);
			ui_object_set_themeable_style_property(ui_idle_container_statusinfo, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_COLOR,
																						 _ui_theme_color_button);
			break;
		default:
			rom_strcpy(buff, MSG_STATUS_IDLE);
			ui_object_set_themeable_style_property(ui_idle_container_statusinfo, LV_PART_MAIN | LV_STATE_DEFAULT, LV_STYLE_BG_COLOR,
																						 _ui_theme_color_checked);
			break;
		}
	}

	lv_label_set_text(ui_idle_label_statusvalue, buff);
}

static void append_modal_state(char *buffer, char word, uint8_t code, uint8_t mantissa)
{
	char tmp[32];
	memset(tmp, 0, sizeof(tmp));
	if (mantissa)
	{
		sprintf(tmp, "%c%d.%d", word, code, mantissa);
	}
	else
	{
		sprintf(tmp, "%c%d", word, code);
	}

	strcat(buffer, tmp);
}

/**
 *
 * Sytem menu implementations
 *
 */
static int8_t item_line;
static int8_t item_line_value;
void system_menu_render_header(const char *__s)
{
	// udpate header lables
	lv_label_set_text(ui_navigate_label_headerlabel, __s);
	lv_label_set_text(ui_edit_label_headerlabel, __s);
	item_line = 0;
	item_line_value = 0;
}

bool system_menu_render_menu_item_filter(uint8_t item_index)
{
	static uint8_t menu_top = 0;
	uint8_t current_index = MAX(0, g_system_menu.current_index);
	uint8_t max_lines = MAX_TFT_LINES;

	uint8_t top = menu_top;
	if ((top + max_lines) <= current_index)
	{
		// advance menu top
		menu_top = top = (current_index - max_lines + 1);
	}

	if (top > current_index)
	{
		// rewind menu top
		menu_top = top = current_index;
	}

	return ((top <= item_index) && (item_index < (top + max_lines)));
}

void system_menu_item_render_label(uint8_t render_flags, const char *label)
{
	int8_t line = item_line;
	item_line = line + 1;

	char str[64];
	memset(str, 0, sizeof(str));
	if (label)
	{
		strcpy(str, label);
	}

	switch (line)
	{
	case 0:
		lv_label_set_text(ui_navigate_label_menuitemlabel0, str);
		lv_obj_set_state(ui_navigate_container_menuitem0, LV_STATE_FOCUSED, (render_flags & SYSTEM_MENU_MODE_SELECT));
		break;
	case 1:
		lv_label_set_text(ui_navigate_label_menuitemlabel1, str);
		lv_obj_set_state(ui_navigate_container_menuitem1, LV_STATE_FOCUSED, (render_flags & SYSTEM_MENU_MODE_SELECT));
		break;
	case 2:
		lv_label_set_text(ui_navigate_label_menuitemlabel2, str);
		lv_obj_set_state(ui_navigate_container_menuitem2, LV_STATE_FOCUSED, (render_flags & SYSTEM_MENU_MODE_SELECT));
		break;
	case 3:
		lv_label_set_text(ui_navigate_label_menuitemlabel3, str);
		lv_obj_set_state(ui_navigate_container_menuitem3, LV_STATE_FOCUSED, (render_flags & SYSTEM_MENU_MODE_SELECT));
		break;
	case 4:
		lv_label_set_text(ui_navigate_label_menuitemlabel4, str);
		lv_obj_set_state(ui_navigate_container_menuitem4, LV_STATE_FOCUSED, (render_flags & SYSTEM_MENU_MODE_SELECT));
		break;
	case 5:
		lv_label_set_text(ui_navigate_label_menuitemlabel5, str);
		lv_obj_set_state(ui_navigate_container_menuitem5, LV_STATE_FOCUSED, (render_flags & SYSTEM_MENU_MODE_SELECT));
		break;
	}
}

void system_menu_item_render_arg(uint8_t render_flags, const char *value)
{
	char str[64];
	memset(str, 0, sizeof(str));
	if (value)
	{
		strcpy(str, value);
	}

	int8_t line = item_line_value;
	item_line_value = line + 1;
	switch (line)
	{
	case 0:
		lv_label_set_text(ui_navigate_label_menuitemvalue0, str);
		break;
	case 1:
		lv_label_set_text(ui_navigate_label_menuitemvalue1, str);
		break;
	case 2:
		lv_label_set_text(ui_navigate_label_menuitemvalue2, str);
		break;
	case 3:
		lv_label_set_text(ui_navigate_label_menuitemvalue3, str);
		break;
	case 4:
		lv_label_set_text(ui_navigate_label_menuitemvalue4, str);
		break;
	case 5:
		lv_label_set_text(ui_navigate_label_menuitemvalue5, str);
		break;
	}
}

void system_menu_render_nav_back(bool is_hover)
{
	if (is_hover)
	{
		lv_obj_add_state(ui_navigate_button_btnclose, LV_STATE_FOCUSED);
		lv_obj_add_state(ui_edit_button_btnclose, LV_STATE_FOCUSED);
		lv_obj_add_state(ui_jog_button_btnclose, LV_STATE_FOCUSED);
	}
	else
	{
		lv_obj_remove_state(ui_navigate_button_btnclose, LV_STATE_FOCUSED);
		lv_obj_remove_state(ui_edit_button_btnclose, LV_STATE_FOCUSED);
		lv_obj_remove_state(ui_jog_button_btnclose, LV_STATE_FOCUSED);
	}
}

void system_menu_render_footer(void)
{
	if (g_system_menu.flags & SYSTEM_MENU_MODE_EDIT)
	{
		lv_disp_load_scr(ui_edit);
	}
	else
	{
		lv_disp_load_scr(ui_navigate);
		while (item_line < MAX_TFT_LINES)
		{
			system_menu_item_render_label(0, "");
		}
		while (item_line_value < MAX_TFT_LINES)
		{
			system_menu_item_render_arg(0, "");
		}
	}
}

void system_menu_render_startup(void)
{
	lv_label_set_text(ui_startup_label_versioninfo, "uCNC v" CNC_VERSION);
	lv_disp_load_scr(ui_startup);
}

void system_menu_render_idle(void)
{
	//  starts from the bottom up
	float axis[MAX(AXIS_COUNT, 3)];
	int32_t steppos[STEPPER_COUNT];
	itp_get_rt_position(steppos);
	kinematics_apply_forward(steppos, axis);
	kinematics_apply_reverse_transform(axis);
	char buffer[64];

	memset(buffer, 0, sizeof(buffer));
	render_status(buffer);

#if (AXIS_COUNT >= 6)
	memset(buffer, 0, sizeof(buffer));
	system_menu_flt_to_str(buffer, axis[5]);
	lv_label_set_text(ui_comp_get_child(ui_idle_axisinfo_axisinfoc, UI_COMP_CONTAINER_AXISINFO_LABEL_AXISVALUE), buffer);
#else
	lv_obj_set_width(ui_idle_axisinfo_axisinfoc, 0);
#endif
#if (AXIS_COUNT >= 5)
	memset(buffer, 0, sizeof(buffer));
	system_menu_flt_to_str(buffer, axis[4]);
	lv_label_set_text(ui_comp_get_child(ui_idle_axisinfo_axisinfob, UI_COMP_CONTAINER_AXISINFO_LABEL_AXISVALUE), buffer);
#else
	lv_obj_set_width(ui_idle_axisinfo_axisinfob, 0);
#endif
#if (AXIS_COUNT >= 4)
	memset(buffer, 0, sizeof(buffer));
	system_menu_flt_to_str(buffer, axis[3]);
	lv_label_set_text(ui_comp_get_child(ui_idle_axisinfo_axisinfoa, UI_COMP_CONTAINER_AXISINFO_LABEL_AXISVALUE), buffer);
#else
	lv_obj_set_height(ui_idle_container_container9, 0); // hide the full container
#endif
#if (AXIS_COUNT >= 3)
	memset(buffer, 0, sizeof(buffer));
	system_menu_flt_to_str(buffer, axis[2]);
	lv_label_set_text(ui_comp_get_child(ui_idle_axisinfo_axisinfoz, UI_COMP_CONTAINER_AXISINFO_LABEL_AXISVALUE), buffer);
#else
	lv_obj_set_width(ui_idle_axisinfo_axisinfoz, 0);
#endif
#if (AXIS_COUNT >= 2)
	memset(buffer, 0, sizeof(buffer));
	system_menu_flt_to_str(buffer, axis[1]);
	lv_label_set_text(ui_comp_get_child(ui_idle_axisinfo_axisinfoy, UI_COMP_CONTAINER_AXISINFO_LABEL_AXISVALUE), buffer);
#else
	lv_obj_set_width(ui_idle_axisinfo_axisinfoy, 0);
#endif
#if (AXIS_COUNT >= 1)
	memset(buffer, 0, sizeof(buffer));
	system_menu_flt_to_str(buffer, axis[0]);
	lv_label_set_text(ui_comp_get_child(ui_idle_axisinfo_axisinfox, UI_COMP_CONTAINER_AXISINFO_LABEL_AXISVALUE), buffer);
#endif

	memset(buffer, 0, sizeof(buffer));
	io_states_str(buffer);
	lv_label_set_text(ui_idle_label_switchvalue, buffer);

	memset(buffer, 0, sizeof(buffer));
	system_menu_int_to_str(buffer, (int32_t)itp_get_rt_feed());
	lv_label_set_text(ui_idle_label_feedvalue, buffer);

	// Tool
	uint8_t modalgroups[14];
	uint16_t feed;
	uint16_t spindle;
	parser_get_modes(modalgroups, &feed, &spindle);
	memset(buffer, 0, sizeof(buffer));
	system_menu_int_to_str(buffer, modalgroups[11]);
	lv_label_set_text(ui_idle_label_toolnumbervalue, buffer);
	// Realtime tool speed
	memset(buffer, 0, sizeof(buffer));
	system_menu_int_to_str(buffer, tool_get_speed());
	lv_label_set_text(ui_idle_label_toolspeedvalue, buffer);

	/**shoe some modal states */
	memset(buffer, 0, sizeof(buffer));
	append_modal_state(buffer, 'G', modalgroups[0], modalgroups[12]);
	for (uint8_t i = 1; i < 7; i++)
	{
		append_modal_state(buffer, 'G', modalgroups[i], 0);
	}

	append_modal_state(buffer, 'M', modalgroups[8], 0);
#ifdef ENABLE_COOLANT
	if (modalgroups[9] == M9)
	{
		append_modal_state(buffer, 'M', 9, 0);
	}
#ifndef M7_SAME_AS_M8
	if (modalgroups[9] & M7)
	{
		append_modal_state(buffer, 'M', 7, 0);
	}
#endif
	if (modalgroups[9] & M8)
	{
		append_modal_state(buffer, 'M', 8, 0);
	}
#else
	// permanent M9
	append_modal_state(buffer, 'M', 9, 0);
#endif

	lv_label_set_text(ui_idle_label_modalmodesvalue, buffer);

	// lv_obj_invalidate(ui_idle);
	lv_disp_load_scr(ui_idle);
}

void system_menu_render_jog(void)
{
	lv_disp_load_scr(ui_jog);
}

// void system_menu_render_alarm(void);

// void system_menu_render_modal_popup(const char *__s);
