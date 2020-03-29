// Includes
#include "../ReflexEngine2/ReflexEngine/ReflexEngine/Include.h"

using namespace Reflex::Core;

class GameState : public State
{
public:
	GameState( StateManager& stateManager, Context context );

protected:
	Reflex::Components::Steering::Handle CreateBoid( const sf::Vector2f& pos );
	void Render() override;

protected:
	// Data Members
	Reflex::Components::Camera::Handle camera;

	float maxVelocity = 50.0f;
	float slowingRadius = 0.0f;
	float wanderCircleRadius = 1.0f;
	float wanderCircleDistance = 0.0f;
	float wanderAngleDelta = 0.0f;
};