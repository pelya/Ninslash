#include <game/generated/protocol.h>
#include <game/server/gamecontext.h>
#include "building.h"
#include "lightning.h"
#include "teslacoil.h"

CTeslacoil::CTeslacoil(CGameWorld *pGameWorld, vec2 Pos, int Team)
: CBuilding(pGameWorld, Pos, BUILDING_TESLACOIL, Team)
{
	m_ProximityRadius = TeslacoilPhysSize;
	m_Life = 80;
	m_MaxLife = 80;
	
	m_OwnerPlayer = -1;
	m_AttackTick = Server()->Tick() + Server()->TickSpeed()*frandom();
	
	m_Center = vec2(0, -55);
	m_FlipY = 1;
	
	if (GameServer()->Collision()->IsTileSolid(Pos.x, Pos.y - 40))
	{
		m_Mirror = true;
		m_Center = vec2(0, +55);
		m_FlipY = -1;
	}
}





void CTeslacoil::Tick()
{
	if (m_Life < 40)
		m_aStatus[BSTATUS_REPAIR] = 1;
	else
		m_aStatus[BSTATUS_REPAIR] = 0;
	
	UpdateStatus();

	if (Server()->Tick() > m_AttackTick + Server()->TickSpeed()*(0.3f + frandom()*0.3f))
		Fire();
	
	// destroy
	if (m_Life <= 0)
	{
		GameServer()->CreateExplosion(m_Pos + vec2(0, -50*m_FlipY), m_DamageOwner, WEAPON_HAMMER, 0, false, false);
		GameServer()->CreateSound(m_Pos + vec2(0, -50*m_FlipY), SOUND_GRENADE_EXPLODE);
		GameServer()->m_World.DestroyEntity(this);
	}
}


void CTeslacoil::Fire()
{
	m_AttackTick = Server()->Tick();

	vec2 TurretPos = m_Pos+vec2(0, -67*m_FlipY);
	
	bool Sound = false;
	
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;

		if (pPlayer->GetTeam() == m_Team && GameServer()->m_pController->IsTeamplay())
			continue;

		CCharacter *pCharacter = pPlayer->GetCharacter();
		if (!pCharacter)
			continue;
		
		if ((!pCharacter->IsAlive() || pCharacter->GetPlayer()->GetCID() == m_OwnerPlayer) && !GameServer()->m_pController->IsTeamplay())
			continue;
		
		if (GameServer()->m_pController->IsCoop() && !pCharacter->m_IsBot)
			continue;
		
		if (pCharacter->Invisible())
			continue;
			
		int Distance = distance(pCharacter->m_Pos, TurretPos);
		if (Distance < 700 && !GameServer()->Collision()->FastIntersectLine(pCharacter->m_Pos, TurretPos))
		{
			new CLightning(GameWorld(), TurretPos, pCharacter->m_Pos);
			pCharacter->TakeDamage(vec2(0, 0), 5, m_DamageOwner, DEATHTYPE_TESLACOIL, vec2(0, 0), DAMAGETYPE_ELECTRIC, false);
			Sound = true;
		}
	}
	
	if (Sound)
		GameServer()->CreateSound(m_Pos + vec2(0, -50*m_FlipY), SOUND_TESLACOIL_FIRE);
}


void CTeslacoil::TickPaused()
{
}

void CTeslacoil::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	CNetObj_Building *pP = static_cast<CNetObj_Building *>(Server()->SnapNewItem(NETOBJTYPE_BUILDING, m_ID, sizeof(CNetObj_Building)));
	if(!pP)
		return;

	pP->m_X = (int)m_Pos.x;
	pP->m_Y = (int)m_Pos.y;
	pP->m_Status = m_Status;
	pP->m_Type = m_Type;

	if (!GameServer()->m_pController->IsTeamplay() && SnappingClient == m_OwnerPlayer)
		pP->m_Team = TEAM_BLUE;
	else
		pP->m_Team = m_Team;
}
