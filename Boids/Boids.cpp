// Includes
#include "Boids.h"
#include "BoidVisual.h"

// Entry point
int main()
{
	Engine engine( "AI Steering Demo", sf::seconds( 1.0f / 30.0f ), false );
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
	auto newBoid = GetWorld().CreateObject( pos, 0.0f, sf::Vector2f( 3.0f, 3.0f ) );
	auto boid = newBoid.AddComponent< Reflex::Components::Steering >();

	auto circle = newBoid.AddComponent< BoidVisual >( sf::Color::White );
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
			Alignment( !updateBehaviour ); break;
		case DemoTypes::Cohesion:
			Cohesion( !updateBehaviour ); break;
		case DemoTypes::Separation:
			Separation( !updateBehaviour ); break;
		case DemoTypes::Flocking:
			Flocking( !updateBehaviour ); break;
		case DemoTypes::FlockingWithPredators:
			Flocking( !updateBehaviour ); break;
			Predators( !updateBehaviour ); break;
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
		RenderFlee( changed ); break;
	case DemoTypes::Alignment:
		RenderAlignment( changed ); break;
	case DemoTypes::Cohesion:
		RenderCohesion( changed ); break;
	case DemoTypes::Separation:
		RenderSeparation( changed ); break;
	case DemoTypes::Flocking:
		RenderFlocking( changed ); break;
	case DemoTypes::FlockingWithPredators:
		RenderFlocking( changed ); break;
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
	bool changed = ImGui::SliderFloat( "Slowing Radius", &slowingRadius, 0.0f, 1000.0f ) || velocityChanged;
	changed = ImGui::SliderFloat( "Effective Distance", &ignoreDistance, 0.0f, 1000.0f ) || changed;
	
	if( changed )
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
		boid->Evade( mousePosition, ignoreDistance, maxVelocity );
	} );
}

void GameState::RenderFlee( bool velocityChanged )
{
	if( ImGui::SliderFloat( "Effective Distance", &ignoreDistance, 0.0f, 1000.0f ) || velocityChanged )
		Flee( false );
}

void GameState::Alignment( bool defaultValues )
{
	GetWorld().GetSystem< Reflex::Systems::SteeringSystem >()->ForEachObject< Reflex::Components::Steering >(
		[&]( const Reflex::Components::Steering::Handle& boid )
	{
		boid->ClearBehaviours();
		boid->Wander( wanderCircleRadius, wanderCircleDistance, wanderAngleDelta, maxVelocity );
		boid->Alignment( neighbourRange, alignmentForce, maxVelocity );
	} );
}

void GameState::RenderAlignment( bool velocityChanged )
{
	bool changed = ImGui::SliderFloat( "Alignment Force", &alignmentForce, 0.0f, 2.0f ) || velocityChanged;
	changed = ImGui::SliderFloat( "Neighbour Range", &neighbourRange, 0.0f, 3000.0f ) || changed;

	if( changed )
		Alignment( false );
}

void GameState::Cohesion( bool defaultValues )
{
	GetWorld().GetSystem< Reflex::Systems::SteeringSystem >()->ForEachObject< Reflex::Components::Steering >(
		[&]( const Reflex::Components::Steering::Handle& boid )
	{
		boid->ClearBehaviours();
		boid->Wander( wanderCircleRadius, wanderCircleDistance, wanderAngleDelta, maxVelocity );
		boid->Cohesion( neighbourRange, cohesionForce, maxVelocity );
	} );
}

void GameState::RenderCohesion( bool velocityChanged )
{
	bool changed = ImGui::SliderFloat( "Cohesion Force", &cohesionForce, 0.0f, 2.0f ) || velocityChanged;
	changed = ImGui::SliderFloat( "Neighbour Range", &neighbourRange, 0.0f, 3000.0f ) || changed;

	if( changed )
		Cohesion( false );
}

void GameState::Separation( bool defaultValues )
{
	GetWorld().GetSystem< Reflex::Systems::SteeringSystem >()->ForEachObject< Reflex::Components::Steering >(
		[&]( const Reflex::Components::Steering::Handle& boid )
	{
		boid->ClearBehaviours();
		boid->Wander( wanderCircleRadius, wanderCircleDistance, wanderAngleDelta, maxVelocity );
		boid->Separation( neighbourRange, separationForce, maxVelocity );
	} );
}

void GameState::RenderSeparation( bool velocityChanged )
{
	bool changed = ImGui::SliderFloat( "Separation Force", &separationForce, 0.0f, 2.0f ) || velocityChanged;
	changed = ImGui::SliderFloat( "Neighbour Range", &neighbourRange, 0.0f, 3000.0f ) || changed;

	if( changed )
		Separation( false );
}

void GameState::Flocking( bool defaultValues )
{
	GetWorld().GetSystem< Reflex::Systems::SteeringSystem >()->ForEachObject< Reflex::Components::Steering >(
		[&]( const Reflex::Components::Steering::Handle& boid )
	{
		boid->ClearBehaviours();
		boid->Wander( wanderCircleRadius, wanderCircleDistance, wanderAngleDelta, maxVelocity );
		boid->Flocking( neighbourRange, alignmentForce, cohesionForce, separationForce, maxVelocity );
	} );
}

void GameState::RenderFlocking( bool velocityChanged )
{
	bool changed = ImGui::SliderFloat( "Alignment Force", &alignmentForce, 0.0f, 2.0f ) || velocityChanged; 
	changed = ImGui::SliderFloat( "Separation Force", &separationForce, 0.0f, 2.0f ) || changed;
	changed = ImGui::SliderFloat( "Cohesion Force", &cohesionForce, 0.0f, 2.0f ) || changed;
	changed = ImGui::SliderFloat( "Neighbour Range", &neighbourRange, 0.0f, 3000.0f ) || changed;

	if( changed )
		Flocking( false );
}

void GameState::Predators( bool defaultValues )
{

}

void GameState::RenderPredators( bool velocityChanged )
{

}