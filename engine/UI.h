#pragma once
#include "basics.h"
#include "Sprite.h"
#include <memory>
#include <string>
#include <optional>
#include <map>

struct BoundingBox {
	float left;
	float right;
	float top;
	float bottom;

	BoundingBox(
		float left,
		float right,
		float top,
		float bottom
	) : left(left), right(right), top(top), bottom(bottom) {}

	BoundingBox(
		float x,
		float y
	) : left(x), right(x), top(y), bottom(y) {}

	BoundingBox(
		float amount
	) : left(amount), right(amount), top(amount), bottom(amount) {}

	BoundingBox() : left(0), right(0), top(0), bottom(0) {}
};

enum Axis { X, Y };

class UI;

enum UIListenerType {
	LMBDown, LMBUp,
	RMBDown, RMBUp,
	MouseMove,
	KeyDown, KeyUp
};

class UIElement : public Drawable, public Updatable {
protected:
	Transform transform;
	vec2 dimensions;

	shared_ptr<UI> ui;
	UIElement* parent = nullptr;
	string id;
	map<UIListenerType, unordered_map<string, function<void(GameState&, UIElement*)>>> listeners = {
		{ LMBDown, {} }, { LMBUp, {} },
		{ RMBDown, {} }, { RMBUp, {} },
		{ MouseMove, {} },
		{ KeyDown, {} }, { KeyUp, {} },
	};

	bool is_pressed_l = false;
	bool is_pressed_r = false;

	virtual vec2 get_min_dimensions() const = 0;
	virtual void shrink_to_fit();
	virtual void expand_to_fit();

	virtual vec2 get_calculated_offset() const;
public:
	HorizontalAlignment horizontal_alignment = Left;
	VerticalAlignment vertical_alignment = Top;

	bool fit_content = false;
	vector<shared_ptr<UIElement>> children;

	UIElement(const string& id, shared_ptr<UI> ui, vec2 position = vec2(), vec2 dimensions = vec2()) : id(id), ui(ui), dimensions(dimensions) {
		transform.position = position;
	}

	void add_child(shared_ptr<UIElement> element);
	void add_children(initializer_list<shared_ptr<UIElement>> elements);
	shared_ptr<UIElement> get_element_by_id(const string& id);

	virtual const vec2& get_dimensions() const { return dimensions; }
	virtual void set_dimensions(const vec2& new_dimensions) { dimensions = new_dimensions; }

	virtual inline Transform& get_transform_mut() { return transform; }
	virtual inline const Transform& get_transform() const { return transform; }

	virtual inline const vec2& get_position() const { return get_transform().position; }
	virtual inline void set_position(const vec2& new_position) { get_transform_mut().position = new_position; }

	virtual inline const float& get_rotation() const { return get_transform().rotation; }
	virtual inline void set_rotation(const float& new_rotation) { get_transform_mut().rotation = new_rotation; }

	virtual inline const vec2& get_scale() const { return get_transform().scale; }
	virtual inline void set_scale(const vec2& new_scale) { get_transform_mut().scale = new_scale; }

	virtual void draw(SDL_Renderer* renderer, const RendererState& renderer_state) const override;
	virtual void update(const float& delta, GameState& game_state) override;

	bool is_mouse_inside_element(GameState& game_state) const;

	// adds an event listener by type and function pointer
	// returns a unique id for that listener type in the current UIElement
	const string& add_event_listener(
		UIListenerType type, 
		const string& callback_id, 
		function<void(GameState&, UIElement*)> callback
	);
	// removes the event listener by type and unique id
	void remove_event_listener(UIListenerType type, const string& callback_id);

	virtual void on_lmb_down(GameState& game_state)		{ for (auto& pair : listeners.at(LMBDown))		pair.second(game_state, this); }
	virtual void on_lmb_up(GameState& game_state)		{ for (auto& pair : listeners.at(LMBUp))		pair.second(game_state, this); }
	virtual void on_rmb_down(GameState& game_state)		{ for (auto& pair : listeners.at(RMBDown))		pair.second(game_state, this); }
	virtual void on_rmb_up(GameState& game_state)		{ for (auto& pair : listeners.at(RMBUp))		pair.second(game_state, this); }

	virtual void on_mouse_move(GameState& game_state)	{ for (auto& pair : listeners.at(MouseMove))	pair.second(game_state, this); }

	virtual void on_key_down(GameState& game_state)		{ for (auto& pair : listeners.at(KeyDown))		pair.second(game_state, this); }
	virtual void on_key_up(GameState& game_state)		{ for (auto& pair : listeners.at(KeyDown))		pair.second(game_state, this); }
};

class UITexture : public Texture {
public:
	struct UIProperties {
		float scaling = 1;
		bool stretch_x = true;
		bool stretch_y = true;
		uint left = 0;
		uint right = 0;
		uint top = 0;
		uint bottom = 0;
	} ui_properties;

	UITexture(
		const ushort& w, 
		const ushort& h, 
		SDL_Texture* texture, 
		UIProperties ui_properties
	) : Texture(w, h, texture), ui_properties(ui_properties) {}
	UITexture() = default;

	const UIProperties& get_ui_properties() const { return ui_properties; }
};

class UISprite : public Sprite {
public:
	vec2 dimensions;
	UISprite(
		Texture* texture
	) : Sprite(texture) {}
	
	virtual void draw(SDL_Renderer* renderer, const RendererState& renderer_state) const override;
};

class VisualUIElement : public UIElement, public UISprite {
public:
	VisualUIElement(
		const string& id,
		shared_ptr<UI> ui,
		Texture* texture = nullptr,
		vec2 position = vec2(),
		vec2 dimensions = vec2(100, 100)
	) : UIElement(id, ui, position, dimensions), UISprite(texture) {}

	virtual void update(const float& delta, GameState& game_state) override;
	virtual void draw(SDL_Renderer* renderer, const RendererState& renderer_state) const override;
};

class LayoutUIElement : public UIElement {
public:
	LayoutUIElement(
		const string& id, 
		shared_ptr<UI> ui,
		vec2 position = vec2(),
		vec2 dimensions = vec2(100, 100)
	) : UIElement(id, ui, position, dimensions) {}

	virtual void update(const float& delta, GameState& game_state) override { layout_children(delta, game_state); }

	virtual void layout_children(const float& delta, GameState& game_state) = 0;
};

class UI : public Drawable, public Updatable {
	float scaling = 1;
public:
	bool scaling_just_changed = false;

	shared_ptr<UIElement> root_element;
	SDL_Renderer* renderer;

	UI(SDL_Renderer* renderer, shared_ptr<UIElement> root_element = nullptr) : renderer(renderer), root_element(root_element) {}

	void draw(SDL_Renderer* renderer, const RendererState& renderer_state) const override;
	void update(const float& delta, GameState& game_state) override;
	const float& get_scaling() const;
};