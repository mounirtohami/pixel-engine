/**************************************************************************/
/*  color_picker.cpp                                                      */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
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

/**************************************************************************/
/*                             PIXEL ENGINE                               */
/* Copyright (c) 2024-present Pixel Engine contributors (see AUTHORS.md). */
/**************************************************************************/
/* NOTICE:                                                                */
/* This file contains modifications and additions specific to the Pixel   */
/* Engine project. While these changes are licensed under the MIT license */
/* for compatibility, we request proper attribution if reused in any      */
/* derivative works, including meta-forks.                                */
/**************************************************************************/

#include "color_picker.h"

#include "scene/gui/aspect_ratio_container.h"
#include "scene/gui/button.h"
#include "scene/gui/color_mode.h"
#include "scene/gui/foldable_container.h"
#include "scene/gui/grid_container.h"
#include "scene/gui/label.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/margin_container.h"
#include "scene/gui/menu_button.h"
#include "scene/gui/panel.h"
#include "scene/gui/panel_container.h"
#include "scene/gui/popup_menu.h"
#include "scene/gui/scroll_container.h"
#include "scene/gui/slider.h"
#include "scene/gui/spin_box.h"
#include "scene/gui/texture_rect.h"
#include "scene/resources/atlas_texture.h"
#include "scene/resources/gradient_texture.h"
#include "scene/resources/image_texture.h"
#include "scene/resources/style_box_flat.h"
#include "scene/theme/theme_db.h"
#include "thirdparty/misc/ok_color_shader.h"

PackedColorArray ColorPicker::swatches;

void ColorPicker::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			_update_color();
		} break;

		case NOTIFICATION_READY: {
			// FIXME: The embedding check is needed to fix a bug in single-window mode (GH-93718).
			if (DisplayServer::get_singleton()->has_feature(DisplayServer::FEATURE_SCREEN_CAPTURE) && !get_tree()->get_root()->is_embedding_subwindows()) {
				btn_pick->set_tooltip_text(ETR("Pick a color from the screen."));
				btn_pick->connect(SceneStringName(pressed), callable_mp(this, &ColorPicker::_pick_button_pressed));
			} else {
				// On unsupported platforms, use a legacy method for color picking.
				btn_pick->set_tooltip_text(ETR("Pick a color from the application window."));
				btn_pick->connect(SceneStringName(pressed), callable_mp(this, &ColorPicker::_pick_button_pressed_legacy));
			}
		} break;

		case NOTIFICATION_THEME_CHANGED: {
			btn_pick->set_button_icon(theme_cache.screen_picker);
			// swatcher_foldable_container->set_button_icon(0, theme_cache.add_preset);

			uv_edit->set_custom_minimum_size(Size2(theme_cache.sv_width, theme_cache.sv_height));
			w_edit->set_custom_minimum_size(Size2(theme_cache.h_width, 0));
			sample->set_custom_minimum_size(Size2(64, theme_cache.sample_height));

			wheel_edit->set_custom_minimum_size(Size2(theme_cache.sv_width, theme_cache.sv_height));

			float char_size = get_theme_font(SceneStringName(font), SNAME("Label"))->get_char_size('W', get_theme_font_size(SceneStringName(font_size), SNAME("Label"))).width;
			for (int i = 0; i < SLIDER_COUNT; i++) {
				labels[i]->set_custom_minimum_size(Size2(char_size, 0));
				sliders[i]->queue_redraw();
			}
			alpha_label->set_custom_minimum_size(Size2(char_size, 0));
			alpha_slider->queue_redraw();

			shape_popup->set_item_icon(shape_popup->get_item_index(SHAPE_HSV_RECTANGLE), theme_cache.shape_rect);
			shape_popup->set_item_icon(shape_popup->get_item_index(SHAPE_HSV_WHEEL), theme_cache.shape_rect_wheel);
			shape_popup->set_item_icon(shape_popup->get_item_index(SHAPE_VHS_CIRCLE), theme_cache.shape_circle);
			shape_popup->set_item_icon(shape_popup->get_item_index(SHAPE_OKHSL_CIRCLE), theme_cache.shape_circle);

			if (current_shape != SHAPE_NONE) {
				btn_shape->set_button_icon(shape_popup->get_item_icon(current_shape));
			}

			if (text_is_constructor) {
				text_type->set_button_icon(theme_cache.hex_code_icon);
			} else {
				text_type->set_button_icon(theme_cache.hex_icon);
			}

			Control::SizeFlags hb_size_flag = theme_cache.expand_shape > 0 ? SIZE_EXPAND_FILL : SIZE_FILL;
			hb_edit->set_v_size_flags(hb_size_flag);

			swatches_foldable->set_button_icon(0, theme_cache.add_preset);

			for (int i = 0; i < swatches_hbc->get_child_count(); i++) {
				Object::cast_to<SwatchButton>(swatches_hbc->get_child(i))->set_custom_minimum_size(Size2(theme_cache.swatch_size, theme_cache.swatch_size));
			}

			_update_controls();
		} break;

		case NOTIFICATION_WM_CLOSE_REQUEST: {
			if (picker_window != nullptr && picker_window->is_visible()) {
				picker_window->hide();
			}
		} break;

		case NOTIFICATION_INTERNAL_PROCESS: {
			if (!is_picking_color) {
				return;
			}
			DisplayServer *ds = DisplayServer::get_singleton();
			Vector2 ofs = ds->mouse_get_position();
			picker_window->set_position(ofs - Vector2(28, 28));

			Color c = DisplayServer::get_singleton()->screen_get_pixel(ofs);

			picker_preview_style_box_color->set_bg_color(c);
			picker_preview_style_box->set_bg_color(c.get_luminance() < 0.5 ? Color(1.0f, 1.0f, 1.0f) : Color(0.0f, 0.0f, 0.0f));

			if (ds->has_feature(DisplayServer::FEATURE_SCREEN_EXCLUDE_FROM_CAPTURE)) {
				Ref<Image> zoom_preview_img = ds->screen_get_image_rect(Rect2i(ofs.x - 8, ofs.y - 8, 17, 17));
				picker_window->set_position(ofs - Vector2(28, 28));
				picker_texture_zoom->set_texture(ImageTexture::create_from_image(zoom_preview_img));
			} else {
				Size2i screen_size = ds->screen_get_size();
				picker_window->set_position(ofs + Vector2(ofs.x < screen_size.width / 2 ? 8 : -36, ofs.y < screen_size.height / 2 ? 8 : -36));
			}

			set_pick_color(c);
		}
	}
}

void ColorPicker::_update_theme_item_cache() {
	VBoxContainer::_update_theme_item_cache();
}

void ColorPicker::init_shaders() {
	wheel_shader.instantiate();
	wheel_shader->set_code(R"(
// ColorPicker wheel shader.

shader_type canvas_item;

uniform float wheel_radius = 0.4;

void fragment() {
	float x = UV.x - 0.5;
	float y = UV.y - 0.5;
	float a = atan(y, x);
	x += 0.001;
	y += 0.001;
	float b = float(sqrt(x * x + y * y) < 0.5) * float(sqrt(x * x + y * y) > wheel_radius);
	x -= 0.002;
	float b2 = float(sqrt(x * x + y * y) < 0.5) * float(sqrt(x * x + y * y) > wheel_radius);
	y -= 0.002;
	float b3 = float(sqrt(x * x + y * y) < 0.5) * float(sqrt(x * x + y * y) > wheel_radius);
	x += 0.002;
	float b4 = float(sqrt(x * x + y * y) < 0.5) * float(sqrt(x * x + y * y) > wheel_radius);

	COLOR = vec4(clamp((abs(fract(((a - TAU) / TAU) + vec3(3.0, 2.0, 1.0) / 3.0) * 6.0 - 3.0) - 1.0), 0.0, 1.0), (b + b2 + b3 + b4) / 4.00);
}
)");

	circle_shader.instantiate();
	circle_shader->set_code(R"(
// ColorPicker circle shader.

shader_type canvas_item;

uniform float v = 1.0;

void fragment() {
	float x = UV.x - 0.5;
	float y = UV.y - 0.5;
	float a = atan(y, x);
	x += 0.001;
	y += 0.001;
	float b = float(sqrt(x * x + y * y) < 0.5);
	x -= 0.002;
	float b2 = float(sqrt(x * x + y * y) < 0.5);
	y -= 0.002;
	float b3 = float(sqrt(x * x + y * y) < 0.5);
	x += 0.002;
	float b4 = float(sqrt(x * x + y * y) < 0.5);

	COLOR = vec4(mix(vec3(1.0), clamp(abs(fract(vec3((a - TAU) / TAU) + vec3(1.0, 2.0 / 3.0, 1.0 / 3.0)) * 6.0 - vec3(3.0)) - vec3(1.0), 0.0, 1.0), ((float(sqrt(x * x + y * y)) * 2.0)) / 1.0) * vec3(v), (b + b2 + b3 + b4) / 4.00);
})");

	circle_ok_color_shader.instantiate();
	circle_ok_color_shader->set_code(OK_COLOR_SHADER + R"(
// ColorPicker ok color hsv circle shader.

uniform float ok_hsl_l = 1.0;

void fragment() {
	float x = UV.x - 0.5;
	float y = UV.y - 0.5;
	float h = atan(y, x) / (2.0 * M_PI);
	float s = sqrt(x * x + y * y) * 2.0;
	vec3 col = okhsl_to_srgb(vec3(h, s, ok_hsl_l));
	x += 0.001;
	y += 0.001;
	float b = float(sqrt(x * x + y * y) < 0.5);
	x -= 0.002;
	float b2 = float(sqrt(x * x + y * y) < 0.5);
	y -= 0.002;
	float b3 = float(sqrt(x * x + y * y) < 0.5);
	x += 0.002;
	float b4 = float(sqrt(x * x + y * y) < 0.5);
	COLOR = vec4(col, (b + b2 + b3 + b4) / 4.00);
})");
}

void ColorPicker::finish_shaders() {
	wheel_shader.unref();
	circle_shader.unref();
	circle_ok_color_shader.unref();
}

void ColorPicker::set_focus_on_line_edit() {
	callable_mp((Control *)c_text, &Control::grab_focus).call_deferred();
}

void ColorPicker::_update_controls() {
	int mode_sliders_count = modes[current_mode]->get_slider_count();

	for (int i = current_slider_count; i < mode_sliders_count; i++) {
		sliders[i]->show();
		labels[i]->show();
		values[i]->show();
	}
	for (int i = mode_sliders_count; i < current_slider_count; i++) {
		sliders[i]->hide();
		labels[i]->hide();
		values[i]->hide();
	}
	current_slider_count = mode_sliders_count;

	for (int i = 0; i < current_slider_count; i++) {
		labels[i]->set_text(modes[current_mode]->get_slider_label(i));
	}

	alpha_value->set_visible(edit_alpha);
	alpha_slider->set_visible(edit_alpha);
	alpha_label->set_visible(edit_alpha);

	bool has_shape = current_shape != SHAPE_NONE;
	hb_edit->set_visible(has_shape);
	btn_shape->set_visible(has_shape);
	if (has_shape) {
		bool is_hsv_rect = current_shape == SHAPE_HSV_RECTANGLE;
		bool is_hsv_wheel = current_shape == SHAPE_HSV_WHEEL;
		wheel_edit->set_visible(!is_hsv_rect);
		w_edit->set_visible(!is_hsv_wheel);
		uv_edit->set_visible(is_hsv_rect);
		if (!is_hsv_rect) {
			wheel->set_material(is_hsv_wheel ? wheel_mat : circle_mat);
			if (!is_hsv_wheel) {
				circle_mat->set_shader(current_shape == SHAPE_VHS_CIRCLE ? circle_shader : circle_ok_color_shader);
			}
		}
	}
}

void ColorPicker::_set_pick_color(const Color &p_color, bool p_update_sliders) {
	if (text_changed) {
		text_changed = false;
	}

	color = p_color;
	if (color != last_color) {
		_copy_color_to_hsv();
		last_color = color;
	}

	if (!is_inside_tree()) {
		return;
	}

	_update_color(p_update_sliders);
	_update_selected_swatch();
}

void ColorPicker::set_pick_color(const Color &p_color) {
	_set_pick_color(p_color, true); //because setters can't have more arguments
}

void ColorPicker::set_old_color(const Color &p_color) {
	old_color = p_color;
}

void ColorPicker::set_display_old_color(bool p_enabled) {
	display_old_color = p_enabled;
}

bool ColorPicker::is_displaying_old_color() const {
	return display_old_color;
}

void ColorPicker::set_edit_alpha(bool p_show) {
	if (edit_alpha == p_show) {
		return;
	}
	edit_alpha = p_show;
	_update_controls();

	if (!is_inside_tree()) {
		return;
	}

	_update_color();
	sample->queue_redraw();
}

bool ColorPicker::is_editing_alpha() const {
	return edit_alpha;
}

void ColorPicker::_slider_drag_started() {
	currently_dragging = true;
}

void ColorPicker::_slider_value_changed() {
	if (updating) {
		return;
	}

	color = modes[current_mode]->get_color();
	modes[current_mode]->_value_changed();

	if (current_mode == MODE_HSV) {
		h = sliders[0]->get_value() / 360.0;
		s = sliders[1]->get_value() / 100.0;
		v = sliders[2]->get_value() / 100.0;
		ok_hsl_h = color.get_ok_hsl_h();
		ok_hsl_s = color.get_ok_hsl_s();
		ok_hsl_l = color.get_ok_hsl_l();
		last_color = color;
	} else if (current_mode == MODE_OKHSL) {
		ok_hsl_h = sliders[0]->get_value() / 360.0;
		ok_hsl_s = sliders[1]->get_value() / 100.0;
		ok_hsl_l = sliders[2]->get_value() / 100.0;
		h = color.get_h();
		s = color.get_s();
		v = color.get_v();
		last_color = color;
	}

	_set_pick_color(color, false);
	if (!deferred_mode_enabled || !currently_dragging) {
		emit_signal(SNAME("color_changed"), color);
	}
}

void ColorPicker::_slider_drag_ended() {
	currently_dragging = false;
	if (deferred_mode_enabled) {
		emit_signal(SNAME("color_changed"), color);
	}
}

void ColorPicker::add_mode(ColorMode *p_mode) {
	modes.push_back(p_mode);
}

HSlider *ColorPicker::get_slider(int p_idx) {
	if (p_idx < SLIDER_COUNT) {
		return sliders[p_idx];
	}
	return alpha_slider;
}

Vector<float> ColorPicker::get_active_slider_values() {
	Vector<float> cur_values;
	for (int i = 0; i < current_slider_count; i++) {
		cur_values.push_back(sliders[i]->get_value());
	}
	cur_values.push_back(alpha_slider->get_value());
	return cur_values;
}

void ColorPicker::_copy_color_to_hsv() {
	ok_hsl_h = color.get_ok_hsl_h();
	ok_hsl_s = color.get_ok_hsl_s();
	ok_hsl_l = color.get_ok_hsl_l();
	h = color.get_h();
	s = color.get_s();
	v = color.get_v();
}

void ColorPicker::_copy_hsv_to_color() {
	if (current_shape == SHAPE_OKHSL_CIRCLE) {
		color.set_ok_hsl(ok_hsl_h, ok_hsl_s, ok_hsl_l, color.a);
	} else {
		color.set_hsv(h, s, v, color.a);
	}
}

void ColorPicker::_html_submitted(const String &p_html) {
	if (updating || text_is_constructor || !c_text->is_visible()) {
		return;
	}

	Color new_color = Color::from_string(p_html.strip_edges(), color);
	String html_no_prefix = p_html.strip_edges().trim_prefix("#");
	if (html_no_prefix.is_valid_hex_number(false)) {
		// Convert invalid HTML color codes that software like Figma supports.
		if (html_no_prefix.length() == 1) {
			// Turn `#1` into `#111111`.
			html_no_prefix = html_no_prefix.repeat(6);
		} else if (html_no_prefix.length() == 2) {
			// Turn `#12` into `#121212`.
			html_no_prefix = html_no_prefix.repeat(3);
		} else if (html_no_prefix.length() == 5) {
			// Turn `#12345` into `#11223344`.
			html_no_prefix = html_no_prefix.left(4);
		} else if (html_no_prefix.length() == 7) {
			// Turn `#1234567` into `#123456`.
			html_no_prefix = html_no_prefix.left(6);
		}
	}
	new_color = Color::from_string(html_no_prefix, new_color);

	if (!is_editing_alpha()) {
		new_color.a = color.a;
	}

	if (new_color.to_argb32() == color.to_argb32()) {
		return;
	}
	color = new_color;

	if (!is_inside_tree()) {
		return;
	}

	set_pick_color(color);
	emit_signal(SNAME("color_changed"), color);
}

void ColorPicker::_update_color(bool p_update_sliders) {
	updating = true;

	if (p_update_sliders) {
		float step = modes[current_mode]->get_slider_step();
		float spinbox_arrow_step = modes[current_mode]->get_spinbox_arrow_step();
		for (int i = 0; i < current_slider_count; i++) {
			sliders[i]->set_max(modes[current_mode]->get_slider_max(i));
			sliders[i]->set_allow_greater(modes[current_mode]->can_allow_greater());
			sliders[i]->set_step(step);
			values[i]->set_custom_arrow_step(spinbox_arrow_step);
			sliders[i]->set_value(modes[current_mode]->get_slider_value(i));
		}
		alpha_slider->set_max(modes[current_mode]->get_slider_max(current_slider_count));
		alpha_slider->set_step(step);
		alpha_slider->set_value(modes[current_mode]->get_slider_value(current_slider_count));
	}

	_update_text_value();

	sample->queue_redraw();
	uv_edit->queue_redraw();
	w_edit->queue_redraw();
	for (int i = 0; i < current_slider_count; i++) {
		sliders[i]->queue_redraw();
	}
	alpha_slider->queue_redraw();
	wheel->queue_redraw();
	wheel_uv->queue_redraw();
	updating = false;
}

void ColorPicker::_text_type_toggled() {
	text_is_constructor = !text_is_constructor;
	if (text_is_constructor) {
		text_type->set_button_icon(theme_cache.hex_code_icon);

		c_text->set_editable(false);
		c_text->set_tooltip_text(RTR("Copy this constructor in a script."));
	} else {
		text_type->set_button_icon(theme_cache.hex_icon);

		c_text->set_editable(true);
		c_text->set_tooltip_text(ETR("Enter a hex code (\"#ff0000\") or named color (\"red\")."));
	}
	_update_color();
}

Color ColorPicker::get_pick_color() const {
	return color;
}

Color ColorPicker::get_old_color() const {
	return old_color;
}

void ColorPicker::set_picker_shape(PickerShapeType p_shape) {
	ERR_FAIL_INDEX(p_shape, SHAPE_MAX);
	if (p_shape == current_shape) {
		return;
	}
	if (current_shape != SHAPE_NONE) {
		shape_popup->set_item_checked(current_shape, false);
	}
	if (p_shape != SHAPE_NONE) {
		shape_popup->set_item_checked(p_shape, true);
		btn_shape->set_button_icon(shape_popup->get_item_icon(p_shape));
	}

	current_shape = p_shape;

	_copy_color_to_hsv();

	_update_controls();
	_update_color();
}

ColorPicker::PickerShapeType ColorPicker::get_picker_shape() const {
	return current_shape;
}

void ColorPicker::set_wheel_radius(float p_wheel_radius) {
	if (wheel_radius == p_wheel_radius) {
		return;
	}

	wheel_radius = p_wheel_radius;
	float _radius = CLAMP((1.0 - wheel_radius) * 0.5, 0.25, 0.45);
	wheel_mat->set_shader_parameter("wheel_radius", _radius);

	if (current_shape == SHAPE_HSV_WHEEL) {
		_update_color();
	}
}

float ColorPicker::get_wheel_radius() const {
	return wheel_radius;
}

void ColorPicker::set_color_mode(ColorModeType p_mode) {
	ERR_FAIL_INDEX(p_mode, MODE_MAX);

	if (current_mode == p_mode) {
		return;
	}

	if (p_mode < MODE_BUTTON_COUNT) {
		mode_btns[p_mode]->set_pressed(true);
	} else if (current_mode < MODE_BUTTON_COUNT) {
		mode_btns[current_mode]->set_pressed(false);
	}

	current_mode = p_mode;

	if (!is_inside_tree()) {
		return;
	}

	_update_controls();
	_update_color();
}

ColorPicker::ColorModeType ColorPicker::get_color_mode() const {
	return current_mode;
}

void ColorPicker::set_colorize_sliders(bool p_colorize_sliders) {
	if (colorize_sliders == p_colorize_sliders) {
		return;
	}

	colorize_sliders = p_colorize_sliders;

	if (colorize_sliders) {
		Ref<StyleBoxEmpty> style_box_empty(memnew(StyleBoxEmpty));
		for (int i = 0; i < SLIDER_COUNT; i++) {
			sliders[i]->add_theme_style_override("slider", style_box_empty);
		}
		alpha_slider->add_theme_style_override("slider", style_box_empty);
	} else {
		for (int i = 0; i < SLIDER_COUNT; i++) {
			sliders[i]->remove_theme_style_override("slider");
		}
		alpha_slider->remove_theme_style_override("slider");
	}
}

bool ColorPicker::is_colorizing_sliders() const {
	return colorize_sliders;
}

void ColorPicker::set_deferred_mode(bool p_enabled) {
	deferred_mode_enabled = p_enabled;
}

bool ColorPicker::is_deferred_mode() const {
	return deferred_mode_enabled;
}

void ColorPicker::_update_text_value() {
	bool text_visible = true;
	if (text_is_constructor) {
		String t = "Color(" + String::num(color.r, 3) + ", " + String::num(color.g, 3) + ", " + String::num(color.b, 3);
		if (edit_alpha && color.a < 1) {
			t += ", " + String::num(color.a, 3) + ")";
		} else {
			t += ")";
		}
		c_text->set_text(t);
	}

	if (color.r > 1 || color.g > 1 || color.b > 1 || color.r < 0 || color.g < 0 || color.b < 0) {
		text_visible = false;
	} else if (!text_is_constructor) {
		c_text->set_text(color.to_html(edit_alpha && color.a < 1));
	}

	text_type->set_visible(text_visible);
	c_text->set_visible(text_visible);
}

void ColorPicker::_sample_input(const Ref<InputEvent> &p_event) {
	if (!display_old_color) {
		return;
	}
	const Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid() && mb->is_pressed() && mb->get_button_index() == MouseButton::LEFT) {
		const Rect2 rect_old = Rect2(Point2(), Size2(sample->get_size().width * 0.5, sample->get_size().height * 0.95));
		if (rect_old.has_point(mb->get_position())) {
			// Revert to the old color when left-clicking the old color sample.
			set_pick_color(old_color);
			emit_signal(SNAME("color_changed"), color);
		}
	}
}

void ColorPicker::_sample_draw() {
	// Covers the right half of the sample if the old color is being displayed,
	// or the whole sample if it's not being displayed.
	Rect2 rect_new;

	if (display_old_color) {
		rect_new = Rect2(Point2(sample->get_size().width * 0.5, 0), Size2(sample->get_size().width * 0.5, sample->get_size().height * 0.95));

		// Draw both old and new colors for easier comparison (only if spawned from a ColorPickerButton).
		const Rect2 rect_old = Rect2(Point2(), Size2(sample->get_size().width * 0.5, sample->get_size().height * 0.95));

		if (old_color.a < 1.0) {
			sample->draw_texture_rect(theme_cache.sample_bg, rect_old, true);
		}

		sample->draw_rect(rect_old, old_color);

		if (!old_color.is_equal_approx(color)) {
			// Draw a revert indicator to indicate that the old sample can be clicked to revert to this old color.
			// Adapt icon color to the background color (taking alpha checkerboard into account) so that it's always visible.
			sample->draw_texture(theme_cache.sample_revert,
					rect_old.size * 0.5 - theme_cache.sample_revert->get_size() * 0.5,
					Math::lerp(0.75f, old_color.get_luminance(), old_color.a) < 0.455 ? Color(1, 1, 1) : (Color(0.01, 0.01, 0.01)));
		}

		if (old_color.r > 1 || old_color.g > 1 || old_color.b > 1) {
			// Draw an indicator to denote that the old color is "overbright" and can't be displayed accurately in the preview.
			sample->draw_texture(theme_cache.overbright_indicator, Point2());
		}
	} else {
		rect_new = Rect2(Point2(), Size2(sample->get_size().width, sample->get_size().height * 0.95));
	}

	if (color.a < 1.0) {
		sample->draw_texture_rect(theme_cache.sample_bg, rect_new, true);
	}

	sample->draw_rect(rect_new, color);

	if (color.r > 1 || color.g > 1 || color.b > 1) {
		// Draw an indicator to denote that the new color is "overbright" and can't be displayed accurately in the preview.
		sample->draw_texture(theme_cache.overbright_indicator, Point2(display_old_color ? sample->get_size().width * 0.5 : 0, 0));
	}
}

void ColorPicker::_hsv_draw(int p_which, Control *c) {
	if (!c) {
		return;
	}

	if (p_which == 0) {
		Color col = color;
		Vector2 center = c->get_size() / 2.0;

		if (current_shape == SHAPE_HSV_RECTANGLE || current_shape == SHAPE_HSV_WHEEL) {
			Vector<Point2> points;
			Vector<Color> colors;
			Vector<Color> colors2;
			if (current_shape == SHAPE_HSV_RECTANGLE) {
				points.append(Vector2());
				points.append(Vector2(c->get_size().x, 0));
				points.append(c->get_size());
				points.append(Vector2(0, c->get_size().y));
			} else {
				float _radius = CLAMP((1.0 - wheel_radius) * 0.5, 0.25, 0.45);
				real_t ring_radius_x = Math_SQRT12 * c->get_size().width * _radius;
				real_t ring_radius_y = Math_SQRT12 * c->get_size().height * _radius;

				points.append(center - Vector2(ring_radius_x, ring_radius_y));
				points.append(center + Vector2(ring_radius_x, -ring_radius_y));
				points.append(center + Vector2(ring_radius_x, ring_radius_y));
				points.append(center + Vector2(-ring_radius_x, ring_radius_y));
			}
			colors.append(Color(1, 1, 1, 1));
			colors.append(Color(1, 1, 1, 1));
			colors.append(Color(0, 0, 0, 1));
			colors.append(Color(0, 0, 0, 1));
			c->draw_polygon(points, colors);

			col.set_hsv(h, 1, 1);
			col.a = 0;
			colors2.append(col);
			col.a = 1;
			colors2.append(col);
			col.set_hsv(h, 1, 0);
			colors2.append(col);
			col.a = 0;
			colors2.append(col);
			c->draw_polygon(points, colors2);
		}

		int x;
		int y;
		float _radius = CLAMP((1.0 - wheel_radius) * 0.5, 0.25, 0.45);
		if (current_shape == SHAPE_VHS_CIRCLE || current_shape == SHAPE_OKHSL_CIRCLE) {
			Vector2 hue_offset;
			if (current_shape == SHAPE_OKHSL_CIRCLE) {
				hue_offset = center * Vector2(Math::cos(ok_hsl_h * Math_TAU), Math::sin(ok_hsl_h * Math_TAU)) * ok_hsl_s;
			} else {
				hue_offset = center * Vector2(Math::cos(h * Math_TAU), Math::sin(h * Math_TAU)) * s;
			}
			x = center.x + hue_offset.x - (theme_cache.picker_cursor->get_width() / 2);
			y = center.y + hue_offset.y - (theme_cache.picker_cursor->get_height() / 2);
		} else {
			real_t corner_x = (c == wheel_uv) ? center.x - Math_SQRT12 * c->get_size().width * _radius : 0;
			real_t corner_y = (c == wheel_uv) ? center.y - Math_SQRT12 * c->get_size().height * _radius : 0;

			Size2 real_size(c->get_size().x - corner_x * 2, c->get_size().y - corner_y * 2);
			x = CLAMP(real_size.x * s, 0, real_size.x) + corner_x - (theme_cache.picker_cursor->get_width() / 2);
			y = CLAMP(real_size.y - real_size.y * v, 0, real_size.y) + corner_y - (theme_cache.picker_cursor->get_height() / 2);
		}
		Color _col = color;
		_col.a = 1.0;
		c->draw_texture(theme_cache.picker_cursor_bg, Point2(x, y), _col);

		if (current_shape == SHAPE_HSV_WHEEL) {
			float radius = 1.0 - (CLAMP(wheel_radius, 0.1, 0.5) * 0.5);
			Point2 pos = center - (theme_cache.picker_cursor->get_size() * 0.5) + Point2(center.x * Math::cos(h * Math_TAU) * radius, center.y * Math::sin(h * Math_TAU) * radius);
			_col.set_hsv(_col.get_h(), 1.0, 1.0);
			c->draw_texture(theme_cache.picker_cursor_bg, pos, _col);
			c->draw_texture(theme_cache.picker_cursor, pos);
		}
		c->draw_texture(theme_cache.picker_cursor, Point2(x, y));

	} else if (p_which == 1) {
		if (current_shape == SHAPE_HSV_RECTANGLE) {
			c->draw_set_transform(Point2(), -Math_PI / 2, Size2(c->get_size().x, -c->get_size().y));
			c->draw_texture_rect(theme_cache.color_hue, Rect2(Point2(), Size2(1, 1)));
			c->draw_set_transform(Point2(), 0, Size2(1, 1));
			int y = c->get_size().y - c->get_size().y * (1.0 - h);
			Color col;
			col.set_hsv(h, 1, 1);
			c->draw_line(Point2(0, y), Point2(c->get_size().x, y), col.inverted());
		} else if (current_shape == SHAPE_OKHSL_CIRCLE) {
			Vector<Point2> points;
			Vector<Color> colors;
			Color col;
			col.set_ok_hsl(ok_hsl_h, ok_hsl_s, 1);
			Color col2;
			col2.set_ok_hsl(ok_hsl_h, ok_hsl_s, 0.5);
			Color col3;
			col3.set_ok_hsl(ok_hsl_h, ok_hsl_s, 0);
			points.resize(6);
			colors.resize(6);
			points.set(0, Vector2(c->get_size().x, 0));
			points.set(1, Vector2(c->get_size().x, c->get_size().y * 0.5));
			points.set(2, c->get_size());
			points.set(3, Vector2(0, c->get_size().y));
			points.set(4, Vector2(0, c->get_size().y * 0.5));
			points.set(5, Vector2());
			colors.set(0, col);
			colors.set(1, col2);
			colors.set(2, col3);
			colors.set(3, col3);
			colors.set(4, col2);
			colors.set(5, col);
			c->draw_polygon(points, colors);
			int y = c->get_size().y - c->get_size().y * CLAMP(ok_hsl_l, 0, 1);
			col.set_ok_hsl(ok_hsl_h, 1, ok_hsl_l);
			c->draw_line(Point2(0, y), Point2(c->get_size().x, y), col.inverted());
		} else if (current_shape == SHAPE_VHS_CIRCLE) {
			Vector<Point2> points;
			Vector<Color> colors;
			Color col;
			col.set_hsv(h, s, 1);
			points.resize(4);
			colors.resize(4);
			points.set(0, Vector2());
			points.set(1, Vector2(c->get_size().x, 0));
			points.set(2, c->get_size());
			points.set(3, Vector2(0, c->get_size().y));
			colors.set(0, col);
			colors.set(1, col);
			colors.set(2, Color(0, 0, 0));
			colors.set(3, Color(0, 0, 0));
			c->draw_polygon(points, colors);
			int y = c->get_size().y - c->get_size().y * CLAMP(v, 0, 1);
			col.set_hsv(h, 1, v);
			c->draw_line(Point2(0, y), Point2(c->get_size().x, y), col.inverted());
		}
	} else if (p_which == 2) {
		c->draw_rect(Rect2(Point2(), c->get_size()), Color(1, 1, 1));
		if (current_shape == SHAPE_VHS_CIRCLE) {
			circle_mat->set_shader_parameter("v", v);
		} else if (current_shape == SHAPE_OKHSL_CIRCLE) {
			circle_mat->set_shader_parameter("ok_hsl_l", ok_hsl_l);
		}
	}
}

void ColorPicker::_slider_draw(int p_which) {
	if (colorize_sliders) {
		modes[current_mode]->slider_draw(p_which);
	}
}

void ColorPicker::_uv_input(const Ref<InputEvent> &p_event, Control *c) {
	Ref<InputEventMouseButton> bev = p_event;

	if (bev.is_valid()) {
		if (bev->is_pressed() && bev->get_button_index() == MouseButton::LEFT) {
			Vector2 center = c->get_size() / 2.0;
			if (current_shape == SHAPE_VHS_CIRCLE || current_shape == SHAPE_OKHSL_CIRCLE) {
				real_t dist = center.distance_to(bev->get_position());
				if (dist <= center.x) {
					real_t rad = center.angle_to_point(bev->get_position());
					h = ((rad >= 0) ? rad : (Math_TAU + rad)) / Math_TAU;
					s = CLAMP(dist / center.x, 0, 1);
					ok_hsl_h = h;
					ok_hsl_s = s;
				} else {
					return;
				}
			} else {
				float _radius = CLAMP((1.0 - wheel_radius) * 0.5, 0.25, 0.45);
				real_t corner_x = (c == wheel_uv) ? center.x - Math_SQRT12 * c->get_size().width * _radius : 0;
				real_t corner_y = (c == wheel_uv) ? center.y - Math_SQRT12 * c->get_size().height * _radius : 0;
				Size2 real_size(c->get_size().x - corner_x * 2, c->get_size().y - corner_y * 2);

				if (bev->get_position().x < corner_x || bev->get_position().x > c->get_size().x - corner_x ||
						bev->get_position().y < corner_y || bev->get_position().y > c->get_size().y - corner_y) {
					{
						real_t dist = center.distance_to(bev->get_position());

						if (dist >= center.x * (_radius * 2) && dist <= center.x) {
							real_t rad = center.angle_to_point(bev->get_position());
							h = ((rad >= 0) ? rad : (Math_TAU + rad)) / Math_TAU;
							spinning = true;
						} else {
							return;
						}
					}
				}

				if (!spinning) {
					real_t x = CLAMP(bev->get_position().x - corner_x, 0, real_size.x);
					real_t y = CLAMP(bev->get_position().y - corner_y, 0, real_size.y);

					s = x / real_size.x;
					v = 1.0 - y / real_size.y;
				}
			}

			changing_color = true;

			_copy_hsv_to_color();
			last_color = color;
			set_pick_color(color);

			if (!deferred_mode_enabled) {
				emit_signal(SNAME("color_changed"), color);
			}
		} else if (!bev->is_pressed() && bev->get_button_index() == MouseButton::LEFT) {
			if (deferred_mode_enabled) {
				emit_signal(SNAME("color_changed"), color);
			}
			// add_recent_preset(color);
			changing_color = false;
			spinning = false;
		} else {
			changing_color = false;
			spinning = false;
		}
	}

	Ref<InputEventMouseMotion> mev = p_event;

	if (mev.is_valid()) {
		if (!changing_color) {
			return;
		}

		Vector2 center = c->get_size() / 2.0;
		if (current_shape == SHAPE_VHS_CIRCLE || current_shape == SHAPE_OKHSL_CIRCLE) {
			real_t dist = center.distance_to(mev->get_position());
			real_t rad = center.angle_to_point(mev->get_position());
			h = ((rad >= 0) ? rad : (Math_TAU + rad)) / Math_TAU;
			s = CLAMP(dist / center.x, 0, 1);
			ok_hsl_h = h;
			ok_hsl_s = s;
		} else {
			if (spinning) {
				real_t rad = center.angle_to_point(mev->get_position());
				h = ((rad >= 0) ? rad : (Math_TAU + rad)) / Math_TAU;
			} else {
				float _radius = CLAMP((1.0 - wheel_radius) * 0.5, 0.25, 0.45);
				real_t corner_x = (c == wheel_uv) ? center.x - Math_SQRT12 * c->get_size().width * _radius : 0;
				real_t corner_y = (c == wheel_uv) ? center.y - Math_SQRT12 * c->get_size().height * _radius : 0;
				Size2 real_size(c->get_size().x - corner_x * 2, c->get_size().y - corner_y * 2);

				real_t x = CLAMP(mev->get_position().x - corner_x, 0, real_size.x);
				real_t y = CLAMP(mev->get_position().y - corner_y, 0, real_size.y);

				s = x / real_size.x;
				v = 1.0 - y / real_size.y;
			}
		}

		_copy_hsv_to_color();
		last_color = color;
		set_pick_color(color);

		if (!deferred_mode_enabled) {
			emit_signal(SNAME("color_changed"), color);
		}
	}
}

void ColorPicker::_w_input(const Ref<InputEvent> &p_event) {
	Ref<InputEventMouseButton> bev = p_event;

	if (bev.is_valid()) {
		if (bev->is_pressed() && bev->get_button_index() == MouseButton::LEFT) {
			changing_color = true;
			float y = CLAMP((float)bev->get_position().y, 0, w_edit->get_size().height);
			if (current_shape == SHAPE_VHS_CIRCLE || current_shape == SHAPE_OKHSL_CIRCLE) {
				v = 1.0 - (y / w_edit->get_size().height);
				ok_hsl_l = v;
			} else {
				h = y / w_edit->get_size().height;
			}
		} else {
			changing_color = false;
		}

		_copy_hsv_to_color();
		last_color = color;
		set_pick_color(color);

		if (!bev->is_pressed() && bev->get_button_index() == MouseButton::LEFT) {
			// add_recent_preset(color);
			emit_signal(SNAME("color_changed"), color);
		} else if (!deferred_mode_enabled) {
			emit_signal(SNAME("color_changed"), color);
		}
	}

	Ref<InputEventMouseMotion> mev = p_event;

	if (mev.is_valid()) {
		if (!changing_color) {
			return;
		}
		float y = CLAMP((float)mev->get_position().y, 0, w_edit->get_size().height);
		if (current_shape == SHAPE_VHS_CIRCLE || current_shape == SHAPE_OKHSL_CIRCLE) {
			v = 1.0 - (y / w_edit->get_size().height);
			ok_hsl_l = v;
		} else {
			h = y / w_edit->get_size().height;
		}

		_copy_hsv_to_color();
		last_color = color;
		set_pick_color(color);

		if (!deferred_mode_enabled) {
			emit_signal(SNAME("color_changed"), color);
		}
	}
}

void ColorPicker::_slider_or_spin_input(const Ref<InputEvent> &p_event) {
	if (line_edit_mouse_release) {
		line_edit_mouse_release = false;
		return;
	}
}

void ColorPicker::_line_edit_input(const Ref<InputEvent> &p_event) {
	Ref<InputEventMouseButton> bev = p_event;
	if (bev.is_valid() && !bev->is_pressed() && bev->get_button_index() == MouseButton::LEFT) {
		line_edit_mouse_release = true;
	}
}

void ColorPicker::_text_changed(const String &) {
	text_changed = true;
}
void ColorPicker::_pick_button_pressed() {
	is_picking_color = true;

	if (!picker_window) {
		picker_window = memnew(Popup);
		bool has_feature_exclude_from_capture = DisplayServer::get_singleton()->has_feature(DisplayServer::FEATURE_SCREEN_EXCLUDE_FROM_CAPTURE);
		if (!has_feature_exclude_from_capture) {
			picker_window->set_size(Vector2i(28, 28));
		} else {
			picker_window->set_size(Vector2i(55, 72));
			picker_window->set_flag(Window::FLAG_EXCLUDE_FROM_CAPTURE, true); // Only supported on MacOS and Windows.
		}
		picker_window->connect(SceneStringName(visibility_changed), callable_mp(this, &ColorPicker::_pick_finished));
		picker_window->connect(SceneStringName(window_input), callable_mp(this, &ColorPicker::_target_gui_input));

		picker_preview = memnew(Panel);
		picker_preview->set_mouse_filter(MOUSE_FILTER_IGNORE);
		picker_preview->set_size(Vector2i(55, 72));
		picker_window->add_child(picker_preview);

		picker_preview_color = memnew(Panel);
		picker_preview_color->set_mouse_filter(MOUSE_FILTER_IGNORE);
		if (!has_feature_exclude_from_capture) {
			picker_preview_color->set_size(Vector2i(24, 24));
			picker_preview_color->set_position(Vector2i(2, 2));
		} else {
			picker_preview_color->set_size(Vector2i(51, 15));
			picker_preview_color->set_position(Vector2i(2, 55));
		}
		picker_preview->add_child(picker_preview_color);

		if (has_feature_exclude_from_capture) {
			picker_texture_zoom = memnew(TextureRect);
			picker_texture_zoom->set_mouse_filter(MOUSE_FILTER_IGNORE);
			picker_texture_zoom->set_custom_minimum_size(Vector2i(51, 51));
			picker_texture_zoom->set_position(Vector2i(2, 2));
			picker_texture_zoom->set_texture_filter(CanvasItem::TEXTURE_FILTER_NEAREST);
			picker_preview->add_child(picker_texture_zoom);
		}

		picker_preview_style_box.instantiate();
		picker_preview->add_theme_style_override(SceneStringName(panel), picker_preview_style_box);

		picker_preview_style_box_color.instantiate();
		picker_preview_color->add_theme_style_override(SceneStringName(panel), picker_preview_style_box_color);

		add_child(picker_window, false, INTERNAL_MODE_FRONT);
	}
	set_process_internal(true);

	picker_window->popup();
}

void ColorPicker::_target_gui_input(const Ref<InputEvent> &p_event) {
	const Ref<InputEventMouseButton> mouse_event = p_event;
	if (mouse_event.is_valid() && mouse_event->is_pressed()) {
		picker_window->hide();
		_pick_finished();
	}
}

void ColorPicker::_pick_finished() {
	if (picker_window->is_visible()) {
		return;
	}

	if (Input::get_singleton()->is_action_just_pressed(SNAME("ui_cancel"))) {
		set_pick_color(old_color);
	} else {
		emit_signal(SNAME("color_changed"), color);
	}
	is_picking_color = false;
	set_process_internal(false);
	picker_window->hide();
}

void ColorPicker::_pick_button_pressed_legacy() {
	if (!is_inside_tree()) {
		return;
	}

	if (!picker_window) {
		picker_window = memnew(Popup);
		picker_window->hide();
		picker_window->set_transient(true);
		add_child(picker_window, false, INTERNAL_MODE_FRONT);

		picker_texture_rect = memnew(TextureRect);
		picker_texture_rect->set_anchors_preset(Control::PRESET_FULL_RECT);
		picker_texture_rect->set_expand_mode(TextureRect::EXPAND_IGNORE_SIZE);
		picker_texture_rect->set_default_cursor_shape(Control::CURSOR_CROSS);
		picker_window->add_child(picker_texture_rect);
		picker_texture_rect->connect(SceneStringName(gui_input), callable_mp(this, &ColorPicker::_picker_texture_input));

		bool has_feature_exclude_from_capture = DisplayServer::get_singleton()->has_feature(DisplayServer::FEATURE_SCREEN_EXCLUDE_FROM_CAPTURE);
		picker_preview = memnew(Panel);
		picker_preview->set_mouse_filter(MOUSE_FILTER_IGNORE);
		if (!has_feature_exclude_from_capture) {
			picker_preview->set_size(Vector2i(28, 28));
		} else {
			picker_preview->set_size(Vector2i(55, 72));
		}
		picker_window->add_child(picker_preview);

		picker_preview_color = memnew(Panel);
		picker_preview_color->set_mouse_filter(MOUSE_FILTER_IGNORE);
		if (!has_feature_exclude_from_capture) {
			picker_preview_color->set_size(Vector2i(24, 24));
			picker_preview_color->set_position(Vector2i(2, 2));
		} else {
			picker_preview_color->set_size(Vector2i(51, 15));
			picker_preview_color->set_position(Vector2i(2, 55));
		}
		picker_preview->add_child(picker_preview_color);

		if (has_feature_exclude_from_capture) {
			picker_texture_zoom = memnew(TextureRect);
			picker_texture_zoom->set_mouse_filter(MOUSE_FILTER_IGNORE);
			picker_texture_zoom->set_custom_minimum_size(Vector2i(51, 51));
			picker_texture_zoom->set_position(Vector2i(2, 2));
			picker_texture_zoom->set_texture_filter(CanvasItem::TEXTURE_FILTER_NEAREST);
			picker_preview->add_child(picker_texture_zoom);
		}

		picker_preview_style_box.instantiate();
		picker_preview->add_theme_style_override(SceneStringName(panel), picker_preview_style_box);

		picker_preview_style_box_color.instantiate();
		picker_preview_color->add_theme_style_override(SceneStringName(panel), picker_preview_style_box_color);
	}

	Rect2i screen_rect;
	if (picker_window->is_embedded()) {
		Ref<ImageTexture> tx = ImageTexture::create_from_image(picker_window->get_embedder()->get_texture()->get_image());
		screen_rect = picker_window->get_embedder()->get_visible_rect();
		picker_window->set_position(Point2i());
		picker_texture_rect->set_texture(tx);

		Ref<AtlasTexture> atlas;
		atlas.instantiate();
		atlas->set_atlas(tx);
		picker_texture_zoom->set_texture(atlas);
	} else {
		screen_rect = picker_window->get_parent_rect();
		picker_window->set_position(screen_rect.position);

		Ref<Image> target_image = Image::create_empty(screen_rect.size.x, screen_rect.size.y, false, Image::FORMAT_RGB8);
		DisplayServer *ds = DisplayServer::get_singleton();

		// Add the Texture of each Window to the Image.
		Vector<DisplayServer::WindowID> wl = ds->get_window_list();
		// FIXME: sort windows by visibility.
		for (const DisplayServer::WindowID &window_id : wl) {
			Window *w = Window::get_from_id(window_id);
			if (!w) {
				continue;
			}

			Ref<Image> img = w->get_texture()->get_image();
			if (img.is_null() || img->is_empty()) {
				continue;
			}
			img->convert(Image::FORMAT_RGB8);
			target_image->blit_rect(img, Rect2i(Point2i(0, 0), img->get_size()), w->get_position());
		}

		picker_texture_rect->set_texture(ImageTexture::create_from_image(target_image));
	}

	picker_window->set_size(screen_rect.size);
	picker_window->popup();
}

void ColorPicker::_picker_texture_input(const Ref<InputEvent> &p_event) {
	if (!is_inside_tree()) {
		return;
	}

	Ref<InputEventMouseButton> bev = p_event;
	if (bev.is_valid() && bev->get_button_index() == MouseButton::LEFT && !bev->is_pressed()) {
		set_pick_color(picker_color);
		emit_signal(SNAME("color_changed"), color);
		picker_window->hide();
	}

	Ref<InputEventMouseMotion> mev = p_event;
	if (mev.is_valid()) {
		Ref<Image> img = picker_texture_rect->get_texture()->get_image();
		if (img.is_valid() && !img->is_empty()) {
			Vector2 ofs = mev->get_position();
			picker_preview->set_position(ofs - Vector2(28, 28));
			Vector2 scale = picker_texture_rect->get_size() / img->get_size();
			ofs /= scale;
			picker_color = img->get_pixel(ofs.x, ofs.y);
			picker_preview_style_box_color->set_bg_color(picker_color);
			picker_preview_style_box->set_bg_color(picker_color.get_luminance() < 0.5 ? Color(1.0f, 1.0f, 1.0f) : Color(0.0f, 0.0f, 0.0f));

			Ref<AtlasTexture> atlas = picker_texture_zoom->get_texture();
			if (atlas.is_valid()) {
				atlas->set_region(Rect2i(ofs.x - 8, ofs.y - 8, 17, 17));
			}
		}
	}
}

void ColorPicker::_html_focus_exit() {
	if (c_text->is_menu_visible()) {
		return;
	}

	if (is_visible_in_tree()) {
		_html_submitted(c_text->get_text());
	} else {
		_update_text_value();
	}
}

void ColorPicker::_on_swatch_button_right_pressed(int p_index) {
	remove_swatch_color_by_index(p_index);
	emit_signal(SNAME("swatches_changed"));
}

void ColorPicker::_on_swatch_button_pressed(const Color &p_color) {
	set_pick_color(p_color);
	emit_signal(SNAME("color_changed"), p_color);
}

void ColorPicker::_swatches_folding_changed(bool p_folded) {
	if (can_add_swatches) {
		swatches_foldable->set_button_hidden(0, p_folded);
	}
}

void ColorPicker::_add_swatch_button_pressed() {
	bool emit_changed = !swatches.has(color);
	add_swatch_color(color);
	if (emit_changed) {
		emit_signal(SNAME("swatches_changed"));
	}
}

void ColorPicker::_update_selected_swatch() {
	SwatchButton *cur_button = Object::cast_to<SwatchButton>(swatches_group->get_pressed_button());
	if (cur_button) {
		if (cur_button->get_color() == color) {
			return;
		}
		cur_button->set_pressed_no_signal(false);
	}
	for (int i = 0; i < swatches_hbc->get_child_count(); i++) {
		SwatchButton *button = Object::cast_to<SwatchButton>(swatches_hbc->get_child(i));
		if (button->get_color() == color) {
			button->set_pressed_no_signal(true);
			return;
		}
	}
}

void ColorPicker::set_modes_visible(bool p_visible) {
	if (color_modes_visible == p_visible) {
		return;
	}
	color_modes_visible = p_visible;
	mode_hbc->set_visible(p_visible);
}

bool ColorPicker::are_modes_visible() const {
	return color_modes_visible;
}

void ColorPicker::set_sampler_visible(bool p_visible) {
	if (sampler_visible == p_visible) {
		return;
	}
	sampler_visible = p_visible;
	sample_hbc->set_visible(p_visible);
}

bool ColorPicker::is_sampler_visible() const {
	return sampler_visible;
}

void ColorPicker::set_can_pick_screen_color(bool p_enabled) {
	if (can_pick_screen_color != p_enabled) {
		can_pick_screen_color = p_enabled;
		btn_pick->set_visible(can_pick_screen_color);
	}
}

bool ColorPicker::get_can_pick_screen_color() const {
	return can_pick_screen_color;
}

void ColorPicker::set_sliders_visible(bool p_visible) {
	if (sliders_visible == p_visible) {
		return;
	}
	sliders_visible = p_visible;
	sliders_vbc->set_visible(p_visible);
}

bool ColorPicker::are_sliders_visible() const {
	return sliders_visible;
}

void ColorPicker::set_hex_visible(bool p_visible) {
	if (hex_visible == p_visible) {
		return;
	}
	hex_visible = p_visible;
	hex_hbc->set_visible(p_visible);
}

bool ColorPicker::is_hex_visible() const {
	return hex_visible;
}

void ColorPicker::set_swatches(const PackedColorArray &p_swatches) {
	swatches.clear();
	for (Color col : p_swatches) {
		if (!swatches.has(col)) {
			swatches.push_back(col);
		}
	}
	update_swatches();
}

PackedColorArray ColorPicker::get_swatches() const {
	return swatches;
}

void ColorPicker::set_swatches_visible(bool p_visible) {
	if (swatches_visible != p_visible) {
		swatches_visible = p_visible;
		swatches_foldable->set_visible(swatches_visible);
	}
}

bool ColorPicker::is_swatches_visible() const {
	return swatches_visible;
}

void ColorPicker::set_can_add_swatches(bool p_enabled) {
	if (can_add_swatches != p_enabled) {
		can_add_swatches = p_enabled;
		if (swatches_foldable->is_expanded()) {
			swatches_foldable->set_button_hidden(0, !can_add_swatches);
		}
	}
}

bool ColorPicker::get_can_add_swatches() const {
	return can_add_swatches;
}

void ColorPicker::add_swatch_color(const Color &p_color) {
	if (!swatches.has(p_color)) {
		swatches.insert(0, p_color);
		SwatchButton *button = memnew(SwatchButton);
		button->set_custom_minimum_size(Size2(theme_cache.swatch_size, theme_cache.swatch_size));
		button->set_color(p_color);
		button->set_button_group(swatches_group);
		SwatchButton *cur_button = Object::cast_to<SwatchButton>(swatches_group->get_pressed_button());
		if (cur_button) {
			cur_button->set_pressed_no_signal(false);
		}
		button->set_pressed_no_signal(true);
		swatches_hbc->add_child(button);
		swatches_hbc->move_child(button, 0);
		button->connect(SNAME("color_selected"), callable_mp(this, &ColorPicker::_on_swatch_button_pressed));
		button->connect(SNAME("right_click_pressed"), callable_mp(this, &ColorPicker::_on_swatch_button_right_pressed));
	} else {
		int idx = swatches.find(p_color);
		if (swatches.size() == 1 || idx == 0) {
			return;
		}
		swatches.remove_at(idx);
		swatches.insert(0, p_color);
		SwatchButton *button = Object::cast_to<SwatchButton>(swatches_hbc->get_child(idx));
		swatches_hbc->move_child(button, 0);
	}
}

void ColorPicker::remove_swatch_color(const Color &p_color) {
	if (swatches.has(p_color)) {
		remove_swatch_color_by_index(swatches.find(p_color));
	}
}

void ColorPicker::remove_swatch_color_by_index(int p_index) {
	ERR_FAIL_INDEX(p_index, swatches.size());
	swatches.remove_at(p_index);
	SwatchButton *button = Object::cast_to<SwatchButton>(swatches_hbc->get_child(p_index));
	button->disconnect(SNAME("color_selected"), callable_mp(this, &ColorPicker::_on_swatch_button_pressed));
	button->disconnect(SNAME("right_click_pressed"), callable_mp(this, &ColorPicker::_on_swatch_button_right_pressed));
	swatches_hbc->remove_child(button);
	button->queue_free();
}

void ColorPicker::clear_swatches() {
	if (swatches.size() > 0) {
		for (int i = swatches_hbc->get_child_count() - 1; i >= 0; i--) {
			remove_swatch_color_by_index(i);
		}
	}
}

void ColorPicker::update_swatches() {
	for (int i = swatches_hbc->get_child_count() - 1; i >= 0; i--) {
		SwatchButton *button = Object::cast_to<SwatchButton>(swatches_hbc->get_child(i));
		button->disconnect(SNAME("color_selected"), callable_mp(this, &ColorPicker::_on_swatch_button_pressed));
		button->disconnect(SNAME("right_click_pressed"), callable_mp(this, &ColorPicker::_on_swatch_button_right_pressed));
		swatches_hbc->remove_child(button);
		button->queue_free();
	}
	for (Color col : swatches) {
		SwatchButton *button = memnew(SwatchButton);
		button->set_custom_minimum_size(Size2(theme_cache.swatch_size, theme_cache.swatch_size));
		button->set_color(col);
		button->set_button_group(swatches_group);
		swatches_hbc->add_child(button);
		if (col == color) {
			button->set_pressed_no_signal(true);
		}
		button->connect("color_selected", callable_mp(this, &ColorPicker::_on_swatch_button_pressed));
		button->connect("right_click_pressed", callable_mp(this, &ColorPicker::_on_swatch_button_right_pressed));
	}
}

void ColorPicker::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_pick_color", "color"), &ColorPicker::set_pick_color);
	ClassDB::bind_method(D_METHOD("get_pick_color"), &ColorPicker::get_pick_color);
	ClassDB::bind_method(D_METHOD("set_old_color", "color"), &ColorPicker::set_old_color);
	ClassDB::bind_method(D_METHOD("set_display_old_color", "display"), &ColorPicker::set_display_old_color);
	ClassDB::bind_method(D_METHOD("is_displaying_old_color"), &ColorPicker::is_displaying_old_color);
	ClassDB::bind_method(D_METHOD("set_deferred_mode", "mode"), &ColorPicker::set_deferred_mode);
	ClassDB::bind_method(D_METHOD("is_deferred_mode"), &ColorPicker::is_deferred_mode);
	ClassDB::bind_method(D_METHOD("set_color_mode", "color_mode"), &ColorPicker::set_color_mode);
	ClassDB::bind_method(D_METHOD("get_color_mode"), &ColorPicker::get_color_mode);
	ClassDB::bind_method(D_METHOD("set_edit_alpha", "show"), &ColorPicker::set_edit_alpha);
	ClassDB::bind_method(D_METHOD("is_editing_alpha"), &ColorPicker::is_editing_alpha);
	ClassDB::bind_method(D_METHOD("set_modes_visible", "visible"), &ColorPicker::set_modes_visible);
	ClassDB::bind_method(D_METHOD("are_modes_visible"), &ColorPicker::are_modes_visible);
	ClassDB::bind_method(D_METHOD("set_sampler_visible", "visible"), &ColorPicker::set_sampler_visible);
	ClassDB::bind_method(D_METHOD("is_sampler_visible"), &ColorPicker::is_sampler_visible);
	ClassDB::bind_method(D_METHOD("set_can_pick_screen_color", "enabled"), &ColorPicker::set_can_pick_screen_color);
	ClassDB::bind_method(D_METHOD("get_can_pick_screen_color"), &ColorPicker::get_can_pick_screen_color);
	ClassDB::bind_method(D_METHOD("set_sliders_visible", "visible"), &ColorPicker::set_sliders_visible);
	ClassDB::bind_method(D_METHOD("are_sliders_visible"), &ColorPicker::are_sliders_visible);
	ClassDB::bind_method(D_METHOD("set_hex_visible", "visible"), &ColorPicker::set_hex_visible);
	ClassDB::bind_method(D_METHOD("is_hex_visible"), &ColorPicker::is_hex_visible);
	ClassDB::bind_method(D_METHOD("set_picker_shape", "shape"), &ColorPicker::set_picker_shape);
	ClassDB::bind_method(D_METHOD("get_picker_shape"), &ColorPicker::get_picker_shape);
	ClassDB::bind_method(D_METHOD("set_wheel_radius", "wheel_radius"), &ColorPicker::set_wheel_radius);
	ClassDB::bind_method(D_METHOD("get_wheel_radius"), &ColorPicker::get_wheel_radius);
	ClassDB::bind_method(D_METHOD("set_colorize_sliders", "colorize_sliders"), &ColorPicker::set_colorize_sliders);
	ClassDB::bind_method(D_METHOD("is_colorizing_sliders"), &ColorPicker::is_colorizing_sliders);
	ClassDB::bind_method(D_METHOD("set_swatches", "swatches"), &ColorPicker::set_swatches);
	ClassDB::bind_method(D_METHOD("get_swatches"), &ColorPicker::get_swatches);
	ClassDB::bind_method(D_METHOD("set_swatches_visible", "visible"), &ColorPicker::set_swatches_visible);
	ClassDB::bind_method(D_METHOD("is_swatches_visible"), &ColorPicker::is_swatches_visible);
	ClassDB::bind_method(D_METHOD("add_swatch_color", "color"), &ColorPicker::add_swatch_color);
	ClassDB::bind_method(D_METHOD("remove_swatch_color", "color"), &ColorPicker::remove_swatch_color);
	ClassDB::bind_method(D_METHOD("remove_swatch_color_by_index", "index"), &ColorPicker::remove_swatch_color_by_index);
	ClassDB::bind_method(D_METHOD("set_can_add_swatches", "enabled"), &ColorPicker::set_can_add_swatches);
	ClassDB::bind_method(D_METHOD("get_can_add_swatches"), &ColorPicker::get_can_add_swatches);
	ClassDB::bind_method(D_METHOD("clear_swatches"), &ColorPicker::clear_swatches);
	ClassDB::bind_method(D_METHOD("update_swatches"), &ColorPicker::update_swatches);

	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "color"), "set_pick_color", "get_pick_color");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "edit_alpha"), "set_edit_alpha", "is_editing_alpha");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "color_mode", PROPERTY_HINT_ENUM, "RGB,HSV,RAW,OKHSL"), "set_color_mode", "get_color_mode");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "deferred_mode"), "set_deferred_mode", "is_deferred_mode");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "wheel_radius"), "set_wheel_radius", "get_wheel_radius");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "picker_shape", PROPERTY_HINT_ENUM, "HSV Rectangle,HSV Rectangle Wheel,VHS Circle,OKHSL Circle,None"), "set_picker_shape", "get_picker_shape");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "colorize_sliders"), "set_colorize_sliders", "is_colorizing_sliders");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "can_add_swatches"), "set_can_add_swatches", "get_can_add_swatches");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "can_pick_screen_color"), "set_can_pick_screen_color", "get_can_pick_screen_color");
	ADD_GROUP("Customization", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "sampler_visible"), "set_sampler_visible", "is_sampler_visible");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "color_modes_visible"), "set_modes_visible", "are_modes_visible");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "sliders_visible"), "set_sliders_visible", "are_sliders_visible");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "hex_visible"), "set_hex_visible", "is_hex_visible");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "swatches_visible"), "set_swatches_visible", "is_swatches_visible");

	ADD_SIGNAL(MethodInfo("color_changed", PropertyInfo(Variant::COLOR, "color")));
	ADD_SIGNAL(MethodInfo("swatches_changed"));

	BIND_ENUM_CONSTANT(MODE_RGB);
	BIND_ENUM_CONSTANT(MODE_HSV);
	BIND_ENUM_CONSTANT(MODE_RAW);
	BIND_ENUM_CONSTANT(MODE_OKHSL);

	BIND_ENUM_CONSTANT(SHAPE_HSV_RECTANGLE);
	BIND_ENUM_CONSTANT(SHAPE_HSV_WHEEL);
	BIND_ENUM_CONSTANT(SHAPE_VHS_CIRCLE);
	BIND_ENUM_CONSTANT(SHAPE_OKHSL_CIRCLE);
	BIND_ENUM_CONSTANT(SHAPE_NONE);

	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, ColorPicker, sv_width);
	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, ColorPicker, sv_height);
	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, ColorPicker, h_width);
	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, ColorPicker, sample_height);
	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, ColorPicker, expand_shape);
	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, ColorPicker, swatch_size);

	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ColorPicker, screen_picker);
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ColorPicker, expanded_arrow);
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ColorPicker, folded_arrow);
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ColorPicker, add_preset);

	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ColorPicker, shape_rect);
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ColorPicker, shape_rect_wheel);
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ColorPicker, shape_circle);

	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ColorPicker, sample_bg);
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ColorPicker, sample_revert);
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ColorPicker, overbright_indicator);
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ColorPicker, picker_cursor);
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ColorPicker, picker_cursor_bg);
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ColorPicker, color_hue);
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ColorPicker, color_okhsl_hue);
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ColorPicker, hex_icon);
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, ColorPicker, hex_code_icon);
}

ColorPicker::ColorPicker() {
	set_theme_type_variation("ColorPickerVBox");

	hb_edit = memnew(HBoxContainer);
	hb_edit->set_theme_type_variation("ColorPickerHBox");
	add_child(hb_edit, false, INTERNAL_MODE_FRONT);

	uv_edit = memnew(Control);
	hb_edit->add_child(uv_edit);
	uv_edit->connect(SceneStringName(gui_input), callable_mp(this, &ColorPicker::_uv_input).bind(uv_edit));
	uv_edit->set_mouse_filter(MOUSE_FILTER_PASS);
	uv_edit->set_h_size_flags(SIZE_EXPAND_FILL);
	uv_edit->set_v_size_flags(SIZE_EXPAND_FILL);
	uv_edit->connect(SceneStringName(draw), callable_mp(this, &ColorPicker::_hsv_draw).bind(0, uv_edit));

	sample_hbc = memnew(HBoxContainer);
	sample_hbc->set_theme_type_variation("ColorPickerHBox");
	add_child(sample_hbc, false, INTERNAL_MODE_FRONT);

	btn_pick = memnew(Button);
	btn_pick->set_theme_type_variation("PickerButton");
	btn_pick->set_icon_alignment(HORIZONTAL_ALIGNMENT_CENTER);
	btn_pick->set_size_mode(BaseButton::SIZE_MODE_FIT_HEIGHT);
	sample_hbc->add_child(btn_pick);

	sample = memnew(TextureRect);
	sample_hbc->add_child(sample);
	sample->set_h_size_flags(SIZE_EXPAND_FILL);
	sample->connect(SceneStringName(gui_input), callable_mp(this, &ColorPicker::_sample_input));
	sample->connect(SceneStringName(draw), callable_mp(this, &ColorPicker::_sample_draw));

	btn_shape = memnew(MenuButton);
	btn_shape->set_theme_type_variation("PickerMenuButton");
	btn_shape->set_icon_alignment(HORIZONTAL_ALIGNMENT_CENTER);
	btn_shape->set_size_mode(BaseButton::SIZE_MODE_FIT_HEIGHT);
	btn_shape->set_flat(false);
	sample_hbc->add_child(btn_shape);
	btn_shape->set_toggle_mode(true);
	btn_shape->set_tooltip_text(ETR("Select a picker shape."));

	current_shape = SHAPE_HSV_RECTANGLE;

	shape_popup = btn_shape->get_popup();
	shape_popup->add_radio_check_item("HSV Rectangle", SHAPE_HSV_RECTANGLE);
	shape_popup->add_radio_check_item("HSV Wheel", SHAPE_HSV_WHEEL);
	shape_popup->add_radio_check_item("VHS Circle", SHAPE_VHS_CIRCLE);
	shape_popup->add_radio_check_item("OKHSL Circle", SHAPE_OKHSL_CIRCLE);
	shape_popup->set_item_checked(current_shape, true);
	shape_popup->connect(SceneStringName(id_pressed), callable_mp(this, &ColorPicker::set_picker_shape));

	add_mode(new ColorModeRGB(this));
	add_mode(new ColorModeHSV(this));
	add_mode(new ColorModeRAW(this));
	add_mode(new ColorModeOKHSL(this));

	sliders_vbc = memnew(VBoxContainer);
	sliders_vbc->add_theme_constant_override("separation", 0);
	add_child(sliders_vbc, false, INTERNAL_MODE_FRONT);
	mode_hbc = memnew(HBoxContainer);
	mode_hbc->add_theme_constant_override("separation", 0);
	sliders_vbc->add_child(mode_hbc);

	Ref<ButtonGroup> mode_group(memnew(ButtonGroup));

	for (int i = 0; i < MODE_BUTTON_COUNT; i++) {
		mode_btns[i] = memnew(Button);
		mode_btns[i]->set_theme_type_variation("PickerModeButton");
		mode_hbc->add_child(mode_btns[i]);
		mode_btns[i]->set_focus_mode(FOCUS_NONE);
		mode_btns[i]->set_h_size_flags(SIZE_EXPAND_FILL);
		mode_btns[i]->set_toggle_mode(true);
		mode_btns[i]->set_text(modes[i]->get_name());
		mode_btns[i]->set_button_group(mode_group);
		mode_btns[i]->connect(SceneStringName(pressed), callable_mp(this, &ColorPicker::set_color_mode).bind((ColorModeType)i));
	}
	mode_btns[0]->set_pressed(true);

	current_mode = MODE_RGB;

	PanelContainer *sliders_panel = memnew(PanelContainer);
	sliders_panel->set_theme_type_variation("PickerSlidersPanel");
	sliders_panel->set_h_size_flags(SIZE_EXPAND_FILL);
	sliders_vbc->add_child(sliders_panel);

	slider_gc = memnew(GridContainer);
	slider_gc->set_theme_type_variation("ColorPickerGrid");
	slider_gc->set_columns(3);
	sliders_panel->add_child(slider_gc);

	Ref<StyleBoxEmpty> style_box_empty;
	for (int i = 0; i < SLIDER_COUNT + 1; i++) {
		Label *lbl = memnew(Label);
		lbl->set_v_size_flags(SIZE_SHRINK_CENTER);
		lbl->set_auto_translate_mode(AUTO_TRANSLATE_MODE_DISABLED);
		lbl->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
		slider_gc->add_child(lbl);

		HSlider *slider = memnew(HSlider);
		slider->set_theme_type_variation("PickerHSlider");
		slider->set_v_size_flags(SIZE_SHRINK_CENTER);
		slider->set_focus_mode(FOCUS_NONE);
		slider_gc->add_child(slider);

		if (colorize_sliders) {
			if (style_box_empty.is_null()) {
				style_box_empty.instantiate();
			}
			slider->add_theme_style_override("slider", style_box_empty);
		}

		SpinBox *val = memnew(SpinBox);
		slider->share(val);
		val->set_select_all_on_focus(true);
		slider_gc->add_child(val);

		LineEdit *vle = val->get_line_edit();
		vle->set_theme_type_variation("PickerLineEdit");
		vle->connect(SceneStringName(text_changed), callable_mp(this, &ColorPicker::_text_changed));
		vle->connect(SceneStringName(gui_input), callable_mp(this, &ColorPicker::_line_edit_input));
		vle->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_RIGHT);

		val->connect(SceneStringName(gui_input), callable_mp(this, &ColorPicker::_slider_or_spin_input));

		slider->set_h_size_flags(SIZE_EXPAND_FILL);

		slider->connect("drag_started", callable_mp(this, &ColorPicker::_slider_drag_started));
		slider->connect(SceneStringName(value_changed), callable_mp(this, &ColorPicker::_slider_value_changed).unbind(1));
		slider->connect("drag_ended", callable_mp(this, &ColorPicker::_slider_drag_ended).unbind(1));
		slider->connect(SceneStringName(draw), callable_mp(this, &ColorPicker::_slider_draw).bind(i));
		slider->connect(SceneStringName(gui_input), callable_mp(this, &ColorPicker::_slider_or_spin_input));

		if (i < SLIDER_COUNT) {
			sliders[i] = slider;
			values[i] = val;
			labels[i] = lbl;
		} else {
			alpha_slider = slider;
			alpha_value = val;
			alpha_label = lbl;
			alpha_label->set_text("A");
		}
	}

	hex_hbc = memnew(HBoxContainer);
	hex_hbc->set_theme_type_variation("ColorPickerHBox");
	hex_hbc->set_alignment(ALIGNMENT_BEGIN);
	add_child(hex_hbc, false, INTERNAL_MODE_FRONT);

	text_type = memnew(Button);
	text_type->set_theme_type_variation("PickerButton");
	text_type->set_icon_alignment(HORIZONTAL_ALIGNMENT_CENTER);
	text_type->set_size_mode(BaseButton::SIZE_MODE_FIT_HEIGHT);
	hex_hbc->add_child(text_type);
	text_type->set_button_icon(theme_cache.hex_icon);
	text_type->set_tooltip_text(RTR("Switch between hexadecimal and code values."));
	text_type->connect(SceneStringName(pressed), callable_mp(this, &ColorPicker::_text_type_toggled));

	c_text = memnew(LineEdit);
	c_text->set_theme_type_variation("PickerLineEdit");
	hex_hbc->add_child(c_text);
	c_text->set_h_size_flags(SIZE_EXPAND_FILL);
	c_text->set_select_all_on_focus(true);
	c_text->set_tooltip_text(ETR("Enter a hex code (\"#ff0000\") or named color (\"red\")."));
	c_text->set_placeholder(ETR("Hex code or named color"));
	c_text->connect(SceneStringName(text_submitted), callable_mp(this, &ColorPicker::_html_submitted));
	c_text->connect(SceneStringName(text_changed), callable_mp(this, &ColorPicker::_text_changed));
	c_text->connect(SceneStringName(focus_exited), callable_mp(this, &ColorPicker::_html_focus_exit));

	wheel_edit = memnew(AspectRatioContainer);
	wheel_edit->set_h_size_flags(SIZE_EXPAND_FILL);
	hb_edit->add_child(wheel_edit);

	wheel_mat.instantiate();
	wheel_mat->set_shader(wheel_shader);
	wheel_radius = 0.15;
	float _radius = CLAMP((1.0 - wheel_radius) * 0.5, 0.25, 0.45);
	wheel_mat->set_shader_parameter("wheel_radius", _radius);

	circle_mat.instantiate();
	circle_mat->set_shader(circle_shader);

	wheel_margin = memnew(MarginContainer);
	wheel_margin->set_margin_all(0);
	wheel_edit->add_child(wheel_margin);

	wheel = memnew(Control);
	wheel_margin->add_child(wheel);
	wheel->set_mouse_filter(MOUSE_FILTER_PASS);
	wheel->connect(SceneStringName(draw), callable_mp(this, &ColorPicker::_hsv_draw).bind(2, wheel));

	wheel_uv = memnew(Control);
	wheel_margin->add_child(wheel_uv);
	wheel_uv->connect(SceneStringName(gui_input), callable_mp(this, &ColorPicker::_uv_input).bind(wheel_uv));
	wheel_uv->connect(SceneStringName(draw), callable_mp(this, &ColorPicker::_hsv_draw).bind(0, wheel_uv));

	w_edit = memnew(Control);
	w_edit->set_draw_behind_parent(true);
	hb_edit->add_child(w_edit);
	w_edit->set_h_size_flags(SIZE_FILL);
	w_edit->set_v_size_flags(SIZE_EXPAND_FILL);
	w_edit->connect(SceneStringName(gui_input), callable_mp(this, &ColorPicker::_w_input));
	w_edit->connect(SceneStringName(draw), callable_mp(this, &ColorPicker::_hsv_draw).bind(1, w_edit));

	swatches_foldable = memnew(FoldableContainer(ETR("Swatches")));
	swatches_foldable->set_focus_mode(FOCUS_NONE);
	swatches_foldable->set_theme_type_variation("SwatchesFoldableContainer");
	swatches_foldable->set_expanded(false);
	swatches_foldable->add_button(theme_cache.add_preset);
	swatches_foldable->set_button_hidden(0, true);
	swatches_foldable->set_button_tooltip(0, ETR("Add current color as a preset."));
	swatches_foldable->connect("folding_changed", callable_mp(this, &ColorPicker::_swatches_folding_changed));
	swatches_foldable->connect("button_pressed", callable_mp(this, &ColorPicker::_add_swatch_button_pressed).unbind(2));

	ScrollContainer *swatches_scroll = memnew(ScrollContainer);
	swatches_scroll->set_ignore_scroll_bar_min_size(false);
	swatches_scroll->add_theme_constant_override("v_separation", 2);
	swatches_scroll->set_vertical_scroll_mode(ScrollContainer::SCROLL_MODE_DISABLED);
	swatches_foldable->add_child(swatches_scroll);

	swatches_hbc = memnew(HBoxContainer);
	swatches_hbc->set_theme_type_variation("ColorPickerHBox");
	swatches_hbc->set_h_size_flags(SIZE_EXPAND_FILL);
	swatches_hbc->set_v_size_flags(SIZE_EXPAND_FILL);
	swatches_scroll->add_child(swatches_hbc);

	swatches_group.instantiate();
	add_child(swatches_foldable, false, INTERNAL_MODE_FRONT);
	callable_mp(this, &ColorPicker::update_swatches).call_deferred();

	_update_controls();
	updating = false;

	set_pick_color(Color(1, 1, 1));
}

ColorPicker::~ColorPicker() {
	for (int i = 0; i < modes.size(); i++) {
		delete modes[i];
	}
}

/////////////////

void ColorPickerPopupPanel::_input_from_window(const Ref<InputEvent> &p_event) {
	if (p_event->is_action_pressed(SNAME("ui_accept"), false, true)) {
		_close_pressed();
	}
	PopupPanel::_input_from_window(p_event);
}

/////////////////

void ColorPickerButton::_about_to_popup() {
	set_pressed(true);
	if (picker) {
		picker->set_old_color(get_color());
	}
}

void ColorPickerButton::_color_changed(const Color &p_color) {
	set_color(p_color);
}

void ColorPickerButton::_modal_closed() {
	if (Input::get_singleton()->is_action_just_pressed(SNAME("ui_cancel"))) {
		set_pick_color(picker->get_old_color());
		emit_signal(SNAME("color_changed"), get_color());
	}
	emit_signal(SNAME("popup_closed"));
	set_pressed(false);
}

void ColorPickerButton::pressed() {
	_update_picker();

	Size2 minsize = popup->get_contents_minimum_size();
	float viewport_height = get_viewport_rect().size.y;

	popup->reset_size();

	// Determine in which direction to show the popup. By default popup horizontally centered below the button.
	// But if the popup doesn't fit below and the button is in the bottom half of the viewport, show above.
	bool show_above = false;
	if (get_global_position().y + get_size().y + minsize.y > viewport_height && get_global_position().y * 2 + get_size().y > viewport_height) {
		show_above = true;
	}

	float h_offset = (get_size().x - minsize.x) / 2;
	float v_offset = show_above ? -minsize.y : get_size().y;
	popup->set_position(get_screen_position() + Vector2(h_offset, v_offset));
	popup->popup();
	if (DisplayServer::get_singleton()->has_hardware_keyboard()) {
		picker->set_focus_on_line_edit();
	}
}

void ColorPickerButton::set_pick_color(const Color &p_color) {
	set_color_no_signal(p_color);
	if (picker) {
		picker->set_pick_color(p_color);
	}
}

Color ColorPickerButton::get_pick_color() const {
	return get_color();
}

void ColorPickerButton::set_edit_alpha(bool p_show) {
	if (edit_alpha == p_show) {
		return;
	}
	edit_alpha = p_show;
	if (picker) {
		picker->set_edit_alpha(p_show);
	}
}

bool ColorPickerButton::is_editing_alpha() const {
	return edit_alpha;
}

ColorPicker *ColorPickerButton::get_picker() {
	_update_picker();
	return picker;
}

PopupPanel *ColorPickerButton::get_popup() {
	_update_picker();
	return popup;
}

void ColorPickerButton::_update_picker() {
	if (!picker) {
		popup = memnew(ColorPickerPopupPanel);
		popup->add_theme_style_override("panel", theme_cache.popup_panel_style);
		popup->set_wrap_controls(true);
		picker = memnew(ColorPicker);
		picker->set_anchors_and_offsets_preset(PRESET_FULL_RECT);
		popup->add_child(picker);
		add_child(popup, false, INTERNAL_MODE_FRONT);
		picker->connect("color_changed", callable_mp(this, &ColorPickerButton::_color_changed));
		popup->connect("about_to_popup", callable_mp(this, &ColorPickerButton::_about_to_popup));
		popup->connect("popup_hide", callable_mp(this, &ColorPickerButton::_modal_closed));
		picker->connect(SceneStringName(minimum_size_changed), callable_mp((Window *)popup, &Window::reset_size));
		picker->set_pick_color(get_color());
		picker->set_edit_alpha(edit_alpha);
		picker->set_display_old_color(true);
		emit_signal(SNAME("picker_created"));
	}
}

void ColorPickerButton::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_WM_CLOSE_REQUEST: {
			if (popup) {
				popup->hide();
			}
		} break;

		case NOTIFICATION_VISIBILITY_CHANGED: {
			if (popup && !is_visible_in_tree()) {
				popup->hide();
			}
		} break;

		case NOTIFICATION_THEME_CHANGED: {
			if (popup) {
				popup->add_theme_style_override("panel", theme_cache.popup_panel_style);
			}
		} break;
	}
}

void ColorPickerButton::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_pick_color", "color"), &ColorPickerButton::set_pick_color);
	ClassDB::bind_method(D_METHOD("get_pick_color"), &ColorPickerButton::get_pick_color);
	ClassDB::bind_method(D_METHOD("get_picker"), &ColorPickerButton::get_picker);
	ClassDB::bind_method(D_METHOD("get_popup"), &ColorPickerButton::get_popup);
	ClassDB::bind_method(D_METHOD("set_edit_alpha", "show"), &ColorPickerButton::set_edit_alpha);
	ClassDB::bind_method(D_METHOD("is_editing_alpha"), &ColorPickerButton::is_editing_alpha);
	ClassDB::bind_method(D_METHOD("_about_to_popup"), &ColorPickerButton::_about_to_popup);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "edit_alpha"), "set_edit_alpha", "is_editing_alpha");

	ADD_SIGNAL(MethodInfo("popup_closed"));
	ADD_SIGNAL(MethodInfo("picker_created"));

	BIND_THEME_ITEM_CUSTOM(Theme::DATA_TYPE_STYLEBOX, ColorPickerButton, popup_panel_style, "popup_panel");
}

ColorPickerButton::ColorPickerButton() {
	set_toggle_mode(true);
}

void SwatchButton::gui_input(const Ref<InputEvent> &p_event) {
	Ref<InputEventMouseButton> b = p_event;
	if (b.is_valid() && b->get_button_index() == MouseButton::RIGHT && b->is_pressed()) {
		emit_signal(SNAME("right_click_pressed"), get_index());
	}
	BaseButton::gui_input(p_event);
}

void SwatchButton::toggled(bool p_toggled_on) {
	if (p_toggled_on) {
		emit_signal(SNAME("color_selected"), get_color());
	}
}

void SwatchButton::_bind_methods() {
	ADD_SIGNAL(MethodInfo("color_selected", PropertyInfo(Variant::COLOR, "color")));
	ADD_SIGNAL(MethodInfo("right_click_pressed", PropertyInfo(Variant::INT, "index")));
}

SwatchButton::SwatchButton() {
	set_toggle_mode(true);
}
