#pragma once
#include "../UI.h"


class FlexContainer : public LayoutUIElement {
	BoundingBox padding;
	float gap;
	Axis direction;

	vec2 get_min_dimensions() const override {
		vec2 result;
		
		for (const auto& el : children) {
			const vec2& el_dims = el->get_dimensions();
			if (direction == X) {
				result.x += el_dims.x + gap;
				result.y = fmaxf(result.y, el_dims.y);
			}
			if (direction == Y) {
				result.x = fmaxf(result.x, el_dims.x);
				result.y += el_dims.y + gap;
			}
		}
		if (direction == X) result.x -= gap;
		if (direction == Y) result.y -= gap;

		result += vec2(
			padding.left + padding.right,
			padding.top + padding.bottom
		);

		return result;
	}
public:
	bool stretch;
	enum Alignment {
		Start, Middle, End
	} alignment = Start;

	FlexContainer(
		const string& id,
		shared_ptr<UI> ui,
		BoundingBox padding = BoundingBox(),
		const float& gap = 0,
		Axis direction = X,
		vec2 position = vec2(),
		vec2 dimensions = vec2(100, 100),
		bool stretch = false
	) : 
		LayoutUIElement(id, ui, position, dimensions), 
		padding(padding), gap(gap), direction(direction), stretch(stretch) {}

	void layout_children() override {
		float accumulated_position = 0;
		const vec2& fc_dims = get_dimensions();
		for (auto& el : children) {
			if (direction == Y) {
				float el_height = 0;
				if (stretch)
					el_height = fc_dims.y / static_cast<float>(children.size()) - gap;

				el->set_position(
					vec2(
						padding.left,
						padding.top + accumulated_position
					)
				);

				vec2 new_el_dims(0, el_height);

				if (!el->fit_content)
					new_el_dims.x = fc_dims.x - padding.left - padding.right;

				el->set_dimensions(new_el_dims);

				// update the element to make sure the dimensions are correct
				el->update_layout(true);

				// flex alignment

				vec2 current_el_dims = el->get_dimensions();
				vec2 current_el_pos = el->get_position();

				if (el->fit_content) {
					if (alignment == Alignment::Middle)
						el->set_position(vec2(fc_dims.x / 2 - current_el_dims.x / 2, current_el_pos.y));
					if (alignment == Alignment::End)
						el->set_position(vec2(fc_dims.x - current_el_dims.x, current_el_pos.y));
				}

				// update the element to make sure the position is correct
				el->update_layout(true);

				accumulated_position += el->get_dimensions().y + gap;
			}
			if (direction == X) {
				float el_width = 0;
				if (stretch)
					el_width = fc_dims.x / static_cast<float>(children.size()) - gap;

				el->set_position(
					vec2(
						padding.left + accumulated_position,
						padding.top
					)
				);
				el->set_dimensions(
					vec2(
						el_width, 
						fc_dims.y - padding.top - padding.bottom
					)
				);
				el->update_layout(true);

				// flex alignment

				vec2 current_el_dims = el->get_dimensions();
				vec2 current_el_pos = el->get_position();

				if (el->fit_content) {
					if (alignment == Middle)
						el->set_position(vec2(current_el_pos.x, fc_dims.y / 2 - current_el_dims.y / 2));
					if (alignment == End)
						el->set_position(vec2(current_el_pos.x, fc_dims.y - current_el_dims.y));
				}

				// update the element to make sure the position is correct
				el->update_layout(true);

				accumulated_position += el->get_dimensions().x + gap;
			}
		}
	}
};