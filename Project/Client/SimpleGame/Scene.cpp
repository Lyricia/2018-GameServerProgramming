#include "stdafx.h"
#include "Renderer.h"
#include "Client.h"
#include "Timer.h"
#include "Scene.h"
#include "Object.h"
#include "mdump.h"

using namespace std;

Scene::Scene()
{
}

Scene::~Scene()
{
}

void Scene::buildScene()
{
	m_Renderer = new Renderer(WINDOW_WIDTH, WINDOW_HEIGHT);
	// Initialize Renderer
	if (!m_Renderer->IsInitialized())
	{
		std::cout << "Renderer could not be initialized.. \n";
	}
	
	memset(m_Board, -1, sizeof(int) * BOARD_WIDTH * BOARD_HEIGHT);
	ChessPiece = m_Renderer->CreatePngTexture("Assets/Image/chess piece.png");
	ReadGroundFile();
	GameStatus = GAMESTATUS::STOP;
}

void Scene::releaseScene()
{
	delete		m_Renderer;
	delete		g_Timer;
}

void Scene::keyinput(unsigned char key)
{
	cs_packet_up* my_packet = new cs_packet_up;
	switch (key)
	{
	case 'p':
	case 'P':
		if (GameStatus == GAMESTATUS::RUNNING)
		{
			GameStatus = GAMESTATUS::PAUSE;
		}
		else if (GameStatus == GAMESTATUS::PAUSE)
		{
			GameStatus = GAMESTATUS::RUNNING;
		}
		break;

	case 'w':
	case 'W':
		my_packet->size = sizeof(my_packet);
		my_packet->type = CS_UP;
		m_Client->SendPacket((char*)my_packet);
		break;

	case 'a':
	case 'A':
		my_packet->size = sizeof(my_packet);
		my_packet->type = CS_LEFT;
		m_Client->SendPacket((char*)my_packet);
		break;

	case 's':
	case 'S':
		my_packet->size = sizeof(my_packet);
		my_packet->type = CS_DOWN;
		m_Client->SendPacket((char*)my_packet);
		break;

	case 'd':
	case 'D':
		my_packet->size = sizeof(my_packet);
		my_packet->type = CS_RIGHT;
		m_Client->SendPacket((char*)my_packet);
		break;

	default:
		break;
	}
}

void Scene::keyspcialinput(int key)
{
	switch (key)
	{
	case 0:
	default:
		break;
	}
}

// 멀티태스킹을 가정하고 업 다운 글로벌을 가지고 있어야한다.
// 밖에서 누르고 안에서 업 할 수도 있기 때문에
void Scene::mouseinput(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP && GameStatus == GAMESTATUS::RUNNING)
	{
	}
}

void Scene::update()
{
	g_Timer->getTimeset();
	double timeElapsed = g_Timer->getTimeElapsed();
	if (GameStatus == GAMESTATUS::RUNNING) 
	{
		for (auto& p : m_Players) {
			float time = p->getMsgTimer();
			if (time > 0) {
				p->setMsgTimer(time - timeElapsed);
			}
			else if (time < 0) {
				p->setChatMsg(L"");
			}
		}
	}
	else if (GameStatus == GAMESTATUS::PAUSE)
	{
	}
	else
	{
	}
}

void Scene::ProcessPacket(char * p)
{
	switch (p[1])
	{
	case SC_LOGINOK:
	{
		cout << "OK" << endl;
		GameStatus = GAMESTATUS::RUNNING;
		break;
	}
	case SC_LOGINFAIL:
	{
		cout << "Failed" << endl;
		system("pause");
		exit(0);
	}
	case SC_PUT_PLAYER:
	{
		sc_packet_put_player *my_packet = reinterpret_cast<sc_packet_put_player *>(p);
		Object* cl = new Object();
		cl->setID(my_packet->id);
		cl->setPosition(Vec3i{ (int)my_packet->x, (int)my_packet->y, 0 });
		if (Player == nullptr) {
			Player = cl;
			cl->setType(OBJTYPE::KING);
			cl->setTeam(TEAM::WHITE);
			cl->setPriority(0.1);
		}
		else {
			if (cl->getID() > NPC_START) {
				cl->setType(OBJTYPE::PAWN);
				cl->setTeam(TEAM::BLACK);
				cl->setPriority(0.5);
			}
			else {
				cl->setType(OBJTYPE::BISHOP);
				cl->setTeam(TEAM::WHITE);
				cl->setPriority(0.3);
			}
		}
		m_Players.push_back(cl);
		m_Player_ID_Matcher[my_packet->id] = cl;
		m_Board[my_packet->x][my_packet->y] = my_packet->id;
		break;
	}

	case SC_POS:
	{
		sc_packet_pos *my_packet = reinterpret_cast<sc_packet_pos *>(p);
		for (auto player : m_Players) {
			if (player->getID() == my_packet->id) {
				m_Board[(int)player->getPosition().x][(int)player->getPosition().y] = -1;
				player->setPosition(my_packet->x, my_packet->y, 0);
				m_Board[my_packet->x][my_packet->y] = my_packet->id;
			}
		}
		break;
	}

	case SC_REMOVE_PLAYER:
	{
		sc_packet_remove_player *my_packet = reinterpret_cast<sc_packet_remove_player *>(p);
		m_Players.remove_if([&](Object* player)->bool {
			if (player->getID() == my_packet->id) {
				m_Board[(int)player->getPosition().x][(int)player->getPosition().y] = -1;
				return true;
			}
			else
				return false;
		});
		break;
	}
	case SC_CHAT: 
	{
		sc_packet_chat *my_packet = reinterpret_cast<sc_packet_chat *>(p);
		m_Player_ID_Matcher[my_packet->id]->setChatMsg(my_packet->message);
		m_Player_ID_Matcher[my_packet->id]->setMsgTimer(5.f);
		break;
	}

	default:
		printf("Unknown PACKET type [%d]\n", p[1]);
	}
}

struct pos {
	int x, y;
};

void Scene::ReadGroundFile()
{
	int nStride = sizeof(pos);
	int nVerts = 0;

	std::string Filepath = "./Assets/Ground.pos";
	FILE* fp = nullptr;
	fopen_s(&fp, Filepath.c_str(), "rb");
	fread_s(&nVerts, sizeof(UINT), sizeof(UINT), 1, fp);
	pos* pVerts = new pos[nVerts];
	fread_s(pVerts, nStride * nVerts, nStride, nVerts, fp);

	for (int i = 0; i < nVerts; ++i) {
		m_Board[pVerts[i].x][pVerts[i].y] = 1;
		//cout << (int)pVerts[i].x << ", " << (int)pVerts[i].y << "\n";
	}
}

void Scene::render()
{
	int currx = 0, curry = 0;
	if(Player!= nullptr)
		currx = Player->getPosition().x, curry = Player->getPosition().y;

	int t = (currx + curry) % 2;
	
	// Checker Board
	for (int i = 0; i < 14; ++i)
		for (int j = 0; j < 14; ++j) {
			m_Renderer->DrawSolidRect(
				-375 + 60 * i - 30 * t,
				375 - 60 * j,
				0, 30, 0.416f, 0.236f, 0.136f, 0.3, 0.9);
			m_Renderer->DrawSolidRect(
				-375 + 60 * i + 30 * t + 30,
				375 - 60 * j + 30,
				0, 30, 0.416f, 0.236f, 0.136f, 0.3, 0.9);
		}
	
	// Player Objects
	list<Object*> plist = m_Players;
	for (auto p : plist)
	{
		if (p == Player) {
			m_Renderer->DrawTexturedRectSeq(
				-15, 15, 0, PIECESIZE, 1, 1, 1, 1, ChessPiece,
				p->getType(),
				p->getTeam(),
				6, 2, p->getPriority());
		}

		else {
			m_Renderer->DrawTexturedRectSeq(
				-345 + (11 + p->getPosition().x - currx) * 30,
				345 - (11 + p->getPosition().y - curry) * 30,
				0, PIECESIZE, 1, 1, 1, 1, ChessPiece,
				p->getType(),
				p->getTeam(),
				6, 2, p->getPriority());
		}
		if (p->getMsgTimer() > 0) {
			m_Renderer->DrawTextW(
				-345 + (11 + p->getPosition().x - currx) * 30,
				345 - (11 + p->getPosition().y - curry) * 30 - 20,
				GLUT_BITMAP_HELVETICA_18, 0.5f, 0.5f, 0.2f, p->getChatMsg());
		}
	}

	for (int i = 0; i < BOARD_WIDTH; ++i) {
		for (int j = 0; j < BOARD_HEIGHT; ++j) {
			if (m_Board[i][j] == 1) {
				m_Renderer->DrawSolidRect(
					-345 + (11 + i - currx) * 30,
					345 - (11 + j - curry) * 30,
					0, 30, 0, 0, 0, 1, 0.1);
			}
		}
	}

	// Board Line, Point Coord
	char buf[50];
	sprintf(buf, "( %d, %d )", currx, curry);
	m_Renderer->DrawTextW(0, 0, GLUT_BITMAP_HELVETICA_18, 0.5, 0, 0.5, buf);

	for (int x = currx - 16; x < currx + 16; ++x) {
		for (int y = curry - 16; y < curry + 16; ++y) {
			if (x >= 0 && y >= 0 && x <= BOARD_WIDTH && y <= BOARD_HEIGHT) {
				if (x % 10 == 0 || y % 10 == 0) {
					m_Renderer->DrawSolidRect(
						-15 + 30 * (x - currx),
						15 - 30 * (y - curry),
						0, 30, 1, 0, 0, 0.5, 0.9
					);

					if (x % 10 == 0 && y % 10 == 0) {
						sprintf(buf, "( %d, %d )", x, y);
						m_Renderer->DrawText(-15 + 30 * (x - currx), 15 - 30 * (y - curry), GLUT_BITMAP_HELVETICA_12, 0.2, 0, 0.8, buf);
					}
				}
			}
		}
	}
}
