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

void Player::SetSeq(int mx, int my, int lx, int ly)
{
	m_AnimationSeqX = 0;
	m_AnimationSeqY = 0;
	m_MaxSeqX = mx;
	m_MaxSeqY = my;
	m_limitSeqX = lx;
	m_limitSeqY = ly;
}

void Player::update(const double timeElapsed)
{
	move(timeElapsed);
	m_AnimationTime += timeElapsed;
	if (m_AnimationTime > 0.1) {
		m_AnimationTime = 0;
		if (m_AnimationSeqX++ > m_MaxSeqX)
		{
			m_AnimationSeqX = 0;
			if (m_AnimationSeqY++ > m_MaxSeqY)
				m_AnimationSeqY = 0;
		}
	}
}

void Player::render(Renderer* renderer, int texID)
{
	float gauge = m_Life / 100;
	if (m_Team == TEAM::RED)
	{
		DrawSolidRectGaugeByMatrix(m_Position, renderer, 25, 40, 5, { 1,0,0,1 }, gauge, 0.1);
	}
	else if (m_Team == TEAM::BLUE)
	{
		DrawSolidRectGaugeByMatrix(m_Position, renderer, 25, 40, 5, { 0,0,1,1 }, gauge, 0.1);
	}
	DrawTexturedSeqRectByMatrix(m_Position, renderer, 40, { 1,1,1 }, m_AnimationSeqX, m_AnimationSeqY, m_limitSeqX, m_limitSeqY, texID, 0.1);
	//DrawTexturedRectByMatrix(m_Position, renderer, m_Size, COLOR({ 1, 1, 1, 1 }), texID, 0.1);
	//DrawSolidRectByMatrix(m_Position, renderer, m_Size, m_Color, 0.2);
}

bool Player::cooltimeChk(const double timeElapsed)
{
	m_cooltime += timeElapsed;
	if (m_cooltime > 3)
	{
		m_cooltime = 0;
		return true;
	}
	return false;
}

Projectile* Player::ShootArrow()
{
	Projectile* a = new Projectile(OBJTYPE::OBJ_ARROW, 4, Vector3D<float>{0, 0, 0});
	a->setPosition(getPosition());
	a->setLifetime(5);
	a->setLife(10);
	a->setDirection(Vector3D<float>((rand() % 9 - 4.5f), (rand() % 9 - 4.5f), 0.f).Normalize());
	a->setSpeed(100);
	a->setTeam(getTeam());
	if(getTeam()==TEAM::RED)
		a->setDefaultColor(COLOR{ 0.5f, 0.2f, 0.7f, 1 } );
	else if(getTeam()==TEAM::BLUE)
		a->setDefaultColor(COLOR{ 1, 1, 0, 1 }); 
	return a;
}

//////////////////////////////////////////////////////////////////////////////////////////////////

Projectile* Building::ShootBullet()
{
	Projectile* b = new Projectile(OBJTYPE::OBJ_BULLET, 4, getPosition());
	b->setLifetime(5);
	b->setLife(20);
	b->setDirection(Vector3D<float>((rand() % 9 - 4.5f), (rand() % 9 - 4.5f), 0.f).Normalize());
	b->setSpeed(350);
	b->setTeam(getTeam());
	if (getTeam() == TEAM::RED) 
	{
		b->setDefaultColor(COLOR{ 1,0,0,1 });
	}
	else if (getTeam()==TEAM::BLUE)
	{
		b->setDefaultColor(COLOR{ 0,0,1,1 });
	}
	return b;
}

bool Building::cooltimeChk(const double timeElapsed)
{
	m_cooltime += timeElapsed;
	if (m_damagedtime > 0)
		m_damagedtime -= timeElapsed;
	
	if (m_cooltime > 2)
	{
		m_cooltime = 0;
		return true;
	}
	return false;
}

void Building::update(const double timeElapsed)
{
}

void Building::render(Renderer* renderer, int texID)
{
	float gauge = m_Life / 500;
	if (m_Team == TEAM::RED)
	{
		DrawSolidRectGaugeByMatrix(m_Position, renderer, 60, 70, 8, { 1,0,0,1 }, gauge, 0.1);
	}
	else if (m_Team == TEAM::BLUE)
	{
		DrawSolidRectGaugeByMatrix(m_Position, renderer, 60, 70, 8, { 0,0,1,1 }, gauge, 0.1);
	}
	if (m_damagedtime <= 0)
		DrawTexturedRectByMatrix(m_Position, renderer, m_Size, m_Color, texID, 0.1);
		//DrawSolidRectByMatrix(m_Position, renderer, m_Size, m_Color);
}

//////////////////////////////////////////////////////////////////////////////////////////////////


void Projectile::update(const double timeElapsed)
{
	move(timeElapsed);
	m_AnimationTime += timeElapsed;
}

void Projectile::render(Renderer* renderer, int texID)
{
	renderer->DrawParticle(m_Position.x, m_Position.y, m_Position.z, 10, 1, 1, 1, 1, -m_Direction.x, -m_Direction.y, texID, m_AnimationTime,0.2);
	DrawSolidRectByMatrix(m_Position, renderer, m_Size, m_Color, 0.3);
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
