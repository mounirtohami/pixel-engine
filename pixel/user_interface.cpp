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
#include "scene/gui/color_picker.h"

void UserInterface::_notification(int p_what) {
	// if (p_what == NOTIFICATION_DRAW) {
	// 	RS::get_singleton()->canvas_item_add_rect(get_canvas_item(), Rect2(0, 0, get_size().x, get_size().y), Color(0.5, 0, 0, 0.5));
	// }
}

void UserInterface::_test_color(const Color &p_color) {
	get_tree()->get_root()->set_clear_color(p_color);
}

UserInterface::UserInterface() {
	set_name("UserInterface");
	set_anchors_and_offsets_preset(PRESET_FULL_RECT);
	print_line("USER INTERFACE");
	ColorPickerButton *cpb = memnew(ColorPickerButton);
	cpb->set_custom_minimum_size(Vector2(100, 64));
	add_child(cpb);
	cpb->connect("color_changed", callable_mp(this, &UserInterface::_test_color));
}
