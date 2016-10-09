#include "customstuff.h"



CCustomStuff::CCustomStuff()
{
	Reset();
	m_CameraTargetCenter = vec2(0, 0);
	m_CameraCenter = vec2(0, 0);
	m_SelectedGroup = 0;
}


void CCustomStuff::Reset()
{
	m_CameraShake = 0.0f;
	
	m_LocalPos = vec2(0, 0);
	m_LocalWeapon = 0;
	m_LocalColor = vec4(0, 0, 0, 0);
	
	m_WeaponDropTick = 0;
	m_SwitchTick = 0;
	m_SelectedWeapon = 0;
	m_LocalWeapons = 0;
	m_LocalKits = 0;
	m_Picker = 0;
	m_LastUpdate = time_get();
	m_Tick = 0;
	
	m_LocalTeam = TEAM_SPECTATORS;
	
	for (int i = 0; i < MAX_CLIENTS; i++)
		m_aPlayerInfo[i].Reset();
	
	m_WantedWeapon = 1;
	m_SawbladeAngle = 0;
	m_MonsterAnim = 0;
	
	for (int i = 0; i < MAX_MONSTERS; i++)
	{
		m_MonsterDamageIntensity[i] = 0.0f;
		m_MonsterDamageType[i] = 0;
	}
	
	for (int i = 0; i < NUM_PLAYERITEMS; i++)
		m_aLocalItems[i] = 0;
	
	for (int i = 0; i < 64; i++)
	{
		m_FlametrapState[i] = 0;
		m_FlametrapSoundTick[i] = 0;
		m_FlametrapLastSound[i] = 0;
	}
	
	m_WeaponpickTimer = 0;
	m_WeaponpickWeapon = 0;
	
	m_LastWeaponPicked = true;
}





void CCustomStuff::Tick(bool Paused)
{
	m_Tick++;
	
	// Client side building animation
	m_SawbladeAngle += 0.07f;
	
	if (m_WeaponpickTimer > 0.0f)
	{
		m_WeaponpickTimer -= 0.0035f;
		
		if (m_LastWeaponPicked)
			m_WeaponpickTimer -= 0.0035f;
	}
	
	m_MonsterAnim += 0.006f;
	//if (m_MonsterAnim >= 1.0f)
	//	m_MonsterAnim -= 1.0f;
	
	if (!Paused)
	{
		for (int i = 0; i < MAX_CLIENTS; i++)
			m_aPlayerInfo[i].Tick();
		
		for (int i = 0; i < MAX_MONSTERS; i++)
			if (m_MonsterDamageIntensity[i] > 0.0f)
				m_MonsterDamageIntensity[i] -= 0.05f;
			
		for (int i = 0; i < 64; i++)
		{
			if (m_FlametrapState[i] > 0)
			{
				m_FlametrapState[i]++;
				
				if (m_FlametrapState[i] > 13*6)
					m_FlametrapState[i] = 0;
			}
		}
		
		/*
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (m_aPlayerInfo[i].m_InUse)
			{
				m_pClient->AddFluidForce(m_aPlayerInfo[i].m_Pos, m_aPlayerInfo[i].m_Vel);
			}
		}
		*/
	}
	
	// Camera
	
	m_CameraCenter.x += (m_CameraTargetCenter.x-m_CameraCenter.x) / 24.0f;
	m_CameraCenter.y += (m_CameraTargetCenter.y-m_CameraCenter.y) / 24.0f;
	
	if (m_CameraShake > 0.0f)
		m_CameraShake -= 0.2f;
}


void CCustomStuff::Update(bool Paused)
{
	int64 currentTime = time_get();
	if ((currentTime-m_LastUpdate > time_freq()) || (m_LastUpdate == 0))
		m_LastUpdate = currentTime;
		
	int step = time_freq()/120;
	
	if (step <= 0)
		step = 1;
	
	int i = 0;
	for (;m_LastUpdate < currentTime; m_LastUpdate += step)
	{
		Tick(Paused);
		if (i++ > 20)
			break;
	}
	
}
