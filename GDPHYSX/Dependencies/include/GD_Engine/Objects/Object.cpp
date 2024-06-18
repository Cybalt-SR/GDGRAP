#include "Object.h"
#include <glm/gtx/matrix_decompose.hpp>

namespace gde {
	void Object::MatToTrans(Transform* target, glm::mat4 mat)
	{
		if (target->changed == false) {
			//return;
		}

		glm::vec3 scale;
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(mat, scale, rotation, translation, skew, perspective);

		target->position = translation;
		target->rotation = glm::eulerAngles(rotation) * 3.14159f / 180.f;
		target->scale = scale;
		target->Forward = Vector3(this->GetWorldSpaceMatrix()[2]);
		target->Up = Vector3(this->GetWorldSpaceMatrix()[1]);
		target->Right = Vector3(this->GetWorldSpaceMatrix()[0]);

		target->changed = false;
	}

	void Object::UpdateTransforms()
	{
		this->local.changed = true;

		for (auto child : this->children)
		{
			child->world.changed = true;
		}
	}

	Object* Object::Copy_self()
	{
		return new Object(*this);
	}

	Object::Object()
	{
		this->transform = glm::mat4(1.0f);
		this->parent = nullptr;
		this->prev_world_space_matrix = this->GetWorldSpaceMatrix();
	}

	Object::~Object() {
		this->SetParent(nullptr);
	}

	Object* Object::Copy()
	{
		auto copy = this->Copy_self();
		copy->children.clear();

		for (auto child : this->children)
		{
			copy->children.push_back(child->Copy());
		}

		return copy;
	}

	glm::mat4 Object::GetWorldSpaceMatrix()
	{
		if (this->world.changed == false) {
			//return this->prev_world_space_matrix;
		}

		glm::mat4 parentmat = glm::mat4(1.0f);

		if (this->parent != nullptr)
			parentmat = this->parent->GetWorldSpaceMatrix();

		this->prev_world_space_matrix = parentmat * this->transform;
		return this->prev_world_space_matrix;
	}

	Transform* Object::World()
	{
		this->MatToTrans(&this->world, this->GetWorldSpaceMatrix());
		return &this->world;
	}

	Transform* Object::Local()
	{
		this->MatToTrans(&this->local, this->transform);
		return &this->local;
	}

	void Object::SetLocalPosition(Vector3 vector)
	{
		this->transform = glm::translate(this->transform, -(glm::vec3)this->Local()->position);
		this->transform = glm::translate(this->transform, (glm::vec3)vector);

		this->UpdateTransforms();
	}

	void Object::TranslateWorld(Vector3 vector)
	{
		auto localdelta = glm::vec3(glm::vec4((glm::vec3)vector, 1) * this->GetWorldSpaceMatrix());
		this->transform = glm::translate(this->transform, (glm::vec3)localdelta);

		this->UpdateTransforms();
	}

	void Object::TranslateLocal(Vector3 vector)
	{
		this->transform = glm::translate(this->transform, (glm::vec3)vector);

		this->UpdateTransforms();
	}

	void Object::Scale(Vector3 vector)
	{
		this->transform = glm::scale(this->transform, (glm::vec3)vector);

		this->UpdateTransforms();
	}

	void Object::Rotate(Vector3 axis, float deg_angle)
	{
		this->transform = glm::rotate(glm::mat4(1.0f), glm::radians(deg_angle), (glm::vec3)axis) * this->transform;

		this->UpdateTransforms();
	}

	void Object::Orient(Vector3 forward, Vector3 Up)
	{
		this->transform[2] = glm::vec4((glm::vec3)forward, 0);
		this->transform[1] = glm::vec4((glm::vec3)Up, 0);
		this->transform[0] = glm::vec4((glm::vec3)Up.Cross(forward), 0);

		this->UpdateTransforms();
	}

	void Object::OnEnterHierarchy(Object* newChild)
	{
		auto propagate_upwards = [this](Object* object) {
			Object* current = this->parent;

			while (current != nullptr)
			{
				current->OnEnterHierarchy(object);
				current = current->parent;
			}
		};

		for (auto subchild : newChild->children)
		{
			propagate_upwards(subchild);
		}

		propagate_upwards(newChild);
	}

	void Object::OnExitHierarchy(Object* newChild)
	{
		auto propagate_upwards = [this](Object* object) {
			Object* current = this->parent;

			while (current != nullptr)
			{
				current->OnExitHierarchy(object);
				current = current->parent;
			}
		};

		for (auto subchild : newChild->children)
		{
			propagate_upwards(subchild);
		}

		propagate_upwards(newChild);
	}

	void Object::SetParent(Object* newParent)
	{
		if (parent != nullptr) {
			parent->OnExitHierarchy(this);
			parent->children.remove_if([this](Object* child) {return child == this; });
		}

		if (newParent != nullptr) {
			newParent->OnEnterHierarchy(this);
			newParent->children.push_back(this);
		}

		this->parent = newParent;
	}

	Object* Object::GetChildAt(int i)
	{
		auto start = this->children.begin();

		for (int count = 0; count < i; count++)
			++start;

		return *start;
	}

	int Object::GetChildCount()
	{
		return this->children.size();
	}

	void Object::Destroy()
	{
		this->isDestroyQueued = true;

		for (auto child : this->children) {
			child->Destroy();
		}
	}

	bool Object::get_isDestroyed()
	{
		return this->isDestroyQueued;
	}
}