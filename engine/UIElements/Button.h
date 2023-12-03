#pragma once
#include "../UI.h"
#include "Text.h"

class Button : public VisualUIElement {
	string text_content;
	BoundingBox padding;

	vec2 get_min_dimensions() const override {
		const vec2& text_dims = children[0]->get_dimensions();
		return vec2(
			padding.left + padding.right + text_dims.x,
			padding.top + padding.bottom + text_dims.y
		);
	}
public:
	Button(
		const string& id,
		shared_ptr<UI> ui,
		UITexture* texture,
		const string& text,
		Font* text_font,
		SDL_Color text_color = { 0, 0, 0 },
		BoundingBox padding = BoundingBox(),
		vec2 position = vec2(),
		vec2 dimensions = vec2(0, 0)
	) : VisualUIElement(id, ui, texture, position, dimensions), text_content(text), padding(padding) {
		add_child(
			make_shared<Text>(
				id + "_text", ui,
				text, text_font, text_color
			)
		);
	}

	void update(const float& delta, GameState& game_state) {
		const vec2& button_pos = get_position();
		const vec2& scaling = get_scale();

		shared_ptr<Text> text = dynamic_pointer_cast<Text>(children[0]);

		// position the text element inside the button
		text->set_position(vec2(padding.left * scaling.x, padding.top * scaling.y));
		vec2 text_dims = text->get_dimensions() * text->get_scale();

		// make sure the dimensions of the button are not below the minimum required
		float min_width = text_dims.x + padding.left + padding.right;
		if (dimensions.x < min_width)
			dimensions.x = min_width;
		float min_height = text_dims.y + padding.top + padding.bottom;
		if (dimensions.y < min_height)
			dimensions.y = min_height;

		VisualUIElement::update(delta, game_state);
	}

	void set_text_content(const string& new_text) {
		text_content = new_text;
		shared_ptr<Text> text = dynamic_pointer_cast<Text>(children[0]);
		text->set_content(new_text);
	}

	const string& get_text_content() { return text_content; }
};
