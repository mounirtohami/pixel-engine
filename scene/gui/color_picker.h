/**************************************************************************/
/*  color_picker.h                                                        */
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

#ifndef COLOR_PICKER_H
#define COLOR_PICKER_H

#include "scene/gui/box_container.h"
#include "scene/gui/color_button.h"
#include "scene/gui/popup.h"

class AspectRatioContainer;
class Button;
class ColorMode;
class ColorModeRGB;
class ColorModeHSV;
class ColorModeRAW;
class ColorModeOKHSL;
class Control;
class FoldableContainer;
class GridContainer;
class HSlider;
class Label;
class LineEdit;
class MenuButton;
class PanelContainer;
class PopupMenu;
class SpinBox;
class StyleBox;
class TextureRect;
class StyleBoxFlat;
class FileDialog;

class ColorPicker : public VBoxContainer {
	GDCLASS(ColorPicker, VBoxContainer)

	// These classes poke into theme items for their internal logic.
	friend class ColorModeRGB;
	friend class ColorModeHSV;
	friend class ColorModeRAW;
	friend class ColorModeOKHSL;

public:
	enum ColorModeType {
		MODE_RGB,
		MODE_HSV,
		MODE_RAW,
		MODE_OKHSL,

		MODE_MAX
	};

	enum PickerShapeType {
		SHAPE_HSV_RECTANGLE,
		SHAPE_HSV_WHEEL,
		SHAPE_VHS_CIRCLE,
		SHAPE_OKHSL_CIRCLE,
		SHAPE_NONE,

		SHAPE_MAX
	};

	static const int SLIDER_COUNT = 3;
	static PackedColorArray swatches;

private:
	static inline Ref<Shader> wheel_shader;
	static inline Ref<Shader> circle_shader;
	static inline Ref<Shader> circle_ok_color_shader;

	int current_slider_count = SLIDER_COUNT;
	static const int MODE_BUTTON_COUNT = 4;

	Vector<ColorMode *> modes;

	Popup *picker_window = nullptr;
	TextureRect *picker_texture_zoom = nullptr;
	Panel *picker_preview = nullptr;
	Panel *picker_preview_color = nullptr;
	Ref<StyleBoxFlat> picker_preview_style_box;
	Ref<StyleBoxFlat> picker_preview_style_box_color;

	// Legacy color picking.
	TextureRect *picker_texture_rect = nullptr;
	Color picker_color;
	FileDialog *file_dialog = nullptr;
	Button *menu_btn = nullptr;
	PopupMenu *options_menu = nullptr;

	HBoxContainer *hb_edit = nullptr;
	Control *uv_edit = nullptr;
	Control *w_edit = nullptr;
	AspectRatioContainer *wheel_edit = nullptr;
	MarginContainer *wheel_margin = nullptr;
	Ref<ShaderMaterial> wheel_mat;
	Ref<ShaderMaterial> circle_mat;
	Control *wheel = nullptr;
	Control *wheel_uv = nullptr;
	TextureRect *sample = nullptr;
	Button *btn_pick = nullptr;
	PopupMenu *shape_popup = nullptr;
	PopupMenu *mode_popup = nullptr;
	MenuButton *btn_shape = nullptr;
	HBoxContainer *mode_hbc = nullptr;
	HBoxContainer *sample_hbc = nullptr;
	GridContainer *slider_gc = nullptr;
	HBoxContainer *hex_hbc = nullptr;
	VBoxContainer *sliders_vbc = nullptr;
	Button *mode_btns[MODE_BUTTON_COUNT];
	HSlider *sliders[SLIDER_COUNT];
	SpinBox *values[SLIDER_COUNT];
	Label *labels[SLIDER_COUNT];
	Button *text_type = nullptr;
	LineEdit *c_text = nullptr;
	HSlider *alpha_slider = nullptr;
	SpinBox *alpha_value = nullptr;
	Label *alpha_label = nullptr;
	FoldableContainer *swatches_foldable = nullptr;
	HBoxContainer *swatches_hbc = nullptr;
	Ref<ButtonGroup> swatches_group;

	bool edit_alpha = true;
	Size2i ms;
	bool text_is_constructor = false;
	PickerShapeType current_shape = SHAPE_HSV_RECTANGLE;
	ColorModeType current_mode = MODE_RGB;
	bool colorize_sliders = true;

	Color color;
	Color old_color;
	bool is_picking_color = false;

	bool display_old_color = false;
	bool deferred_mode_enabled = false;
	bool updating = true;
	bool changing_color = false;
	bool spinning = false;
	bool color_modes_visible = true;
	bool sampler_visible = true;
	bool sliders_visible = true;
	bool hex_visible = true;
	bool swatches_visible = true;
	bool can_add_swatches = true;
	bool can_pick_screen_color = true;
	bool line_edit_mouse_release = false;
	bool text_changed = false;
	bool currently_dragging = false;

	float h = 0.0;
	float s = 0.0;
	float v = 0.0;

	float ok_hsl_h = 0.0;
	float ok_hsl_s = 0.0;
	float ok_hsl_l = 0.0;

	float wheel_radius = 0.0;
	Color last_color;

	struct ThemeCache {
		int sv_height = 0;
		int sv_width = 0;
		int h_width = 0;
		int sample_height = 0;
		int expand_shape = 0;
		int swatch_size = 0;

		Ref<Texture2D> screen_picker;
		Ref<Texture2D> expanded_arrow;
		Ref<Texture2D> folded_arrow;
		Ref<Texture2D> add_preset;

		Ref<Texture2D> shape_rect;
		Ref<Texture2D> shape_rect_wheel;
		Ref<Texture2D> shape_circle;

		Ref<Texture2D> sample_bg;
		Ref<Texture2D> sample_revert;
		Ref<Texture2D> overbright_indicator;
		Ref<Texture2D> picker_cursor;
		Ref<Texture2D> picker_cursor_bg;
		Ref<Texture2D> color_hue;
		Ref<Texture2D> color_okhsl_hue;
		Ref<Texture2D> hex_icon;
		Ref<Texture2D> hex_code_icon;
	} theme_cache;

	void _copy_color_to_hsv();
	void _copy_hsv_to_color();

	void _html_submitted(const String &p_html);
	void _slider_drag_started();
	void _slider_value_changed();
	void _slider_drag_ended();
	void _update_controls();
	void _update_color(bool p_update_sliders = true);
	void _update_text_value();
	void _text_type_toggled();
	void _sample_input(const Ref<InputEvent> &p_event);
	void _sample_draw();
	void _hsv_draw(int p_which, Control *c);
	void _slider_draw(int p_which);

	void _uv_input(const Ref<InputEvent> &p_event, Control *c);
	void _w_input(const Ref<InputEvent> &p_event);
	void _slider_or_spin_input(const Ref<InputEvent> &p_event);
	void _line_edit_input(const Ref<InputEvent> &p_event);
	void _text_changed(const String &p_new_text);
	void _html_focus_exit();
	void _pick_button_pressed();
	void _target_gui_input(const Ref<InputEvent> &p_event);
	void _pick_finished();
	// Legacy color picking.
	void _pick_button_pressed_legacy();
	void _picker_texture_input(const Ref<InputEvent> &p_event);

	void _swatches_folding_changed(bool p_folded);
	void _add_swatch_button_pressed();
	void _on_swatch_button_pressed(const Color &p_color);
	void _on_swatch_button_right_pressed(int p_index);
	void _update_selected_swatch();

protected:
	virtual void _update_theme_item_cache() override;

	void _notification(int);
	static void _bind_methods();

public:
	HSlider *get_slider(int idx);
	Vector<float> get_active_slider_values();

	static void init_shaders();
	static void finish_shaders();

	void add_mode(ColorMode *p_mode);

	void set_edit_alpha(bool p_show);
	bool is_editing_alpha() const;

	void _set_pick_color(const Color &p_color, bool p_update_sliders);
	void set_pick_color(const Color &p_color);
	Color get_pick_color() const;
	void set_old_color(const Color &p_color);
	Color get_old_color() const;

	void set_wheel_radius(float p_wheel_radius);
	float get_wheel_radius() const;

	void set_display_old_color(bool p_enabled);
	bool is_displaying_old_color() const;

	void set_picker_shape(PickerShapeType p_shape);
	PickerShapeType get_picker_shape() const;

	void set_color_mode(ColorModeType p_mode);
	ColorModeType get_color_mode() const;

	void set_colorize_sliders(bool p_colorize_sliders);
	bool is_colorizing_sliders() const;

	void set_deferred_mode(bool p_enabled);
	bool is_deferred_mode() const;

	void set_modes_visible(bool p_visible);
	bool are_modes_visible() const;

	void set_sampler_visible(bool p_visible);
	bool is_sampler_visible() const;

	void set_can_pick_screen_color(bool p_enabled);
	bool get_can_pick_screen_color() const;

	void set_sliders_visible(bool p_visible);
	bool are_sliders_visible() const;

	void set_hex_visible(bool p_visible);
	bool is_hex_visible() const;

	void set_focus_on_line_edit();

	void set_swatches(const PackedColorArray &p_swatches);
	PackedColorArray get_swatches() const;

	void set_swatches_visible(bool p_visible);
	bool is_swatches_visible() const;

	void add_swatch_color(const Color &p_color);
	void remove_swatch_color(const Color &p_color);
	void remove_swatch_color_by_index(int p_index);

	void set_can_add_swatches(bool p_enabled);
	bool get_can_add_swatches() const;

	void clear_swatches();
	void update_swatches();

	ColorPicker();
	~ColorPicker();
};

VARIANT_ENUM_CAST(ColorPicker::PickerShapeType);
VARIANT_ENUM_CAST(ColorPicker::ColorModeType);

/////////////////////////////////////////////////////////

class ColorPickerPopupPanel : public PopupPanel {
	virtual void _input_from_window(const Ref<InputEvent> &p_event) override;
};

/////////////////////////////////////////////////////////

class ColorPickerButton : public ColorButton {
	GDCLASS(ColorPickerButton, ColorButton)

	// Initialization is now done deferred,
	// this improves performance in the inspector as the color picker
	// can be expensive to initialize.

	PopupPanel *popup = nullptr;
	ColorPicker *picker = nullptr;
	bool edit_alpha = true;

	struct ThemeCache {
		Ref<StyleBox> popup_panel_style;
	} theme_cache;

	void _about_to_popup();
	void _color_changed(const Color &p_color);
	void _modal_closed();

	virtual void pressed() override;

	void _update_picker();

protected:
	void _notification(int);
	static void _bind_methods();

public:
	void set_pick_color(const Color &p_color);
	Color get_pick_color() const;

	void set_edit_alpha(bool p_show);
	bool is_editing_alpha() const;

	ColorPicker *get_picker();
	PopupPanel *get_popup();

	ColorPickerButton();
};

class SwatchButton : public ColorButton {
	GDCLASS(SwatchButton, ColorButton)

protected:
	virtual void gui_input(const Ref<InputEvent> &p_event) override;
	virtual void toggled(bool p_toggled_on) override;
	static void _bind_methods();

public:
	SwatchButton();
};

#endif // COLOR_PICKER_H
