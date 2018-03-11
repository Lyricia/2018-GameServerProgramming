/*
Copyright 2017 Lee Taek Hee (Korea Polytech University)

This program is free software: you can redistribute it and/or modify
it under the terms of the What The Hell License. Do it plz.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY.
*/

#include "stdafx.h"
#include "Renderer.h"
#include "Scene.h"
#include "Timer.h"

Renderer	*g_Renderer = NULL;
Scene*		CurrentScene;
Timer*		g_Timer;

void RenderScene(void)
{
	CurrentScene->update();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	CurrentScene->render();

	glutSwapBuffers();
}

void Initialize()
{
	srand((unsigned)time(NULL));

	g_Timer = new Timer();
	g_Timer->Init();

	CurrentScene = new Scene();
	//CurrentScene->setRenderer(g_Renderer);
	CurrentScene->setTimer(g_Timer);
	CurrentScene->buildScene();
}

void Idle(void)
{
	RenderScene();
}

void MouseInput(int button, int state, int x, int y)
{
	x = x - WINDOW_WIDTH*0.5;
	y = -(y - WINDOW_HEIGHT*0.5);

	CurrentScene->mouseinput(button, state, x, y);
	RenderScene();
}

void MouseMove(int x, int y)
{
	x = x - WINDOW_WIDTH*0.5;
	y = -(y - WINDOW_HEIGHT*0.5);

	CurrentScene->mouseinput(0, 0, x, y);
	RenderScene();
}

void KeyInput(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'q':
		glutLeaveMainLoop();
		break;

	case 13:
		if (CurrentScene->GetGamestatus() == GAMESTATUS::STOP)
			CurrentScene->SetGamestatus(GAMESTATUS::RUNNING);
		break;
		
	case 'r':
		CurrentScene->releaseScene();
		Initialize();
		break;

	default:
		CurrentScene->keyinput(key);
		break;
	}
	RenderScene();
}

void SpecialKeyInput(int key, int x, int y)
{
	CurrentScene->keyspcialinput(key);
	RenderScene();
}

void SceneChanger(Scene* scene) {
	CurrentScene = scene;
}

int main(int argc, char **argv)
{
	// Initialize GL things
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("Game Software Engineering KPU");



	glewInit();
	if (glewIsSupported("GL_VERSION_3_0"))
	{
		std::cout << " GLEW Version is 3.0\n ";
	}
	else
	{
		std::cout << "GLEW 3.0 not supported\n ";
	}
	
	Initialize();

	glutDisplayFunc(RenderScene);
	glutIdleFunc(Idle);
	glutKeyboardFunc(KeyInput);
	glutMouseFunc(MouseInput);
	glutSpecialFunc(SpecialKeyInput);
	glutPassiveMotionFunc(MouseMove);
	glutMainLoop();

	CurrentScene->releaseScene();
	delete g_Renderer;

    return 0;
}

