#pragma once
#include "../UI.h"
#include "../AssetManager.h"
#include <SDL_ttf.h>

class Text : public VisualUIElement {
	Font* font;
	string content;
	SDL_Color color;

	void render_text() {
		if (texture) {
			texture->destroy();
			delete texture;
		}

		//texture = font->render(ui->renderer, content, color, ui->get_scaling());
		texture = font->render(ui->renderer, content, color, 1);
		set_dimensions(vec2(texture->get_width(), texture->get_height()));
	}

	vec2 get_min_dimensions() const override {
		return vec2(texture->get_width(), texture->get_height());
	}

public:
	Text(
		const string& id,
		shared_ptr<UI> ui,
		string text,
		Font* font,
		SDL_Color text_color = { 0, 0, 0 },
		vec2 position = vec2()
	) : VisualUIElement(id, ui), font(font), content(text), color(text_color) {
		render_text();
	}

	void set_content(const string& new_content) {
		content = new_content;
		render_text();
	}

	const string& get_content() const { return content; }

	void set_color(SDL_Color new_color) {
		color = new_color;
		render_text();
	}

	const SDL_Color& get_color() const { return color; }

	void update(const float& delta, GameState& game_state) {
		if (ui->scaling_just_changed) {
			render_text();
			//set_scale(1 / ui->get_scaling());
		}

		VisualUIElement::update(delta, game_state);
	}
};