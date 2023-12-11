#include "../engine/UI.h"

vec2 UIElement::get_calculated_offset() const {
	vec2 result;

	UIElement* current_parent = parent;
	while (current_parent != nullptr) {
		const vec2& curpar_pos = current_parent->get_position();
		const vec2& curpar_scale = current_parent->get_scale();
		result.x += curpar_pos.x;
		result.x *= curpar_scale.x;
		result.y += curpar_pos.y;
		result.y *= curpar_scale.y;
		current_parent = current_parent->parent;
	}

	return result;
}

void UIElement::add_child(shared_ptr<UIElement> element) {
	children.push_back(element);

	//tie added element's origin_transform to parent's transform
	//element->set_parent_transform(&get_transform_mut());
	element->parent = this;
}

void UIElement::add_children(initializer_list<shared_ptr<UIElement>> elements) {
	for (auto& el : elements)
		add_child(el);
}

shared_ptr<UIElement> UIElement::get_element_by_id(const string& id) {
	for (auto el : children) {
		if (el->id == id)
			return el;
	}
	return nullptr;
}

void UIElement::draw(SDL_Renderer* renderer, const RendererState& renderer_state) const {
	for (auto& el : children)
		el->draw(renderer, renderer_state);
}

bool UIElement::is_mouse_inside_element(GameState& game_state) const {
	const vec2& mouse_pos = game_state.mouse_state.mouse_pos;
	const vec2& pos = get_calculated_offset() + get_position();
	const vec2& dims = get_dimensions();

	return 
		mouse_pos.x >= pos.x && 
		mouse_pos.x <= (pos.x + dims.x) && 
		mouse_pos.y >= pos.y && 
		mouse_pos.y <= pos.y + dims.y;
}

void UIElement::expand_to_fit() {
	vec2 min_dims = get_min_dimensions();

	const vec2& current_dims = get_dimensions();

	if (current_dims.x < min_dims.x)
		set_dimensions(vec2(min_dims.x, current_dims.y));
	if (current_dims.y < min_dims.y)
		set_dimensions(vec2(current_dims.x, min_dims.y));
}

void UIElement::shrink_to_fit() {
	vec2 min_dims = get_min_dimensions();
	set_dimensions(min_dims);
}

void UIElement::update(const float& delta, GameState& game_state) {
	MouseState& m_state = game_state.mouse_state;
	const KeyboardState& k_state = game_state.keyboard_state;
	bool mouse_inside = is_mouse_inside_element(game_state);
	m_state.mouse_on_ui = m_state.mouse_on_ui || mouse_inside;

	if (m_state.lmb_just_pressed && mouse_inside) {
		is_pressed_l = true;
		on_lmb_down(game_state);
	}
	if (m_state.lmb_just_unpressed && (mouse_inside || is_pressed_l)) {
		is_pressed_l = false;
		on_lmb_up(game_state);
	}
	if (m_state.rmb_just_pressed && mouse_inside) {
		is_pressed_r = true;
		on_rmb_down(game_state);
	}
	if (m_state.rmb_just_unpressed && (mouse_inside || is_pressed_r)) {
		is_pressed_r = false;
		on_rmb_up(game_state);
	}
	if (k_state.key_just_pressed)		on_key_down(game_state);
	if (k_state.key_just_unpressed)		on_key_up(game_state);

	if ((m_state.previous_mouse_pos - m_state.mouse_pos).len() != 0.0F)	on_mouse_move(game_state);

	update_layout(true);

	for (auto& el : children)
		el->update(delta, game_state);

	update_layout(true);
}

void UIElement::update_layout(bool update_children) {
	if (fit_content)
		shrink_to_fit();
	else
		expand_to_fit();

	if (update_children)
		for (auto& el : children)
			el->update_layout(true);
}

const string& UIElement::add_event_listener(
	UIListenerType type, 
	const string& callback_id, 
	function<void(GameState&, UIElement*)> callback
) {
	auto& callbacks = listeners.at(type);

	if (callbacks.contains(callback_id))
		return callback_id;
	callbacks.insert({ callback_id, callback });

	return callback_id;
}

void UIElement::remove_event_listener(UIListenerType type, const string& callback_id) {
	auto& callbacks = listeners.at(type);

	if (!callbacks.contains(callback_id))
		return;

	callbacks.erase(callback_id);
}

void UI::draw(SDL_Renderer* renderer, const RendererState& renderer_state) const {
	if (root_element)
		root_element->draw(renderer, renderer_state);
}

void UI::update(const float& delta, GameState& game_state) {
	scaling_just_changed = scaling != game_state.renderer_state.scaling;
	scaling = game_state.renderer_state.scaling;
	if (root_element)
		root_element->update(delta, game_state);
}

const float& UI::get_scaling() const { return scaling; }

void UISprite::draw(SDL_Renderer* renderer, const RendererState& renderer_state) const {
	if (texture == nullptr) return;

	const UITexture* texture = dynamic_cast<const UITexture*>(this->texture);
	if (texture == nullptr) {
		Sprite::draw(renderer, renderer_state);
		return;
	}

	Transform resulting_transform = get_calculated_transform();

	const auto& ui_properties = texture->get_ui_properties();
	const auto& left = ui_properties.left;
	const auto& right = ui_properties.right;
	const auto& top = ui_properties.top;
	const auto& bottom = ui_properties.bottom;
	const auto& scaling = ui_properties.scaling;

	// left-top corner
	SDL_Rect lt;
	lt.x = 0;
	lt.y = 0;
	lt.w = left;
	lt.h = top;

	// top side
	SDL_Rect t;
	t.x = left;
	t.y = 0;
	t.w = texture->get_width() - left - right;
	t.h = top;

	// right-top corner
	SDL_Rect rt;
	rt.x = texture->get_width() - right;
	rt.y = 0;
	rt.w = right;
	rt.h = top;
	
	// right side
	SDL_Rect r;
	r.x = texture->get_width() - right;
	r.y = top;
	r.w = right;
	r.h = texture->get_height() - top - bottom;

	// right-bottom corner
	SDL_Rect rb;
	rb.x = texture->get_width() - right;
	rb.y = texture->get_height() - bottom;
	rb.w = right;
	rb.h = bottom;

	// bottom side
	SDL_Rect b;
	b.x = left;
	b.y = texture->get_height() - bottom;
	b.w = texture->get_width() - left - right;
	b.h = bottom;

	// left-bottom corner
	SDL_Rect lb;
	lb.x = 0;
	lb.y = texture->get_height() - bottom;
	lb.w = left;
	lb.h = bottom;

	// left side
	SDL_Rect l;
	l.x = 0;
	l.y = top;
	l.w = left;
	l.h = texture->get_height() - top - bottom;

	// center
	SDL_Rect center;
	center.x = left;
	center.y = top;
	center.w = texture->get_width() - left - right;
	center.h = texture->get_height() - top - bottom;

	SDL_FRect lt_out = rect_to_frect(lt);
	lt_out.w *= scaling;
	lt_out.h *= scaling;
	SDL_FRect t_out = rect_to_frect(t);
	t_out.x *= scaling;
	t_out.y *= scaling;
	t_out.w = dimensions.x - (left + right) * scaling;
	t_out.h *= scaling;
	SDL_FRect rt_out = rect_to_frect(rt);
	rt_out.x = dimensions.x - right * scaling;
	rt_out.y *= scaling;
	rt_out.w *= scaling;
	rt_out.h *= scaling;
	SDL_FRect r_out = rect_to_frect(r);
	r_out.x = dimensions.x - right * scaling;
	r_out.y *= scaling;
	r_out.w *= scaling;
	r_out.h = dimensions.y - (top + bottom) * scaling;
	SDL_FRect rb_out = rect_to_frect(rb);
	rb_out.x = dimensions.x - right * scaling;
	rb_out.y = dimensions.y - bottom * scaling;
	rb_out.w *= scaling;
	rb_out.h *= scaling;
	SDL_FRect b_out = rect_to_frect(b);
	b_out.x *= scaling;
	b_out.y = dimensions.y - bottom * scaling;
	b_out.w = dimensions.x - (left + right) * scaling;
	b_out.h *= scaling;
	SDL_FRect lb_out = rect_to_frect(lb);
	lb_out.x *= scaling;
	lb_out.y = dimensions.y - bottom * scaling;
	lb_out.w *= scaling;
	lb_out.h *= scaling;
	SDL_FRect l_out = rect_to_frect(l);
	l_out.x *= scaling;
	l_out.y *= scaling;
	l_out.w *= scaling;
	l_out.h = dimensions.y - (top + bottom) * scaling;
	SDL_FRect center_out = rect_to_frect(center);
	center_out.x *= scaling;
	center_out.y *= scaling;
	center_out.w = dimensions.x - (left + right) * scaling;
	center_out.h = dimensions.y - (top + bottom) * scaling;

	vector<SDL_FRect*> to_transform = {
		&lt_out,		&t_out,			&rt_out,
		&l_out,			&center_out,	&r_out,
		&lb_out,		&b_out,			&rb_out
	};

	for (SDL_FRect* tr : to_transform) {
		// apply normal scaling
		float x_ratio = resulting_transform.scale.x * renderer_state.scaling;
		float y_ratio = resulting_transform.scale.y * renderer_state.scaling;

		// apply display size scaling
		if (display_size != nullopt) {
			x_ratio *= display_size.value().x / dimensions.x;
			y_ratio *= display_size.value().y / dimensions.y;
		}

		// apply alignment
		switch (horizontal_alignment) {
		case Left:
			tr->x += resulting_transform.position.x;
			break;
		case Center:
			tr->x += resulting_transform.position.x - dimensions.x / 2.0F;
			break;
		case Right:
			tr->x += resulting_transform.position.x - dimensions.x;
			break;
		}
		switch (vertical_alignment) {
		case Top:
			tr->y += resulting_transform.position.y;
			break;
		case Middle:
			tr->y += resulting_transform.position.y - dimensions.y / 2.0F;
			break;
		case Bottom:
			tr->y += resulting_transform.position.y - dimensions.y;
			break;
		}

		tr->x *= x_ratio;
		tr->w *= x_ratio;
		tr->y *= y_ratio;
		tr->h *= y_ratio;
	}

	SDL_RenderCopyExF(renderer, texture->get_raw(), &lt, &lt_out, resulting_transform.rotation, nullptr, SDL_FLIP_NONE);
	SDL_RenderCopyExF(renderer, texture->get_raw(), &t, &t_out, resulting_transform.rotation, nullptr, SDL_FLIP_NONE);
	SDL_RenderCopyExF(renderer, texture->get_raw(), &rt, &rt_out, resulting_transform.rotation, nullptr, SDL_FLIP_NONE);
	SDL_RenderCopyExF(renderer, texture->get_raw(), &l, &l_out, resulting_transform.rotation, nullptr, SDL_FLIP_NONE);
	SDL_RenderCopyExF(renderer, texture->get_raw(), &center, &center_out, resulting_transform.rotation, nullptr, SDL_FLIP_NONE);
	SDL_RenderCopyExF(renderer, texture->get_raw(), &r, &r_out, resulting_transform.rotation, nullptr, SDL_FLIP_NONE);
	SDL_RenderCopyExF(renderer, texture->get_raw(), &lb, &lb_out, resulting_transform.rotation, nullptr, SDL_FLIP_NONE);
	SDL_RenderCopyExF(renderer, texture->get_raw(), &b, &b_out, resulting_transform.rotation, nullptr, SDL_FLIP_NONE);
	SDL_RenderCopyExF(renderer, texture->get_raw(), &rb, &rb_out, resulting_transform.rotation, nullptr, SDL_FLIP_NONE);
}

void VisualUIElement::update(const float& delta, GameState& game_state) {
	update_layout();
	UIElement::update(delta, game_state);
}

void VisualUIElement::update_layout(bool update_children) {
	UISprite::dimensions = UIElement::dimensions;
	UISprite::global_transform = get_calculated_offset() + get_position();
	UIElement::update_layout(update_children);
}

void VisualUIElement::draw(SDL_Renderer* renderer, const RendererState& renderer_state) const {
	// draw the element itself
	UISprite::draw(renderer, renderer_state);
	// draw children
	UIElement::draw(renderer, renderer_state);
}
