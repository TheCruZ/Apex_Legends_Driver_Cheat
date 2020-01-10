#pragma once
#include <stdio.h>
#include "driver.h"
#include "math.h"
#include "offsets.h"

#define NUM_ENT_ENTRIES			(1 << 12)
#define ENT_ENTRY_MASK			(NUM_ENT_ENTRIES - 1)

struct Bone {
	BYTE shit[0xCC];
	float x;
	BYTE shit2[0xC];
	float y;
	BYTE shit3[0xC];
	float z;
};
class Entity {
public:
	uint64_t ptr;
	BYTE buffer[0x2FF0];
	Vector getPosition();
	bool isPlayer();
	int getTeamId();
	int getHealth();
	int getShield();
	QAngle GetViewAngles();
	Vector GetCamPos();
	QAngle GetRecoil();
	Vector GetViewAnglesV();

	void SetViewAngles(uintptr_t sock, int pid, SVector angles);
	void SetViewAngles(uintptr_t sock, int pid, QAngle& angles);
	Vector getBonePosition(uintptr_t sock, int g_PID, int id);
	uintptr_t Observing(uintptr_t sock, int g_PID, uint64_t entitylist);

private:
	struct Bone {
		BYTE shit[0xCC];
		float x;
		BYTE shit2[0xC];
		float y;
		BYTE shit3[0xC];
		float z;
	};
};
Entity getEntity(uintptr_t sock, int g_PID, uintptr_t ptr);
bool WorldToScreen(Vector from, float* m_vMatrix, int targetWidth, int targetHeight, Vector& to);
//Vector GetEntityBasePosition(SOCKET sock, int pid, uintptr_t ent);
//uintptr_t GetEntityBoneArray(SOCKET sock, int pid, uintptr_t ent);
//Vector GetEntityBonePosition(SOCKET sock, int pid, uintptr_t ent, uint32_t BoneId, Vector BasePosition);
//QAngle GetViewAnglesA(SOCKET sock, int pid, uintptr_t ent);
//void SetViewAngles(SOCKET sock, int pid, uintptr_t ent, SVector angles);
//void SetViewAngles(SOCKET sock, int pid, uintptr_t ent, QAngle angles);
//Vector GetCamPos(SOCKET sock, int pid, uintptr_t ent);
float CalculateFov(Entity& from, Entity& target);
QAngle CalculateBestBoneAim(SOCKET sock, int pid, Entity& from, uintptr_t target, float max_fov, int spectators);