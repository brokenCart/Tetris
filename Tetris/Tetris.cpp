#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <Windows.h>
using namespace std;

wstring tetromino[7];
int nFieldWidth = 12;
int nFieldHeight = 18;
unsigned char* pField = nullptr;

int nScreenWidth = 80; // Console screen width
int nScreenHeight = 30; // Console screen height

// The formulae were derived by
// writing numbers in a grid and
// observing the values as x & y increase.
int Rotate(int px, int py, int r) {
    switch (r % 4) {
    case 0: return py * 4 + px; // 0 degrees
    case 1: return 12 + py - px * 4; // 90 degrees
    case 2: return 15 - py * 4 - px; // 180 degrees
    case 3: return 3 - py + px * 4; // 270 degrees
    }
    return 0;
}

// The position being taken here
// is the top left corner of the tetromino.
bool DoesPieceFit(int nTetromino, int r, int nPosX, int nPosY) {
    for (int px = 0; px < 4; px++) {
        for (int py = 0; py < 4; py++) {
            int pi = Rotate(px, py, r);
            int fi = (nPosY + py) * nFieldWidth + (nPosX + px);

            if (nPosX + px >= 0 && nPosX + px < nFieldWidth) {
                if (nPosY + py >= 0 && nPosY + py < nFieldHeight) {
                    if (tetromino[nTetromino][pi] == L'X' && pField[fi] != 0) {
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

int main()
{
    // Create tetromino assets
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");

    // 1 & 2 are mirror images of each other
    // and can't be rotated into each other.
	// They are chiral, no axis of symmetry.
    tetromino[1].append(L"..X.");
    tetromino[1].append(L".XX.");
    tetromino[1].append(L".X..");
    tetromino[1].append(L"....");

    tetromino[2].append(L".X..");
    tetromino[2].append(L".XX.");
    tetromino[2].append(L"..X.");
    tetromino[2].append(L"....");

    tetromino[3].append(L"....");
    tetromino[3].append(L".XX.");
    tetromino[3].append(L".XX.");
    tetromino[3].append(L"....");

    tetromino[4].append(L"..X.");
    tetromino[4].append(L".XX.");
    tetromino[4].append(L"..X.");
    tetromino[4].append(L"....");

    // Same for 5 & 6 (mirror images)
    tetromino[5].append(L"....");
    tetromino[5].append(L".XX.");
    tetromino[5].append(L"..X.");
    tetromino[5].append(L"..X.");

    tetromino[6].append(L"....");
    tetromino[6].append(L".XX.");
    tetromino[6].append(L".X..");
    tetromino[6].append(L".X..");

    // Initializing the play field
    pField = new unsigned char[nFieldWidth * nFieldHeight]; // Create the play field
    for (int x = 0; x < nFieldWidth; x++) {
        for (int y = 0; y < nFieldHeight; y++) {
            // 0 -> Empty Space
            // 9 -> Border Boundary
            pField[y * nFieldWidth + x] = (x != 0 && x != nFieldWidth - 1 && y != nFieldHeight - 1) ? 0 : 9;
        }
    }

    wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
    for (int i = 0; i < nScreenWidth * nScreenHeight; i++) screen[i] = L' ';
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    // Game State
    bool bGameOver = false;

    int nCurrentPiece = 0;
    int nCurrentRotation = 0;
    int nCurrentX = nFieldWidth / 2;
    int nCurrentY = 0;

    bool bKey[4];
    bool bRotateHold = false;

    int nSpeed = 20;
    int nSpeedCounter = 0;
    bool bForceDown = false;
    int nPieceCount = 0;
    int nScore = 0;

    vector<int> vLines;

    // Game Loop
    while (!bGameOver) {
        // GAME TIMING
        this_thread::sleep_for(50ms); // Game Tick
        nSpeedCounter++;
        if (nSpeedCounter == nSpeed) {
            bForceDown = true;
        }
        else {
            bForceDown = false;
        }


        // INPUT
        for (int k = 0; k < 4; k++) {
            // Right Key: \x27
            // Left Key: \x25
            // Down Key: \x28
            // Z Key: Z (for rotation)
            bKey[k] = (0x8000 & GetAsyncKeyState((unsigned char) ("\x27\x25\x28Z"[k]))) != 0;
        }

        // GAME LOGIC
        if (bKey[0]) { // Right Key
            if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY)) {
                nCurrentX++;
            }
        }
        if (bKey[1]) { // Left Key
            if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY)) {
                nCurrentX--;
            }
        }
        if (bKey[2]) { // Down Key
            if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) {
                nCurrentY++;
            }
        }
        if (bKey[3]) { // Z key
            if (!bRotateHold && DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY)) {
                nCurrentRotation++;
            }
            bRotateHold = true;
        }
        else {
            bRotateHold = false;
        }

        if (bForceDown) {
            if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1)) {
                nCurrentY++;
            }
            else {
                // Lock the current piece in the play field
                for (int px = 0; px < 4; px++) {
                    for (int py = 0; py < 4; py++) {
                        if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X') {
                            pField[(nCurrentY + py) * nFieldWidth + (nCurrentX + px)] = nCurrentPiece + 1;
                        }
                    }
                }

                // Increasing the difficulty
                nPieceCount++;
                if (nPieceCount % 10 == 0) {
                    if (nSpeed >= 10) nSpeed--;
                }

                // Check for horizontal lines
                // We need to check the whole play field
                // and can just check the position where
                // our last tetronimo piece was.
                for (int py = 0; py < 4; py++) {
                    if (nCurrentY + py < nFieldHeight - 1) {
                        bool bLine = true;
                        for (int px = 1; px < nFieldWidth - 1; px++) {
                            if (pField[(nCurrentY + py) * nFieldWidth + px] == 0) {
                                bLine = false;
                                break;
                            }
                        }

                        if (bLine) {
                            // Set line to =
                            for (int px = 1; px < nFieldWidth - 1; px++) {
                                pField[(nCurrentY + py) * nFieldWidth + px] = 8;
                            }
                            vLines.push_back(nCurrentY + py);
                        }
                    }
                }

                // Increasing the score
                nScore += 25; // Per piece
                if (!vLines.empty()) nScore += (1 << vLines.size()) * 100; // 100 * 2^(number of lines)

                // Move on to the next piece
                nCurrentPiece = rand() % 7;
                nCurrentX = nFieldWidth / 2;
                nCurrentY = 0;
                nCurrentRotation = 0;

                // If piece does not fit, then the game is over
                bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
            }
            nSpeedCounter = 0;
        }

        // RENDER OUTPUT
        // Draw Field
        for (int x = 0; x < nFieldWidth; x++) {
            for (int y = 0; y < nFieldHeight; y++) {
                // Creating an offset
                // so that the play field is not
                // rendered directly at the top left corner
                // of the screen.
                screen[(y + 2) * nScreenWidth + (x + 2)] = L" ABCDEFG=#"[pField[y * nFieldWidth + x]];
            }
        }

        // Draw Current Piece
        for (int px = 0; px < 4; px++) {
            for (int py = 0; py < 4; py++) {
                if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X') {
                    screen[(nCurrentY + py + 2) * nScreenWidth + (nCurrentX + px + 2)] = nCurrentPiece + 'A';
                }
            }
        }

        // Draw Score
        swprintf_s(&screen[2 * nScreenWidth + nFieldWidth + 6], 16, L"SCORE: %8d", nScore);

        if (!vLines.empty()) {
            // Display Frame (cheekily to draw lines)
            for (int y = 0; y < nScreenHeight; y++) {
                WriteConsoleOutputCharacter(hConsole, &screen[y * nScreenWidth], nScreenWidth, { 0, (SHORT)y }, &dwBytesWritten);
            }
            this_thread::sleep_for(400ms); // Delay a bit

            // Move down the lines
            for (auto &v : vLines) {
                for (int px = 1; px < nFieldWidth - 1; px++) {
                    for (int py = v; py > 0; py--) {
                        pField[py * nFieldWidth + px] = pField[(py - 1) * nFieldWidth + px];
                    }
                    pField[px] = 0;
                }
            }

            vLines.clear();
        }

        // Display Frame
        for (int y = 0; y < nScreenHeight; y++) {
            // Displaying each row one at a time
            // so that the console width
            // stops mattering.
            // If the whole screen is displayed at the
            // same time then we have to make sure that the console width
            // is appropriate or else we'll get weird wrappings.
            WriteConsoleOutputCharacter(hConsole, &screen[y * nScreenWidth], nScreenWidth, { 0, (SHORT) y }, &dwBytesWritten);
        }
    }

    // Game Over
    CloseHandle(hConsole);
    cout << "Game Over! Score: " << nScore << endl;
    system("pause");
    
    return 0;
}
