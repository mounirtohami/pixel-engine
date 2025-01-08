/**************************************************************************/
/*  pixel_engine.cpp                                                      */
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

#include "pixel_engine.h"

// #include "core/input/input_event.h"
#include "pixel/user_interface.h"
#include "scene/gui/split_container.h"

void PixelEngine::_notification(int p_what) {
}

void PixelEngine::shortcut_input(const Ref<InputEvent> &p_event) {
	Ref<InputEventKey> key = p_event;
	if (key.is_valid() && key->is_pressed()) {
		UserInterface *ui = UserInterface::get_singleton();
		int coll = (int)ui->get_left_split()->get_collapse_mode();
		coll = Math::wrapi(coll + 1, 0, 4);
		ui->get_left_split()->set_collapse_mode(SplitContainer::CollapseMode(coll));
	}
	// Shortcuts.
}

void PixelEngine::input(const Ref<InputEvent> &p_event) {
	// print_line(p_event);
	// Shortcuts.
}

PixelEngine::PixelEngine() {
	set_name("PixelEngine");
	singleton = this;
	set_process_shortcut_input(true);
	set_process_input(true);

	add_child(memnew(UserInterface));
}
