/**************************************************************************/
/*  user_interface.h                                                      */
/**************************************************************************/
/*                             PIXEL ENGINE                               */
/**************************************************************************/
/* Copyright (c) 2024-present Pixel Engine contributors (see AUTHORS.md). */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef USER_INTERFACE_H
#define USER_INTERFACE_H

#include "scene/gui/panel_container.h"

class HBoxContainer;
class VBoxContainer;
class MenuBar;
class Button;
class HSplitContainer;
class VSplitContainer;
class TabContainer;

class UserInterface : public PanelContainer {
	GDCLASS(UserInterface, PanelContainer)

	static inline UserInterface *singleton;
	VBoxContainer *main_vbox = nullptr;
	HBoxContainer *top_hbox = nullptr;
	HBoxContainer *main_hbox = nullptr;
	MenuBar *menu_bar = nullptr;
	HBoxContainer *menu_hbox = nullptr;
	HSplitContainer *left_split = nullptr;
	HSplitContainer *right_split = nullptr;
	TabContainer *left_tab = nullptr;
	VSplitContainer *center_split = nullptr;
	TabContainer *right_tab = nullptr;
	TabContainer *center_tab = nullptr;
	TabContainer *bottom_tab = nullptr;
	HBoxContainer *bottom_hbox = nullptr;

public:
	static UserInterface *get_singleton() { return singleton; }
	HSplitContainer *get_left_split() { return left_split; }
	HSplitContainer *get_right_split() { return right_split; }
	VSplitContainer *get_center_split() { return center_split; }

	void _notification(int p_what);
	UserInterface();
};

#endif // USER_INTERFACE_H
