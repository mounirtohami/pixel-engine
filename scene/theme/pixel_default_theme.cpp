/**************************************************************************/
/*  pixel_default_theme.cpp                                               */
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

#include "pixel_default_theme.h"

#include "core/os/os.h"
#include "default_font.gen.h"
#include "default_theme_icons.gen.h"
#include "scene/resources/font.h"
#include "scene/resources/gradient_texture.h"
#include "scene/resources/image_texture.h"
#include "scene/resources/style_box_flat.h"
#include "scene/resources/style_box_line.h"
#include "scene/resources/theme.h"
#include "scene/scene_string_names.h"
#include "scene/theme/theme_db.h"
#include "servers/text_server.h"

#include "modules/modules_enabled.gen.h" // For svg.
#ifdef MODULE_SVG_ENABLED
#include "modules/svg/image_loader_svg.h"
#endif

static Dictionary icons;

static Ref<StyleBoxFlat> panel_style;
static Ref<StyleBoxFlat> popup_panel_style;
static Ref<StyleBoxFlat> color_picker_popup_panel_style;
static Ref<StyleBoxFlat> tab_panel_style;
static Ref<StyleBoxFlat> color_sliders_panel_style;
static Ref<StyleBoxFlat> tab_selected_style;
static Ref<StyleBoxFlat> tab_unselected_style;
static Ref<StyleBoxFlat> tab_hovered_style;
static Ref<StyleBoxFlat> tab_disabled_style;
static Ref<StyleBoxFlat> tab_focus_style;
static Ref<StyleBoxFlat> button_normal_style;
static Ref<StyleBoxFlat> button_hover_style;
static Ref<StyleBoxFlat> button_pressed_style;
static Ref<StyleBoxFlat> button_disabled_style;
static Ref<StyleBoxFlat> button_focus_style;
static Ref<StyleBoxFlat> color_button_normal_style;
static Ref<StyleBoxFlat> color_button_hover_style;
static Ref<StyleBoxFlat> color_button_pressed_style;
static Ref<StyleBoxFlat> color_button_disabled_style;
static Ref<StyleBoxFlat> color_button_focus_style;
static Ref<StyleBoxFlat> menu_button_normal_style;
static Ref<StyleBoxFlat> menu_button_hover_style;
static Ref<StyleBoxFlat> menu_button_pressed_style;
static Ref<StyleBoxFlat> menu_button_disabled_style;
static Ref<StyleBoxFlat> menu_button_focus_style;
static Ref<StyleBoxFlat> popup_hover_style;
static Ref<StyleBoxFlat> progress_background_style;
static Ref<StyleBoxFlat> progress_fill_style;
static Ref<StyleBoxFlat> grabber_style;
static Ref<StyleBoxFlat> grabber_highlight_style;
static Ref<StyleBoxFlat> slider_style;
static Ref<StyleBoxFlat> picker_slider_style;
static Ref<StyleBoxFlat> h_scroll_style;
static Ref<StyleBoxFlat> v_scroll_style;
static Ref<StyleBoxFlat> foldable_panel_style;
static Ref<StyleBoxFlat> foldable_title_style;
static Ref<StyleBoxFlat> foldable_title_collapsed_style;
static Ref<StyleBoxFlat> foldable_title_hover_style;
static Ref<StyleBoxFlat> foldable_title_collapsed_hover_style;
static Ref<StyleBoxFlat> swatches_foldable_panel_style;
static Ref<StyleBoxFlat> swatches_foldable_title_style;
static Ref<StyleBoxFlat> swatches_foldable_title_collapsed_style;
static Ref<StyleBoxFlat> swatches_foldable_title_hover_style;
static Ref<StyleBoxFlat> swatches_foldable_title_collapsed_hover_style;
static Ref<StyleBoxLine> h_separator_style;
static Ref<StyleBoxLine> v_separator_style;
static Ref<StyleBoxEmpty> flat_button_normal;
static Ref<StyleBoxFlat> embedded_style;
static Ref<StyleBoxFlat> embedded_unfocused_style;
static Ref<StyleBoxFlat> graph_title_style;
static Ref<StyleBoxFlat> graph_frame_title_style;
static Ref<StyleBoxFlat> graph_frame_title_selected_style;
static Ref<StyleBoxFlat> graph_title_selected_style;
static Ref<StyleBoxFlat> graph_panel_style;
static Ref<StyleBoxFlat> graph_panel_selected_style;
static Ref<StyleBoxFlat> color_mode_button_normal_style;
static Ref<StyleBoxFlat> color_mode_button_hovered_style;
static Ref<StyleBoxFlat> color_mode_button_pressed_style;
static Ref<StyleBoxFlat> color_picker_button_normal_style;
static Ref<StyleBoxFlat> color_picker_button_hovered_style;
static Ref<StyleBoxFlat> color_picker_button_pressed_style;
static Ref<StyleBoxFlat> color_picker_button_disabled_style;

static Ref<FontVariation> fallback_font;
static Ref<FontVariation> custom_font;
static Ref<FontVariation> bold_font;
static Ref<FontVariation> bold_italics_font;
static Ref<FontVariation> italics_font;
static bool using_custom_font_variation = false;

// See also `editor_generate_icon()` in `editor/themes/editor_icons.cpp`.
static Ref<Image> generate_icon(int p_index, float p_scale, Color p_font_color, Color p_accent_color) {
	Ref<Image> img = memnew(Image);

#ifdef MODULE_SVG_ENABLED
	String svg = default_theme_icons_sources[p_index];
	svg = svg.replace("\"#fff\"", vformat("\"#%s\"", p_font_color.to_html(false)));
	svg = svg.replace("\"#0f0\"", vformat("\"#%s\"", p_accent_color.to_html(false)));

	Error err = img->load_svg_from_string(svg, p_scale);
	if (err == OK) {
		img->fix_alpha_edges();
	}
	ERR_FAIL_COND_V_MSG(err != OK, Ref<ImageTexture>(), "Failed generating icon, unsupported or invalid SVG data in default theme.");
#else
	// If the SVG module is disabled, we can't really display the UI well, but at least we won't crash.
	// 16 pixels is used as it's the most common base size for Godot icons.
	img = Image::create_empty(Math::round(16 * p_scale), Math::round(16 * p_scale), false, Image::FORMAT_RGBA8);
#endif

	return img;
}

void update_theme_icons(Ref<Theme> &p_theme, Color p_font_color, Color p_accent_color) {
	if (icons.is_empty()) {
		Ref<Texture2D> empty_icon = memnew(ImageTexture);
		p_theme->set_icon("increment", "HScrollBar", empty_icon);
		p_theme->set_icon("increment_highlight", "HScrollBar", empty_icon);
		p_theme->set_icon("increment_pressed", "HScrollBar", empty_icon);
		p_theme->set_icon("decrement", "HScrollBar", empty_icon);
		p_theme->set_icon("decrement_highlight", "HScrollBar", empty_icon);
		p_theme->set_icon("decrement_pressed", "HScrollBar", empty_icon);
		p_theme->set_icon("increment", "VScrollBar", empty_icon);
		p_theme->set_icon("increment_highlight", "VScrollBar", empty_icon);
		p_theme->set_icon("increment_pressed", "VScrollBar", empty_icon);
		p_theme->set_icon("decrement", "VScrollBar", empty_icon);
		p_theme->set_icon("decrement_highlight", "VScrollBar", empty_icon);
		p_theme->set_icon("decrement_pressed", "VScrollBar", empty_icon);
		p_theme->set_icon("updown", "SpinBox", empty_icon);
		{
			const int precision = 7;

			Ref<Gradient> hue_gradient;
			hue_gradient.instantiate();
			PackedFloat32Array offsets;
			offsets.resize(precision);
			PackedColorArray colors;
			colors.resize(precision);

			for (int i = 0; i < precision; i++) {
				float h = i / float(precision - 1);
				offsets.write[i] = h;
				colors.write[i] = Color::from_hsv(h, 1, 1);
			}
			hue_gradient->set_offsets(offsets);
			hue_gradient->set_colors(colors);

			Ref<GradientTexture2D> hue_texture;
			hue_texture.instantiate();
			hue_texture->set_width(512);
			hue_texture->set_height(1);
			hue_texture->set_gradient(hue_gradient);

			p_theme->set_icon("color_hue", "ColorPicker", hue_texture);
		}

		{
			const int precision = 7;

			Ref<Gradient> hue_gradient;
			hue_gradient.instantiate();
			PackedFloat32Array offsets;
			offsets.resize(precision);
			PackedColorArray colors;
			colors.resize(precision);

			for (int i = 0; i < precision; i++) {
				float h = i / float(precision - 1);
				offsets.write[i] = h;
				colors.write[i] = Color::from_ok_hsl(h, 1, 0.5);
			}
			hue_gradient->set_offsets(offsets);
			hue_gradient->set_colors(colors);

			Ref<GradientTexture2D> hue_texture;
			hue_texture.instantiate();
			hue_texture->set_width(512);
			hue_texture->set_height(1);
			hue_texture->set_gradient(hue_gradient);

			p_theme->set_icon("color_okhsl_hue", "ColorPicker", hue_texture);
		}
	}

	const float scale = p_theme->get_default_base_scale();

	for (int i = 0; i < default_theme_icons_count; i++) {
		Ref<Image> img = generate_icon(i, scale, p_font_color, p_accent_color);
		if (icons.has(default_theme_icons_names[i]) && ((Ref<ImageTexture>)icons[default_theme_icons_names[i]])->get_size() == (Vector2)img->get_size()) {
			((Ref<ImageTexture>)icons[default_theme_icons_names[i]])->update(img);
		} else {
			icons[default_theme_icons_names[i]] = ImageTexture::create_from_image(img);
		}
	}

	p_theme->set_icon("arrow", "OptionButton", icons["option_button_arrow"]);
	p_theme->set_icon("checked", "CheckBox", icons["checked"]);
	p_theme->set_icon("checked_disabled", "CheckBox", icons["checked_disabled"]);
	p_theme->set_icon("unchecked", "CheckBox", icons["unchecked"]);
	p_theme->set_icon("unchecked_disabled", "CheckBox", icons["unchecked_disabled"]);
	p_theme->set_icon("radio_checked", "CheckBox", icons["radio_checked"]);
	p_theme->set_icon("radio_checked_disabled", "CheckBox", icons["radio_checked_disabled"]);
	p_theme->set_icon("radio_unchecked", "CheckBox", icons["radio_unchecked"]);
	p_theme->set_icon("radio_unchecked_disabled", "CheckBox", icons["radio_unchecked_disabled"]);
	p_theme->set_icon("checked", "CheckButton", icons["toggle_on"]);
	p_theme->set_icon("checked_disabled", "CheckButton", icons["toggle_on_disabled"]);
	p_theme->set_icon("unchecked", "CheckButton", icons["toggle_off"]);
	p_theme->set_icon("unchecked_disabled", "CheckButton", icons["toggle_off_disabled"]);
	p_theme->set_icon("checked_mirrored", "CheckButton", icons["toggle_on_mirrored"]);
	p_theme->set_icon("checked_disabled_mirrored", "CheckButton", icons["toggle_on_disabled_mirrored"]);
	p_theme->set_icon("unchecked_mirrored", "CheckButton", icons["toggle_off_mirrored"]);
	p_theme->set_icon("unchecked_disabled_mirrored", "CheckButton", icons["toggle_off_disabled_mirrored"]);
	p_theme->set_icon("clear", "LineEdit", icons["line_edit_clear"]);
	p_theme->set_icon("tab", "TextEdit", icons["text_edit_tab"]);
	p_theme->set_icon("space", "TextEdit", icons["text_edit_space"]);
	p_theme->set_icon("tab", "CodeEdit", icons["text_edit_tab"]);
	p_theme->set_icon("space", "CodeEdit", icons["text_edit_space"]);
	p_theme->set_icon("breakpoint", "CodeEdit", icons["breakpoint"]);
	p_theme->set_icon("bookmark", "CodeEdit", icons["bookmark"]);
	p_theme->set_icon("executing_line", "CodeEdit", icons["arrow_right"]);
	p_theme->set_icon("can_fold", "CodeEdit", icons["arrow_down"]);
	p_theme->set_icon("folded", "CodeEdit", icons["arrow_right"]);
	p_theme->set_icon("can_fold_code_region", "CodeEdit", icons["region_unfolded"]);
	p_theme->set_icon("folded_code_region", "CodeEdit", icons["region_folded"]);
	p_theme->set_icon("folded_eol_icon", "CodeEdit", icons["text_edit_ellipsis"]);
	p_theme->set_icon("completion_color_bg", "CodeEdit", icons["mini_checkerboard"]);
	p_theme->set_icon("grabber", "HSlider", icons["slider_grabber"]);
	p_theme->set_icon("grabber_highlight", "HSlider", icons["slider_grabber_hl"]);
	p_theme->set_icon("grabber_disabled", "HSlider", icons["slider_grabber_disabled"]);
	p_theme->set_icon("tick", "HSlider", icons["hslider_tick"]);
	p_theme->set_icon("grabber", "PickerHSlider", icons["color_picker_bar_arrow"]);
	p_theme->set_icon("grabber_highlight", "PickerHSlider", icons["color_picker_bar_arrow"]);
	p_theme->set_icon("grabber", "VSlider", icons["slider_grabber"]);
	p_theme->set_icon("grabber_highlight", "VSlider", icons["slider_grabber_hl"]);
	p_theme->set_icon("grabber_disabled", "VSlider", icons["slider_grabber_disabled"]);
	p_theme->set_icon("tick", "VSlider", icons["vslider_tick"]);
	p_theme->set_icon("up", "SpinBox", icons["value_up"]);
	p_theme->set_icon("up_hover", "SpinBox", icons["value_up"]);
	p_theme->set_icon("up_pressed", "SpinBox", icons["value_up"]);
	p_theme->set_icon("up_disabled", "SpinBox", icons["value_up"]);
	p_theme->set_icon("down", "SpinBox", icons["value_down"]);
	p_theme->set_icon("down_hover", "SpinBox", icons["value_down"]);
	p_theme->set_icon("down_pressed", "SpinBox", icons["value_down"]);
	p_theme->set_icon("down_disabled", "SpinBox", icons["value_down"]);
	p_theme->set_icon("close", "Window", icons["close"]);
	p_theme->set_icon("close_pressed", "Window", icons["close_hl"]);
	p_theme->set_icon("parent_folder", "FileDialog", icons["folder_up"]);
	p_theme->set_icon("back_folder", "FileDialog", icons["arrow_left"]);
	p_theme->set_icon("forward_folder", "FileDialog", icons["arrow_right"]);
	p_theme->set_icon("reload", "FileDialog", icons["reload"]);
	p_theme->set_icon("toggle_hidden", "FileDialog", icons["visibility_visible"]);
	p_theme->set_icon("folder", "FileDialog", icons["folder"]);
	p_theme->set_icon("file", "FileDialog", icons["file"]);
	p_theme->set_icon("create_folder", "FileDialog", icons["folder_create"]);
	p_theme->set_icon("checked", "PopupMenu", icons["checked"]);
	p_theme->set_icon("checked_disabled", "PopupMenu", icons["checked_disabled"]);
	p_theme->set_icon("unchecked", "PopupMenu", icons["unchecked"]);
	p_theme->set_icon("unchecked_disabled", "PopupMenu", icons["unchecked_disabled"]);
	p_theme->set_icon("radio_checked", "PopupMenu", icons["radio_checked"]);
	p_theme->set_icon("radio_checked_disabled", "PopupMenu", icons["radio_checked_disabled"]);
	p_theme->set_icon("radio_unchecked", "PopupMenu", icons["radio_unchecked"]);
	p_theme->set_icon("radio_unchecked_disabled", "PopupMenu", icons["radio_unchecked_disabled"]);
	p_theme->set_icon("submenu", "PopupMenu", icons["popup_menu_arrow_right"]);
	p_theme->set_icon("submenu_mirrored", "PopupMenu", icons["popup_menu_arrow_left"]);
	p_theme->set_icon("port", "GraphNode", icons["graph_port"]);
	p_theme->set_icon("resizer", "GraphNode", icons["resizer_se"]);
	p_theme->set_icon("resizer", "GraphFrame", icons["resizer_se"]);
	p_theme->set_icon("checked", "Tree", icons["checked"]);
	p_theme->set_icon("checked_disabled", "Tree", icons["checked_disabled"]);
	p_theme->set_icon("unchecked", "Tree", icons["unchecked"]);
	p_theme->set_icon("unchecked_disabled", "Tree", icons["unchecked_disabled"]);
	p_theme->set_icon("indeterminate", "Tree", icons["indeterminate"]);
	p_theme->set_icon("indeterminate_disabled", "Tree", icons["indeterminate_disabled"]);
	p_theme->set_icon("updown", "Tree", icons["updown"]);
	p_theme->set_icon("select_arrow", "Tree", icons["option_button_arrow"]);
	p_theme->set_icon("arrow", "Tree", icons["arrow_down"]);
	p_theme->set_icon("arrow_collapsed", "Tree", icons["arrow_right"]);
	p_theme->set_icon("arrow_collapsed_mirrored", "Tree", icons["arrow_left"]);
	p_theme->set_icon("increment", "TabContainer", icons["scroll_button_right"]);
	p_theme->set_icon("increment_highlight", "TabContainer", icons["scroll_button_right_hl"]);
	p_theme->set_icon("decrement", "TabContainer", icons["scroll_button_left"]);
	p_theme->set_icon("decrement_highlight", "TabContainer", icons["scroll_button_left_hl"]);
	p_theme->set_icon("menu", "TabContainer", icons["tabs_menu"]);
	p_theme->set_icon("menu_highlight", "TabContainer", icons["tabs_menu_hl"]);
	p_theme->set_icon("increment", "TabBar", icons["scroll_button_right"]);
	p_theme->set_icon("increment_highlight", "TabBar", icons["scroll_button_right_hl"]);
	p_theme->set_icon("decrement", "TabBar", icons["scroll_button_left"]);
	p_theme->set_icon("decrement_highlight", "TabBar", icons["scroll_button_left_hl"]);
	p_theme->set_icon("close", "TabBar", icons["close"]);
	p_theme->set_icon("close", "Icons", icons["close"]);
	p_theme->set_icon("error_icon", "Icons", icons["error_icon"]);
	p_theme->set_icon("folded_arrow", "ColorPicker", icons["arrow_right"]);
	p_theme->set_icon("expanded_arrow", "ColorPicker", icons["arrow_down"]);
	p_theme->set_icon("screen_picker", "ColorPicker", icons["color_picker_pipette"]);
	p_theme->set_icon("shape_circle", "ColorPicker", icons["picker_shape_circle"]);
	p_theme->set_icon("shape_rect", "ColorPicker", icons["picker_shape_rectangle"]);
	p_theme->set_icon("shape_rect_wheel", "ColorPicker", icons["picker_shape_rectangle_wheel"]);
	p_theme->set_icon("add_preset", "ColorPicker", icons["add"]);
	p_theme->set_icon("sample_bg", "ColorPicker", icons["mini_checkerboard"]);
	p_theme->set_icon("sample_revert", "ColorPicker", icons["reload"]);
	p_theme->set_icon("overbright_indicator", "ColorPicker", icons["color_picker_overbright"]);
	p_theme->set_icon("picker_cursor", "ColorPicker", icons["color_picker_cursor"]);
	p_theme->set_icon("picker_cursor_bg", "ColorPicker", icons["color_picker_cursor_bg"]);
	p_theme->set_icon("hex_icon", "ColorPicker", icons["color_picker_hex"]);
	p_theme->set_icon("hex_code_icon", "ColorPicker", icons["color_picker_hex_code"]);
	p_theme->set_icon("bg", "ColorButton", icons["mini_checkerboard"]);
	p_theme->set_icon("overbright_indicator", "ColorButton", icons["color_picker_overbright"]);
	p_theme->set_icon("arrow", "FoldableContainer", icons["arrow_down"]);
	p_theme->set_icon("arrow_collapsed", "FoldableContainer", icons["arrow_right"]);
	p_theme->set_icon("arrow_collapsed_mirrored", "FoldableContainer", icons["arrow_left"]);
	p_theme->set_icon("zoom_out", "GraphEdit", icons["zoom_less"]);
	p_theme->set_icon("zoom_in", "GraphEdit", icons["zoom_more"]);
	p_theme->set_icon("zoom_reset", "GraphEdit", icons["zoom_reset"]);
	p_theme->set_icon("grid_toggle", "GraphEdit", icons["grid_toggle"]);
	p_theme->set_icon("minimap_toggle", "GraphEdit", icons["grid_minimap"]);
	p_theme->set_icon("snapping_toggle", "GraphEdit", icons["grid_snap"]);
	p_theme->set_icon("layout", "GraphEdit", icons["grid_layout"]);
	p_theme->set_icon("resizer", "GraphEditMinimap", icons["resizer_nw"]);
	p_theme->set_icon("zoom_less", "ZoomWidget", icons["zoom_less"]);
	p_theme->set_icon("zoom_more", "ZoomWidget", icons["zoom_more"]);

	p_theme->set_constant("grabber_offset", "PickerHSlider", Math::round(((Ref<Texture2D>)icons["color_picker_bar_arrow"])->get_size().height * 0.5));
	ThemeDB::get_singleton()->set_fallback_icon(icons["error_icon"]);
}

static Color contrast_color(Color p_color, float p_value) {
	if (Math::is_equal_approx(p_value, 0.f)) {
		return p_color;
	} else if (p_value > 0.0) {
		return p_color.lightened(p_value);
	} else {
		return p_color.darkened(p_value * -1.0);
	}
}

void update_font_color(Ref<Theme> &p_theme, Color p_color) {
	popup_panel_style->set_border_color(p_color);
	color_picker_popup_panel_style->set_border_color(p_color);
	h_separator_style->set_color(p_color);
	v_separator_style->set_color(p_color);
	panel_style->set_border_color(p_color);
	graph_title_style->set_border_color(p_color);
	graph_frame_title_style->set_border_color(p_color);
	graph_panel_style->set_border_color(p_color);
	color_button_pressed_style->set_border_color(p_color);
	p_theme->set_color("font_hovered_color", "TabContainer", p_color);
	p_theme->set_color("font_hovered_color", "TabBar", p_color);
	p_theme->set_color("font_hover_color", "PopupMenu", p_color);
	p_theme->set_color("font_selected_color", "LineEdit", p_color);
	p_theme->set_color("font_selected_color", "TextEdit", p_color);
	p_theme->set_color("font_selected_color", "RichTextLabel", p_color);
	p_theme->set_color("font_selected_color", "CodeEdit", p_color);
	p_theme->set_color("font_selected_color", "ItemList", p_color);
	p_theme->set_color("font_selected_color", "TabContainer", p_color);
	p_theme->set_color("font_selected_color", "TabBar", p_color);
	p_theme->set_color("font_selected_color", "Tree", p_color);
	p_theme->set_color("font_hovered_color", "Tree", p_color);
	p_theme->set_color("caret_background_color", "TextEdit", p_color);
	p_theme->set_color("caret_background_color", "CodeEdit", p_color);
	p_theme->set_color("font_color", "ProgressBar", p_color);
	p_theme->set_color("hover_font_color", "FoldableContainer", p_color);
	p_theme->set_color("font_hover_color", "Button", p_color);
	p_theme->set_color("font_hover_color", "LinkButton", p_color);
	p_theme->set_color("icon_hover_color", "Button", p_color);
	p_theme->set_color("font_hover_color", "MenuBar", p_color);
	p_theme->set_color("font_hover_color", "LinkButton", p_color);
	p_theme->set_color("font_hover_pressed_color", "MenuBar", p_color);
	p_theme->set_color("font_hover_color", "OptionButton", p_color);
	p_theme->set_color("font_hover_pressed_color", "OptionButton", p_color);
	p_theme->set_color("font_hover_color", "MenuButton", p_color);
	p_theme->set_color("font_hover_color", "CheckBox", p_color);
	p_theme->set_color("font_hover_color", "CheckButton", p_color);
	p_theme->set_color("drop_position_color", "Tree", p_color);
	p_theme->set_color("font_hover_color", "ColorButton", p_color);
	p_theme->set_color("up_hover_icon_modulate", "SpinBox", p_color);
	p_theme->set_color("down_hover_icon_modulate", "SpinBox", p_color);
	p_theme->set_color("font_hovered_color", "ItemList", p_color);
	p_theme->set_color("connection_rim_color", "GraphEdit", p_color);
	p_theme->set_color("button_icon_hovered", "FoldableContainer", p_color);
	p_color.a = 0.9;
	p_theme->set_color("parent_hl_line_color", "Tree", p_color);
	p_theme->set_color("icon_focus_color", "Button", p_color);
	p_theme->set_color("font_focus_color", "Button", p_color);
	p_theme->set_color("font_focus_color", "LinkButton", p_color);
	p_theme->set_color("font_focus_color", "MenuBar", p_color);
	p_theme->set_color("title_color", "Window", p_color);
	p_theme->set_color("font_focus_color", "OptionButton", p_color);
	p_theme->set_color("font_focus_color", "MenuButton", p_color);
	p_theme->set_color("font_focus_color", "CheckBox", p_color);
	p_theme->set_color("font_focus_color", "CheckButton", p_color);
	p_theme->set_color("custom_button_font_highlight", "Tree", p_color);
	p_theme->set_color("font_focus_color", "ColorButton", p_color);
	p_theme->set_color("resizer_color", "GraphEditMinimap", p_color);
	p_theme->set_color("resizer_color", "GraphNode", p_color);
	p_theme->set_color("resizer_color", "GraphFrame", p_color);
	p_color.a = 0.8;
	p_theme->set_color(SceneStringName(font_color), "Tree", p_color);
	p_theme->set_color(SceneStringName(font_color), "LinkButton", p_color);
	p_theme->set_color(SceneStringName(font_color), "Button", p_color);
	p_theme->set_color(SceneStringName(font_color), "Label", p_color);
	p_theme->set_color(SceneStringName(font_color), "LineEdit", p_color);
	p_theme->set_color(SceneStringName(font_color), "TextEdit", p_color);
	p_theme->set_color(SceneStringName(font_color), "CodeEdit", p_color);
	p_theme->set_color(SceneStringName(font_color), "PopupMenu", p_color);
	p_theme->set_color(SceneStringName(font_color), "FoldableContainer", p_color);
	p_theme->set_color(SceneStringName(font_color), "MenuBar", p_color);
	p_theme->set_color(SceneStringName(font_color), "OptionButton", p_color);
	p_theme->set_color(SceneStringName(font_color), "MenuButton", p_color);
	p_theme->set_color(SceneStringName(font_color), "CheckBox", p_color);
	p_theme->set_color(SceneStringName(font_color), "CheckButton", p_color);
	p_theme->set_color(SceneStringName(font_color), "GraphNodeTitleLabel", p_color);
	p_theme->set_color(SceneStringName(font_color), "TooltipLabel", p_color);
	p_theme->set_color(SceneStringName(font_color), "GraphFrameTitleLabel", p_color);
	p_theme->set_color(SceneStringName(font_color), "ItemList", p_color);
	p_theme->set_color(SceneStringName(font_color), "ColorButton", p_color);
	p_theme->set_color("icon_normal_color", "Button", p_color);
	p_theme->set_color("grabber_hovered", "SplitContainer", p_color);
	p_theme->set_color("grabber_hovered", "HSplitContainer", p_color);
	p_theme->set_color("grabber_hovered", "VSplitContainer", p_color);
	p_theme->set_color("caret_color", "LineEdit", p_color);
	p_theme->set_color("caret_color", "TextEdit", p_color);
	p_theme->set_color("caret_color", "CodeEdit", p_color);
	p_theme->set_color("clear_button_color", "LineEdit", p_color);
	p_theme->set_color("font_unselected_color", "TabContainer", p_color);
	p_theme->set_color("font_unselected_color", "TabBar", p_color);
	p_theme->set_color("collapsed_font_color", "FoldableContainer", p_color);
	p_theme->set_color("folder_icon_color", "FileDialog", p_color);
	p_theme->set_color("file_icon_color", "FileDialog", p_color);
	p_theme->set_color("title_button_color", "Tree", p_color);
	p_theme->set_color("children_hl_line_color", "Tree", p_color);
	p_theme->set_color("default_color", "RichTextLabel", p_color);
	p_theme->set_color("up_icon_modulate", "SpinBox", p_color);
	p_theme->set_color("down_icon_modulate", "SpinBox", p_color);
	p_theme->set_color("button_icon_normal", "FoldableContainer", p_color);
	p_color.a = 0.6;
	grabber_style->set_bg_color(p_color);
	p_theme->set_color("font_separator_color", "PopupMenu", p_color);
	p_theme->set_color("font_accelerator_color", "PopupMenu", p_color);
	p_theme->set_color("font_disabled_color", "PopupMenu", p_color);
	p_theme->set_color("grabber_normal", "SplitContainer", p_color);
	p_theme->set_color("grabber_normal", "HSplitContainer", p_color);
	p_theme->set_color("grabber_normal", "VSplitContainer", p_color);
	p_theme->set_color("font_uneditable_color", "LineEdit", p_color);
	p_theme->set_color("font_readonly_color", "TextEdit", p_color);
	p_theme->set_color("font_readonly_color", "CodeEdit", p_color);
	p_theme->set_color("font_placeholder_color", "LineEdit", p_color);
	p_theme->set_color("font_placeholder_color", "TextEdit", p_color);
	p_theme->set_color("font_placeholder_color", "CodeEdit", p_color);
	p_theme->set_color("relationship_line_color", "Tree", p_color);
	p_theme->set_color("font_hovered_dimmed_color", "Tree", p_color);
	p_color.a = 0.4;
	p_theme->set_color("search_result_border_color", "TextEdit", p_color);
	p_theme->set_color("search_result_border_color", "CodeEdit", p_color);
	p_theme->set_color("guide_color", "Tree", p_color);
	p_theme->set_color("guide_color", "ItemList", p_color);
	p_theme->set_color("file_disabled_color", "FileDialog", p_color);
	p_theme->set_color("font_disabled_color", "TabContainer", p_color);
	p_theme->set_color("font_disabled_color", "TabBar", p_color);
	p_theme->set_color("icon_disabled_color", "Button", p_color);
	p_theme->set_color("font_disabled_color", "Button", p_color);
	p_theme->set_color("font_disabled_color", "Tree", p_color);
	p_theme->set_color("icon_disabled_color", "LinkButton", p_color);
	p_theme->set_color("font_disabled_color", "MenuBar", p_color);
	p_theme->set_color("font_disabled_color", "LinkButton", p_color);
	p_theme->set_color("font_disabled_color", "OptionButton", p_color);
	p_theme->set_color("font_disabled_color", "MenuButton", p_color);
	p_theme->set_color("font_disabled_color", "CheckBox", p_color);
	p_theme->set_color("font_disabled_color", "CheckButton", p_color);
	p_theme->set_color("font_disabled_color", "ColorButton", p_color);
	p_theme->set_color("up_disabled_icon_modulate", "SpinBox", p_color);
	p_theme->set_color("down_disabled_icon_modulate", "SpinBox", p_color);
	p_theme->set_color("selection_fill", "GraphEdit", p_color);
	p_theme->set_color("completion_scroll_hovered_color", "CodeEdit", p_color);
	p_theme->set_color("completion_scroll_color", "CodeEdit", p_color);
	p_theme->set_color("button_icon_disabled", "FoldableContainer", p_color);
}

void update_font_outline_color(Ref<Theme> &p_theme, Color p_color) {
	p_theme->set_color("font_separator_outline_color", "PopupMenu", p_color);
	p_theme->set_color("font_outline_color", "RichTextLabel", p_color);
	p_theme->set_color("font_outline_color", "FoldableContainer", p_color);
	p_theme->set_color("font_outline_color", "TooltipLabel", p_color);
	p_theme->set_color("font_outline_color", "ColorButton", p_color);
	p_theme->set_color("font_outline_color", "TabBar", p_color);
	p_theme->set_color("font_outline_color", "TabContainer", p_color);
	p_theme->set_color("font_outline_color", "ItemList", p_color);
	p_theme->set_color("font_outline_color", "Tree", p_color);
	p_theme->set_color("font_outline_color", "GraphFrameTitleLabel", p_color);
	p_theme->set_color("font_outline_color", "GraphNodeTitleLabel", p_color);
	p_theme->set_color("font_outline_color", "PopupMenu", p_color);
	p_theme->set_color("title_outline_modulate", "Window", p_color);
	p_theme->set_color("font_outline_color", "CodeEdit", p_color);
	p_theme->set_color("font_outline_color", "TextEdit", p_color);
	p_theme->set_color("font_outline_color", "ProgressBar", p_color);
	p_theme->set_color("font_outline_color", "LineEdit", p_color);
	p_theme->set_color("font_outline_color", "Label", p_color);
	p_theme->set_color("font_outline_color", "CheckButton", p_color);
	p_theme->set_color("font_outline_color", "CheckBox", p_color);
	p_theme->set_color("font_outline_color", "MenuButton", p_color);
	p_theme->set_color("font_outline_color", "OptionButton", p_color);
	p_theme->set_color("font_outline_color", "LinkButton", p_color);
	p_theme->set_color("font_outline_color", "MenuBar", p_color);
	p_theme->set_color("font_outline_color", "Button", p_color);
}

void update_theme_margins(Ref<Theme> &p_theme, int p_margin) {
	int margin = p_margin * MAX(p_theme->get_default_base_scale(), 0.5);

	p_theme->set_constant("separation", "BoxContainer", margin);
	p_theme->set_constant("separation", "HBoxContainer", margin);
	p_theme->set_constant("separation", "VBoxContainer", margin);
	p_theme->set_constant("h_separation", "Button", margin);
	p_theme->set_constant("h_separation", "FoldableContainer", margin);
	p_theme->set_constant("h_separation", "Tree", margin);
	p_theme->set_constant("h_separation", "ScrollContainer", margin);
	p_theme->set_constant("h_separation", "TabBar", margin);
	p_theme->set_constant("h_separation", "OptionButton", margin);
	p_theme->set_constant("h_separation", "MenuButton", margin);
	p_theme->set_constant("h_separation", "CheckBox", margin);
	p_theme->set_constant("h_separation", "CheckButton", margin);
	p_theme->set_constant("h_separation", "FlowContainer", margin);
	p_theme->set_constant("v_separation", "FlowContainer", margin);
	p_theme->set_constant("h_separation", "HFlowContainer", margin);
	p_theme->set_constant("v_separation", "HFlowContainer", margin);
	p_theme->set_constant("h_separation", "VFlowContainer", margin);
	p_theme->set_constant("v_separation", "VFlowContainer", margin);
	p_theme->set_constant("v_separation", "ScrollContainer", margin);
	p_theme->set_constant("button_margin", "Tree", margin);
	p_theme->set_constant("inner_margin_left", "Tree", margin);
	p_theme->set_constant("inner_margin_right", "Tree", margin);
	p_theme->set_constant("separation", "HSeparator", margin);
	p_theme->set_constant("separation", "VSeparator", margin);
	p_theme->set_constant("scrollbar_h_separation", "Tree", margin);
	p_theme->set_constant("scrollbar_v_separation", "Tree", margin);
	p_theme->set_constant("h_separation", "ItemList", margin);
	p_theme->set_constant("v_separation", "ItemList", margin);
	p_theme->set_constant("icon_margin", "ItemList", margin);
	p_theme->set_constant("h_separation", "ColorButton", margin);
	p_theme->set_constant("h_separation", "GridContainer", margin);
	p_theme->set_constant("v_separation", "GridContainer", margin);
}

void update_theme_padding(Ref<Theme> &p_theme, int p_padding) {
	float base_scale = MAX(p_theme->get_default_base_scale(), 0.5);
	int padding = p_padding * base_scale;

	tab_selected_style->set_content_margin_individual(padding, padding + base_scale * 2, padding, padding);
	tab_unselected_style->set_content_margin_individual(padding, padding + base_scale * 2, padding, padding);
	tab_hovered_style->set_content_margin_individual(padding, padding + base_scale * 2, padding, padding);
	tab_disabled_style->set_content_margin_individual(padding, padding + base_scale * 2, padding, padding);
	tab_panel_style->set_content_margin_all(padding);
	progress_background_style->set_content_margin_all(padding);
	graph_frame_title_style->set_content_margin_all(padding);
	graph_frame_title_selected_style->set_content_margin_all(padding);
	foldable_panel_style->set_content_margin_individual(12 * base_scale, padding, padding, padding);
	graph_title_style->set_content_margin_individual(12 * base_scale, padding, padding, padding);
	graph_title_selected_style->set_content_margin_individual(12 * base_scale, padding, padding, padding);
	foldable_title_style->set_content_margin_all(padding);
	foldable_title_hover_style->set_content_margin_all(padding);
	foldable_title_collapsed_style->set_content_margin_all(padding);
	foldable_title_collapsed_hover_style->set_content_margin_all(padding);
	p_theme->set_constant("margin_left", "MarginContainer", padding);
	p_theme->set_constant("margin_top", "MarginContainer", padding);
	p_theme->set_constant("margin_right", "MarginContainer", padding);
	p_theme->set_constant("margin_bottom", "MarginContainer", padding);
	p_theme->set_constant("item_start_padding", "PopupMenu", padding);
	p_theme->set_constant("item_end_padding", "PopupMenu", padding);
}

void update_theme_corner_radius(Ref<Theme> &p_theme, int p_corner_radius) {
	float base_scale = MAX(p_theme->get_default_base_scale(), 0.5);
	int corners = p_corner_radius * base_scale;

	panel_style->set_corner_radius_all(corners);
	button_normal_style->set_corner_radius_all(corners);
	button_hover_style->set_corner_radius_all(corners);
	button_pressed_style->set_corner_radius_all(corners);
	button_disabled_style->set_corner_radius_all(corners);
	color_button_normal_style->set_corner_radius_all(corners);
	color_button_hover_style->set_corner_radius_all(corners);
	color_button_pressed_style->set_corner_radius_all(corners);
	color_button_disabled_style->set_corner_radius_all(corners);
	color_picker_button_hovered_style->set_corner_radius_all(corners);
	color_picker_button_normal_style->set_corner_radius_all(corners);
	color_picker_button_pressed_style->set_corner_radius_all(corners);
	color_picker_button_disabled_style->set_corner_radius_all(corners);
	grabber_style->set_corner_radius_all(corners);
	grabber_highlight_style->set_corner_radius_all(corners);
	slider_style->set_corner_radius_all(corners);
	picker_slider_style->set_corner_radius_all(corners);
	h_scroll_style->set_corner_radius_all(corners);
	v_scroll_style->set_corner_radius_all(corners);
	foldable_title_collapsed_style->set_corner_radius_all(corners);
	swatches_foldable_title_collapsed_style->set_corner_radius_all(corners);
	foldable_title_collapsed_hover_style->set_corner_radius_all(corners);
	swatches_foldable_title_collapsed_hover_style->set_corner_radius_all(corners);
	menu_button_normal_style->set_corner_radius_individual(corners, corners, 0, 0);
	menu_button_hover_style->set_corner_radius_individual(corners, corners, 0, 0);
	menu_button_pressed_style->set_corner_radius_individual(corners, corners, 0, 0);
	menu_button_disabled_style->set_corner_radius_individual(corners, corners, 0, 0);
	foldable_panel_style->set_corner_radius_individual(0, 0, corners, corners);
	swatches_foldable_panel_style->set_corner_radius_individual(0, 0, corners, corners);
	foldable_title_style->set_corner_radius_individual(corners, corners, 0, 0);
	swatches_foldable_title_style->set_corner_radius_individual(corners, corners, 0, 0);
	foldable_title_hover_style->set_corner_radius_individual(corners, corners, 0, 0);
	swatches_foldable_title_hover_style->set_corner_radius_individual(corners, corners, 0, 0);
	graph_panel_style->set_corner_radius_individual(0, 0, corners, corners);
	graph_panel_selected_style->set_corner_radius_individual(0, 0, corners, corners);
	graph_title_style->set_corner_radius_individual(corners, corners, 0, 0);
	graph_frame_title_style->set_corner_radius_individual(corners, corners, 0, 0);
	graph_title_selected_style->set_corner_radius_individual(corners, corners, 0, 0);
	graph_frame_title_selected_style->set_corner_radius_individual(corners, corners, 0, 0);

	int focus_border = MAX(p_corner_radius - 2, 0) * base_scale;
	button_focus_style->set_corner_radius_all(focus_border);
	color_button_focus_style->set_corner_radius_all(focus_border);
	menu_button_focus_style->set_corner_radius_individual(focus_border, focus_border, 0, 0);
}

void update_theme_border_width(Ref<Theme> &p_theme, int p_border_width) {
	int border_width = p_border_width * MAX(p_theme->get_default_base_scale(), 0.5);

	panel_style->set_border_width_all(border_width);
	popup_panel_style->set_content_margin_all(MAX(border_width, 1));
	button_focus_style->set_border_width_all(MAX(border_width, 1));
	menu_button_focus_style->set_border_width_all(MAX(border_width, 1));
}

void update_theme_border_padding(Ref<Theme> &p_theme, int p_border_padding) {
	int border_padding = p_border_padding * MAX(p_theme->get_default_base_scale(), 0.5);

	panel_style->set_content_margin_all(border_padding);
	button_normal_style->set_content_margin_all(border_padding);
	button_hover_style->set_content_margin_all(border_padding);
	button_pressed_style->set_content_margin_all(border_padding);
	button_disabled_style->set_content_margin_all(border_padding);
	menu_button_normal_style->set_content_margin_all(border_padding);
	menu_button_hover_style->set_content_margin_all(border_padding);
	menu_button_pressed_style->set_content_margin_all(border_padding);
	menu_button_disabled_style->set_content_margin_all(border_padding);
	flat_button_normal->set_content_margin_all(border_padding);
	p_theme->set_constant("arrow_margin", "OptionButton", border_padding);
}

void update_font_outline_size(Ref<Theme> &p_theme, int p_outline_size) {
	int outline_size = p_outline_size * MAX(p_theme->get_default_base_scale(), 0.5);

	p_theme->set_constant("separator_outline_size", "PopupMenu", outline_size);
	p_theme->set_constant("outline_size", "RichTextLabel", outline_size);
	p_theme->set_constant("outline_size", "FoldableContainer", outline_size);
	p_theme->set_constant("outline_size", "TooltipLabel", outline_size);
	p_theme->set_constant("outline_size", "TabBar", outline_size);
	p_theme->set_constant("outline_size", "TabContainer", outline_size);
	p_theme->set_constant("outline_size", "ItemList", outline_size);
	p_theme->set_constant("outline_size", "Tree", outline_size);
	p_theme->set_constant("outline_size", "GraphFrameTitleLabel", outline_size);
	p_theme->set_constant("outline_size", "GraphNodeTitleLabel", outline_size);
	p_theme->set_constant("outline_size", "PopupMenu", outline_size);
	p_theme->set_constant("title_outline_size", "Window", outline_size);
	p_theme->set_constant("outline_size", "CodeEdit", outline_size);
	p_theme->set_constant("outline_size", "TextEdit", outline_size);
	p_theme->set_constant("outline_size", "ProgressBar", outline_size);
	p_theme->set_constant("outline_size", "LineEdit", outline_size);
	p_theme->set_constant("outline_size", "Label", outline_size);
	p_theme->set_constant("outline_size", "CheckButton", outline_size);
	p_theme->set_constant("outline_size", "CheckBox", outline_size);
	p_theme->set_constant("outline_size", "MenuButton", outline_size);
	p_theme->set_constant("outline_size", "OptionButton", outline_size);
	p_theme->set_constant("outline_size", "LinkButton", outline_size);
	p_theme->set_constant("outline_size", "MenuBar", outline_size);
	p_theme->set_constant("outline_size", "Button", outline_size);
}

void update_font_size(Ref<Theme> &p_theme, int p_font_size) {
	float base_scale = MAX(p_theme->get_default_base_scale(), 0.5);
	int font_size = p_font_size * base_scale;

	p_theme->set_default_font_size(font_size);

	p_theme->set_font_size(SceneStringName(font_size), "HeaderSmall", font_size + 4 * base_scale);
	p_theme->set_font_size(SceneStringName(font_size), "HeaderMedium", font_size + 8 * base_scale);
	p_theme->set_font_size(SceneStringName(font_size), "GraphFrameTitleLabel", font_size + 8 * base_scale);
	p_theme->set_font_size(SceneStringName(font_size), "HeaderLarge", font_size + 12 * base_scale);
}

void update_font_embolden(float p_embolden) {
	if (custom_font.is_valid() && !using_custom_font_variation) {
		custom_font->set_variation_embolden(p_embolden);
	}
	fallback_font->set_variation_embolden(p_embolden);
	bold_font->set_variation_embolden(p_embolden + 0.2);
	bold_italics_font->set_variation_embolden(p_embolden + 0.2);
	italics_font->set_variation_embolden(p_embolden);
}

void update_font_spacing_glyph(int p_spacing) {
	if (custom_font.is_valid() && !using_custom_font_variation) {
		custom_font->set_spacing(TextServer::SPACING_GLYPH, p_spacing);
	}
	fallback_font->set_spacing(TextServer::SPACING_GLYPH, p_spacing);
	bold_font->set_spacing(TextServer::SPACING_GLYPH, p_spacing);
	bold_italics_font->set_spacing(TextServer::SPACING_GLYPH, p_spacing);
	italics_font->set_spacing(TextServer::SPACING_GLYPH, p_spacing);
}

void update_font_spacing_space(int p_spacing) {
	if (custom_font.is_valid() && !using_custom_font_variation) {
		custom_font->set_spacing(TextServer::SPACING_SPACE, p_spacing);
	}
	fallback_font->set_spacing(TextServer::SPACING_SPACE, p_spacing);
	bold_font->set_spacing(TextServer::SPACING_SPACE, p_spacing);
	bold_italics_font->set_spacing(TextServer::SPACING_SPACE, p_spacing);
	italics_font->set_spacing(TextServer::SPACING_SPACE, p_spacing);
}

void update_font_spacing_top(int p_spacing) {
	if (custom_font.is_valid() && !using_custom_font_variation) {
		custom_font->set_spacing(TextServer::SPACING_TOP, p_spacing);
	}
	fallback_font->set_spacing(TextServer::SPACING_TOP, p_spacing);
	bold_italics_font->set_spacing(TextServer::SPACING_TOP, p_spacing);
	bold_font->set_spacing(TextServer::SPACING_TOP, p_spacing);
	italics_font->set_spacing(TextServer::SPACING_TOP, p_spacing);
}

void update_font_spacing_bottom(int p_spacing) {
	if (custom_font.is_valid() && !using_custom_font_variation) {
		custom_font->set_spacing(TextServer::SPACING_BOTTOM, p_spacing);
	}
	fallback_font->set_spacing(TextServer::SPACING_BOTTOM, p_spacing);
	bold_font->set_spacing(TextServer::SPACING_BOTTOM, p_spacing);
	bold_italics_font->set_spacing(TextServer::SPACING_BOTTOM, p_spacing);
	italics_font->set_spacing(TextServer::SPACING_BOTTOM, p_spacing);
}

void update_theme_scale(Ref<Theme> &p_theme) {
	float base_scale = MAX(p_theme->get_default_base_scale(), 0.5);
	int int_scale = MAX(Math::floor(base_scale), 1);
	int x2_scale = 2 * base_scale;
	int x4_scale = 4 * base_scale;
	int x6_scale = 6 * base_scale;

	popup_panel_style->set_border_width_all(int_scale);
	color_picker_popup_panel_style->set_border_width_all(int_scale);
	p_theme->set_constant("children_hl_line_width", "Tree", int_scale);
	p_theme->set_constant("shadow_offset_x", "Label", int_scale);
	p_theme->set_constant("shadow_offset_y", "Label", int_scale);
	p_theme->set_constant("shadow_outline_size", "Label", int_scale);
	p_theme->set_constant("shadow_offset_x", "GraphNodeTitleLabel", int_scale);
	p_theme->set_constant("shadow_offset_y", "GraphNodeTitleLabel", int_scale);
	p_theme->set_constant("shadow_outline_size", "GraphNodeTitleLabel", int_scale);
	p_theme->set_constant("shadow_offset_x", "GraphFrameTitleLabel", int_scale);
	p_theme->set_constant("shadow_offset_y", "GraphFrameTitleLabel", int_scale);
	p_theme->set_constant("shadow_outline_size", "GraphFrameTitleLabel", int_scale);
	p_theme->set_constant("shadow_offset_x", "TooltipLabel", int_scale);
	p_theme->set_constant("shadow_offset_y", "TooltipLabel", int_scale);
	p_theme->set_constant("shadow_offset_x", "RichTextLabel", int_scale);
	p_theme->set_constant("shadow_offset_y", "RichTextLabel", int_scale);
	p_theme->set_constant("shadow_outline_size", "RichTextLabel", int_scale);
	p_theme->set_constant("caret_width", "LineEdit", int_scale);
	p_theme->set_constant("caret_width", "TextEdit", int_scale);
	p_theme->set_constant("relationship_line_width", "Tree", int_scale);
	p_theme->set_constant("parent_hl_line_width", "Tree", 2 * int_scale);
	p_theme->set_constant("indent", "PopupMenu", 10 * base_scale);
	p_theme->set_constant("buttons_separation", "AcceptDialog", 10 * base_scale);
	p_theme->set_constant("scroll_speed", "Tree", 12 * base_scale);
	p_theme->set_constant("item_margin", "Tree", 16 * base_scale);
	p_theme->set_constant("buttons_width", "SpinBox", 16 * base_scale);
	p_theme->set_constant("close_h_offset", "Window", 18 * base_scale);
	p_theme->set_constant("port_hotzone_inner_extent", "GraphEdit", 22 * base_scale);
	p_theme->set_constant("close_v_offset", "Window", 24 * base_scale);
	p_theme->set_constant("port_hotzone_outer_extent", "GraphEdit", 26 * base_scale);
	p_theme->set_constant("h_width", "ColorPicker", 30 * base_scale);
	p_theme->set_constant("sample_height", "ColorPicker", 30 * base_scale);
	p_theme->set_constant("swatch_size", "ColorPicker", 32 * base_scale);
	p_theme->set_constant("title_height", "Window", 36 * base_scale);
	p_theme->set_constant("completion_max_width", "CodeEdit", 50 * base_scale);
	p_theme->set_constant("sv_width", "ColorPicker", 256 * base_scale);
	p_theme->set_constant("sv_height", "ColorPicker", 256 * base_scale);

	picker_slider_style->set_content_margin(SIDE_TOP, 16 * base_scale);
	tab_selected_style->set_border_width(SIDE_TOP, x2_scale);
	color_button_focus_style->set_border_width_all(x2_scale);
	tab_focus_style->set_border_width_all(x2_scale);
	h_separator_style->set_thickness(x2_scale);
	v_separator_style->set_thickness(x2_scale);
	slider_style->set_content_margin_all(x2_scale);
	p_theme->set_constant("line_spacing", "Label", x2_scale);
	p_theme->set_constant("line_spacing", "GraphNodeTitleLabel", x2_scale);
	p_theme->set_constant("line_spacing", "GraphFrameTitleLabel", x2_scale);
	p_theme->set_constant("underline_spacing", "LinkButton", x2_scale);
	p_theme->set_constant("field_and_buttons_separation", "SpinBox", x2_scale);
	p_theme->set_constant("icon_separation", "TabContainer", x2_scale);
	p_theme->set_constant("text_highlight_h_padding", "RichTextLabel", x2_scale);
	p_theme->set_constant("text_highlight_v_padding", "RichTextLabel", x2_scale);
	p_theme->set_constant("separation", "GraphNode", x2_scale);
	p_theme->set_constant(SceneStringName(line_separation), "ItemList", x2_scale);
	p_theme->set_constant("table_h_separation", "RichTextLabel", x2_scale);
	p_theme->set_constant("table_v_separation", "RichTextLabel", x2_scale);
	graph_panel_style->set_border_width(SIDE_LEFT, x2_scale);
	graph_panel_style->set_border_width(SIDE_RIGHT, x2_scale);
	graph_panel_style->set_border_width(SIDE_BOTTOM, x2_scale);
	graph_panel_selected_style->set_border_width(SIDE_LEFT, x2_scale);
	graph_panel_selected_style->set_border_width(SIDE_RIGHT, x2_scale);
	graph_panel_selected_style->set_border_width(SIDE_BOTTOM, x2_scale);
	graph_title_style->set_border_width(SIDE_LEFT, x2_scale);
	graph_title_style->set_border_width(SIDE_RIGHT, x2_scale);
	graph_title_style->set_border_width(SIDE_TOP, x2_scale);
	graph_frame_title_style->set_border_width(SIDE_LEFT, x2_scale);
	graph_frame_title_style->set_border_width(SIDE_RIGHT, x2_scale);
	graph_frame_title_style->set_border_width(SIDE_TOP, x2_scale);
	graph_title_selected_style->set_border_width(SIDE_LEFT, x2_scale);
	graph_title_selected_style->set_border_width(SIDE_RIGHT, x2_scale);
	graph_title_selected_style->set_border_width(SIDE_TOP, x2_scale);
	graph_frame_title_selected_style->set_border_width(SIDE_LEFT, x2_scale);
	graph_frame_title_selected_style->set_border_width(SIDE_RIGHT, x2_scale);
	graph_frame_title_selected_style->set_border_width(SIDE_TOP, x2_scale);
	color_mode_button_pressed_style->set_border_width(SIDE_TOP, x2_scale);
	color_sliders_panel_style->set_content_margin_all(x2_scale);

	h_scroll_style->set_content_margin_individual(0, x4_scale, 0, x4_scale);
	v_scroll_style->set_content_margin_individual(x4_scale, 0, x4_scale, 0);
	color_button_pressed_style->set_border_width_all(x4_scale);
	swatches_foldable_panel_style->set_content_margin_all(x4_scale);
	swatches_foldable_title_style->set_content_margin_all(x4_scale);
	swatches_foldable_title_hover_style->set_content_margin_all(x4_scale);
	swatches_foldable_title_collapsed_style->set_content_margin_all(x4_scale);
	swatches_foldable_title_collapsed_hover_style->set_content_margin_all(x4_scale);
	color_mode_button_hovered_style->set_content_margin_all(x4_scale);
	color_mode_button_normal_style->set_content_margin_all(x4_scale);
	color_mode_button_pressed_style->set_content_margin_all(x4_scale);
	color_picker_button_hovered_style->set_content_margin_all(x4_scale);
	color_picker_button_normal_style->set_content_margin_all(x4_scale);
	color_picker_button_pressed_style->set_content_margin_all(x4_scale);
	color_picker_button_disabled_style->set_content_margin_all(x4_scale);
	p_theme->set_constant("h_separation", "SwatchesFoldableContainer", x4_scale);
	p_theme->set_constant("grabber_thickness", "SplitContainer", x4_scale);
	p_theme->set_constant("grabber_thickness", "HSplitContainer", x4_scale);
	p_theme->set_constant("grabber_thickness", "VSplitContainer", x4_scale);
	p_theme->set_constant("line_spacing", "TextEdit", x4_scale);
	p_theme->set_constant("line_spacing", "CodeEdit", x4_scale);
	p_theme->set_constant("h_separation", "PopupMenu", x4_scale);
	p_theme->set_constant("v_separation", "PopupMenu", x4_scale);
	p_theme->set_constant("v_separation", "Tree", x4_scale);
	p_theme->set_constant("h_separation", "MenuBar", x4_scale);
	p_theme->set_constant("scroll_border", "Tree", x4_scale);
	p_theme->set_constant("resize_margin", "Window", x4_scale);
	p_theme->set_constant("drop_mark_width", "TabContainer", x4_scale);
	p_theme->set_constant("drop_mark_width", "TabBar", x4_scale);
	p_theme->set_constant("h_separation", "ColorPickerGrid", x4_scale);
	p_theme->set_constant("v_separation", "ColorPickerGrid", x4_scale);

	color_button_normal_style->set_content_margin_all(x6_scale);
	color_button_hover_style->set_content_margin_all(x6_scale);
	color_button_pressed_style->set_content_margin_all(x6_scale);
	color_button_disabled_style->set_content_margin_all(x6_scale);
	color_picker_popup_panel_style->set_content_margin_all(x6_scale);
	p_theme->set_constant("minimum_grab_thickness", "SplitContainer", x6_scale);
	p_theme->set_constant("minimum_grab_thickness", "HSplitContainer", x6_scale);
	p_theme->set_constant("minimum_grab_thickness", "VSplitContainer", x6_scale);
	p_theme->set_constant("separation", "ColorPickerVBox", x6_scale);
	p_theme->set_constant("separation", "ColorPickerHBox", x6_scale);
	p_theme->set_constant("separation", "SplitContainer", x6_scale);
	p_theme->set_constant("separation", "HSplitContainer", x6_scale);
	p_theme->set_constant("separation", "VSplitContainer", x6_scale);
}

void update_theme_colors(Ref<Theme> &p_theme, Color p_base_color, Color p_accent_color, float p_contrast, float p_base2_contrast, float p_base3_contrast, float p_base4_contrast, float p_accent2_contrast, float p_bg_contrast) {
	Color contrasted_color = contrast_color(p_base_color, p_contrast);
	Color base_color2 = p_base_color.lerp(contrasted_color, p_base2_contrast);
	Color base_color3 = p_base_color.lerp(contrasted_color, p_base3_contrast);
	Color base_color4 = p_base_color.lerp(contrasted_color, p_base4_contrast);
	Color accent_color2 = p_accent_color.lerp(p_base_color, p_accent2_contrast);
	Color bg_color = p_base_color.lerp(contrasted_color, p_bg_contrast);

	button_pressed_style->set_bg_color(p_base_color);
	color_button_pressed_style->set_bg_color(p_base_color);
	color_picker_button_pressed_style->set_bg_color(p_base_color);
	menu_button_pressed_style->set_bg_color(p_base_color);
	tab_selected_style->set_border_color(p_accent_color);
	color_mode_button_pressed_style->set_border_color(p_accent_color);
	tab_focus_style->set_border_color(p_accent_color);
	graph_panel_selected_style->set_border_color(p_accent_color);
	graph_frame_title_selected_style->set_border_color(p_accent_color);
	graph_title_selected_style->set_border_color(p_accent_color);
	button_focus_style->set_bg_color(p_accent_color);
	color_button_focus_style->set_bg_color(p_accent_color);
	button_focus_style->set_border_color(p_accent_color);
	color_button_focus_style->set_border_color(p_accent_color);
	menu_button_focus_style->set_bg_color(p_accent_color);
	menu_button_focus_style->set_border_color(p_accent_color);
	progress_fill_style->set_bg_color(p_accent_color);
	grabber_highlight_style->set_bg_color(p_accent_color);
	p_theme->set_color("drop_mark_color", "TabContainer", p_accent_color);
	p_theme->set_color("drop_mark_color", "TabBar", p_accent_color);
	p_theme->set_color("icon_pressed_color", "Button", p_accent_color);
	p_theme->set_color("icon_hover_pressed_color", "Button", p_accent_color);
	p_theme->set_color("font_pressed_color", "Button", p_accent_color);
	p_theme->set_color("font_pressed_color", "LinkButton", p_accent_color);
	p_theme->set_color("font_hover_pressed_color", "Button", p_accent_color);
	p_theme->set_color("font_hover_pressed_color", "LinkButton", p_accent_color);
	p_theme->set_color("grabber_pressed", "SplitContainer", p_accent_color);
	p_theme->set_color("grabber_pressed", "HSplitContainer", p_accent_color);
	p_theme->set_color("grabber_pressed", "VSplitContainer", p_accent_color);
	p_theme->set_color("clear_button_color_pressed", "LineEdit", p_accent_color);
	p_theme->set_color("font_pressed_color", "MenuBar", p_accent_color);
	p_theme->set_color("font_pressed_color", "OptionButton", p_accent_color);
	p_theme->set_color("font_pressed_color", "MenuButton", p_accent_color);
	p_theme->set_color("font_pressed_color", "CheckBox", p_accent_color);
	p_theme->set_color("font_hover_pressed_color", "CheckBox", p_accent_color);
	p_theme->set_color("font_pressed_color", "CheckButton", p_accent_color);
	p_theme->set_color("font_hover_pressed_color", "CheckButton", p_accent_color);
	p_theme->set_color("down_pressed_icon_modulate", "SpinBox", p_accent_color);
	p_theme->set_color("up_pressed_icon_modulate", "SpinBox", p_accent_color);
	p_theme->set_color("selection_stroke", "GraphEdit", p_accent_color);
	p_theme->set_color("button_icon_pressed", "FoldableContainer", p_accent_color);
	p_theme->set_color("word_highlighted_color", "TextEdit", base_color4);
	p_theme->set_color("word_highlighted_color", "CodeEdit", base_color4);
	p_theme->set_color("current_line_color", "TextEdit", bg_color);
	p_theme->set_color("current_line_color", "CodeEdit", bg_color);
	p_theme->set_color("search_result_color", "TextEdit", accent_color2);
	p_theme->set_color("search_result_color", "CodeEdit", accent_color2);
	p_theme->set_color("selection_color", "LineEdit", base_color3);
	p_theme->set_color("selection_color", "TextEdit", base_color3);
	p_theme->set_color("selection_color", "CodeEdit", base_color3);
	p_theme->set_color("selection_color", "RichTextLabel", base_color3);
	popup_panel_style->set_bg_color(base_color3);
	color_picker_popup_panel_style->set_bg_color(base_color3);
	button_hover_style->set_bg_color(base_color2);
	color_button_hover_style->set_bg_color(base_color2);
	tab_hovered_style->set_bg_color(base_color2);
	color_mode_button_hovered_style->set_bg_color(base_color2);
	color_picker_button_hovered_style->set_bg_color(base_color2);
	menu_button_hover_style->set_bg_color(base_color2);
	slider_style->set_bg_color(base_color2);
	picker_slider_style->set_bg_color(base_color2);
	foldable_title_hover_style->set_bg_color(base_color2);
	swatches_foldable_title_hover_style->set_bg_color(base_color2);
	foldable_title_collapsed_hover_style->set_bg_color(base_color2);
	swatches_foldable_title_collapsed_hover_style->set_bg_color(base_color2);
	panel_style->set_bg_color(bg_color);
	graph_panel_style->set_bg_color(bg_color);
	graph_panel_selected_style->set_bg_color(bg_color);
	embedded_style->set_bg_color(base_color3);
	embedded_unfocused_style->set_bg_color(base_color4);
	tab_panel_style->set_bg_color(base_color3);
	color_sliders_panel_style->set_bg_color(base_color3);
	tab_selected_style->set_bg_color(base_color3);
	color_mode_button_pressed_style->set_bg_color(base_color3);
	popup_hover_style->set_bg_color(base_color2);
	progress_background_style->set_bg_color(base_color3);
	foldable_panel_style->set_bg_color(base_color3);
	swatches_foldable_panel_style->set_bg_color(base_color3);
	tab_unselected_style->set_bg_color(base_color4);
	button_normal_style->set_bg_color(base_color4);
	color_button_normal_style->set_bg_color(base_color4);
	color_mode_button_normal_style->set_bg_color(base_color4);
	color_picker_button_normal_style->set_bg_color(base_color4);
	menu_button_normal_style->set_bg_color(base_color4);
	h_scroll_style->set_bg_color(base_color4);
	v_scroll_style->set_bg_color(base_color4);
	foldable_title_style->set_bg_color(base_color4);
	swatches_foldable_title_style->set_bg_color(base_color4);
	foldable_title_collapsed_style->set_bg_color(base_color4);
	swatches_foldable_title_collapsed_style->set_bg_color(base_color4);
	graph_title_style->set_bg_color(base_color4);
	graph_frame_title_style->set_bg_color(base_color4);
	graph_title_selected_style->set_bg_color(p_base_color);
	graph_frame_title_selected_style->set_bg_color(p_base_color);
	Color transparent_base_color4 = base_color4;
	transparent_base_color4.a = 0.4;
	tab_disabled_style->set_bg_color(transparent_base_color4);
	button_disabled_style->set_bg_color(transparent_base_color4);
	color_button_disabled_style->set_bg_color(transparent_base_color4);
	color_picker_button_disabled_style->set_bg_color(transparent_base_color4);
	menu_button_disabled_style->set_bg_color(transparent_base_color4);

	float v = p_base_color.get_luminance() <= 0.5 ? 1.f : 0.f;
	p_theme->set_color("grid_minor", "GraphEdit", Color(v, v, v, 0.05));
	p_theme->set_color("grid_major", "GraphEdit", Color(v, v, v, 0.2));
	p_theme->set_color("activity", "GraphEdit", Color(v, v, v));
	p_theme->set_color("connection_hover_tint_color", "GraphEdit", Color(1.f - v, 1.f - v, 1.f - v, 0.3));
	p_theme->set_color("connection_valid_target_tint_color", "GraphEdit", Color(v, v, v, 0.4));
}

void update_theme_font(Ref<Theme> &p_theme, Ref<Font> p_font) {
	if (p_font.is_valid()) {
		if (p_font->is_class("FontVariation")) {
			custom_font = p_font;
			using_custom_font_variation = true;

			Ref<Font> base_font = custom_font->get_base_font();
			bold_font->set_base_font(base_font);
			bold_italics_font->set_base_font(base_font);
			italics_font->set_base_font(base_font);
		} else {
			custom_font = ThemeDB::get_singleton()->get_fallback_font()->duplicate();
			custom_font->set_base_font(p_font);
			using_custom_font_variation = false;

			bold_font->set_base_font(p_font);
			bold_italics_font->set_base_font(p_font);
			italics_font->set_base_font(p_font);
		}
		p_theme->set_default_font(custom_font);

	} else {
		if (custom_font.is_valid()) {
			custom_font = Ref<FontVariation>();
			using_custom_font_variation = false;

			Ref<Font> base_font = fallback_font->get_base_font();
			bold_font->set_base_font(base_font);
			bold_italics_font->set_base_font(base_font);
			italics_font->set_base_font(base_font);
		}
		p_theme->set_default_font(Ref<Font>());
	}
}

void update_font_subpixel_positioning(TextServer::SubpixelPositioning p_font_subpixel_positioning) {
	Ref<FontFile> base_font = fallback_font->get_base_font();
	base_font->set_subpixel_positioning(p_font_subpixel_positioning);
}

void update_font_antialiasing(TextServer::FontAntialiasing p_font_antialiasing) {
	Ref<FontFile> base_font = fallback_font->get_base_font();
	base_font->set_antialiasing(p_font_antialiasing);
}

void update_font_lcd_subpixel_layout(TextServer::FontLCDSubpixelLayout p_font_lcd_subpixel_layout) {
	Ref<FontFile> base_font = fallback_font->get_base_font();
	base_font->set_lcd_subpixel_layout(p_font_lcd_subpixel_layout);
}

void update_font_hinting(TextServer::Hinting p_font_hinting) {
	Ref<FontFile> base_font = fallback_font->get_base_font();
	base_font->set_hinting(p_font_hinting);
}

void update_font_msdf(bool p_font_msdf) {
	Ref<FontFile> base_font = fallback_font->get_base_font();
	base_font->set_multichannel_signed_distance_field(p_font_msdf);
}

void update_font_generate_mipmaps(bool p_font_generate_mipmaps) {
	Ref<FontFile> base_font = fallback_font->get_base_font();
	base_font->set_generate_mipmaps(p_font_generate_mipmaps);
}

void make_default_theme(Ref<Font> p_font, float p_scale, TextServer::SubpixelPositioning p_font_subpixel, TextServer::Hinting p_font_hinting, TextServer::FontAntialiasing p_font_antialiasing, TextServer::FontLCDSubpixelLayout p_font_lcd_subpixel_layout, bool p_font_msdf, bool p_font_generate_mipmaps, Color p_base_color, Color p_accent_color, Color p_font_color, Color p_font_outline_color, float p_contrast, float p_base2_contrast, float p_base3_contrast, float p_base4_contrast, float p_accent2_contrast, float p_bg_contrast, int p_margin, int p_padding, int p_border_width, int p_corner_radius, int p_font_size, int p_font_outline, float p_font_embolden, int p_font_spacing_glyph, int p_font_spacing_space, int p_font_spacing_top, int p_font_spacing_bottom) {
	float scale = CLAMP(p_scale, 0.5, 8.0);

	Ref<Theme> t;
	t.instantiate();

	Ref<FontFile> base_font;
	base_font.instantiate();
	base_font->set_data_ptr(_font_OpenSans_SemiBold, _font_OpenSans_SemiBold_size);

	fallback_font.instantiate();
	fallback_font->set_base_font(base_font);

	ThemeDB::get_singleton()->set_fallback_font(fallback_font);

	bold_font.instantiate();
	bold_italics_font.instantiate();
	italics_font.instantiate();

	bold_italics_font->set_variation_transform(Transform2D(1.0, 0.2, 0.0, 1.0, 0.0, 0.0));
	italics_font->set_variation_transform(Transform2D(1.0, 0.2, 0.0, 1.0, 0.0, 0.0));

	t->set_default_base_scale(scale);

	t->set_type_variation("FlatButton", "Button");
	t->set_type_variation("FlatMenuButton", "MenuButton");
	t->set_type_variation("GraphNodeTitleLabel", "Label");
	t->set_type_variation("GraphFrameTitleLabel", "Label");
	t->set_type_variation("TooltipPanel", "PopupPanel");
	t->set_type_variation("TooltipLabel", "Label");
	t->set_type_variation("FlatPanel", "PanelContainer");
	t->set_type_variation("HeaderSmall", "Label");
	t->set_type_variation("HeaderMedium", "Label");
	t->set_type_variation("HeaderLarge", "Label");
	t->set_type_variation("PickerHSlider", "HSlider");
	t->set_type_variation("PickerModeButton", "Button");
	t->set_type_variation("PickerSlidersPanel", "PanelContainer");
	t->set_type_variation("PickerButton", "Button");
	t->set_type_variation("PickerMenuButton", "MenuButton");
	t->set_type_variation("PickerLineEdit", "LineEdit");
	t->set_type_variation("ColorPickerHBox", "HBoxContainer");
	t->set_type_variation("ColorPickerVBox", "VBoxContainer");
	t->set_type_variation("ColorPickerGrid", "GridContainer");
	t->set_type_variation("SwatchesFoldableContainer", "FoldableContainer");

	Ref<StyleBoxEmpty> empty_style(memnew(StyleBoxEmpty));

	panel_style.instantiate();
	popup_panel_style.instantiate();
	color_picker_popup_panel_style.instantiate();
	tab_panel_style.instantiate();
	color_sliders_panel_style.instantiate();
	tab_selected_style.instantiate();
	tab_unselected_style.instantiate();
	tab_hovered_style.instantiate();
	tab_disabled_style.instantiate();
	tab_focus_style.instantiate();
	button_normal_style.instantiate();
	button_hover_style.instantiate();
	button_pressed_style.instantiate();
	button_disabled_style.instantiate();
	button_focus_style.instantiate();
	color_button_normal_style.instantiate();
	color_button_hover_style.instantiate();
	color_button_pressed_style.instantiate();
	color_button_disabled_style.instantiate();
	color_button_focus_style.instantiate();
	menu_button_normal_style.instantiate();
	menu_button_hover_style.instantiate();
	menu_button_pressed_style.instantiate();
	menu_button_disabled_style.instantiate();
	menu_button_focus_style.instantiate();
	popup_hover_style.instantiate();
	progress_background_style.instantiate();
	progress_fill_style.instantiate();
	grabber_style.instantiate();
	grabber_highlight_style.instantiate();
	slider_style.instantiate();
	picker_slider_style.instantiate();
	h_scroll_style.instantiate();
	v_scroll_style.instantiate();
	foldable_panel_style.instantiate();
	foldable_title_style.instantiate();
	foldable_title_collapsed_style.instantiate();
	foldable_title_hover_style.instantiate();
	foldable_title_collapsed_hover_style.instantiate();
	swatches_foldable_panel_style.instantiate();
	swatches_foldable_title_style.instantiate();
	swatches_foldable_title_collapsed_style.instantiate();
	swatches_foldable_title_hover_style.instantiate();
	swatches_foldable_title_collapsed_hover_style.instantiate();
	h_separator_style.instantiate();
	v_separator_style.instantiate();
	flat_button_normal.instantiate();
	embedded_style.instantiate();
	embedded_unfocused_style.instantiate();
	graph_title_style.instantiate();
	graph_frame_title_style.instantiate();
	graph_frame_title_selected_style.instantiate();
	graph_title_selected_style.instantiate();
	graph_panel_style.instantiate();
	graph_panel_selected_style.instantiate();
	color_mode_button_normal_style.instantiate();
	color_mode_button_hovered_style.instantiate();
	color_mode_button_pressed_style.instantiate();
	color_picker_button_normal_style.instantiate();
	color_picker_button_hovered_style.instantiate();
	color_picker_button_pressed_style.instantiate();
	color_picker_button_disabled_style.instantiate();

	update_theme_font(t, p_font);
	update_font_color(t, p_font_color);
	update_font_size(t, p_font_size);
	update_font_outline_color(t, p_font_outline_color);
	update_theme_margins(t, p_margin);
	update_theme_padding(t, p_padding);
	update_theme_corner_radius(t, p_corner_radius);
	update_theme_border_width(t, p_border_width);
	update_font_outline_size(t, p_font_outline);
	update_theme_border_padding(t, p_border_width + p_padding);
	update_theme_scale(t);
	update_theme_colors(t, p_base_color, p_accent_color, p_contrast, p_base2_contrast, p_base3_contrast, p_base4_contrast, p_accent2_contrast, p_bg_contrast);
	update_theme_icons(t, p_font_color, p_accent_color);
	update_font_embolden(p_font_embolden);
	update_font_spacing_glyph(p_font_spacing_glyph);
	update_font_spacing_space(p_font_spacing_space);
	update_font_spacing_top(p_font_spacing_top);
	update_font_spacing_bottom(p_font_spacing_bottom);
	update_font_subpixel_positioning(p_font_subpixel);
	update_font_lcd_subpixel_layout(p_font_lcd_subpixel_layout);
	update_font_antialiasing(p_font_antialiasing);
	update_font_hinting(p_font_hinting);
	update_font_msdf(p_font_msdf);
	update_font_generate_mipmaps(p_font_generate_mipmaps);

	t->set_stylebox(SceneStringName(panel), "Panel", panel_style);
	t->set_stylebox(SceneStringName(panel), "PanelContainer", panel_style);
	t->set_stylebox(SceneStringName(panel), "PopupMenu", popup_panel_style);
	t->set_stylebox(SceneStringName(panel), "PopupPanel", popup_panel_style);
	t->set_stylebox(SceneStringName(panel), "AcceptDialog", popup_panel_style);
	t->set_stylebox(SceneStringName(panel), "TabContainer", tab_panel_style);
	t->set_stylebox(SceneStringName(panel), "PickerSlidersPanel", color_sliders_panel_style);
	t->set_stylebox(CoreStringName(normal), "PickerModeButton", color_mode_button_normal_style);
	t->set_stylebox(SceneStringName(pressed), "PickerModeButton", color_mode_button_pressed_style);
	t->set_stylebox("hover_pressed", "PickerModeButton", color_mode_button_pressed_style);
	t->set_stylebox("hover", "PickerModeButton", color_mode_button_hovered_style);
	t->set_stylebox("disabled", "PickerModeButton", empty_style);
	t->set_stylebox("focus", "PickerModeButton", tab_focus_style);
	t->set_stylebox("tab_selected", "TabContainer", tab_selected_style);
	t->set_stylebox("tab_unselected", "TabContainer", tab_unselected_style);
	t->set_stylebox("tab_hovered", "TabContainer", tab_unselected_style);
	t->set_stylebox("tab_disabled", "TabContainer", tab_disabled_style);
	t->set_stylebox("tab_focus", "TabContainer", tab_focus_style);
	t->set_stylebox("tab_selected", "TabBar", tab_selected_style);
	t->set_stylebox("tab_unselected", "TabBar", tab_unselected_style);
	t->set_stylebox("tab_hovered", "TabBar", tab_unselected_style);
	t->set_stylebox("tab_disabled", "TabBar", tab_disabled_style);
	t->set_stylebox("tab_focus", "TabBar", tab_focus_style);
	t->set_stylebox(CoreStringName(normal), "Button", button_normal_style);
	t->set_stylebox("hover", "Button", button_hover_style);
	t->set_stylebox(SceneStringName(pressed), "Button", button_pressed_style);
	t->set_stylebox("hover_pressed", "Button", button_pressed_style);
	t->set_stylebox("disabled", "Button", button_disabled_style);
	t->set_stylebox("focus", "Button", button_focus_style);
	t->set_stylebox(CoreStringName(normal), "LineEdit", button_normal_style);
	t->set_stylebox("read_only", "LineEdit", button_disabled_style);
	t->set_stylebox("focus", "LineEdit", button_focus_style);
	t->set_stylebox(CoreStringName(normal), "TextEdit", button_normal_style);
	t->set_stylebox("read_only", "TextEdit", button_disabled_style);
	t->set_stylebox("focus", "TextEdit", button_focus_style);
	t->set_stylebox(CoreStringName(normal), "MenuButton", menu_button_normal_style);
	t->set_stylebox("hover", "MenuButton", menu_button_hover_style);
	t->set_stylebox(SceneStringName(pressed), "MenuButton", menu_button_pressed_style);
	t->set_stylebox("hover_pressed", "MenuButton", menu_button_pressed_style);
	t->set_stylebox("disabled", "MenuButton", menu_button_disabled_style);
	t->set_stylebox("focus", "MenuButton", menu_button_focus_style);
	t->set_stylebox("focus", "OptionButton", button_focus_style);
	t->set_stylebox("hover", "PopupMenu", popup_hover_style);
	t->set_stylebox("background", "ProgressBar", progress_background_style);
	t->set_stylebox("fill", "ProgressBar", progress_fill_style);
	t->set_stylebox("grabber_area", "HSlider", grabber_style);
	t->set_stylebox("grabber_area", "VSlider", grabber_style);
	t->set_stylebox("grabber_area_highlight", "HSlider", grabber_highlight_style);
	t->set_stylebox("grabber_area_highlight", "VSlider", grabber_highlight_style);
	t->set_stylebox("slider", "HSlider", slider_style);
	t->set_stylebox("slider", "VSlider", slider_style);
	t->set_stylebox("slider", "PickerHSlider", picker_slider_style);
	t->set_stylebox("grabber", "HScrollBar", grabber_style);
	t->set_stylebox("grabber", "VScrollBar", grabber_style);
	t->set_stylebox("grabber_highlight", "HScrollBar", grabber_highlight_style);
	t->set_stylebox("grabber_highlight", "VScrollBar", grabber_highlight_style);
	t->set_stylebox("grabber_pressed", "HScrollBar", grabber_highlight_style);
	t->set_stylebox("grabber_pressed", "VScrollBar", grabber_highlight_style);
	t->set_stylebox("scroll", "HScrollBar", h_scroll_style);
	t->set_stylebox("scroll", "VScrollBar", v_scroll_style);
	t->set_stylebox("focus", "FoldableContainer", button_focus_style);
	t->set_stylebox(SceneStringName(panel), "FoldableContainer", foldable_panel_style);
	t->set_stylebox("title_panel", "FoldableContainer", foldable_title_style);
	t->set_stylebox("title_collapsed_panel", "FoldableContainer", foldable_title_collapsed_style);
	t->set_stylebox("title_hover_panel", "FoldableContainer", foldable_title_hover_style);
	t->set_stylebox("title_collapsed_hover_panel", "FoldableContainer", foldable_title_collapsed_hover_style);
	t->set_stylebox(SceneStringName(panel), "SwatchesFoldableContainer", swatches_foldable_panel_style);
	t->set_stylebox("title_panel", "SwatchesFoldableContainer", swatches_foldable_title_style);
	t->set_stylebox("title_collapsed_panel", "SwatchesFoldableContainer", swatches_foldable_title_collapsed_style);
	t->set_stylebox("title_hover_panel", "SwatchesFoldableContainer", swatches_foldable_title_hover_style);
	t->set_stylebox("title_collapsed_hover_panel", "SwatchesFoldableContainer", swatches_foldable_title_collapsed_hover_style);
	t->set_stylebox("button_normal_style", "FoldableContainer", color_picker_button_normal_style);
	t->set_stylebox("button_hovered_style", "FoldableContainer", color_picker_button_hovered_style);
	t->set_stylebox("button_pressed_style", "FoldableContainer", color_picker_button_pressed_style);
	t->set_stylebox("button_disabled_style", "FoldableContainer", color_picker_button_disabled_style);
	t->set_stylebox("up_background_hovered", "SpinBox", button_hover_style);
	t->set_stylebox("down_background_hovered", "SpinBox", button_hover_style);
	t->set_stylebox("up_background_pressed", "SpinBox", button_pressed_style);
	t->set_stylebox("down_background_pressed", "SpinBox", button_pressed_style);
	t->set_stylebox("custom_button", "Tree", button_normal_style);
	t->set_stylebox("custom_button_hover", "Tree", button_hover_style);
	t->set_stylebox("custom_button_pressed", "Tree", button_pressed_style);
	t->set_stylebox("focus", "Tree", button_focus_style);
	t->set_stylebox(SceneStringName(panel), "Tree", button_normal_style);
	t->set_stylebox("selected", "Tree", popup_hover_style);
	t->set_stylebox("selected_focus", "Tree", popup_hover_style);
	t->set_stylebox("title_button_hover", "Tree", button_hover_style);
	t->set_stylebox("title_button_normal", "Tree", button_normal_style);
	t->set_stylebox("title_button_pressed", "Tree", button_pressed_style);
	t->set_stylebox("cursor", "Tree", button_focus_style);
	t->set_stylebox("cursor_unfocused", "Tree", button_focus_style);
	t->set_stylebox("button_pressed", "Tree", button_pressed_style);
	t->set_stylebox("labeled_separator_left", "PopupMenu", h_separator_style);
	t->set_stylebox("labeled_separator_right", "PopupMenu", h_separator_style);
	t->set_stylebox("separator", "PopupMenu", h_separator_style);
	t->set_stylebox("separator", "HSeparator", h_separator_style);
	t->set_stylebox("separator", "VSeparator", v_separator_style);
	t->set_stylebox(SceneStringName(panel), "FlatPanel", popup_panel_style);
	t->set_stylebox(CoreStringName(normal), "MenuBar", button_normal_style);
	t->set_stylebox("hover", "MenuBar", button_hover_style);
	t->set_stylebox(SceneStringName(pressed), "MenuBar", button_pressed_style);
	t->set_stylebox("disabled", "MenuBar", button_disabled_style);
	t->set_stylebox(CoreStringName(normal), "OptionButton", button_normal_style);
	t->set_stylebox("hover", "OptionButton", button_hover_style);
	t->set_stylebox(SceneStringName(pressed), "OptionButton", button_pressed_style);
	t->set_stylebox("disabled", "OptionButton", button_disabled_style);
	t->set_stylebox("normal_mirrored", "OptionButton", button_normal_style);
	t->set_stylebox("hover_mirrored", "OptionButton", button_hover_style);
	t->set_stylebox("pressed_mirrored", "OptionButton", button_pressed_style);
	t->set_stylebox("disabled_mirrored", "OptionButton", button_disabled_style);
	t->set_stylebox(CoreStringName(normal), "CheckBox", flat_button_normal);
	t->set_stylebox(SceneStringName(pressed), "CheckBox", flat_button_normal);
	t->set_stylebox("disabled", "CheckBox", flat_button_normal);
	t->set_stylebox("hover", "CheckBox", flat_button_normal);
	t->set_stylebox("hover_pressed", "CheckBox", flat_button_normal);
	t->set_stylebox("focus", "CheckBox", button_focus_style);
	t->set_stylebox("focus", "LinkButton", flat_button_normal);
	t->set_stylebox(CoreStringName(normal), "CheckButton", flat_button_normal);
	t->set_stylebox(SceneStringName(pressed), "CheckButton", flat_button_normal);
	t->set_stylebox("disabled", "CheckButton", flat_button_normal);
	t->set_stylebox("hover", "CheckButton", flat_button_normal);
	t->set_stylebox("hover_pressed", "CheckButton", flat_button_normal);
	t->set_stylebox("focus", "CheckButton", button_focus_style);
	t->set_stylebox(CoreStringName(normal), "Label", empty_style);
	t->set_stylebox(CoreStringName(normal), "GraphNodeTitleLabel", empty_style);
	t->set_stylebox(CoreStringName(normal), "GraphFrameTitleLabel", empty_style);
	t->set_stylebox("field_and_buttons_separator", "SpinBox", empty_style);
	t->set_stylebox("up_down_buttons_separator", "SpinBox", empty_style);
	t->set_stylebox(CoreStringName(normal), "ColorButton", color_button_normal_style);
	t->set_stylebox(SceneStringName(pressed), "ColorButton", color_button_pressed_style);
	t->set_stylebox("hover_pressed", "ColorButton", color_button_pressed_style);
	t->set_stylebox("hover", "ColorButton", color_button_hover_style);
	t->set_stylebox("disabled", "ColorButton", color_button_disabled_style);
	t->set_stylebox("focus", "ColorButton", color_button_focus_style);
	t->set_stylebox("focus", "RichTextLabel", button_focus_style);
	t->set_stylebox(CoreStringName(normal), "RichTextLabel", flat_button_normal);
	t->set_stylebox(CoreStringName(normal), "CodeEdit", button_normal_style);
	t->set_stylebox("focus", "CodeEdit", button_focus_style);
	t->set_stylebox("read_only", "CodeEdit", button_disabled_style);
	t->set_stylebox("completion", "CodeEdit", empty_style);
	t->set_stylebox(CoreStringName(normal), "FlatButton", flat_button_normal);
	t->set_stylebox("hover", "FlatButton", button_hover_style);
	t->set_stylebox(SceneStringName(pressed), "FlatButton", button_pressed_style);
	t->set_stylebox("disabled", "FlatButton", flat_button_normal);
	t->set_stylebox(CoreStringName(normal), "FlatMenuButton", flat_button_normal);
	t->set_stylebox("hover", "FlatMenuButton", flat_button_normal);
	t->set_stylebox(SceneStringName(pressed), "FlatMenuButton", button_pressed_style);
	t->set_stylebox("disabled", "FlatMenuButton", flat_button_normal);
	t->set_stylebox("scroll_focus", "HScrollBar", empty_style);
	t->set_stylebox("scroll_focus", "VScrollBar", empty_style);
	t->set_stylebox("up_background", "SpinBox", button_normal_style);
	t->set_stylebox("down_background", "SpinBox", button_normal_style);
	t->set_stylebox("up_background_disabled", "SpinBox", button_disabled_style);
	t->set_stylebox("down_background_disabled", "SpinBox", button_disabled_style);
	t->set_stylebox("tabbar_background", "TabContainer", empty_style);
	t->set_stylebox("button_pressed", "TabBar", button_pressed_style);
	t->set_stylebox("button_highlight", "TabBar", button_normal_style);
	t->set_stylebox(SceneStringName(panel), "ScrollContainer", empty_style);
	t->set_stylebox(SceneStringName(panel), "TooltipPanel", popup_panel_style);
	t->set_stylebox(SceneStringName(panel), "ItemList", panel_style);
	t->set_stylebox("focus", "ItemList", button_focus_style);
	t->set_stylebox("cursor", "ItemList", button_focus_style);
	t->set_stylebox("cursor_unfocused", "ItemList", button_focus_style);
	t->set_stylebox(SceneStringName(panel), "PopupDialog", popup_panel_style);
	t->set_stylebox("popup_panel", "ColorPickerButton", color_picker_popup_panel_style);
	t->set_stylebox("preset_fg", "ColorPresetButton", button_pressed_style);
	t->set_stylebox("hovered", "ItemList", button_hover_style);
	t->set_stylebox("selected", "ItemList", button_pressed_style);
	t->set_stylebox("selected_focus", "ItemList", button_pressed_style);
	t->set_stylebox("embedded_border", "Window", embedded_style);
	t->set_stylebox("embedded_unfocused_border", "Window", embedded_unfocused_style);
	t->set_stylebox("menu_panel", "GraphEdit", button_disabled_style);
	t->set_stylebox(SceneStringName(panel), "GraphEdit", tab_panel_style);
	t->set_stylebox("camera", "GraphEditMinimap", button_focus_style);
	t->set_stylebox(SceneStringName(panel), "GraphEditMinimap", tab_disabled_style);
	t->set_stylebox("node", "GraphEditMinimap", tab_unselected_style);
	t->set_stylebox("titlebar", "GraphFrame", graph_frame_title_style);
	t->set_stylebox("titlebar_selected", "GraphFrame", graph_frame_title_selected_style);
	t->set_stylebox(SceneStringName(panel), "GraphFrame", graph_panel_style);
	t->set_stylebox("panel_selected", "GraphFrame", graph_panel_selected_style);
	t->set_stylebox(SceneStringName(panel), "GraphNode", graph_panel_style);
	t->set_stylebox("panel_selected", "GraphNode", graph_panel_selected_style);
	t->set_stylebox("titlebar", "GraphNode", graph_title_style);
	t->set_stylebox("titlebar_selected", "GraphNode", graph_title_selected_style);
	t->set_stylebox("slot", "GraphNode", empty_style);
	t->set_stylebox(CoreStringName(normal), "PickerButton", color_picker_button_normal_style);
	t->set_stylebox("disabled", "PickerButton", color_picker_button_disabled_style);
	t->set_stylebox(SceneStringName(pressed), "PickerButton", color_picker_button_pressed_style);
	t->set_stylebox("hover_pressed", "PickerButton", color_picker_button_pressed_style);
	t->set_stylebox("hover", "PickerButton", color_picker_button_hovered_style);
	t->set_stylebox(CoreStringName(normal), "PickerMenuButton", color_picker_button_normal_style);
	t->set_stylebox("disabled", "PickerMenuButton", color_picker_button_normal_style);
	t->set_stylebox(SceneStringName(pressed), "PickerMenuButton", color_picker_button_pressed_style);
	t->set_stylebox("hover_pressed", "PickerMenuButton", color_picker_button_pressed_style);
	t->set_stylebox("hover", "PickerMenuButton", color_picker_button_hovered_style);
	t->set_stylebox("focus", "PickerMenuButton", button_focus_style);
	t->set_stylebox(CoreStringName(normal), "PickerLineEdit", color_picker_button_normal_style);
	t->set_stylebox("read_only", "PickerLineEdit", color_picker_button_normal_style);

	v_separator_style->set_vertical(true);
	tab_focus_style->set_draw_center(false);
	button_focus_style->set_draw_center(false);
	color_button_focus_style->set_draw_center(false);
	menu_button_focus_style->set_draw_center(false);

	t->set_font_size(SceneStringName(font_size), "Button", -1);
	t->set_font_size(SceneStringName(font_size), "MenuBar", -1);
	t->set_font_size(SceneStringName(font_size), "LinkButton", -1);
	t->set_font_size(SceneStringName(font_size), "OptionButton", -1);
	t->set_font_size(SceneStringName(font_size), "MenuButton", -1);
	t->set_font_size(SceneStringName(font_size), "CheckBox", -1);
	t->set_font_size(SceneStringName(font_size), "CheckButton", -1);
	t->set_font_size(SceneStringName(font_size), "Label", -1);
	t->set_font_size(SceneStringName(font_size), "LineEdit", -1);
	t->set_font_size(SceneStringName(font_size), "ProgressBar", -1);
	t->set_font_size(SceneStringName(font_size), "TextEdit", -1);
	t->set_font_size(SceneStringName(font_size), "CodeEdit", -1);
	t->set_font_size(SceneStringName(font_size), "PopupMenu", -1);
	t->set_font_size(SceneStringName(font_size), "GraphNodeTitleLabel", -1);
	t->set_font_size(SceneStringName(font_size), "Tree", -1);
	t->set_font_size(SceneStringName(font_size), "ItemList", -1);
	t->set_font_size(SceneStringName(font_size), "TabContainer", -1);
	t->set_font_size(SceneStringName(font_size), "TabBar", -1);
	t->set_font_size(SceneStringName(font_size), "TooltipLabel", -1);
	t->set_font_size(SceneStringName(font_size), "FoldableContainer", -1);
	t->set_font_size("title_font_size", "Window", -1);
	t->set_font_size("font_separator_size", "PopupMenu", -1);
	t->set_font_size("title_button_font_size", "Tree", -1);
	t->set_font_size("normal_font_size", "RichTextLabel", -1);
	t->set_font_size("bold_font_size", "RichTextLabel", -1);
	t->set_font_size("italics_font_size", "RichTextLabel", -1);
	t->set_font_size("bold_italics_font_size", "RichTextLabel", -1);
	t->set_font_size("mono_font_size", "RichTextLabel", -1);
	t->set_constant("scrollbar_margin_left", "Tree", -1);
	t->set_constant("scrollbar_margin_top", "Tree", -1);
	t->set_constant("scrollbar_margin_right", "Tree", -1);
	t->set_constant("scrollbar_margin_bottom", "Tree", -1);

	t->set_constant("icon_max_width", "Button", 0);
	t->set_constant("icon_max_width", "Tree", 0);
	t->set_constant("align_to_largest_stylebox", "Button", 0);
	t->set_constant("check_v_offset", "CheckBox", 0);
	t->set_constant("check_v_offset", "CheckButton", 0);
	t->set_constant("center_grabber", "HSlider", 0);
	t->set_constant("grabber_offset", "HSlider", 0);
	t->set_constant("center_grabber", "VSlider", 0);
	t->set_constant("grabber_offset", "VSlider", 0);
	t->set_constant("inner_item_margin_bottom", "Tree", 0);
	t->set_constant("inner_item_margin_top", "Tree", 0);
	t->set_constant("inner_item_margin_left", "Tree", 0);
	t->set_constant("inner_item_margin_right", "Tree", 0);
	t->set_constant("buttons_vertical_separation", "SpinBox", 0);
	t->set_constant("icon_max_width", "PopupMenu", 0);
	t->set_constant("parent_hl_line_margin", "Tree", 0);
	t->set_constant("icon_max_width", "TabContainer", 0);
	t->set_constant("icon_max_width", "TabBar", 0);
	t->set_constant("draw_guides", "Tree", 0);
	t->set_constant("side_margin", "TabContainer", 0);
	t->set_constant("modulate_arrow", "OptionButton", 0);
	t->set_constant("port_h_offset", "GraphNode", 0);
	t->set_constant(SceneStringName(line_separation), "RichTextLabel", 0);

	t->set_constant("autohide", "SplitContainer", 1);
	t->set_constant("autohide", "HSplitContainer", 1);
	t->set_constant("autohide", "VSplitContainer", 1);
	t->set_constant("draw_relationship_lines", "Tree", 1);
	t->set_constant("center_grabber", "PickerHSlider", 1);

	t->set_constant("minimum_character_width", "LineEdit", 4);
	t->set_constant("completion_scroll_width", "CodeEdit", 6);
	t->set_constant("completion_lines", "CodeEdit", 7);

	t->set_color("font_shadow_color", "Label", Color(0, 0, 0, 0));
	t->set_color("font_shadow_color", "GraphNodeTitleLabel", Color(0, 0, 0, 0));
	t->set_color("font_shadow_color", "GraphFrameTitleLabel", Color(0, 0, 0, 0));
	t->set_color("font_shadow_color", "TooltipLabel", Color(0, 0, 0, 0));
	t->set_color("font_shadow_color", "RichTextLabel", Color(0, 0, 0, 0));
	t->set_color("background_color", "TextEdit", Color(0, 0, 0, 0));
	t->set_color("background_color", "CodeEdit", Color(0, 0, 0, 0));
	t->set_color("table_odd_row_bg", "RichTextLabel", Color(0, 0, 0, 0));
	t->set_color("table_even_row_bg", "RichTextLabel", Color(0, 0, 0, 0));
	t->set_color("table_border", "RichTextLabel", Color(0, 0, 0, 0));

	t->set_color("completion_background_color", "CodeEdit", Color(0.17, 0.16, 0.2));
	t->set_color("completion_selected_color", "CodeEdit", Color(0.26, 0.26, 0.27));
	t->set_color("completion_existing_color", "CodeEdit", Color(0.87, 0.87, 0.87, 0.13));
	t->set_color("bookmark_color", "CodeEdit", Color(0.5, 0.64, 1, 0.8));
	t->set_color("breakpoint_color", "CodeEdit", Color(0.9, 0.29, 0.3));
	t->set_color("executing_line_color", "CodeEdit", Color(0.98, 0.89, 0.27));
	t->set_color("code_folding_color", "CodeEdit", Color(0.8, 0.8, 0.8, 0.8));
	t->set_color("folded_code_region_color", "CodeEdit", Color(0.68, 0.46, 0.77, 0.2));
	t->set_color("brace_mismatch_color", "CodeEdit", Color(1, 0.2, 0.2));
	t->set_color("line_number_color", "CodeEdit", Color(0.67, 0.67, 0.67, 0.4));
	t->set_color("line_length_guideline_color", "CodeEdit", Color(0.3, 0.5, 0.8, 0.1));

	embedded_style->set_content_margin_individual(10, 28, 10, 8);
	embedded_unfocused_style->set_content_margin_individual(10, 28, 10, 8);
	embedded_style->set_expand_margin_individual(8, 32, 8, 6);
	embedded_unfocused_style->set_expand_margin_individual(8, 32, 8, 6);

	t->set_font("bold_font", "RichTextLabel", bold_font);
	t->set_font("italics_font", "RichTextLabel", italics_font);
	t->set_font("bold_italics_font", "RichTextLabel", bold_italics_font);
	t->set_font(SceneStringName(font), "TabBar", Ref<Font>());
	t->set_font(SceneStringName(font), "TabContainer", Ref<Font>());
	t->set_font(SceneStringName(font), "ItemList", Ref<Font>());
	t->set_font(SceneStringName(font), "Tree", Ref<Font>());
	t->set_font(SceneStringName(font), "GraphNodeTitleLabel", Ref<Font>());
	t->set_font(SceneStringName(font), "PopupMenu", Ref<Font>());
	t->set_font(SceneStringName(font), "CodeEdit", Ref<Font>());
	t->set_font(SceneStringName(font), "TextEdit", Ref<Font>());
	t->set_font(SceneStringName(font), "ProgressBar", Ref<Font>());
	t->set_font(SceneStringName(font), "LineEdit", Ref<Font>());
	t->set_font(SceneStringName(font), "Label", Ref<Font>());
	t->set_font(SceneStringName(font), "CheckButton", Ref<Font>());
	t->set_font(SceneStringName(font), "CheckBox", Ref<Font>());
	t->set_font(SceneStringName(font), "MenuButton", Ref<Font>());
	t->set_font(SceneStringName(font), "OptionButton", Ref<Font>());
	t->set_font(SceneStringName(font), "LinkButton", Ref<Font>());
	t->set_font(SceneStringName(font), "MenuBar", Ref<Font>());
	t->set_font(SceneStringName(font), "Button", Ref<Font>());
	t->set_font(SceneStringName(font), "FoldableContainer", Ref<Font>());
	t->set_font(SceneStringName(font), "TooltipLabel", Ref<Font>());
	t->set_font("title_button_font", "Tree", Ref<Font>());
	t->set_font("font_separator", "PopupMenu", Ref<Font>());
	t->set_font("title_font", "Window", Ref<Font>());
	t->set_font("normal_font", "RichTextLabel", Ref<Font>());
	t->set_font("mono_font", "RichTextLabel", Ref<Font>());

	ThemeDB::get_singleton()->set_default_theme(t);
	ThemeDB::get_singleton()->set_fallback_base_scale(scale);
	ThemeDB::get_singleton()->set_fallback_stylebox(empty_style);
	ThemeDB::get_singleton()->set_fallback_font_size(p_font_size * scale);
}

void finalize_default_theme() {
	icons.clear();

	panel_style.unref();
	popup_panel_style.unref();
	color_picker_popup_panel_style.unref();
	tab_panel_style.unref();
	color_sliders_panel_style.unref();
	tab_selected_style.unref();
	tab_unselected_style.unref();
	tab_hovered_style.unref();
	tab_disabled_style.unref();
	tab_focus_style.unref();
	button_normal_style.unref();
	button_hover_style.unref();
	button_pressed_style.unref();
	button_disabled_style.unref();
	button_focus_style.unref();
	color_button_normal_style.unref();
	color_button_hover_style.unref();
	color_button_pressed_style.unref();
	color_button_disabled_style.unref();
	color_button_focus_style.unref();
	menu_button_normal_style.unref();
	menu_button_hover_style.unref();
	menu_button_pressed_style.unref();
	menu_button_disabled_style.unref();
	menu_button_focus_style.unref();
	popup_hover_style.unref();
	progress_background_style.unref();
	progress_fill_style.unref();
	grabber_style.unref();
	grabber_highlight_style.unref();
	slider_style.unref();
	picker_slider_style.unref();
	h_scroll_style.unref();
	v_scroll_style.unref();
	foldable_panel_style.unref();
	foldable_title_style.unref();
	foldable_title_collapsed_style.unref();
	foldable_title_hover_style.unref();
	foldable_title_collapsed_hover_style.unref();
	swatches_foldable_panel_style.unref();
	swatches_foldable_title_style.unref();
	swatches_foldable_title_collapsed_style.unref();
	swatches_foldable_title_hover_style.unref();
	swatches_foldable_title_collapsed_hover_style.unref();
	h_separator_style.unref();
	v_separator_style.unref();
	flat_button_normal.unref();
	embedded_style.unref();
	embedded_unfocused_style.unref();
	graph_title_style.unref();
	graph_frame_title_style.unref();
	graph_frame_title_selected_style.unref();
	graph_title_selected_style.unref();
	graph_panel_style.unref();
	graph_panel_selected_style.unref();
	color_mode_button_normal_style.unref();
	color_mode_button_hovered_style.unref();
	color_mode_button_pressed_style.unref();
	color_picker_button_normal_style.unref();
	color_picker_button_hovered_style.unref();
	color_picker_button_pressed_style.unref();
	color_picker_button_disabled_style.unref();

	if (custom_font.is_valid()) {
		custom_font.unref();
	}
	fallback_font.unref();
	bold_font.unref();
	bold_italics_font.unref();
	italics_font.unref();
}
