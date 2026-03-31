#pragma once

#include "SkyRenderer.h"
#include "TextureAtlas.h"
#include <stdint.h>
#include <vector>

class GUIRenderer {
public:
  GUIRenderer();
  ~GUIRenderer();

  void render(int screenWidth, int screenHeight, uint8_t heldBlock, const uint8_t* hotbarItems, int hotbarCount, int selectedIndex);

private:
  SimpleTexture m_iconsTex;
  SimpleTexture m_guiTex;
  SimpleTexture m_terrainTex;
  SkyVertex *m_vertices;

  void drawTexturedRect(float x, float y, float w, float h, float u0, float v0, float u1, float v1, uint32_t color, SimpleTexture& tex);
  void drawBlockIcon(float x, float y, float size, uint8_t bid);
};
