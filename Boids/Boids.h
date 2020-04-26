// Includes
#include "../ReflexEngine2/ReflexEngine/ReflexEngine/Include.h"

#include <string>

using namespace Reflex::Core;

class GameState : public State
{
public:
	GameState( StateManager& stateManager );

protected:
	Reflex::Object CreateBoid( const sf::Vector2f& pos );
	Reflex::Object CreateCollider( const sf::Vector2f& pos, const float radius );

	void Render() override;
	void Update( const float deltaTime ) override;

protected:
	// Data Members
	Reflex::Components::Camera::Handle camera;

	enum class DemoTypes
	{
		Wander,
		Seek,
		Flee,
		Alignment,
		Cohesion,
		Separation,
		Flocking,
		FlockingWithPredators,
		CollisionAvoidance,
		PathFollowing,
		LeaderFollowing,
		Queueing,
	};

	DemoTypes demoType = DemoTypes::Wander;
	Reflex::Components::Steering::Handle selected, first;

	int numBoids = 100;
	Reflex::Object mousePosition;
};