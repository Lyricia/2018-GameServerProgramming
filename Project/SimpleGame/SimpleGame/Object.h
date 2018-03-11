#pragma once

struct COLOR {
	float r;
	float g;
	float b;
	float a;
};

class Object
{
protected:
	int					m_id;
	OBJTYPE				m_type;
	Vector3D<float>		m_Position;
	Vector3D<float>		m_Direction;
	float				m_Speed;
	int					m_Size;
	RECT				m_BoundingBox;
	COLOR				m_DefaultColor;
	COLOR				m_Color;

	Object*				m_TargetBind = NULL;

	double				m_Life = 0.f;
	double				m_Lifetime = 0.f;
	double				m_damagedtime = 0.f;

	int					m_Team;

public:
	Object() {};
	Object(OBJTYPE type, int size, Vector3D<float> pos) : m_type(type), m_Position(pos), m_Size(size) {};
	~Object() { delete m_TargetBind; };
	void releaseObject();

	Vector3D<float>	getPosition() { return m_Position; }
	int				getSize() { return m_Size; }
	bool			isAlive() { if (m_Life <= 0 || m_Lifetime <= 0) return false; else return true; }

	void setPosition(float x, float y, float z) { m_Position = { x, y, z }; }
	void setPosition(Vector3D<float> pos) { m_Position = pos; }
	void setDirection(Vector3D<float> dir) { m_Direction = dir; }
	void setDirection(float x, float y, float z) { m_Direction = { x, y, z }; }
	void setSpeed(float speed) { m_Speed = speed; }
	void setOOBB(RECT boundingbox) { m_BoundingBox = boundingbox; }
	void setDefaultColor(COLOR color) { m_DefaultColor = m_Color = color; }
	void setColor(COLOR color) { m_Color = color; }
	void setID(int id) { m_id = id; }
	void setLifetime(double lifetime) { m_Lifetime = lifetime; }
	void setLife(double life) { m_Life = life; }
	void setTeam(int team) { m_Team = team; }
	void decreaseLife(double dmg) { m_Life -= dmg; m_damagedtime = 0.1f; }

	void move(const double timeElapsed) { m_Position += m_Direction * m_Speed * timeElapsed; };

	void setTarget(Object* target) { m_TargetBind = target; }
	Object* getTarget() { return m_TargetBind; }
	OBJTYPE getType() { return m_type; }
	int getTeam() { return m_Team; }
	double getLife() { return m_Life; }
	double getLifetime() { return m_Lifetime; }
	const int getID() { return m_id; }
	Vector3D<float> getDirection() { return m_Direction; }
	void releaseTarget() { m_TargetBind = nullptr; }

	bool wallchk();
	bool isIntersect(Object* target);

	virtual void update(const double timeElapsed) = 0;
	virtual void render(Renderer* renderer, int texID = NULL) = 0;

	void resetObject();
};

class Projectile : public Object
{
private:
	int		m_Owner;
	double	m_AnimationTime = 0.f;

public:
	Projectile() {};
	Projectile(OBJTYPE type, int size, Vector3D<float> pos) : Object(type, size, pos) {};
	~Projectile() {};

	void setOwner(int id) { m_Owner = id; }
	int getOwner() { return m_Owner; }
	virtual void update(const double timeElapsed);
	virtual void render(Renderer* renderer, int texID = NULL);
};

class Player : public Object
{
private:
	double	m_cooltime = 0;
	double	m_AnimationTime = 0.f;
	int		m_AnimationSeqX, m_AnimationSeqY;
	int		m_MaxSeqX, m_MaxSeqY, m_limitSeqX, m_limitSeqY;

public:
	Player() {};
	Player(OBJTYPE type, int size, Vector3D<float> pos) :Object(type, size, pos) {};
	~Player() {};

	void SetSeq(int mx, int my, int lx, int ly);
	virtual void update(const double timeElapsed);
	virtual void render(Renderer* renderer, int texID = NULL);
	Projectile* ShootArrow();
	bool cooltimeChk(const double timeElapsed);
};

class Building : public Object
{
private:
	double		m_cooltime = 0.f;

public:
	Building() {};
	Building(int size, Vector3D<float> pos) : Object(OBJTYPE::OBJ_BUILDING, size, pos) {}
	~Building() {};

	Projectile* ShootBullet();
	bool cooltimeChk(const double timeElapsed);
	virtual void update(const double timeElapsed);
	virtual void render(Renderer* renderer, int texID = NULL);
};

class Sprite : public Object
{
private:
	double			m_AnimationTime = 0.f;
	int				m_AnimationSeqX, m_AnimationSeqY;
	int				m_MaxSeqX, m_MaxSeqY, m_limitSeqX, m_limitSeqY;

public:
	Sprite() {}
	Sprite(Vector3D<float> pos) { setPosition(pos); SetSeq(3, 3, 4, 4); }
	~Sprite() {}
	virtual void update(const double timeElapsed) {};
	virtual void render(Renderer* renderer, int texID = NULL);
	void SetSeq(int mx, int my, int lx, int ly);
	bool AddSeq(const double timeElapsed);
};

inline void DrawSolidRectByMatrix(Vector3D<float> pos, Renderer* Renderer, int size, COLOR color, float level)
{
	Renderer->DrawSolidRect(pos.x, pos.y, pos.z, size, color.r, color.g, color.b, color.a, level);
}

inline void DrawTexturedSeqRectByMatrix(Vector3D<float> pos, Renderer* Renderer, int size, COLOR color, int x, int y, int totalx, int totaly, GLuint texID, float level)
{
	Renderer->DrawTexturedRectSeq(pos.x, pos.y, pos.z, size, color.r, color.g, color.b, color.a, texID, x, y, totalx, totaly, level);
}

inline void DrawTexturedRectByMatrix(Vector3D<float> pos, Renderer* Renderer, int size, COLOR color, GLuint texID, float level)
{
	Renderer->DrawTexturedRect(pos.x, pos.y, pos.z, size, color.r, color.g, color.b, color.a, texID, level);
}

inline void DrawSolidRectGaugeByMatrix(Vector3D<float> pos, Renderer* Renderer, int size, float width, float height, COLOR color, float gauge, float level)
{
	Renderer->DrawSolidRectGauge(pos.x, pos.y + size, pos.z, width, height, color.r, color.g, color.b, color.a, gauge, level);
}