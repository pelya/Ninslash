

#ifndef GAME_SERVER_ENTITIES_LASER_H
#define GAME_SERVER_ENTITIES_LASER_H

#include <game/server/entity.h>

class CLaser : public CEntity
{
public:
	CLaser(CGameWorld *pGameWorld, vec2 Pos, vec2 Direction, float StartEnergy, int Owner, int Damage, int PowerLevel, class CBuilding *OwnerBuilding = NULL);

	virtual void Reset();
	virtual void Tick();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);

	class CBuilding *m_OwnerBuilding;

protected:
	bool HitScythe(vec2 From, vec2 To);
	bool HitCharacter(vec2 From, vec2 To);
	bool HitMonster(vec2 From, vec2 To);
	bool HitBuilding(vec2 From, vec2 To);
	void DoBounce();

private:
	vec2 m_From;
	vec2 m_Dir;
	float m_Energy;
	int m_Bounces;
	int m_EvalTick;
	int m_Owner;
	int m_Damage;
	int m_PowerLevel;
	
	int m_IgnoreScythe;
};

#endif
