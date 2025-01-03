/**************************************************************************/
/*  split_container.cpp                                                   */
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

#include "split_container.h"

#include "scene/main/viewport.h"
#include "scene/theme/theme_db.h"

void SplitContainerDragger::gui_input(const Ref<InputEvent> &p_event) {
	ERR_FAIL_COND(p_event.is_null());

	Control *first = sc->_get_child(0);
	Control *second = sc->_get_child(1);

	Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid() && mb->get_button_index() == MouseButton::LEFT) {
		if (mb->is_pressed()) {
			sc->_compute_split_offset(true);
			dragging = true;
			drag_ofs = sc->split_offset;
			if (sc->vertical) {
				drag_from = get_transform().xform(mb->get_position()).y;
			} else {
				drag_from = get_transform().xform(mb->get_position()).x;
			}
		} else {
			dragging = false;
			get_viewport()->update_mouse_cursor_state();
		}
		queue_redraw();
	}

	Ref<InputEventMouseMotion> mm = p_event;
	if (mm.is_valid() && dragging) {
		Vector2i in_parent_pos = get_transform().xform(mm->get_position());
		int axis = sc->vertical ? 1 : 0;
		if (sc->collapse_mode == SplitContainer::COLLAPSE_FIRST || sc->collapse_mode == SplitContainer::COLLAPSE_ALL) {
			if (in_parent_pos[axis] >= first->get_combined_minimum_size()[axis] * 0.5) {
				if (!first->is_visible()) {
					first->show();
					sc->split_offset += first->get_combined_minimum_size()[axis];
				}
			} else {
				if (first->is_visible()) {
					first->hide();
					sc->split_offset -= first->get_combined_minimum_size()[axis];
				}
			}
		}
		if (sc->collapse_mode == SplitContainer::COLLAPSE_SECOND || sc->collapse_mode == SplitContainer::COLLAPSE_ALL) {
			if (in_parent_pos[axis] <= sc->get_size()[axis] - second->get_combined_minimum_size()[axis] * 0.5) {
				if (!second->is_visible()) {
					second->show();
					sc->split_offset -= second->get_combined_minimum_size()[axis];
				}
			} else {
				if (second->is_visible()) {
					second->hide();
					sc->split_offset += second->get_combined_minimum_size()[axis];
				}
			}
		}
		if (first->is_visible() && second->is_visible()) {
			if (!sc->vertical && is_layout_rtl()) {
				sc->split_offset = drag_ofs - (in_parent_pos.x - drag_from);
			} else {
				sc->split_offset = drag_ofs + ((sc->vertical ? in_parent_pos.y : in_parent_pos.x) - drag_from);
			}
		}
		sc->_compute_split_offset(true);
		sc->queue_sort();
		sc->emit_signal(SNAME("dragged"), sc->split_offset);
	}
}

Control::CursorShape SplitContainerDragger::get_cursor_shape(const Point2 &p_pos) const {
	return (sc->vertical ? CURSOR_VSPLIT : CURSOR_HSPLIT);
}

void SplitContainerDragger::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_MOUSE_ENTER: {
			mouse_inside = true;
			queue_redraw();
		} break;

		case NOTIFICATION_MOUSE_EXIT: {
			mouse_inside = false;
			queue_redraw();
		} break;

		case NOTIFICATION_DRAW: {
			if (!dragging && !mouse_inside && sc->theme_cache.autohide) {
				return;
			}
			int thickness = CLAMP(sc->theme_cache.grabber_thickness, 0, sc->is_vertical() ? get_size().y : get_size().x);
			if (thickness == 0) {
				return;
			}
			Color color = sc->theme_cache.grabber_normal;
			if (dragging) {
				color = sc->theme_cache.grabber_pressed;
			} else {
				if (mouse_inside) {
					color = sc->theme_cache.grabber_hovered;
				}
			}
			Point2 from;
			Point2 to;
			if (sc->is_vertical()) {
				from = Point2(0, get_size().y / 2.0);
				to = Point2(get_size().x, get_size().y / 2.0);
			} else {
				from = Point2(get_size().x / 2.0, 0);
				to = Point2(get_size().x / 2.0, get_size().y);
			}
			draw_line(from, to, color, thickness);
		} break;
	}
}

SplitContainerDragger::SplitContainerDragger(SplitContainer *p_sc) {
	sc = p_sc;
}

Control *SplitContainer::_get_child(int p_idx) const {
	if (p_idx < get_child_count(false)) {
		return Object::cast_to<Control>(get_child(p_idx, false));
	}
	return nullptr;
}

void SplitContainer::_compute_split_offset(bool p_clamp) {
	Control *first = _get_child(0);
	Control *second = _get_child(1);

	// Determine expanded children.
	bool first_expanded = (vertical ? first->get_v_size_flags() : first->get_h_size_flags()) & SIZE_EXPAND;
	bool second_expanded = (vertical ? second->get_v_size_flags() : second->get_h_size_flags()) & SIZE_EXPAND;

	// Compute the minimum size.
	int axis = vertical ? 1 : 0;
	int size = get_size()[axis];
	int ms_first = first->is_visible() ? first->get_combined_minimum_size()[axis] : 0;
	int ms_second = second->is_visible() ? second->get_combined_minimum_size()[axis] : 0;

	// Determine the separation between items.
	int sep = MAX(theme_cache.separation, theme_cache.minimum_grab_thickness);

	// Compute the wished separation_point.
	int wished_middle_sep = 0;
	int split_offset_with_collapse = split_offset;
	if (first_expanded && second_expanded) {
		float ratio = first->get_stretch_ratio() / (first->get_stretch_ratio() + second->get_stretch_ratio());
		wished_middle_sep = size * ratio - sep / 2 + split_offset_with_collapse;
	} else if (first_expanded) {
		wished_middle_sep = size - sep + split_offset_with_collapse;
	} else {
		wished_middle_sep = split_offset_with_collapse;
	}

	// Clamp the middle sep to acceptatble values.
	middle_sep = CLAMP(wished_middle_sep, ms_first, size - sep - ms_second);

	// Clamp the split_offset if requested.
	if (p_clamp) {
		split_offset -= wished_middle_sep - middle_sep;
	}
}

Size2 SplitContainer::get_minimum_size() const {
	Size2i ms;
	Control *first = _get_child(0);
	Control *second = _get_child(1);
	if (first && first->is_visible()) {
		ms = first->get_combined_minimum_size();
	}
	int axis = vertical ? 1 : 0;
	if (second && second->is_visible()) {
		Size2i ms2 = second->get_combined_minimum_size();
		int cross_axis = vertical ? 0 : 1;
		ms[axis] += ms2[axis];
		ms[cross_axis] = MAX(ms[cross_axis], ms2[cross_axis]);
	}
	if (dragger_control->is_visible()) {
		ms[axis] += MAX(theme_cache.grabber_thickness, theme_cache.minimum_grab_thickness);
	}
	return ms;
}

void SplitContainer::_validate_property(PropertyInfo &p_property) const {
	if (is_fixed && p_property.name == "vertical") {
		p_property.usage = PROPERTY_USAGE_NONE;
	}
}

void SplitContainer::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_TRANSLATION_CHANGED:
		case NOTIFICATION_LAYOUT_DIRECTION_CHANGED: {
			queue_sort();
		} break;

		case NOTIFICATION_SORT_CHILDREN: {
			Control *first = _get_child(0);
			ERR_FAIL_NULL(first);
			bool first_visible = first->is_visible();
			Control *second = _get_child(1);
			ERR_FAIL_COND(!second && !first_visible);
			bool second_visible = second && second->is_visible();
			ERR_FAIL_COND(!first_visible && !second_visible);

			Size2 size = get_size();
			int dragger_size = MAX(theme_cache.separation, theme_cache.minimum_grab_thickness);

			if (first_visible && !second) {
				fit_child_in_rect(first, Rect2(Point2(), size));
				dragger_control->hide();
				return;
			} else if (!first_visible && second_visible) {
				if (collapse_mode == COLLAPSE_FIRST || collapse_mode == COLLAPSE_ALL) {
					if (vertical) {
						fit_child_in_rect(dragger_control, Rect2(0, 0, size.width, dragger_size));
						fit_child_in_rect(second, Rect2(0, dragger_size, size.width, size.height - dragger_size));
					} else {
						fit_child_in_rect(dragger_control, Rect2(0, 0, dragger_size, size.height));
						fit_child_in_rect(second, Rect2(dragger_size, 0, size.width - dragger_size, size.height));
					}
					dragger_control->show();
					dragger_control->queue_redraw();
					split_offset = -999999;
					return;
				}
			} else if (first_visible && !second_visible) {
				if (collapse_mode == COLLAPSE_SECOND || collapse_mode == COLLAPSE_ALL) {
					if (vertical) {
						fit_child_in_rect(first, Rect2(0, 0, size.width, size.height - dragger_size));
						fit_child_in_rect(dragger_control, Rect2(0, size.height - dragger_size, size.width, dragger_size));
					} else {
						fit_child_in_rect(first, Rect2(0, 0, size.width - dragger_size, size.height));
						fit_child_in_rect(dragger_control, Rect2(size.width - dragger_size, 0, dragger_size, size.height));
					}
					dragger_control->show();
					dragger_control->queue_redraw();
					split_offset = 999999;
					return;
				}
			}
			first->show();
			second->show();
			_compute_split_offset(true);

			if (vertical) {
				fit_child_in_rect(first, Rect2(Point2(), Size2(get_size().width, middle_sep)));
				int sofs = middle_sep + dragger_size;
				fit_child_in_rect(second, Rect2(Point2(0, sofs), Size2(get_size().width, get_size().height - sofs)));
			} else {
				if (is_layout_rtl()) {
					middle_sep = get_size().width - middle_sep - dragger_size;
					fit_child_in_rect(second, Rect2(Point2(), Size2(middle_sep, get_size().height)));
					int sofs = middle_sep + dragger_size;
					fit_child_in_rect(first, Rect2(Point2(sofs, 0), Size2(get_size().width - sofs, get_size().height)));
				} else {
					fit_child_in_rect(first, Rect2(Point2(), Size2(middle_sep, get_size().height)));
					int sofs = middle_sep + dragger_size;
					fit_child_in_rect(second, Rect2(Point2(sofs, 0), Size2(get_size().width - sofs, get_size().height)));
				}
			}

			if (vertical) {
				dragger_control->set_rect(Rect2(Point2(0, middle_sep - (dragger_size - dragger_size) / 2), Size2(get_size().width, dragger_size)));
			} else {
				dragger_control->set_rect(Rect2(Point2(middle_sep - (dragger_size - dragger_size) / 2, 0), Size2(dragger_size, get_size().height)));
			}
			dragger_control->show();
			dragger_control->queue_redraw();
		} break;

		case NOTIFICATION_THEME_CHANGED: {
			update_minimum_size();
		} break;
	}
}

void SplitContainer::set_split_offset(int p_offset) {
	if (split_offset != p_offset) {
		split_offset = p_offset;
		queue_sort();
	}
}

void SplitContainer::clamp_split_offset() {
	if (_get_child(0) && _get_child(1)) {
		_compute_split_offset(true);
		queue_sort();
	}
}

void SplitContainer::set_vertical(bool p_vertical) {
	ERR_FAIL_COND_MSG(is_fixed, "Can't change orientation of " + get_class() + ".");
	vertical = p_vertical;
	update_minimum_size();
	queue_sort();
}

void SplitContainer::set_collapse_mode(CollapseMode p_mode) {
	ERR_FAIL_INDEX(p_mode, COLLAPSE_MODE_MAX);
	if (collapse_mode != p_mode) {
		collapse_mode = p_mode;
		queue_sort();
	}
}

Vector<int> SplitContainer::get_allowed_size_flags_horizontal() const {
	Vector<int> flags;
	flags.append(SIZE_FILL);
	if (!vertical) {
		flags.append(SIZE_EXPAND);
	}
	flags.append(SIZE_SHRINK_BEGIN);
	flags.append(SIZE_SHRINK_CENTER);
	flags.append(SIZE_SHRINK_END);
	return flags;
}

Vector<int> SplitContainer::get_allowed_size_flags_vertical() const {
	Vector<int> flags;
	flags.append(SIZE_FILL);
	if (vertical) {
		flags.append(SIZE_EXPAND);
	}
	flags.append(SIZE_SHRINK_BEGIN);
	flags.append(SIZE_SHRINK_CENTER);
	flags.append(SIZE_SHRINK_END);
	return flags;
}

void SplitContainer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_split_offset", "offset"), &SplitContainer::set_split_offset);
	ClassDB::bind_method(D_METHOD("get_split_offset"), &SplitContainer::get_split_offset);
	ClassDB::bind_method(D_METHOD("clamp_split_offset"), &SplitContainer::clamp_split_offset);

	ClassDB::bind_method(D_METHOD("set_vertical", "vertical"), &SplitContainer::set_vertical);
	ClassDB::bind_method(D_METHOD("is_vertical"), &SplitContainer::is_vertical);

	ClassDB::bind_method(D_METHOD("set_collapse_mode", "mode"), &SplitContainer::set_collapse_mode);
	ClassDB::bind_method(D_METHOD("get_collapse_mode"), &SplitContainer::get_collapse_mode);
	ClassDB::bind_method(D_METHOD("get_dragger"), &SplitContainer::get_dragger);

	ADD_SIGNAL(MethodInfo("dragged", PropertyInfo(Variant::INT, "offset")));

	ADD_PROPERTY(PropertyInfo(Variant::INT, "split_offset", PROPERTY_HINT_NONE, "suffix:px"), "set_split_offset", "get_split_offset");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "vertical"), "set_vertical", "is_vertical");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collapse_mode", PROPERTY_HINT_ENUM, "None,First,Second,All"), "set_collapse_mode", "get_collapse_mode");

	BIND_ENUM_CONSTANT(COLLAPSE_NONE);
	BIND_ENUM_CONSTANT(COLLAPSE_FIRST);
	BIND_ENUM_CONSTANT(COLLAPSE_SECOND);
	BIND_ENUM_CONSTANT(COLLAPSE_ALL);

	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, SplitContainer, separation);
	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, SplitContainer, minimum_grab_thickness);
	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, SplitContainer, autohide);
	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, SplitContainer, grabber_thickness);
	BIND_THEME_ITEM(Theme::DATA_TYPE_COLOR, SplitContainer, grabber_normal);
	BIND_THEME_ITEM(Theme::DATA_TYPE_COLOR, SplitContainer, grabber_hovered);
	BIND_THEME_ITEM(Theme::DATA_TYPE_COLOR, SplitContainer, grabber_pressed);
}

SplitContainer::SplitContainer(bool p_vertical) {
	vertical = p_vertical;

	dragger_control = memnew(SplitContainerDragger(this));
	add_child(dragger_control, false, Node::INTERNAL_MODE_BACK);
}
