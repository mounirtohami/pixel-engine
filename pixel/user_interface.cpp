/**************************************************************************/
/*  user_interface.cpp                                                    */
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

#include "user_interface.h"

#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/menu_bar.h"
#include "scene/gui/split_container.h"
#include "scene/gui/tab_container.h"

#include "scene/gui/color_picker.h"
#include "scene/gui/spin_slider.h"

void UserInterface::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			get_tree()->get_root()->print_tree_pretty();
		} break;
	}
}

UserInterface::UserInterface() {
	singleton = this;
	set_name("UserInterface");
	// set_custom_minimum_size(Size2(800, 600));
	set_anchors_and_offsets_preset(PRESET_FULL_RECT);
	set_theme_type_variation("FlatPanel");

	/* #region MainVBox */
	main_vbox = memnew(VBoxContainer);
	main_vbox->set_h_size_flags(SIZE_EXPAND_FILL);
	main_vbox->set_v_size_flags(SIZE_EXPAND_FILL);
	main_vbox->set_name("MainVBox");
	add_child(main_vbox);
	/* #endregion */

	/* #region TopHBox */
	top_hbox = memnew(HBoxContainer);
	top_hbox->set_name("TopHBox");
	top_hbox->set_h_size_flags(SIZE_EXPAND_FILL);
	main_vbox->add_child(top_hbox);
	/* #endregion */

	/* #region MenuBar */
	menu_bar = memnew(MenuBar);
	menu_bar->set_name("MenuBar");
	menu_bar->set_flat(true);
	menu_bar->set_h_size_flags(SIZE_EXPAND_FILL);
	top_hbox->add_child(menu_bar);
	/* #endregion */

	/* #region MenuBarTest */
	PopupMenu *menu_bar_test = memnew(PopupMenu);
	menu_bar_test->set_name("MenuBarTest");
	menu_bar->add_child(menu_bar_test);
	PopupMenu *menu_bar_test2 = memnew(PopupMenu);
	menu_bar_test2->set_name("MenuBarTest2");
	menu_bar->add_child(menu_bar_test2);
	/* #endregion */

	/* #region MainHBox */
	main_hbox = memnew(HBoxContainer);
	main_hbox->set_name("MainHBox");
	main_hbox->set_h_size_flags(SIZE_EXPAND_FILL);
	main_hbox->set_v_size_flags(SIZE_EXPAND_FILL);
	main_vbox->add_child(main_hbox);
	/* #endregion */

	/* #region LeftHSplit */
	left_split = memnew(HSplitContainer);
	left_split->set_name("LeftHSplit");
	left_split->set_h_size_flags(SIZE_EXPAND_FILL);
	left_split->set_collapse_mode(SplitContainer::COLLAPSE_FIRST);
	main_hbox->add_child(left_split);
	/* #endregion */

	/* #region LeftTab */
	left_tab = memnew(TabContainer);
	left_tab->set_name("LeftTab");
	left_tab->set_drag_to_rearrange_enabled(true);
	left_tab->set_custom_minimum_size(Size2(128, 128));
	left_tab->set_stretch_ratio(0.5);
	left_tab->set_h_size_flags(SIZE_EXPAND_FILL);
	left_tab->set_v_size_flags(SIZE_EXPAND_FILL);
	left_split->add_child(left_tab);
	/* #region LeftTabTest */
	Control *top_left_tab_test = memnew(Control);
	top_left_tab_test->set_name("LeftTabTest");
	top_left_tab_test->set_h_size_flags(SIZE_EXPAND_FILL);
	left_tab->add_child(top_left_tab_test);
	SpinSlider *spin = memnew(SpinSlider);
	spin->set_anchors_and_offsets_preset(PRESET_TOP_WIDE);
	top_left_tab_test->add_child(spin);
	spin->set_step(0.01);
	spin->set_suffix("px");
	Control *top_left_tab_test2 = memnew(Control);
	top_left_tab_test2->set_name("LeftTabTest2");
	top_left_tab_test2->set_h_size_flags(SIZE_EXPAND_FILL);
	left_tab->add_child(top_left_tab_test2);
	/* #endregion */
	/* #endregion */

	/* #region RightHSplit */
	right_split = memnew(HSplitContainer);
	right_split->set_name("RightHSplit");
	right_split->set_h_size_flags(SIZE_EXPAND_FILL);
	right_split->set_collapse_mode(SplitContainer::COLLAPSE_SECOND);
	left_split->add_child(right_split);
	/* #endregion */

	/* #region CenterSplit */
	center_split = memnew(VSplitContainer);
	center_split->set_name("CenterSplit");
	center_split->set_h_size_flags(SIZE_EXPAND_FILL);
	center_split->set_v_size_flags(SIZE_EXPAND_FILL);
	center_split->set_collapse_mode(SplitContainer::COLLAPSE_SECOND);
	right_split->add_child(center_split);
	/* #endregion */

	/* #region CenterTab */
	center_tab = memnew(TabContainer);
	center_tab->set_name("CenterTab");
	center_tab->set_drag_to_rearrange_enabled(true);
	center_tab->set_custom_minimum_size(Size2(128, 128));
	center_tab->set_tab_alignment(TabBar::ALIGNMENT_CENTER);
	center_tab->set_h_size_flags(SIZE_EXPAND_FILL);
	center_tab->set_v_size_flags(SIZE_EXPAND_FILL);
	center_split->add_child(center_tab);
	TabBar *center_tab_bar = center_tab->get_tab_bar();
	center_tab_bar->set_theme_type_variation("ButtonsTab");
	center_tab->remove_child(center_tab_bar);
	center_tab_bar->set_h_size_flags(SIZE_EXPAND_FILL);
	top_hbox->add_child(center_tab_bar);
	/* #region CenterTabTest */
	Control *center_tab_test = memnew(Control);
	center_tab_test->set_name("CenterTabTest");
	center_tab_test->set_h_size_flags(SIZE_EXPAND_FILL);
	center_tab->add_child(center_tab_test);
	ColorPicker *color_picker = memnew(ColorPicker);
	color_picker->set_anchors_and_offsets_preset(PRESET_FULL_RECT);
	center_tab_test->add_child(color_picker);
	Control *center_tab_test2 = memnew(Control);
	center_tab_test2->set_name("CenterTabTest2");
	center_tab_test2->set_h_size_flags(SIZE_EXPAND_FILL);
	center_tab->add_child(center_tab_test2);
	/* #endregion */
	/* #endregion */

	/* #region BottomTab */
	bottom_tab = memnew(TabContainer);
	bottom_tab->set_name("BottomTab");
	bottom_tab->set_drag_to_rearrange_enabled(true);
	bottom_tab->set_custom_minimum_size(Size2(128, 128));
	bottom_tab->set_tabs_position(TabContainer::TabPosition::POSITION_BOTTOM);
	bottom_tab->set_stretch_ratio(0.4);
	bottom_tab->set_v_size_flags(SIZE_EXPAND_FILL);
	center_split->add_child(bottom_tab);
	/* #region BottomTabTest */
	Control *bottom_tab_test = memnew(Control);
	bottom_tab_test->set_name("BottomTabTest");
	bottom_tab_test->set_h_size_flags(SIZE_EXPAND_FILL);
	bottom_tab->add_child(bottom_tab_test);
	Control *bottom_tab_test2 = memnew(Control);
	bottom_tab_test2->set_name("BottomTabTest2");
	bottom_tab_test2->set_h_size_flags(SIZE_EXPAND_FILL);
	bottom_tab->add_child(bottom_tab_test2);
	/* #endregion */
	/* #endregion */

	/* #region RightTab */
	right_tab = memnew(TabContainer);
	right_tab->set_name("RightTab");
	right_tab->set_drag_to_rearrange_enabled(true);
	right_tab->set_custom_minimum_size(Size2(128, 128));
	right_tab->set_h_size_flags(SIZE_EXPAND_FILL);
	right_tab->set_v_size_flags(SIZE_EXPAND_FILL);
	right_split->add_child(right_tab);
	/* #region RightTabTest */
	{
		Control *right_tab_test = memnew(Control);
		right_tab_test->set_name("RightTabTest");
		right_tab_test->set_h_size_flags(SIZE_EXPAND_FILL);
		right_tab->add_child(right_tab_test);
		Control *right_tab_test2 = memnew(Control);
		right_tab_test2->set_name("RightTabTest2");
		right_tab_test2->set_h_size_flags(SIZE_EXPAND_FILL);
		right_tab->add_child(right_tab_test2);
	}
	/* #endregion */
	/* #endregion */

	/* #region BottomHBox */
	bottom_hbox = memnew(HBoxContainer);
	bottom_hbox->set_name("BottomHBox");
	bottom_hbox->set_h_size_flags(SIZE_EXPAND_FILL);
	main_vbox->add_child(bottom_hbox);
	/* #region TestBottomHBox */
	Button *bottom_hbox_test = memnew(Button);
	bottom_hbox_test->set_name("BottomHBox");
	bottom_hbox->add_child(bottom_hbox_test);
	/* #endregion */
	/* #endregion */

	/* #region MenuHBox */
	menu_hbox = memnew(HBoxContainer);
	menu_hbox->set_h_size_flags(SIZE_EXPAND | SIZE_SHRINK_END);
	menu_hbox->set_name("MenuHBox");
	top_hbox->add_child(menu_hbox);
	/* #endregion */

	/* #region FileMenuTest */
	Button *test_button = memnew(Button);
	test_button->set_name("MenuTest");
	test_button->set_text("MenuTest");
	menu_hbox->add_child(test_button);
	Button *test_button2 = memnew(Button);
	test_button2->set_name("MenuTest2");
	test_button2->set_text("MenuTest2");
	menu_hbox->add_child(test_button2);
	/* #endregion */

	// PopupMenu *file_menu = memnew(PopupMenu);
	// file_menu->set_name("File");
	// menu_bar->add_child(file_menu);
	// set_process(true);
}
