#ifndef GAME_SERVER_ENTITIES_PROJECTILE_H
#define GAME_SERVER_ENTITIES_PROJECTILE_H

enum ProjectileExtraInfo
{
	PROJ_BIGEXPLOSION,
	PROJ_SUPEREXPLOSION,
	PROJ_SLEEPEFFECT
};

enum ExplosionType
{
	NO_EXPLOSION,
	EXPLOSION_EXPLOSION,
	EXPLOSION_FLAME,
	EXPLOSION_ELECTRIC,
};

class CProjectile : public CEntity
{
public:
	CProjectile(CGameWorld *pGameWorld, int Weapon, int Owner, vec2 Pos, vec2 Dir, int Span,
		int Damage, int Explosive, float Force, int SoundImpact);

	vec2 GetPos(float Time);
	void FillInfo(CNetObj_Projectile *pProj);
	
	void SetPowerLevel(int PowerLevel);
	
	bool Bounce(vec2 Pos);
	int BounceTick;
	
	virtual void Reset();
	virtual void Tick();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);
	
	class CBuilding *m_OwnerBuilding;

private:
	vec2 m_Direction;
	int m_PowerLevel;
	int m_LifeSpan;
	int m_Owner;
	int m_Weapon;
	int m_Damage;
	int m_SoundImpact;
	float m_Force;
	int m_StartTick;
	int m_Explosive;
	bool m_Bouncy;
	
	int m_ElectroTimer;
};

#endif
