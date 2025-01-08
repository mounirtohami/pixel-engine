/**************************************************************************/
/*  main_tree.cpp                                                         */
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

#include "main_tree.h"

#include "core/config/project_settings.h"
#include "pixel/pixel_engine.h"
#include "scene/main/window.h"

void MainTree::initialize() {
	SceneTree::initialize();

	get_root()->set_min_size(Size2i(640, 480));

	ProjectSettings::get_singleton()->set("debug/settings/stdout/print_fps", true);
	print_line(Engine::get_singleton()->get_max_fps());
	// OS::get_singleton()->set_low_processor_usage_mode(true);
	DisplayServer::get_singleton()->window_set_vsync_mode(DisplayServer::VSyncMode::VSYNC_DISABLED);

	OS::get_singleton()->set_low_processor_usage_mode_sleep_usec(0);
	print_line(OS::get_singleton()->is_in_low_processor_usage_mode());

	// List<PropertyInfo> plist;
	// ProjectSettings::get_singleton()->get_property_list(&plist);

	// for (const PropertyInfo &E : plist) {
	// 	print_line(E.name);
	// }

	// print_line(PixelEngine::get_singleton()->get_name());
	// print_line(Engine::get_singleton()->get_version_info());
	// print_line(OS::get_singleton()->get_current_rendering_driver_name());
	// print_line(OS::get_singleton()->get_current_rendering_method());
	// OS::get_singleton()->shell_show_in_file_manager(OS::get_singleton()->get_user_data_dir(), true);

	if (!FileAccess::exists(OS::get_singleton()->get_user_data_dir().path_join("settings.pixel"))) {
		ProjectSettings::get_singleton()->save();
	}
}

MainTree::MainTree() {
	ERR_FAIL_NULL_MSG(get_root(), "MainTree root is null");

	singleton = this;
	set_auto_accept_quit(true);
	set_quit_on_go_back(false);

	root = get_root();
	root->set_title("Pixel Engine");
	root->set_name("Root");
	root->set_auto_translate_mode(Node::AUTO_TRANSLATE_MODE_DISABLED);
	root->add_child(memnew(PixelEngine));
}
