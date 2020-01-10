#include "main.h"

bool active = true;
SOCKET g_Sock;
SOCKET g_Sock2;
SOCKET g_Sock3;
DWORD g_PID;
uint64_t g_Base;

uintptr_t aimentity = 0;
uintptr_t tmp_aimentity = 0;
float max = 999.0f;
int team_player = 0;
int spectators = 0;
int tmp_spectators = 0;
int playerId = 0;
int s_FOV = 15;
int toRead = 150;

unsigned long getms() {
	namespace sc = std::chrono;
	sc::milliseconds ms = sc::duration_cast<sc::milliseconds>(sc::system_clock::now().time_since_epoch());
	return ms.count();
}

void ProcessPlayer(SOCKET sock, Entity& LPlayer, Entity& target, UINT64 entitylist) {
	if (target.Observing(sock, g_PID, entitylist) == LPlayer.ptr) {
		tmp_spectators++;
		system("color 4E");
	}

	Vector EntityPosition = target.getPosition();
	Vector LocalPlayerPosition = LPlayer.getPosition();
	float dist = LocalPlayerPosition.DistTo(EntityPosition);
	if (dist > 8000.0f) return;

	int health = target.getHealth();
	if (health < 1 || health > 100)
		return;

	int entity_team = target.getTeamId();
	if (entity_team < 0 || entity_team>50 || entity_team == team_player) return;

	//driver::write<BYTE>(sock, g_PID, centity + OFFSET_GLOW_ENABLE, 1);
	//driver::write<BYTE>(sock, g_PID, centity + OFFSET_GLOW_CONTEXT, 1);
	//driver::write<Color>(sock, g_PID, centity + OFFSET_GLOW_COLORS, Color(50.0f,0.0f,0.0f));
	//driver::write<Fade>(sock, g_PID, centity + OFFSET_GLOW_FADE, Fade(FLT_MAX));
	//driver::write<float>(sock, g_PID, centity + OFFSET_GLOW_RANGE, FLT_MAX);

	float fov = CalculateFov(LPlayer, target);
	if (fov < max) {
		max = fov;
		tmp_aimentity = target.ptr;
	}
}

void DoActions() {
	SOCKET sock = g_Sock;
	J();
	unsigned long ms = getms();
	while (active)
	{
		Sleep(1);
		J();
		if (GetKeyState(0x73) & 0x8000) {
			active = false;
			break;
		}
		if (getms() - 500UL > ms) {
			uint64_t LocalPlayer = driver::read<uint64_t>(sock, g_PID, g_Base + OFFSET_LOCAL_ENT);
			if (LocalPlayer == 0) break;

			Entity LPlayer = getEntity(sock, g_PID, LocalPlayer);

			team_player = LPlayer.getTeamId();
			if (team_player < 0 || team_player>50) break;

			ms = getms();
			uint64_t entitylist = g_Base + OFFSET_ENTITYLIST;

			uint64_t baseent = driver::read<uint64_t>(sock, g_PID, entitylist);
			if (baseent == 0) break;

			max = 999.0f;
			int cc = 0;
			tmp_spectators = 0;
			int lastIndexReadedCorrectly = 0;

			for (int i = 0; i <= toRead; i++)
			{
				uint64_t centity = driver::read<uint64_t>(sock, g_PID, entitylist + ((uint64_t)i << 5));
				if (centity == 0) continue;
				if (LocalPlayer == centity) continue;

				Entity Target = getEntity(sock, g_PID, centity);
				if (!Target.isPlayer()) continue;

				cc++;
				lastIndexReadedCorrectly = i;

				ProcessPlayer(sock, LPlayer, Target, entitylist);
			}
			if (toRead == 0) {
				toRead = 150;
			}
			else if (lastIndexReadedCorrectly < toRead) {
				toRead = lastIndexReadedCorrectly;
			}
			else {
				toRead += 5;
			}
			spectators = tmp_spectators;
			if (spectators > 0) {
				system("color 4E");
			}
			else {
				system("color 07");
			}
			printf("%d pr %d sp %d tr\n", cc, spectators, toRead);
			aimentity = tmp_aimentity;
		}
	}
}

static DWORD WINAPI ALoop(void* params) {
	SOCKET sock = g_Sock3;
	while (active) {
		Sleep(1);
		if (aimentity == 0 || g_PID == 0 || g_Base == 0 || !(GetKeyState(VK_RBUTTON) & 0x8000))
			continue;
		uint64_t LocalPlayer = driver::read<uint64_t>(sock, g_PID, g_Base + OFFSET_LOCAL_ENT);
		if (LocalPlayer == 0) continue;
		Entity LPlayer = getEntity(sock, g_PID, LocalPlayer);
		//Aimbot
		QAngle Angles = CalculateBestBoneAim(sock, g_PID, LPlayer, aimentity, s_FOV, spectators==0?tmp_spectators:spectators);
		if (Angles.x == 0 && Angles.y == 0) {
			continue;
		}
		LPlayer.SetViewAngles(sock, g_PID, Angles);
	}
	return 0;
}

void render(void* ovv) {
	if (g_PID == 0 || g_Base == 0) {
		return;
	}
	Overlay* ov = (Overlay*)ovv;
	Direct dx = ov->CurrentDirectX;
	SOCKET sock = g_Sock2;

	uint64_t LocalPlayer = driver::read<uint64_t>(sock, g_PID, g_Base + OFFSET_LOCAL_ENT);
	if (LocalPlayer == 0) return;
	Entity LPlayer = getEntity(sock, g_PID, LocalPlayer);
	team_player = LPlayer.getTeamId();
	if (team_player < 0 || team_player>50) return;
	Vector LocalPlayerPosition = LPlayer.getPosition();

	uint64_t viewRenderer = driver::read<uint64_t>(sock, g_PID, g_Base + OFFSET_RENDER);
	uint64_t viewMatrix = driver::read<uint64_t>(sock, g_PID, viewRenderer + OFFSET_MATRIX);
	Matrix m = driver::read<Matrix>(sock, g_PID, viewMatrix);

	uint64_t entitylist = g_Base + OFFSET_ENTITYLIST;
	for (int i = 0; i <= toRead; i++)
	{
		uint64_t centity = driver::read<uint64_t>(sock, g_PID, entitylist + ((uint64_t)i << 5));
		if (centity == 0) continue;
		if (LocalPlayer == centity) continue;

		Entity Target = getEntity(sock, g_PID, centity);
		if (!Target.isPlayer()) continue;

		int health = Target.getHealth();
		if (health < 1 || health > 100) continue;

		int entity_team = Target.getTeamId();
		if (entity_team < 0 || entity_team>50 || entity_team == team_player) continue;

		Vector EntityPosition = Target.getPosition();
		float dist = LocalPlayerPosition.DistTo(EntityPosition);
		if (dist > 8000.0f || dist < 50.0f) continue;

		Vector bs = Vector();
		WorldToScreen(EntityPosition, m.matrix, ov->getWidth(), ov->getHeight(), bs);
		if (bs.x > 0 && bs.y > 0) {
			Vector hs = Vector();
			Vector HeadPosition = Target.getBonePosition(sock, g_PID, 8);
			WorldToScreen(HeadPosition, m.matrix, ov->getWidth(), ov->getHeight(), hs);
			float height = abs(abs(hs.y) - abs(bs.y));
			float width = height / 2.6f;
			float boxMiddle = bs.x - (width / 2);


			std::wstring distance = std::to_wstring(dist / 39.62);
			distance = distance.substr(0, distance.find('.')) + L"m";


			dx.Fill(hs.x - 2.5f, hs.y, 5, 5, 0, 0, 255, 255); //HEAD
			dx.DrawBox2(boxMiddle, hs.y, width, height, 255, 0, 0, 255); //BOX
			dx.DrawString(boxMiddle, bs.y + 1, 255, 0, 255, 0, distance.c_str());  //DISTANCE
			dx.DrawLine(ov->getWidth() / 2, ov->getHeight(), bs.x, bs.y, 255, 0, 0, 255); //LINE FROM MIDDLE SCREEN
		}
	}
}

void getPID() {

	std::wstring xw = std::wstring(L"5a");
	
	while (g_PID == 0)
	{
		if (GetKeyState(0x73) & 0x8000) {
			active = false;
			break;
		}
		printf("NP\n");
		Sleep(5000);
		std::wstring yw = (std::wstring(L"r") + xw + std::wstring(L"pe") + std::wstring(L"x.exe"));
		const wchar_t* ptr = yw.c_str();
		g_PID = driver::get_process_pid(g_Sock,(wchar_t *)ptr, yw.length());
		
		memset((void*)&ptr[0],0x00,yw.size()*2);
		yw.clear();
	}
	printf("P %d\n", g_PID);
}

bool getBase() {
	g_Base = driver::get_process_base_address(g_Sock, g_PID);
	if (g_Base == 0)
	{
		log("Failed  to get base address!");
		g_PID = 0;
		Sleep(500);
		return false;
	}
	printf("B %p\n", g_Base);
	return true;
}

int main(int argc, char** argv)
{
	if (argc > 1) {
		driver::initialize();
		g_Sock = driver::connect();
		driver::deinitialize();
		exit(0);
		return 0;
	}
	J();
	log("Connecting to driver...");
	driver::initialize();
	J();
	log("Connecting...");
	g_Sock = driver::connect();
	g_Sock2 = driver::connect();
	g_Sock3 = driver::connect();
	if (g_Sock == INVALID_SOCKET || g_Sock2 == INVALID_SOCKET || g_Sock3 == INVALID_SOCKET)
	{
		log("Connection failed!");
		Sleep(500);
		exit(0);
		return 0;
	}
	Overlay ov1 = Overlay();
	ov1.SetRender(render);
	ov1.Start();

	DWORD tid = 0;
	CreateThread(0, 0, ALoop, 0, 0, &tid);
	while (active) {
		getPID();
		if (!getBase()) continue;

		ov1.SetTargetHWND(Utils::FindMainWindow(g_PID));

		if (!ov1.isRunning()) {
			ov1.Start();
		}

		DoActions();

		g_PID = 0;
		g_Base = 0;
	}
	ov1.Clear();
	active = false;
	driver::deinitialize();
	J();
	return 0;
}

//uint64_t nextEntity = entitylist;
//while (true)
//{
//	uint64_t centity = driver::read<uint64_t>(g_Sock, g_PID, nextEntity);
//	if (centity == 0) {
//		break;
//	}

//	nextEntity = driver::read<uint64_t>(g_Sock, g_PID, nextEntity + 24);
//	if (nextEntity == 0) {
//		break;
//	}




//void ItemsLoop() {
//	printf("ilog O\n");
//	J();
//	while (active)
//	{
//		Sleep(100);
//		J();
//
//		if (GetKeyState(0x73) & 0x8000) {
//			active = false;
//			break;
//		}
//
//		if (g_PID == 0 || g_Base == 0) continue;
//
//		uint64_t LocalPlayer = driver::read<uint64_t>(g_Sock2, g_PID, g_Base + OFFSET_LOCAL_ENT);
//		if (LocalPlayer == 0) continue;
//
//		uint64_t entitylist = g_Base + OFFSET_ENTITYLIST;
//
//
//		uint64_t baseent = driver::read<uint64_t>(g_Sock2, g_PID, entitylist);
//		if (baseent == 0) continue;
//
//		int itemsGold = 0;
//		uint64_t nextEntity = entitylist;
//		while (true)
//		{
//			uint64_t centity = driver::read<uint64_t>(g_Sock2, g_PID, nextEntity);
//			if (centity == 0) {
//				continue;
//			}
//
//			nextEntity = driver::read<uint64_t>(g_Sock2, g_PID, nextEntity + 24);
//			if (nextEntity == 0) {
//				break;
//			}
//
//			if (LocalPlayer == centity) continue;
//			//int x = driver::read<int>(g_Sock, g_PID, centity + 0x2FD0);
//
//			uint64_t name = driver::read<uint64_t>(g_Sock2, g_PID, centity + OFFSET_NAME);
//			if (name == 125780153691248) {
//				continue;
//			}
//
//
//			Color u = driver::read<Color>(g_Sock2, g_PID, centity + 0x1B8);
//			if (!(u.r == 1.0f && u.g < 0.81f && u.g > 0.79f && u.b < 0.25f) &&
//				!(u.r > 0.48f && u.r < 0.50f && u.g == 0.0f && u.b == 1.0f)) {//0.490196 0.000000 1.000000
//				continue;
//			}
//			itemsGold++;
//			//Vector EntityPosition = GetEntityBasePosition(centity);
//			//Vector LocalPlayerPosition = GetEntityBasePosition(LocalPlayer);
//			//float dist = LocalPlayerPosition.DistTo(EntityPosition);
//			//if (dist > 150.0f) continue;
//
//
//			//dpm
//			//driver::write<int>(g_Sock, g_PID, centity + 0x278, 0x4D407D7E); //raton encima cosa//136 cuando no ç
//
//			//driver::write<float>(g_Sock, g_PID, centity + 0x2D4, 100.0f); //IDK
//
//			//driver::write<Color>(g_Sock, g_PID, centity + 0x1B8, Color(1.0f, 0.0f, 0.0f));
//
//			//dpm2
//			driver::write<BYTE>(g_Sock2, g_PID, centity + 0x27B, 0x40);
//			driver::write<BYTE>(g_Sock2, g_PID, centity + 0x27A, 0x50); // border size
//			driver::write<BYTE>(g_Sock2, g_PID, centity + 0x279, 0x7D); // ITEM glow status
//			driver::write<BYTE>(g_Sock2, g_PID, centity + 0x278, 0x88); //raton encima cosa//0x88 cuando no ç
//
//			driver::write<BYTE>(g_Sock2, g_PID, centity + 0x2D4, 0x01); //IDK
//
//
//			//driver::write<Color>(g_Sock, g_PID, centity + 0x1B8, Color(1.0f, 0.0f, 0.0f));
//
//
//		}
//		printf("%d gi\n", itemsGold);
//		
//	}
//}


				//std::ofstream myfile;
				//myfile.open("C:\\Nlog.txt", std::ios::out | std::ios::app);
				//myfile << "Ent(" << std::hex << centity << ")\n";
				//for (int offset = 0x0; offset <= 0x2FF0; offset += 0x4)
				//	myfile << std::hex << offset << " -> " << driver::read<int>(g_Sock, g_PID, centity + offset) << "\n";
				//myfile.close();
				//printf("printed\n");