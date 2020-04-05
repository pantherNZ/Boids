// Includes
#include "../ReflexEngine2/ReflexEngine/ReflexEngine/Include.h"

#include <string>

using namespace Reflex::Core;

class GameState : public State
{
public:
	GameState( StateManager& stateManager, Context context );

protected:
	Reflex::Components::Steering::Handle CreateBoid( const sf::Vector2f& pos );

	void Render() override;
	void Update( const float deltaTime ) override;

	// Pre defined behaviours
	void Wander( bool defaultValues = true );
	void RenderWander( bool velocityChanged );
	
	void Seek( bool defaultValues = true );
	void RenderSeek( bool velocityChanged );

	void Flee( bool defaultValues = true );
	void RenderFlee( bool velocityChanged );

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

	int numBoids = 100;
	float maxVelocity = 50.0f;
	float slowingRadius = 0.0f;
	float wanderCircleRadius = 1.0f;
	float wanderCircleDistance = 0.0f;
	float wanderAngleDelta = 0.0f;

	Reflex::Object mousePosition;
};