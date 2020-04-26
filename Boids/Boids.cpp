// Includes
#include "Boids.h"
#include "BoidVisual.h"

using namespace Reflex::Components;

// Entry point
int main()
{
	Engine::EngineParams params;
	params.windowName = "AI Steering Demo";
	params.gravity = sf::Vector2f( 0.0f, 9.8f );

	Engine engine( std::move( params ) );
	engine.RegisterState< GameState >( true );
	engine.Run();

	return 0;
}

GameState::GameState( StateManager& stateManager )
	: State( stateManager )
{
	auto camera = GetWorld().CreateObject().AddComponent< Camera >( GetWorld().GetWindowCentre(), GetWorld().GetWindowSize() );
	camera->EnableWASDPanning( sf::Vector2f( 300.0f, 300.0f ) );
	camera->EnableArrowKeyPanning();
	camera->EnableMouseZooming( 1.2f, true );

	mousePosition = GetWorld().CreateObject();

	auto bounds = GetWorld().CreateObject( GetWorld().GetWindowCentre() );
	auto rect = bounds.AddComponent< Reflex::Components::RectangleShape >( GetWorld().GetWindowSize(), sf::Color::Transparent );
	rect->setOutlineColor( sf::Color::White );
	rect->setOutlineThickness( 2.0f );

	for( unsigned i = 0; i < 20; ++i )
		CreateCollider( GetWorld().RandomWindowPosition(), 70.0f );

	for( unsigned i = 0; i < 1; ++i )
		if( const auto boid = CreateBoid( GetWorld().RandomWindowPosition() ) )
			if( i == 0 )
				first = boid;
}

Reflex::Object GameState::CreateBoid( const sf::Vector2f& pos )
{
	auto newBoid = GetWorld().CreateObject( pos, 0.0f, sf::Vector2f( 5.0f, 5.0f ) );
	auto boid = newBoid.AddComponent< Steering >();
	boid->SetTargetObject( mousePosition );

	auto circle = newBoid.AddComponent< BoidVisual >( sf::Color::White );
	return newBoid;
}

Reflex::Object GameState::CreateCollider( const sf::Vector2f& pos, const float radius )
{
	auto newCollider = GetWorld().CreateObject( pos );
	auto circle = newCollider.AddComponent< CircleShape >( radius, 30, sf::Color::Red );
	circle->CreateRigidBody( b2BodyType::b2_staticBody );
	return newCollider;
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
		"LeaderFollowing",
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
		numBoids = Reflex::Clamp( numBoids, 1, 10000 );

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

	//int item_current = ( int )demoType;
	//if( ImGui::Combo( "Demo Type", &item_current, demoTypeNames, IM_ARRAYSIZE( demoTypeNames ) ) || updateBehaviour )
	//{
	//	demoType = DemoTypes( item_current );
	//
	//	switch( demoType )
	//	{
	//	case DemoTypes::Wander:
	//		Wander( !updateBehaviour ); break;
	//	case DemoTypes::Seek:
	//		Seek( !updateBehaviour ); break;
	//	case DemoTypes::Flee:
	//		Flee( !updateBehaviour ); break;
	//	case DemoTypes::Alignment:
	//		Alignment( !updateBehaviour ); break;
	//	case DemoTypes::Cohesion:
	//		Cohesion( !updateBehaviour ); break;
	//	case DemoTypes::Separation:
	//		Separation( !updateBehaviour ); break;
	//	case DemoTypes::Flocking:
	//		Flocking( !updateBehaviour ); break;
	//	case DemoTypes::FlockingWithPredators:
	//		Flocking( !updateBehaviour ); break;
	//		Predators( !updateBehaviour ); break;
	//	case DemoTypes::CollisionAvoidance:
	//		Avoidance( !updateBehaviour ); break;
	//	case DemoTypes::PathFollowing:
	//	case DemoTypes::LeaderFollowing:
	//		//LeaderFollowing(!updateBehaviour);
	//		break;
	//	case DemoTypes::Queueing:
	//		//Queueing(!updateBehaviour);
	//		break;
	//	}
	//}

	ImGui::NewLine();

	auto boid = selected ? selected : first;
	ImGui::Text( selected ? "Selected" : "All" );

	bool changed = ImGui::SliderFloat( "Steering Force Multiplier", &boid->m_forceMultiplier, 0.0f, 1000.0f );
	float maxVel = boid->GetTransform()->GetMaxVelocity();
	if( ImGui::SliderFloat( "Max Velocity", &maxVel, 0.0f, 1000.0f ) )
	{
		boid->GetTransform()->SetMaxVelocity( maxVel );
		changed = true;
	}
	changed = ImGui::SliderFloat( "Max Force", &boid->m_maxForce, 0.0f, 1000.0f ) || changed;

	if( ImGui::CollapsingHeader( "Behaviours" ) )
	{
		bool neighbourRangeRenderer = false;

		for( unsigned i = 0; i < (unsigned )SteeringBehaviours::NumBehaviours; ++i )
		{
			const auto b = SteeringBehaviours( i );
			bool set = boid->IsBehaviourSet( b );
			if( ImGui::Checkbox( steeringBehaviourNames[i].c_str(), &set ) )
			{
				set ? boid->EnableBehaviour( b ) : boid->DisableBehaviour( b );
				changed = true;
			}

			if( set )
			{
				if( !neighbourRangeRenderer && ( b == SteeringBehaviours::Alignment || b == SteeringBehaviours::Cohesion || b == SteeringBehaviours::Separation ) )
				{
					changed = ImGui::SliderFloat( "Neighbour Range", &boid->m_neighbourRange, 0.0f, 3000.0f ) || changed;
					neighbourRangeRenderer = true;
				}

				switch( b )
				{
				case SteeringBehaviours::Wander:
					changed = ImGui::SliderFloat( "Wander Circle Radius", &boid->m_wanderCircleRadius, 1.0f, 1000.0f ) || changed;
					changed = ImGui::SliderFloat( "Wander Circle Distance", &boid->m_wanderCircleDistance, 0.0f, 1000.0f ) || changed;
					changed = ImGui::SliderFloat( "Wander Jitter", &boid->m_wanderJitter, 0.0f, 1000.0f ) || changed;
					changed = ImGui::SliderFloat( "Wander Force", &boid->m_wanderForce, 0.0f, 5.0f ) || changed;
					break;
				case SteeringBehaviours::Arrival:
					boid->DisableBehaviour( SteeringBehaviours::Flee );
					boid->DisableBehaviour( SteeringBehaviours::Seek );
					changed = ImGui::SliderFloat( "Effective Distance", &boid->m_ignoreDistance, 0.0f, 1000.0f ) || changed;
					changed = ImGui::SliderFloat( "Slowing Radius", &boid->m_slowingRadius, 0.0f, 1000.0f ) || changed;
					changed = ImGui::SliderFloat( "Seek Force", &boid->m_cohesionForce, 0.0f, 5.0f ) || changed;
					break;
				case SteeringBehaviours::Seek:
					boid->DisableBehaviour( SteeringBehaviours::Flee );
					boid->DisableBehaviour( SteeringBehaviours::Arrival );
					changed = ImGui::SliderFloat( "Effective Distance", &boid->m_ignoreDistance, 0.0f, 1000.0f ) || changed;
					changed = ImGui::SliderFloat( "Seek Force", &boid->m_cohesionForce, 0.0f, 5.0f ) || changed;
					break;
				case SteeringBehaviours::Flee:
					boid->DisableBehaviour( SteeringBehaviours::Seek );
					boid->DisableBehaviour( SteeringBehaviours::Arrival );
					changed = ImGui::SliderFloat( "Effective Distance", &boid->m_ignoreDistance, 0.0f, 1000.0f ) || changed;
					changed = ImGui::SliderFloat( "Flee Force", &boid->m_seekForce, 0.0f, 1000.0f ) || changed;
					break;
				case SteeringBehaviours::Alignment:
					changed = ImGui::SliderFloat( "Alignment Force", &boid->m_alignmentForce, 0.0f, 100.0f ) || changed;
					break;
				case SteeringBehaviours::Cohesion:
					changed = ImGui::SliderFloat( "Cohesion Force", &boid->m_cohesionForce, 0.0f, 100.0f ) || changed;
					break;
				case SteeringBehaviours::Separation:
					changed = ImGui::SliderFloat( "Separation Force", &boid->m_separationForce, 0.0f, 100.0f ) || changed;
					break;
				case SteeringBehaviours::ObstacleAvoidance:
					changed = ImGui::SliderFloat( "Avoidance Trace Length", &boid->m_avoidanceTraceLength, 1.0f, 100.0f ) || changed;
					changed = ImGui::SliderFloat( "Avoidance Force", &boid->m_avoidanceForce, 0.0f, 5.0f ) || changed;
					break;
				}
			}
		}
	}

	const bool randomise = ImGui::Button( "Random Positions" );

	if( randomise || ( changed && !selected ) )
	{
		GetWorld().GetSystem< Reflex::Systems::SteeringSystem >()->ForEachObject< Reflex::Components::Steering >(
			[&]( const Steering::Handle& boid )
		{
			if( randomise )
				boid->GetTransform()->setPosition( GetWorld().RandomWindowPosition() );

			if( changed && boid != first )
				boid->CopyValuesFrom( first );
		} );
	}

	ImGui::End();
}

void GameState::Update( const float deltaTime )
{
	auto boid = selected ? selected : first;
	if( boid->IsBehaviourSet( SteeringBehaviours::Seek ) ||
		boid->IsBehaviourSet( SteeringBehaviours::Flee ) ||
		boid->IsBehaviourSet( SteeringBehaviours::Arrival ) )
	{
		mousePosition.GetTransform()->setPosition( GetWorld().GetMousePosition() );
	}
}
