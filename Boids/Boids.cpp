// Includes
#include "Boids.h"

// Entry point
int main()
{
	Engine engine( "AI Steering Demo" );
	engine.RegisterState< GameState >( true );
	engine.Run();

	return 0;
}

GameState::GameState( StateManager& stateManager, Context context )
	: State( stateManager, context )
{
	auto camera = GetWorld().CreateObject().AddComponent< Reflex::Components::Camera >( Reflex::Vector2uToVector2f( context.window.getSize() ) / 2.0f, Reflex::Vector2uToVector2f( context.window.getSize() ) );
	camera->EnableWASDPanning( sf::Vector2f( 300.0f, 300.0f ) );
	camera->EnableArrowKeyPanning();
	camera->EnableMouseZooming( 1.2f, true );

	for( unsigned i = 0; i < 100; ++i )
		CreateBoid( GetWorld().RandomWindowPosition() );

	Wander();
}

Reflex::Components::Steering::Handle GameState::CreateBoid( const sf::Vector2f& pos )
{
	auto newBoid = GetWorld().CreateObject( pos );
	auto boid = newBoid.AddComponent< Reflex::Components::Steering >();

	auto circle = newBoid.AddComponent< Reflex::Components::SFMLObject >( sf::CircleShape( 10.0f, 30 ) );
	circle->GetCircleShape().setFillColor( sf::Color::Red );
	
	return boid;
}

namespace
{
	const char* demoTypeNames[] =
	{
		"Wander",
		"Seek",
		"Flee",
		"Alignment",
		"Cohesion",
		"Separation",
		"Flocking",
		"FlockingWithPredators",
		"CollisionAvoidance",
		"PathFollowing",
		"LeaderFollowwing",
		"Queueing",
	};
}

void GameState::Render()
{
	ImGui::SetNextWindowPos( sf::Vector2( 5.0f, 0.17f * GetWorld().GetWindow().getSize().y ), ImGuiCond_::ImGuiCond_Once );
	ImGui::Begin( "AI Demo", nullptr, ImGuiWindowFlags_AlwaysAutoResize );

	const auto oldCount = numBoids;
	bool updateBehaviour = false;

	if( ImGui::InputInt( "Boids Count", &numBoids ) )
	{
		numBoids = Reflex::Clamp( numBoids, 0, 10000 );

		if( numBoids > oldCount )
		{
			updateBehaviour = true;
			for( int i = 0; i < numBoids - oldCount; ++i )
				CreateBoid( GetWorld().RandomWindowPosition() );
		}
		else
		{
			auto objects = GetWorld().GetObjects();
			for( int i = 0; i < oldCount - numBoids; ++i )
				objects[objects.size() - 1 - i].Destroy();
		}
	}

	int item_current = ( int )demoType;
	if( ImGui::Combo( "Demo Type", &item_current, demoTypeNames, IM_ARRAYSIZE( demoTypeNames ) ) || updateBehaviour )
	{
		demoType = DemoTypes( item_current );

		switch( demoType )
		{
		case DemoTypes::Wander:
			Wander( !updateBehaviour ); break;
		case DemoTypes::Seek:
			Seek( !updateBehaviour ); break;
		case DemoTypes::Flee:
			Flee( !updateBehaviour ); break;
		case DemoTypes::Alignment:
		case DemoTypes::Cohesion:
		case DemoTypes::Separation:
		case DemoTypes::Flocking:
		case DemoTypes::FlockingWithPredators:
		case DemoTypes::CollisionAvoidance:
		case DemoTypes::PathFollowing:
		case DemoTypes::LeaderFollowing:
			//LeaderFollowing(!updateBehaviour);
			break;
		case DemoTypes::Queueing:
			//Queueing(!updateBehaviour);
			break;
		}
	}

	ImGui::NewLine();

	bool changed = ImGui::SliderFloat( "Max Velocity", &maxVelocity, 0.0f, 1000.0f );

	switch( demoType)
	{
	case DemoTypes::Wander:
		RenderWander( changed ); break;
	case DemoTypes::Seek:
		RenderSeek( changed ); break;
	case DemoTypes::Flee:
		break;
	case DemoTypes::Alignment:
		break;
	case DemoTypes::Cohesion:
		break;
	case DemoTypes::Separation:
		break;
	case DemoTypes::Flocking:
		break;
	case DemoTypes::FlockingWithPredators:
		break;
	case DemoTypes::CollisionAvoidance:
		break;
	case DemoTypes::PathFollowing:
		break;
	case DemoTypes::LeaderFollowing:
		break;
	case DemoTypes::Queueing:
		break;
	}

	const bool reset = ImGui::Button( "Reset" );

	if( reset )
	{
		GetWorld().GetSystem< Reflex::Systems::SteeringSystem >()->ForEachObject< Reflex::Components::Steering >(
			[&]( const Reflex::Components::Steering::Handle& boid )
		{
			boid->GetTransform()->setPosition( GetWorld().RandomWindowPosition() );
		} );
	}

	ImGui::End();
}

void GameState::Update( const float deltaTime )
{
	switch( demoType )
	{
	case DemoTypes::Seek:
	case DemoTypes::Flee:
		mousePosition.GetTransform()->setPosition( GetWorld().GetMousePosition() );
		break;
	}
}

void GameState::Wander( bool defaultValues )
{
	if( defaultValues )
	{
		wanderCircleRadius = 45.0f;
		wanderCircleDistance = 45.0f;
		wanderAngleDelta = TORADIANS( 25.0f );
	}

	GetWorld().GetSystem< Reflex::Systems::SteeringSystem >()->ForEachObject< Reflex::Components::Steering >(
		[&]( const Reflex::Components::Steering::Handle& boid )
	{
		boid->ClearBehaviours();
		boid->Wander( wanderCircleRadius, wanderCircleDistance, wanderAngleDelta, maxVelocity );
	} );
}

void GameState::RenderWander( bool velocityChanged )
{
	bool changed = ImGui::SliderFloat( "Wander Circle Radius", &wanderCircleRadius, 1.0f, 1000.0f ) || velocityChanged;
	changed = ImGui::SliderFloat( "Wander Circle Distance", &wanderCircleDistance, 0.0f, 1000.0f ) || changed;
	changed = ImGui::SliderAngle( "Wander Angle Delta", &wanderAngleDelta ) || changed;

	if( changed )
		Wander( false );
}

void GameState::Seek( bool defaultValues )
{
	if( defaultValues )
	{
		slowingRadius = 10.0f;

		if( !mousePosition )
			mousePosition = GetWorld().CreateObject();
	}

	GetWorld().GetSystem< Reflex::Systems::SteeringSystem >()->ForEachObject< Reflex::Components::Steering >(
		[&]( const Reflex::Components::Steering::Handle& boid )
	{
		boid->ClearBehaviours();
		boid->Pursue( mousePosition, slowingRadius, maxVelocity );
	} );
}

void GameState::RenderSeek( bool velocityChanged )
{
	if( ImGui::SliderFloat( "Slowing Radius", &slowingRadius, 0.0f, 1000.0f ) || velocityChanged )
		Seek( false );
}

void GameState::Flee( bool defaultValues )
{
	if( defaultValues && !mousePosition )
		mousePosition = GetWorld().CreateObject();

	GetWorld().GetSystem< Reflex::Systems::SteeringSystem >()->ForEachObject< Reflex::Components::Steering >(
		[&]( const Reflex::Components::Steering::Handle& boid )
	{
		boid->ClearBehaviours();
		boid->Evade( mousePosition, maxVelocity );
	} );
}