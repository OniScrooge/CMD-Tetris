#include <chrono>
#include <iostream>
#include <stdio.h>
#include <thread>
#include <vector>
#include <Windows.h>
#include "Functions.h"


int main()
{
	//Create screen buffer
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	for (int i{ 0 }; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	//Create assets
	tetromino[0].append(L"..X...X...X...X."); // Tetronimos 4x4
	tetromino[1].append(L"..X..XX...X.....");
	tetromino[2].append(L".....XX..XX.....");
	tetromino[3].append(L"..X..XX..X......");
	tetromino[4].append(L".X...XX...X.....");
	tetromino[5].append(L".X...X...XX.....");
	tetromino[6].append(L"..X...X..XX.....");

	pField = new unsigned char[nFieldWidth * nFieldHeight]; //Create play field buffer
	for (int x{ 0 }; x < nFieldWidth; x++)					//Board boundary
		for (int y{ 0 }; y < nFieldHeight; y++)
			pField[y * nFieldWidth + x] = (x == 0 || x == (nFieldWidth - 1) || y == (nFieldHeight - 1)) ? 9 : 0;

	int nCurrentPiece{ 0 };
	int nCurrentRotation{ 0 };
	int nCurrentX{ nFieldWidth / 2 };
	int nCurrentY{ 0 };

	int nSpeed{ 20 };
	int nSpeedCounter{ 0 };
	bool bForceDown{ false };
	int nPieceCount{ 0 };
	int nScore{ 0 };

	//Control with Arrow Keys and 'Z'
	bool bKey[4];
	bool bRotateHold {false};

	std::vector<int> vLines{};

	bool bGameOver {false};
	while (!bGameOver)
	{
		//Game timing
		std::this_thread::sleep_for(std::chrono::milliseconds(50)); //Game tick
		nSpeedCounter++;
		bForceDown = (nSpeedCounter == nSpeed);

		//Input
		for (int k{ 0 }; k < 4; k++)			//Hexadecimal for   R   L   D Z  Keys
			bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0;

		//Game logic
		nCurrentX += (bKey[0] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) ? 1 : 0;
		nCurrentX -= (bKey[1] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) ? 1 : 0;
		nCurrentY += (bKey[2] && DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) ? 1 : 0;

		//Only permits key presses for rotation, not holding down for rotation
		if (bKey[3])
		{
			nCurrentRotation += (!bRotateHold && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) ? 1 : 0;
			bRotateHold = true;
		}
		else
			bRotateHold = false;
		
		if (bForceDown)
		{
			if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
				nCurrentY++; //Check to see if piece moving down is possible
			else
			{
				//Locks current piece into field
				for (int px{ 0 }; px < 4; px++)
					for (int py{ 0 }; py < 4; py++)
						if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
							pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;

				//Increments speed over time
				nPieceCount++;
				if (nPieceCount % 10 == 0)
					if (nSpeed >= 10) nSpeed--;

				//Check for HORIZONTAL lines
				for (int py{ 0 }; py < 4; py++)
					if (nCurrentY + py < nFieldHeight - 1)
					{
						bool bLine{ true };
						for (int px{ 1 }; px < nFieldWidth - 1; px++)
							bLine &= (pField[(nCurrentY + py) * nFieldWidth + px]) != 0;

						if (bLine)
						{
							//Remove line, set to '='
							for (int px{ 1 }; px < nFieldWidth - 1; px++)
								pField[(nCurrentY + py) * nFieldWidth + px] = 8;

							vLines.push_back(nCurrentY + py);
						}
					}

				nScore += 25;
				if (!vLines.empty()) nScore += (1 << vLines.size()) * 100;

				//Choose next piece
				nCurrentX = nFieldWidth / 2;
				nCurrentY = 0;
				nCurrentRotation = 0;
				nCurrentPiece = rand() % 7;

				//If piece can't fit, GAME OVER
				bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
			}
			nSpeedCounter = 0;
		}

		//Draw Score
		swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 16, L"SCORE: %8d", nScore);

		//Draw Field
		for (int x{ 0 }; x < nFieldWidth; x++)
			for (int y{ 0 }; y < nFieldHeight; y++)
				screen[(y + 2) * nScreenWidth + (x + 2)] = L" ABCDEFG=#"[pField[y * nFieldWidth + x]];

		//Draw current piece
		for (int px{ 0 }; px < 4; px++)
			for (int py{ 0 }; py < 4; py++)
				if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
					screen[(nCurrentY + py + 2) * nScreenWidth + (nCurrentX + px + 2)] = nCurrentPiece + 65;

		if (!vLines.empty())
		{
			//Display frame to draw lines
			WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
			std::this_thread::sleep_for(std::chrono::milliseconds(400));

			for (auto& v : vLines)
				for (int px{ 1 }; px < (nFieldWidth - 1); px++)
				{
					for (int py{ v }; py > 0; py--)
						pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
					pField[px] = 0;
				}
			vLines.clear();
		}

		//Display Frame
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, {0,0}, &dwBytesWritten);
	}

	//Game over screen
	CloseHandle(hConsole); //Close console in order to use std::cout
	std::cout << "Game Over!! Score: " << nScore << std::endl;
	system("pause");

	return 0;
}