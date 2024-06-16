#include "InputPlayer.h"

gde::InputPlayer::InputPlayer(std::string player_name)
{
	this->player_name = player_name;
}

std::string gde::InputPlayer::get_player_name()
{
	return this->player_name;
}

void gde::InputPlayer::OnEnterHierarchy(Object* other)
{
	inputhandler.TryAdd(other);
}

void gde::InputPlayer::OnExitHierarchy(Object* other)
{
	inputhandler.Remove(other);
}
