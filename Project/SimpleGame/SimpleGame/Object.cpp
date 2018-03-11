#include "stdafx.h"
#include "Renderer.h"
#include "Object.h"


void Object::releaseObject()
{
}

bool Object::wallchk()
{
	if (m_Position.x > WINDOW_WIDTH / 2 || m_Position.x < -WINDOW_WIDTH / 2) {
		m_Direction.x *= -1;
		m_Position.x += m_Direction.x * 3;
		return true;
	}
	if (m_Position.y > WINDOW_HEIGHT / 2 || m_Position.y < -WINDOW_HEIGHT / 2) {
		m_Direction.y *= -1;
		m_Position.y += m_Direction.y * 3;
		return true;
	}
	return false;
}

bool Object::isIntersect(Object* target)
{
	if ((m_Position.x - target->getPosition().x)*(m_Position.x - target->getPosition().x) +
		(m_Position.y - target->getPosition().y)*(m_Position.y - target->getPosition().y) <
		(m_Size + target->getSize())*(m_Size + target->getSize())*0.25)
	{
		setTarget(target);
		target->setTarget(this);
		//setColor(COLOR{ 1,1,1,1 });
		return true;
	}
	else {
		releaseTarget();
		target->releaseTarget();
		//setColor(m_DefaultColor);
	}
	return false;
}

void Object::resetObject()
{
	m_Position = { -500, -500, 0 };
	m_Direction = { 0,0,0 };
	m_Speed = 0;
	m_TargetBind = nullptr;
	m_Life = -1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

void Sprite::render(Renderer * renderer, int texID)
{
	DrawTexturedSeqRectByMatrix(m_Position, renderer, 40, { 1,1,1 }, m_AnimationSeqX, m_AnimationSeqY, m_limitSeqX, m_limitSeqY, texID, 0.1);
}

void Sprite::SetSeq(int mx, int my, int lx, int ly)
{
	m_AnimationSeqX = 0;
	m_AnimationSeqY = 0;
	m_MaxSeqX = mx;
	m_MaxSeqY = my;
	m_limitSeqX = lx;
	m_limitSeqY = ly;
}

bool Sprite::AddSeq(const double timeElapsed)
{
	m_AnimationTime += timeElapsed;
	if (m_AnimationTime > 0.1) {
		m_AnimationTime = 0;
		m_AnimationSeqX++;
		if (m_AnimationSeqX > m_MaxSeqX)
		{
			m_AnimationSeqY++;
			m_AnimationSeqX = 0;
		}
		if (m_AnimationSeqY > m_MaxSeqY)
		{
			return true;
		}
	}
	return false;
}
