#pragma once

#include "../../../ReflexEngine/ReflexEngine/Component.h"
#include <SFML/Graphics/Color.hpp>

class BoidVisual : public Reflex::Components::Component< BoidVisual >
{
public:
	BoidVisual( const Reflex::Object& object, const sf::Color& colour = sf::Color::White )
		: Reflex::Components::Component< BoidVisual >( object )
	{
		vertices.setPrimitiveType( sf::Triangles );
		const float length = 4.5f;
		const float tail = 2.0f;
		const float width = 4.0f;
		vertices.append( sf::Vector2f( -1.0f, 0.0f ) );
		vertices.append( sf::Vector2f( -tail, width / 2.0f ) );
		vertices.append( sf::Vector2f( length, 0.0f ) );

		vertices.append( sf::Vector2f( -1.0f, 0.0f ) );
		vertices.append( sf::Vector2f( -tail, -width / 2.0f ) );
		vertices.append( sf::Vector2f( length, 0.0f ) );

		SetColour( colour );
	}

	bool IsRenderComponent() const final { return true; }
	void Render( sf::RenderTarget& target, sf::RenderStates states ) const final
	{
		target.draw( vertices, states );
	}

	void SetColour( const sf::Color& colour )
	{
		for( unsigned i = 0; i < vertices.getVertexCount(); ++i )
			vertices[i].color = colour;
	}

protected:
	sf::VertexArray vertices;
};