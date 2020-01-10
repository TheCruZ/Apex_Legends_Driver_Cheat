#include "Game.h"


uintptr_t Entity::Observing(uintptr_t sock, int g_PID, uint64_t entitylist) {
	unsigned long ot = *(unsigned long*)(buffer + OFFSET_OBSERVING_TARGET);
	int index = ot & ENT_ENTRY_MASK;
	int ObserverID = index;
	if (ObserverID > 0) {
		uint64_t centity2 = driver::read<uint64_t>(sock, g_PID, entitylist + ((uint64_t)ObserverID << 5));
		return centity2;
	}
	return 0;
}
int Entity::getTeamId() {
	return *(int*)(buffer + OFFSET_TEAM);
}
int Entity::getHealth() {
	return *(int*)(buffer + OFFSET_HEALTH);
}
int Entity::getShield() {
	return *(int*)(buffer + OFFSET_SHIELD);
}
Vector Entity::getPosition() {
	return *(Vector*)(buffer + OFFSET_ORIGIN);
}
bool Entity::isPlayer() {
	return *(uint64_t*)(buffer + OFFSET_NAME) == 125780153691248;
}
Vector Entity::getBonePosition(uintptr_t sock, int g_PID, int id) {
	Vector position = getPosition();
	uintptr_t boneArray = *(uintptr_t*)(buffer + OFFSET_BONES);
	Vector bone = Vector();
	uint32_t boneloc = (id * 0x30);
	Bone bo = {};
	bo = driver::read<Bone>(sock, g_PID, boneArray + boneloc);
	bone.x = bo.x + position.x;
	bone.y = bo.y + position.y;
	bone.z = bo.z + position.z;
	return bone;
}

QAngle Entity::GetViewAngles()
{
	return *(QAngle*)(buffer + OFFSET_VIEWANGLES);
}

Vector Entity::GetViewAnglesV()
{
	return *(Vector*)(buffer + OFFSET_VIEWANGLES);
}

void Entity::SetViewAngles(uintptr_t sock, int pid, SVector angles)
{
	driver::write<SVector>(sock, pid, ptr + OFFSET_VIEWANGLES, angles);
}
void Entity::SetViewAngles(uintptr_t sock, int pid, QAngle& angles)
{
	SetViewAngles(sock, pid, SVector(angles));
}

Vector Entity::GetCamPos()
{
	return *(Vector*)(buffer + OFFSET_CAMERAPOS);
}

QAngle Entity::GetRecoil()
{
	return *(QAngle*)(buffer + OFFSET_AIMPUNCH);
}

float CalculateFov(Entity& from, Entity& target) {
	QAngle ViewAngles = from.GetViewAngles();
	Vector LocalCamera = from.GetCamPos();
	Vector EntityPosition = target.getPosition();
	QAngle Angle = Math::CalcAngle(LocalCamera, EntityPosition);
	return Math::GetFov(ViewAngles, Angle);
}

QAngle CalculateBestBoneAim(SOCKET sock, int pid, Entity& from, uintptr_t t, float max_fov, int spectators) {
	Entity target = getEntity(sock, pid, t);
	int health = target.getHealth();
	if (health < 1 || health > 100) {
		return QAngle(0, 0, 0);
	}

	Vector EntityPosition = target.getPosition();
	Vector LocalPlayerPosition = from.getPosition();
	float dist = LocalPlayerPosition.DistTo(EntityPosition);

	int bone = 2;
	if (dist < 500) {
		bone = 5;
	}

	Vector LocalCamera = from.GetCamPos();
	Vector BonePosition = target.getBonePosition(sock,pid,bone);
	QAngle CalculatedAngles = Math::CalcAngle(LocalCamera, BonePosition);
	QAngle ViewAngles = from.GetViewAngles();
	QAngle Delta = CalculatedAngles - ViewAngles;

	double fov = Math::GetFov(ViewAngles, CalculatedAngles);
	if (fov > max_fov) {
		return QAngle(0, 0, 0);
	}

	QAngle RecoilVec = from.GetRecoil();
	if (RecoilVec.x != 0 || RecoilVec.y != 0) {
		Delta -= RecoilVec;
	}

	

	double smooth = 60.0f;
	if (spectators > 0) {
		smooth = 80.0f;
	}

	Math::NormalizeAngles(Delta);
	if (fov > 2.0f || spectators>0) {
		if (Delta.x > 0.0f) {
			Delta.x /= smooth;
		}
		else {
			Delta.x = ((Delta.x * -1L) / smooth) * -1;
		}

		if (Delta.y > 0.0f) {
			Delta.y /= smooth;
		}
		else {
			Delta.y = ((Delta.y * -1L) / smooth) * -1;
		}
	}

	QAngle SmoothedAngles = ViewAngles + Delta;


	Math::NormalizeAngles(SmoothedAngles);

	return SmoothedAngles;
}

Entity getEntity(uintptr_t sock, int g_PID, uintptr_t ptr) {
	Entity entity = Entity();
	entity.ptr = ptr;
	driver::read_memory(sock, g_PID, ptr, (uintptr_t)entity.buffer, 0x2FF0);
	return entity;
}

bool WorldToScreen(Vector from, float* m_vMatrix, int targetWidth, int targetHeight, Vector& to)
{
	float w = m_vMatrix[12] * from.x + m_vMatrix[13] * from.y + m_vMatrix[14] * from.z + m_vMatrix[15];

	if (w < 0.01f) return false;

	to.x = m_vMatrix[0] * from.x + m_vMatrix[1] * from.y + m_vMatrix[2] * from.z + m_vMatrix[3];
	to.y = m_vMatrix[4] * from.x + m_vMatrix[5] * from.y + m_vMatrix[6] * from.z + m_vMatrix[7];

	float invw = 1.0f / w;
	to.x *= invw;
	to.y *= invw;

	float x = targetWidth / 2;
	float y = targetHeight / 2;

	x += 0.5 * to.x * targetWidth + 0.5;
	y -= 0.5 * to.y * targetHeight + 0.5;

	to.x = x;
	to.y = y;
	to.z = 0;

	return true;
}
