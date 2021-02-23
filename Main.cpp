#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <winsock.h>
#include <commdlg.h>
#include <filesystem>
#include <utility>
#include <codecvt>
#include <locale>

//to convert wstring to string for loading
using convert_t = std::codecvt_utf8<wchar_t>;
std::wstring_convert<convert_t, wchar_t> strconverter;

using namespace std::experimental::filesystem;

// Override base class with your custom functionality
class MapBuilder : public olc::PixelGameEngine
{
public:
	MapBuilder()
	{
		// Name your application
		sAppName = "Map Builder";
	}

private:
	olc::vi2d vWorldSize = { 0, 0 };
	olc::vi2d vTileSize = { 0, 0 };

	olc::vi2d vTileCursor = { 0, 0 };
	olc::vi2d vTileCursorInter = { 0, 0 };
	olc::vi2d vDimensions = { 0, 0 };
	olc::vi2d vDimensionsInter = { 0, 0 };

	olc::vf2d vCameraPos = { 0.0f, 0.0f };
	olc::vf2d vCameraZoom = { 1.0f, 1.0f };
	olc::vf2d vCameraPosSel = { 0.0f, 0.0f };
	olc::vf2d vCameraZoomSel = { 1.0f, 1.0f };

	olc::Renderable rTileSet;
	olc::Renderable rInteractSet;
	int* pWorld = nullptr;
	int* pWorldLayer = nullptr;
	int* pWorldInter = nullptr;

	olc::Sprite* sCursor = nullptr;
	olc::Decal* dCursor = nullptr;

	int CurrentMode = 0;
	std::string* inputs = new std::string[4]{ "" };
	int inputSel = 0;
	int currentLayer = 1;

	std::wstring tilePath;
	std::wstring savePath;

public:
	bool OnUserCreate() override
	{
		rInteractSet.Load("InterSheet.png");

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		//set everything to black
		Clear(olc::BLACK);

		//get mouse location
		olc::vi2d vMouse = { GetMouseX(), GetMouseY() };

		switch (CurrentMode)
		{
		case(0):
		{
			//main menu
			//highlight the selection before the user clicks the mouse
			if (vMouse.x >= 160 && vMouse.x < 272 && vMouse.y >= 4 && vMouse.y < 20)
			{
				DrawString(160, 4, "New Map", olc::YELLOW, 2);
				//change to new map mode
				if (GetMouse(0).bPressed) CurrentMode = 1;
			}
			else
				DrawString(160, 4, "New Map", olc::WHITE, 2);

			if (vMouse.x >= 160 && vMouse.x < 288 && vMouse.y >= 20 && vMouse.y < 36)
			{
				DrawString(160, 20, "Save Map", olc::YELLOW, 2);
				//makes sure a map is loaded and opens a file explorer to save a map
				if (GetMouse(0).bPressed) if (tilePath.length() != 0)
				{
					FindFile("Save");
					if (savePath.length() != 0)
						SaveFile();
				}
			}
			else
				DrawString(160, 20, "Save Map", olc::WHITE, 2);

			if (vMouse.x >= 160 && vMouse.x < 288 && vMouse.y >= 36 && vMouse.y < 52)
			{
				DrawString(160, 36, "Load Map", olc::YELLOW, 2);
				//opens a file explorer to open a map
				if (GetMouse(0).bPressed)
				{
					FindFile("LoadMap");
					//makes sure a level file was actually loaded before loading the file and going into the editor
					if (tilePath.length() != 0 && (tilePath.find(L".txt") > 0 || tilePath.find(L".lvl") > 0))
					{
						LoadFile();
						CurrentMode = 2;
					}
				}
			}
			else
				DrawString(160, 36, "Load Map", olc::WHITE, 2);

			if (vMouse.x >= 160 && vMouse.x < 256 && vMouse.y >= 52 && vMouse.y < 68)
			{
				DrawString(160, 52, "Resize", olc::YELLOW, 2);
				//check to make sure a map is loaded before trying to return
				if (GetMouse(0).bPressed) if (tilePath.length() != 0) 
				{ 
					inputs[0] = std::to_string(vWorldSize.x); 
					inputs[1] = std::to_string(vWorldSize.y); 
					CurrentMode = 3; 
				}
			}
			else
				DrawString(160, 52, "Resize", olc::WHITE, 2);

			if (vMouse.x >= 160 && vMouse.x < 256 && vMouse.y >= 68 && vMouse.y < 84)
			{
				DrawString(160, 68, "Return", olc::YELLOW, 2);
				//check to make sure a map is loaded before trying to return
				if (GetMouse(0).bPressed) if (tilePath.length() != 0) CurrentMode = 2;
			}
			else
				DrawString(160, 68, "Return", olc::WHITE, 2);

			//if a map is loaded return to the editer
			if (GetKey(olc::ESCAPE).bPressed) if (tilePath.length() != 0) CurrentMode = 2;
			break;
		}
		case(1):
		{
			//new map mode
			DrawString(160, 4, "    Map Width:", olc::WHITE, 2);
			DrawString(160, 20, "   Map Height:", olc::WHITE, 2);
			DrawString(160, 36, " Sprite Width:", olc::WHITE, 2);
			DrawString(160, 52, "Sprite Height:", olc::WHITE, 2);

			DrawString(4, 15, "Use your keyboard\nto fill the fields", olc::WHITE);

			//draw the inputs in the correct position
			for (int i = 0; i < 4; i++)
				DrawString(390, ((i * 16) + 4), inputs[i], olc::WHITE, 2);

			//draw an arrow on the current selection
			DrawString(378, ((inputSel * 16) + 4), ">", olc::RED, 2);

			//set the input selection to where the click was
			if (vMouse.x >= 160 && vMouse.y >= 4 && vMouse.y < 20)
				if (GetMouse(0).bPressed) inputSel = 0;
			if (vMouse.x >= 160 && vMouse.y >= 20 && vMouse.y < 36)
				if (GetMouse(0).bPressed) inputSel = 1;
			if (vMouse.x >= 160 && vMouse.y >= 36 && vMouse.y < 52)
				if (GetMouse(0).bPressed) inputSel = 2;
			if (vMouse.x >= 160 && vMouse.y >= 52 && vMouse.y < 68)
				if (GetMouse(0).bPressed) inputSel = 3;

			//Allow for controlled user inputs to fill in the fields and change selections up to 4 digits
			if (GetKey(olc::DOWN).bPressed)
				if (inputSel != 3) inputSel++;
			if (GetKey(olc::UP).bPressed)
				if (inputSel != 0) inputSel--;

			if (GetKey(olc::K1).bPressed || GetKey(olc::NP1).bPressed) if (inputs[inputSel].length() < 4)	inputs[inputSel] += "1";
			if (GetKey(olc::K2).bPressed || GetKey(olc::NP2).bPressed) if (inputs[inputSel].length() < 4)	inputs[inputSel] += "2";
			if (GetKey(olc::K3).bPressed || GetKey(olc::NP3).bPressed) if (inputs[inputSel].length() < 4)	inputs[inputSel] += "3";
			if (GetKey(olc::K4).bPressed || GetKey(olc::NP4).bPressed) if (inputs[inputSel].length() < 4)	inputs[inputSel] += "4";
			if (GetKey(olc::K5).bPressed || GetKey(olc::NP5).bPressed) if (inputs[inputSel].length() < 4)	inputs[inputSel] += "5";
			if (GetKey(olc::K6).bPressed || GetKey(olc::NP6).bPressed) if (inputs[inputSel].length() < 4)	inputs[inputSel] += "6";
			if (GetKey(olc::K7).bPressed || GetKey(olc::NP7).bPressed) if (inputs[inputSel].length() < 4)	inputs[inputSel] += "7";
			if (GetKey(olc::K8).bPressed || GetKey(olc::NP8).bPressed) if (inputs[inputSel].length() < 4)	inputs[inputSel] += "8";
			if (GetKey(olc::K9).bPressed || GetKey(olc::NP9).bPressed) if (inputs[inputSel].length() < 4)	inputs[inputSel] += "9";
			if (GetKey(olc::K0).bPressed || GetKey(olc::NP0).bPressed) if (inputs[inputSel].length() < 4)	inputs[inputSel] += "0";

			//del and backspace will move to the previous selection if there was no input
			if (GetKey(olc::DEL).bPressed)
			{
				if (inputs[inputSel].length() > 0) inputs[inputSel] = inputs[inputSel].substr(0, (inputs[inputSel].length() - 1));
				else if (inputSel != 0) inputSel--;
			}
			if (GetKey(olc::BACK).bPressed)
			{
				if (inputs[inputSel].length() > 0) inputs[inputSel] = inputs[inputSel].substr(0, (inputs[inputSel].length() - 1));
				else if (inputSel != 0) inputSel--;
			}
			//enter should move to the next selection field or to edit mode if all fields are filled
			if (GetKey(olc::ENTER).bPressed || GetKey(olc::RETURN).bPressed) {
				if (inputSel != 3) { inputSel++; }
				else if (inputs[0].length() != 0 || inputs[1].length() != 0 || inputs[2].length() != 0 || inputs[3].length() != 0) {
					//open file explorer to find a tile set
					FindFile("Tileset");
					if (tilePath.length() != 0) {
						initMap();
						CurrentMode = 2;
					}
				}
			}

			//grey out the next button until all fields have inputs
			if (inputs[0].length() == 0 || inputs[1].length() == 0 || inputs[2].length() == 0 || inputs[3].length() == 0)
			{
				DrawString(160, 75, "Next", olc::DARK_GREY, 2);
			}
			else if (vMouse.x >= 160 && vMouse.x < 222 && vMouse.y >= 75 && vMouse.y < 91)
			{
				DrawString(160, 75, "Next", olc::YELLOW, 2);
				//open file explorer to find a tile set, if a tileset is loaded, change to the editer
				if (GetMouse(0).bPressed) {
					FindFile("Tileset");
					if (tilePath.length() != 0) {
						initMap();
						CurrentMode = 2;
					}
				}
			}
			else
				DrawString(160, 75, "Next", olc::WHITE, 2);

			//back button will move back to the main menu
			if (vMouse.x >= 160 && vMouse.x < 222 && vMouse.y >= 91 && vMouse.y < 107)
			{
				DrawString(160, 91, "Back", olc::YELLOW, 2);
				if (GetMouse(0).bPressed) CurrentMode = 0;
			}
			else
				DrawString(160, 91, "Back", olc::WHITE, 2);

			//escape will change back to the main menu screen
			if (GetKey(olc::ESCAPE).bPressed) CurrentMode = 0;
			break;
		}
		case(2):
		{
			//map painting mode
			//mouse info
			olc::vi2d vCell = { vMouse.x / (int)(vTileSize.x * vCameraZoom.x) + (int)vCameraPos.x, vMouse.y / (int)(vTileSize.y * vCameraZoom.y) + (int)vCameraPos.y };
			olc::vi2d vCellPos = { vMouse.x / (int)(vTileSize.x * vCameraZoom.x), vMouse.y / (int)(vTileSize.y * vCameraZoom.y) };

			//prepare the screen and camera
			olc::vf2d vVisibleTiles = { ScreenWidth() / (vTileSize.x * vCameraZoom.x), ScreenHeight() / (vTileSize.y * vCameraZoom.y) };

			//clamp the camera
			if (vCameraPos.x < -1) vCameraPos.x = -1;
			if (vCameraPos.y < -1) vCameraPos.y = -1;
			if (vCameraPos.x >= vWorldSize.x - vVisibleTiles.x + 1) vCameraPos.x = (float)(vWorldSize.x - vVisibleTiles.x + 1);
			if (vCameraPos.y >= vWorldSize.y - vVisibleTiles.y + 1) vCameraPos.y = (float)(vWorldSize.y - vVisibleTiles.y + 1);

			//enter tile selection mode
			if (GetKey(olc::Key::TAB).bHeld) {
				olc::vi2d vCellSel = { vMouse.x / (int)(vTileSize.x * vCameraZoomSel.x) - (int)vCameraPosSel.x, vMouse.y / (int)(vTileSize.y * vCameraZoomSel.y) - (int)vCameraPosSel.y };
				//camera controls
				if (GetKey(olc::UP).bPressed) vCameraPosSel.y++;
				if (GetKey(olc::DOWN).bPressed) vCameraPosSel.y--;
				if (GetKey(olc::LEFT).bPressed) vCameraPosSel.x++;
				if (GetKey(olc::RIGHT).bPressed) vCameraPosSel.x--;
				if (GetKey(olc::Q).bPressed) vCameraZoomSel += { 0.25f, 0.25f };
				if (GetKey(olc::Z).bPressed) if (vCameraZoomSel.x > 0.25f) vCameraZoomSel -= { 0.25f, 0.25f };

				//position to draw the tileset sprite
				olc::vf2d vPos = { (int)vCameraPosSel.x * (vTileSize.x * vCameraZoomSel.x), (int)vCameraPosSel.y * (vTileSize.y * vCameraZoomSel.y) };
				//draw the tileset if not editing layer 3
				if (currentLayer != 3) {
					DrawDecal(vPos, rTileSet.Decal(), vCameraZoomSel);
					//draw the cursor
					DrawDecal((vTileCursor + vCameraPosSel) * (vCameraZoomSel * vTileSize), dCursor, vCameraZoomSel);

					//select a new tile on mouse click
					if (GetMouse(0).bPressed) {
						if (vCellSel.x >= 0 && vCellSel.x < vDimensions.x && vCellSel.y >= 0 && vCellSel.y < vDimensions.y) {
							vTileCursor = vCellSel;
						}
					}
				}
				//draw the layer 3 tileset if editing layer 3
				else
				{
					DrawDecal(vPos, rInteractSet.Decal(), vCameraZoomSel);
					//draw the cursor
					DrawDecal((vTileCursorInter + vCameraPosSel) * (vCameraZoomSel * vTileSize), dCursor, vCameraZoomSel);

					//select a new tile on mouse click
					if (GetMouse(0).bPressed) {
						if (vCellSel.x >= 0 && vCellSel.x < vDimensionsInter.x && vCellSel.y >= 0 && vCellSel.y < vDimensionsInter.y) {
							vTileCursorInter = vCellSel;
						}
					}

				}

				//display mouse position relative to camera
				DrawStringDecal({ 4, 14 }, "Cell     :" + std::to_string(vCellSel.x) + ", " + std::to_string(vCellSel.y));

				//press space to reset the camera and zoom
				if (GetKey(olc::SPACE).bPressed)
				{
					vCameraPosSel = { 0, 0 };
					vCameraZoomSel = { 1.0f, 1.0f };
				}
				return true;
			}

			//camera controls
			if (GetKey(olc::UP).bPressed) vCameraPos.y--;
			if (GetKey(olc::DOWN).bPressed) vCameraPos.y++;
			if (GetKey(olc::LEFT).bPressed) vCameraPos.x--;
			if (GetKey(olc::RIGHT).bPressed) vCameraPos.x++;
			if (GetKey(olc::Q).bPressed) vCameraZoom += { 0.25f, 0.25f };
			if (GetKey(olc::Z).bPressed) if (vCameraZoom.x > 0.25f) vCameraZoom -= { 0.25f, 0.25f };

			//layer control, selecting layer 3 will reset the camera position and zoom on the selection screen
			if (GetKey(olc::K1).bPressed || GetKey(olc::NP1).bPressed) currentLayer = 1;
			if (GetKey(olc::K2).bPressed || GetKey(olc::NP2).bPressed) currentLayer = 2;
			if (GetKey(olc::K3).bPressed || GetKey(olc::NP3).bPressed) { currentLayer = 3; vCameraPosSel = { 0, 0 }; vCameraZoomSel = { 1.0f, 1.0f }; }


			//set the tile the mouse was clicked on to the selected tile on the selected layer
			if (GetMouse(0).bPressed)
			{
				if (currentLayer == 1)
				{
					if (vCell.x >= 0 && vCell.x < vWorldSize.x && vCell.y >= 0 && vCell.y < vWorldSize.y)
						//set the selected tile to the selected tile
						pWorld[vCell.y * vWorldSize.x + vCell.x] = ((vDimensions.x * vTileCursor.y) + vTileCursor.x);
				}
				else if (currentLayer == 2) {

					if (vCell.x >= 0 && vCell.x < vWorldSize.x && vCell.y >= 0 && vCell.y < vWorldSize.y)
						//set the selected tile to the selected tile
						pWorldLayer[vCell.y * vWorldSize.x + vCell.x] = ((vDimensions.x * vTileCursor.y) + vTileCursor.x);
				}
				else if (currentLayer == 3) {
					if (vCell.x >= 0 && vCell.x < vWorldSize.x && vCell.y >= 0 && vCell.y < vWorldSize.y)
						//set the selected tile to the selected tile
						pWorldInter[vCell.y * vWorldSize.x + vCell.x] = ((vDimensionsInter.x * vTileCursorInter.y) + vTileCursorInter.x);
				}
			}

			//delete the tile if the mouse was right clicked
			if (GetMouse(1).bPressed)
			{
				if (currentLayer == 1)
				{
					if (vCell.x >= 0 && vCell.x < vWorldSize.x && vCell.y >= 0 && vCell.y < vWorldSize.y)
						//set the selected tile to the selected tile
						pWorld[vCell.y * vWorldSize.x + vCell.x] = -1;
				}
				else if (currentLayer == 2) {

					if (vCell.x >= 0 && vCell.x < vWorldSize.x && vCell.y >= 0 && vCell.y < vWorldSize.y)
						//set the selected tile to the selected tile
						pWorldLayer[vCell.y * vWorldSize.x + vCell.x] = -1;
				}
				else if (currentLayer == 3) {

					if (vCell.x >= 0 && vCell.x < vWorldSize.x && vCell.y >= 0 && vCell.y < vWorldSize.y)
						//set the selected tile to the selected tile
						pWorldInter[vCell.y * vWorldSize.x + vCell.x] = 0;
				}
			}

			//lambda to find the tile location
			auto ToScreen = [&](int x, int y)
			{
				return olc::vi2d
				(
					(int)(x * vTileSize.x * vCameraZoom.x), (int)(y * vTileSize.y * vCameraZoom.y)
				);
			};

			//lambda to decrypt the id of the tile
			auto GetTile = [&](int x, int y, int*& world)
			{
				if (x >= 0 && x < vWorldSize.x && y >= 0 && y < vWorldSize.y)
					return world[y * vWorldSize.x + x];
				else
					return -1;
			};

			//set pixel mode to alpha for transparency
			SetPixelMode(olc::Pixel::ALPHA);

			//draw the world
			for (int y = 0; y <= vVisibleTiles.y; y++)
			{
				for (int x = 0; x <= vVisibleTiles.x; x++)
				{
					olc::vi2d vWorld = ToScreen(x, y);

					//find the tile ids for both main layers once
					int nTilePos = (GetTile(x + (int)vCameraPos.x, y + (int)vCameraPos.y, pWorld));
					int nTilePosLayer = (GetTile(x + (int)vCameraPos.x, y + (int)vCameraPos.y, pWorldLayer));

					//draw the sprite on every visible tile, and resolve the id to the selected sprite sheet tile only if an id exists
					if (nTilePos > -1) {
						olc::vi2d vTilePos = olc::vi2d((nTilePos % vDimensions.x) * vTileSize.x, (nTilePos / vDimensions.x * vTileSize.y));
						DrawPartialDecal(vWorld, rTileSet.Decal(), vTilePos, vTileSize, vCameraZoom);
					}
					if (nTilePosLayer > -1) {
						olc::vi2d vTilePosLayer = olc::vi2d((nTilePosLayer % vDimensions.x) * vTileSize.x, (nTilePosLayer / vDimensions.x) * vTileSize.y);
						DrawPartialDecal(vWorld, rTileSet.Decal(), vTilePosLayer, vTileSize, vCameraZoom);
					}

					//only draw layer 3 on layer 3 edit mode
					if (currentLayer == 3) {
						int nTilePosInter = (GetTile(x + (int)vCameraPos.x, y + (int)vCameraPos.y, pWorldInter));
						olc::vi2d vTilePosInter = olc::vi2d((nTilePosInter % vDimensionsInter.x) * vTileSize.x, (nTilePosInter / vDimensionsInter.x) * vTileSize.y);
						DrawPartialDecal(vWorld, rInteractSet.Decal(), vTilePosInter, vTileSize, vCameraZoom);
					}
				}
			}

			//set pixel mode back to normal
			SetPixelMode(olc::Pixel::NORMAL);

			//draw a rectangle over the mouse hover location
			DrawDecal(olc::vi2d{ vCellPos.x * (int)(vTileSize.x * vCameraZoom.x), vCellPos.y * (int)(vTileSize.y * vCameraZoom.y) }, dCursor, vCameraZoom, olc::RED);
			DrawStringDecal({ 4, 14 }, "Cell     :" + std::to_string(vCell.x) + ", " + std::to_string(vCell.y));
			DrawStringDecal({ 4, 24 }, "Current Layer: " + std::to_string(currentLayer));

			//escape will return to the main menu to allow for saves, new maps, and loading
			if (GetKey(olc::ESCAPE).bPressed) CurrentMode = 0;
			break;
		}
		case(3):
		{
			//resize page
			DrawString(160, 4, " New Map Width:", olc::WHITE, 2);
			DrawString(160, 20, "New Map Height:", olc::WHITE, 2);

			DrawString(4, 15, "Use your keyboard\nto fill the fields", olc::WHITE);

			//draw the inputs in the correct position
			for (int i = 0; i < 2; i++)
				DrawString(409, ((i * 16) + 4), inputs[i], olc::WHITE, 2);

			//draw an arrow on the current selection
			DrawString(397, ((inputSel * 16) + 4), ">", olc::RED, 2);

			//set the input selection to where the click was
			if (vMouse.x >= 160 && vMouse.y >= 4 && vMouse.y < 20)
				if (GetMouse(0).bPressed) inputSel = 0;
			if (vMouse.x >= 160 && vMouse.y >= 20 && vMouse.y < 36)
				if (GetMouse(0).bPressed) inputSel = 1;

			//Allow for controlled user inputs to fill in the fields and change selections up to 4 digits
			if (GetKey(olc::DOWN).bPressed)
				if (inputSel != 1) inputSel++;
			if (GetKey(olc::UP).bPressed)
				if (inputSel != 0) inputSel--;

			if (GetKey(olc::K1).bPressed || GetKey(olc::NP1).bPressed) if (inputs[inputSel].length() < 4)	inputs[inputSel] += "1";
			if (GetKey(olc::K2).bPressed || GetKey(olc::NP2).bPressed) if (inputs[inputSel].length() < 4)	inputs[inputSel] += "2";
			if (GetKey(olc::K3).bPressed || GetKey(olc::NP3).bPressed) if (inputs[inputSel].length() < 4)	inputs[inputSel] += "3";
			if (GetKey(olc::K4).bPressed || GetKey(olc::NP4).bPressed) if (inputs[inputSel].length() < 4)	inputs[inputSel] += "4";
			if (GetKey(olc::K5).bPressed || GetKey(olc::NP5).bPressed) if (inputs[inputSel].length() < 4)	inputs[inputSel] += "5";
			if (GetKey(olc::K6).bPressed || GetKey(olc::NP6).bPressed) if (inputs[inputSel].length() < 4)	inputs[inputSel] += "6";
			if (GetKey(olc::K7).bPressed || GetKey(olc::NP7).bPressed) if (inputs[inputSel].length() < 4)	inputs[inputSel] += "7";
			if (GetKey(olc::K8).bPressed || GetKey(olc::NP8).bPressed) if (inputs[inputSel].length() < 4)	inputs[inputSel] += "8";
			if (GetKey(olc::K9).bPressed || GetKey(olc::NP9).bPressed) if (inputs[inputSel].length() < 4)	inputs[inputSel] += "9";
			if (GetKey(olc::K0).bPressed || GetKey(olc::NP0).bPressed) if (inputs[inputSel].length() < 4)	inputs[inputSel] += "0";

			//del and backspace will move to the previous selection if there was no input
			if (GetKey(olc::DEL).bPressed)
			{
				if (inputs[inputSel].length() > 0) inputs[inputSel] = inputs[inputSel].substr(0, (inputs[inputSel].length() - 1));
				else if (inputSel != 0) inputSel--;
			}
			if (GetKey(olc::BACK).bPressed)
			{
				if (inputs[inputSel].length() > 0) inputs[inputSel] = inputs[inputSel].substr(0, (inputs[inputSel].length() - 1));
				else if (inputSel != 0) inputSel--;
			}
			//enter should move to the next selection field or to edit mode if all fields are filled
			if (GetKey(olc::ENTER).bPressed || GetKey(olc::RETURN).bPressed) {
				if (inputSel != 1) { inputSel++; }
				else if (inputs[0].length() != 0 || inputs[1].length() != 0) {
					//resize the map and return to the editer
					Resize(std::stoi(inputs[0]), std::stoi(inputs[1]));
					CurrentMode = 2;
				}
			}

			//grey out the next button until all fields have inputs
			if (inputs[0].length() == 0 || inputs[1].length() == 0)
			{
				DrawString(160, 75, "Next", olc::DARK_GREY, 2);
			}
			else if (vMouse.x >= 160 && vMouse.x < 222 && vMouse.y >= 75 && vMouse.y < 91)
			{
				DrawString(160, 75, "Next", olc::YELLOW, 2);
				//resize the map and return to the editer
				if (GetMouse(0).bPressed) {
					Resize(std::stoi(inputs[0]), std::stoi(inputs[1]));
					CurrentMode = 2;
				}
			}
			else
				DrawString(160, 75, "Next", olc::WHITE, 2);

			//back button will move back to the main menu
			if (vMouse.x >= 160 && vMouse.x < 222 && vMouse.y >= 91 && vMouse.y < 107)
			{
				DrawString(160, 91, "Back", olc::YELLOW, 2);
				if (GetMouse(0).bPressed) CurrentMode = 0;
			}
			else
				DrawString(160, 91, "Back", olc::WHITE, 2);

			//escape will change back to the main menu screen
			if (GetKey(olc::ESCAPE).bPressed) CurrentMode = 0;
			break;

			break;
		}
		default: 
		{
			DrawStringDecal({ 4, 14 }, "Error, no current mode. Press escape to return to the main menu");
			if (GetKey(olc::ESCAPE).bPressed) CurrentMode = 0;
			break;
		}
		}


		//mouse position for debugging in all modes
		DrawStringDecal({ 4, 4 }, "Mouse   : " + std::to_string(vMouse.x) + ", " + std::to_string(vMouse.y));

		return true;
	}

	//method to initialize the map
	void initMap() 
	{
		//take user input to determine the map and tiles size
		vWorldSize = { std::stoi(inputs[0]), std::stoi(inputs[1]) };
		vTileSize = { std::stoi(inputs[2]), std::stoi(inputs[3]) };
		
		int nSize = vWorldSize.x * vWorldSize.y;
		std::string tiles(tilePath.begin(), tilePath.end());

		//load the sprites and world
		rTileSet.Load(tiles);
		vDimensions = SpriteSize(tiles);
		vDimensionsInter = SpriteSize("InterSheet.png");
		pWorld = new int[nSize]{ 0 };
		pWorldLayer = new int[nSize]{ 0 };
		std::fill_n(pWorldLayer, nSize, -1);
		pWorldInter = new int[nSize]{ 0 };

		//draw a curser
		sCursor = new olc::Sprite(vTileSize.x, vTileSize.y);
		SetDrawTarget(sCursor);
		Clear(olc::BLANK);
		DrawRect(0, 0, sCursor->width - 1, sCursor->height - 1, olc::WHITE);
		SetDrawTarget(nullptr);
		dCursor = new olc::Decal(sCursor);

		//reset the inputs
		for(int i = 0; i < 4; i++)
			inputs[i] = "";
	}

	//method to find the size of the spritesheet
	olc::vi2d SpriteSize(std::string sprite) {
		std::ifstream file(sprite, std::ios_base::binary | std::ios_base::in);

		if (!file.is_open() || !file) 
		{
			file.close();
			return { 0 , 0 };
		}

		int width, height;

		//skip unrequired bits and read the width and height bits in the header
		file.seekg(16, std::ios_base::cur);
		file.read((char*)&width, 4);
		file.read((char*)&height, 4);

		//change to big endian
		width = _byteswap_ulong(width);
		height = _byteswap_ulong(height);

		width = width / vTileSize.x;
		height = height / vTileSize.y;

		file.close();

		//return the dimensions in terms of sprites
		return { width, height };
	}

	//method to open a file explorer to find a file path
	void FindFile(const std::string& type) {
		//use open file name
		OPENFILENAME file;
		memset(&file, 0, sizeof file);
		wchar_t output[MAX_PATH];
		output[0] = 0;
		file.lStructSize = sizeof file;
		file.hwndOwner = nullptr;
		file.lpstrFile = output;
		file.lpstrFile[0] = '\0';
		//when loading tilesets search for .png files, otherwise search for .txt or .lvl files for saved levels
		if (type == "Tileset") { file.lpstrFilter = L"Tile Sets\0*.png\0\0"; }
		else { file.lpstrFilter = L"Level Files\0*.txt;*.lvl\0\0"; }
		//when saving levels make it a save as button
		if (type == "Save") { file.lpstrTitle = L"Save As"; }
		else { file.lpstrTitle = L"Select A File"; }
		file.nFilterIndex = 1;
		file.nMaxFile = MAX_PATH;
		file.Flags = OFN_NONETWORKBUTTON | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

		//set the loaded path to either the save path or load/file path
		if (type == "Save" || type == "SaveTileset") 
		{
			if (GetSaveFileName(&file) != FALSE) savePath = output;
		}
		else 
		{
			//also works as the load path
			if (GetOpenFileName(&file) != FALSE) tilePath = output;
		}
	}

	//saves the current map in plain text
	void SaveFile() {
		//make sure a file is open and a save path is defined
		if (tilePath.length() != 0 && savePath.length() != 0)
		{
			//create a file, or save over a file
			std::ofstream save((savePath + L".lvl"));

			if (save.is_open())
			{
				int size = vWorldSize.x * vWorldSize.y;
				//print the world size and tile size to the file header
				save << vWorldSize.x << " " << vWorldSize.y << " " << vTileSize.x << " " << vTileSize.y << "\n";
				//print tilePath and tile dimensions on the next line
				save << tilePath << " " << vDimensions.x << " " << vDimensions.y << "\n";
				//print the id of all 3 layers right next to each other in order
				for (int i = 0; i < size; i++)
				{
					save << pWorld[i] << " " << pWorldLayer[i] << " " << pWorldInter[i] << " ";
				}
			}

			save.close();
		}
	}

	//loads a file from plain text
	void LoadFile()
	{
		std::fstream load;
		std::string word;
		load.open(tilePath);

		if (load.is_open())
		{
			//load each word in order
			load >> word;
			vWorldSize.x = std::stoi(word);
			load >> word;
			vWorldSize.y = std::stoi(word);
			load >> word;
			vTileSize.x = std::stoi(word);
			load >> word;
			vTileSize.y = std::stoi(word);
			load >> word;
			tilePath = strconverter.from_bytes(word);
			load >> word;
			vDimensions.x = std::stoi(word);
			load >> word;
			vDimensions.y = std::stoi(word);

			int nSize = vWorldSize.x * vWorldSize.y;
			int index = 0;

			//initialize other needed variables
			std::string tiles(tilePath.begin(), tilePath.end());
			rTileSet.Load(tiles);
			vDimensionsInter = SpriteSize("InterSheet.png");
			pWorld = new int[nSize] { 0 };
			pWorldLayer = new int[nSize] { 0 };
			pWorldInter = new int[nSize] { 0 };

			//fill the arrays
			while (load >> word)
			{
				pWorld[index] = std::stoi(word);
				load >> word;
				pWorldLayer[index] = std::stoi(word);
				load >> word;
				pWorldInter[index] = std::stoi(word);
				index++;
			}

			//draw a curser
			sCursor = new olc::Sprite(vTileSize.x, vTileSize.y);
			SetDrawTarget(sCursor);
			Clear(olc::BLANK);
			DrawRect(0, 0, sCursor->width - 1, sCursor->height - 1, olc::WHITE);
			SetDrawTarget(nullptr);
			dCursor = new olc::Decal(sCursor);

			//reset the inputs
			for (int i = 0; i < 4; i++)
				inputs[i] = "";
		}
	
		load.close();
	}

	//method to resize the map
	void Resize(int x, int y)
	{
		int nSize = x * y;

		//create new arrays to put into the old arrays
		int* newpWorld = new int[nSize] { 0 };
		int* newpWorldLayer = new int[nSize] { 0 };
		std::fill_n(newpWorldLayer, nSize, -1);
		int* newpWorldInter = new int[nSize] { 0 };

		//copy over the arrays into the new arrays
		for(int i = 0; i < y; i++)
			for (int j = 0; j < x; j++)
			{
				//make sure the original arrays have space left
				if (j < vWorldSize.x && i < vWorldSize.y) {
					newpWorld[(i * x) + j] = pWorld[(i * vWorldSize.x) + j];
					newpWorldLayer[(i * x) + j] = pWorldLayer[(i * vWorldSize.x) + j];
					newpWorldInter[(i * x) + j] = pWorldLayer[(i * vWorldSize.x) + j];
				}
			}

		//set the new arrays to the old arrays and update the world size
		pWorld = newpWorld;
		pWorldLayer = newpWorldLayer;
		pWorldInter = newpWorldInter;

		vWorldSize = { x, y };
	}
};

int main()
{
	MapBuilder demo;
	if (demo.Construct(512, 480, 1, 1))
		demo.Start();
	return 0;
}