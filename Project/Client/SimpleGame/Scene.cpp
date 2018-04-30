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

	GameStatus = GAMESTATUS::RUNNING;
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
		my_packet->size = sizeof(my_packet);
		my_packet->type = CS_UP;
		m_Client->SendPacket((char*)my_packet);
		break;

	case 'a':
		my_packet->size = sizeof(my_packet);
		my_packet->type = CS_LEFT;
		m_Client->SendPacket((char*)my_packet);
		break;

	case 's':
		my_packet->size = sizeof(my_packet);
		my_packet->type = CS_DOWN;
		m_Client->SendPacket((char*)my_packet);
		break;

	case 'd':
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
	if (GameStatus == GAMESTATUS::RUNNING) 
	{
	}
	else if (GameStatus == GAMESTATUS::PAUSE)
	{
		g_Timer->getTimeset();
		double timeElapsed = g_Timer->getTimeElapsed();
	}

	else
	{
		g_Timer->getTimeset();
		double timeElapsed = g_Timer->getTimeElapsed();
	}
}

void Scene::render()
{
	int currx = Player->getPosition().x, curry = Player->getPosition().y;
	int t = (currx + curry) % 2;
	
	for (int i = 0; i < 8; ++i)
		for (int j = 0; j < 8; ++j) {
			m_Renderer->DrawSolidRect(
				-390 + 120 * i - 60 * t,
				330 - 120 * j,
				0, 60, 0.416f, 0.236f, 0.136f, 0.3, 0.9);
			m_Renderer->DrawSolidRect(
				-390 + 120 * i + 60 * t + 60,
				330 - 120 * j + 60,
				0, 60, 0.416f, 0.236f, 0.136f, 0.3, 0.9);
		}
	
	for (auto p : m_Players)
	{
		if (p == Player) {
			m_Renderer->DrawTexturedRectSeq(
				-30, 30, 0, PIECESIZE, 1, 1, 1, 1, ChessPiece,
				p->getType(),
				p->getTeam(),
				6, 2, 0.5);
			m_Renderer->DrawSolidRect(-30, 30, 0, 60, 1, 1, 0, 0.5, 0.7);
		}

		else {
			m_Renderer->DrawTexturedRectSeq(
				-330 + (5 + p->getPosition().x - currx) * 60,
				330 - (5 + p->getPosition().y - curry) * 60,
				0, PIECESIZE, 1, 1, 1, 1, ChessPiece,
				p->getType(),
	 			p->getTeam(),
				6, 2, 0.5);
		}
	}

	char buf[100];
	sprintf(buf, "( %d, %d )", currx, curry);
	m_Renderer->DrawTextW(0, 0, GLUT_BITMAP_HELVETICA_18, 0.5, 0, 0.5, buf);


	int note_x = (int)(Player->getPosition().x + 6) % 12, note_y = ((int)Player->getPosition().y + 6) % 12;
	m_Renderer->DrawSolidRect(
		330 - 60 * note_x,
		-330 + 60 * note_y,
		0, 60, 1, 1, 0, 1, 0.9
	);

	sprintf(buf, "( %d, %d )", currx - note_x + 6, curry - note_y + 6);
	m_Renderer->DrawTextW(330 - 60 * note_x - 15, -330 + 60 * note_y - 20, GLUT_BITMAP_HELVETICA_18, 0.5, 0, 0.5, buf);
}

void Scene::ProcessPacket(char * p)
{
	switch (p[1])
	{
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
		}
		else {
			cl->setType(OBJTYPE::PAWN);
			cl->setTeam(TEAM::BLACK);
		}
		m_Players.push_back(cl);
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

	default:
		printf("Unknown PACKET type [%d]\n", p[1]);
	}
}
