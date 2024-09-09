#pragma once

#include "../ActionImplementation.h"
#include "../Action/MouseDrag.h"
#include "../../Window/Window.h"

namespace gde {
	namespace input {
		template<int Trigger>
		class MouseDragImplementation : public ActionImplementation<MouseDrag<Trigger>> {
		private:
			Vector2 old_mouse_pos;
			// Inherited via ActionImplementation
			virtual void UpdateState(Window* target)
            {
                auto key_state = target->GetKeyState(Trigger);

                if (key_state) {
                    auto new_pos = target->GetMousePos();

                    if (this->mState.state == InputAction::State::END) {
                        this->mState.state = InputAction::State::START;
                        old_mouse_pos = new_pos;
                        return;
                    }
                    else
                        this->mState.state = InputAction::State::WHILE;

                    this->mState.delta = new_pos - old_mouse_pos;
                    old_mouse_pos = new_pos;

                    return;
                }
                else {
                    this->mState.delta = Vector2::zero;
                    this->mState.state = InputAction::State::END;
                    return;
                }
            }

            virtual void ResetState(Window* target){}
		};
	}
}