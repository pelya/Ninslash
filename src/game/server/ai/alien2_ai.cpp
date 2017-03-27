#include <engine/shared/config.h>

#include <game/server/ai.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include <game/server/gamecontext.h>

#include "alien2_ai.h"


CAIalien2::CAIalien2(CGameContext *pGameServer, CPlayer *pPlayer)
: CAI(pGameServer, pPlayer)
{
	m_SkipMoveUpdate = 0;
	Player()->SetCustomSkin(5);
	m_StartPos = vec2(0, 0);
}


void CAIalien2::OnCharacterSpawn(CCharacter *pChr)
{
	CAI::OnCharacterSpawn(pChr);
	
	
	m_WaypointDir = vec2(0, 0);
	//Player()->SetRandomSkin();
	
	m_PowerLevel = 4;
	
	m_StartPos = Player()->GetCharacter()->m_Pos;
	m_TargetPos = Player()->GetCharacter()->m_Pos;
	m_Triggered = false;
	
	if (frandom() < 0.4f)
		pChr->GiveCustomWeapon(WEAPON_SHOTGUN);
	else if (frandom() < 0.4f)
		pChr->GiveCustomWeapon(WEAPON_CHAINSAW);
	else if (frandom() < 0.4f)
		pChr->GiveCustomWeapon(WEAPON_GRENADE);
	else
		pChr->GiveCustomWeapon(WEAPON_FLAMER);
	
	pChr->SetHealth(50);
}


void CAIalien2::DoBehavior()
{
	m_Attack = 0;	
	
	HeadToMovingDirection();
	SeekClosestEnemyInSight();

	bool Jump = false;
	
	// if we see a player
	if (m_EnemiesInSight > 0)
	{
		ReactToPlayer();
		
		if (ShootAtClosestEnemy())
		{
		}
		else
		{
			if (SeekClosestEnemy())
			{
				m_TargetPos = m_PlayerPos;
				
				if (WeaponShootRange() - m_PlayerDistance > 200)
					SeekRandomWaypoint();
			}
		}
	}
	else
	{
		//if (Player()->GetCharacter()->PlayerCollision() && frandom()*10 < 3)
		//	Jump = true;
			
		
		ShootAtClosestBuilding();
		
		if (SeekClosestEnemy())
			m_TargetPos = m_PlayerPos;
		else if(SeekRandomEnemy())
		{
			m_TargetPos = m_PlayerPos;
				
			if (WeaponShootRange() - m_PlayerDistance > 200)
				SeekRandomWaypoint();
		}
	}


	if (abs(m_Pos.x - m_TargetPos.x) < 40 && abs(m_Pos.y - m_TargetPos.y) < 40)
	{
		// stand still
		m_Move = 0;
	}
	else
	{
		if (UpdateWaypoint())
		{
			MoveTowardsWaypoint();
		}
		else
		{
			m_WaypointPos = m_TargetPos;
			MoveTowardsWaypoint(true);
		}
	}
	
	if (Jump)
		m_Jump = 1;
	
	Player()->GetCharacter()->m_SkipPickups = 999;

	RandomlyStopShooting();
	
	// next reaction in
	m_ReactionTime = 1 + frandom()*3;
	
}
