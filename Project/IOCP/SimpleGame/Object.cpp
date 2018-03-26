#include "stdafx.h"
#include "Renderer.h"
#include "Object.h"


Object::Object()
{
}

void Object::releaseObject()
{
}

void Object::move(DIR dir)
{
	switch (dir)
	{
	case DIR::UP:
		if (m_Position.y-- < 2) m_Position.y++;
		break;
	case DIR::DOWN:
		if (m_Position.y++ > 8) m_Position.y--;
		break;
	case DIR::LEFT:
		if (m_Position.x-- < 2) m_Position.x++;
		break;
	case DIR::RIGHT:
		if (m_Position.x++ > 8) m_Position.x--;
		break;

	default:break;
	}
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
