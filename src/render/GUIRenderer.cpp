#include "GUIRenderer.h"
#include "../world/Blocks.h"
#include <malloc.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspkernel.h>
#include <string.h>

GUIRenderer::GUIRenderer() {
  m_iconsTex.load("res/icons.png");
  m_guiTex.load("res/gui.png");
  m_terrainTex.load("res/terrain.png");
  m_vertices = (SkyVertex *)memalign(16, 64 * sizeof(SkyVertex));
}

GUIRenderer::~GUIRenderer() {
  if (m_vertices)
    free(m_vertices);
}

void GUIRenderer::drawBlockIcon(float x, float y, float size, uint8_t bid) {
  if (!m_terrainTex.data) return;
  m_terrainTex.bind();

  // Isometric view parameters
  float s = size * 0.5f;        // Half width
  float h = s * 0.5f;           // Diamond vertical radius
  float sideHeight = s * 1.1f;  // Make sides taller to avoid "slab" look
  y -= (sideHeight * 0.4f); 

  // Top Face (Diamond) - Brightest
  int tx = g_blockUV[bid].top_x, ty = g_blockUV[bid].top_y;
  float u0 = tx/16.0f, v0 = ty/16.0f, u1 = (tx+1)/16.0f, v1 = (ty+1)/16.0f;
  uint32_t cTop = 0xFFFFFFFF;
  m_vertices[0] = {u0, v0, cTop, x,       y - h,   0}; // Top
  m_vertices[1] = {u1, v0, cTop, x + s,   y,       0}; // Right
  m_vertices[2] = {u0, v1, cTop, x - s,   y,       0}; // Left
  m_vertices[3] = {u1, v0, cTop, x + s,   y,       0};
  m_vertices[4] = {u1, v1, cTop, x,       y + h,   0}; // Bottom (of top face)
  m_vertices[5] = {u0, v1, cTop, x - s,   y,       0};

  // Left Side - Mid Brightness
  tx = g_blockUV[bid].side_x; ty = g_blockUV[bid].side_y;
  u0 = tx/16.0f; v0 = ty/16.0f; u1 = (tx+1)/16.0f; v1 = (ty+1)/16.0f;
  uint32_t cSide = 0xFFCCCCCC; 
  m_vertices[6]  = {u0, v0, cSide, x - s,   y,       0};
  m_vertices[7]  = {u1, v0, cSide, x,       y + h,   0};
  m_vertices[8]  = {u0, v1, cSide, x - s,   y + sideHeight,   0};
  m_vertices[9]  = {u1, v0, cSide, x,       y + h,   0};
  m_vertices[10] = {u1, v1, cSide, x,       y + h + sideHeight, 0};
  m_vertices[11] = {u0, v1, cSide, x - s,   y + sideHeight,   0};

  // Right Side - Darkest
  uint32_t cFront = 0xFF999999;
  m_vertices[12] = {u0, v0, cFront, x,       y + h,   0};
  m_vertices[13] = {u1, v0, cFront, x + s,   y,       0};
  m_vertices[14] = {u0, v1, cFront, x,       y + h + sideHeight, 0};
  m_vertices[15] = {u1, v0, cFront, x + s,   y,       0};
  m_vertices[16] = {u1, v1, cFront, x + s,   y + sideHeight,   0};
  m_vertices[17] = {u0, v1, cFront, x,       y + h + sideHeight, 0};

  sceKernelDcacheWritebackInvalidateRange(m_vertices, 18 * sizeof(SkyVertex));
  sceGumDrawArray(GU_TRIANGLES, GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_3D, 18, 0, m_vertices);
}

void GUIRenderer::drawTexturedRect(float x, float y, float w, float h, float u0, float v0, float u1, float v1, uint32_t color, SimpleTexture& tex) {
  if (!tex.data) return;
  tex.bind();

  m_vertices[0] = {u0, v0, color, x, y, 0.0f};
  m_vertices[1] = {u1, v0, color, x + w, y, 0.0f};
  m_vertices[2] = {u0, v1, color, x, y + h, 0.0f};

  m_vertices[3] = {u1, v0, color, x + w, y, 0.0f};
  m_vertices[4] = {u1, v1, color, x + w, y + h, 0.0f};
  m_vertices[5] = {u0, v1, color, x, y + h, 0.0f};

  sceKernelDcacheWritebackInvalidateRange(m_vertices, 6 * sizeof(SkyVertex));
  sceGumDrawArray(GU_TRIANGLES, GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_3D, 6, 0, m_vertices);
}

void GUIRenderer::render(int screenWidth, int screenHeight, uint8_t heldBlock, const uint8_t* hotbarItems, int hotbarCount, int selectedIndex) {
  // Setup Orthographic Projection for 2D UI
  sceGumMatrixMode(GU_PROJECTION);
  sceGumPushMatrix();
  sceGumLoadIdentity();
  sceGumOrtho(0, screenWidth, screenHeight, 0, -1.0f, 1.0f);

  sceGumMatrixMode(GU_VIEW);
  sceGumPushMatrix();
  sceGumLoadIdentity();

  sceGumMatrixMode(GU_MODEL);
  sceGumPushMatrix();
  sceGumLoadIdentity();

  sceGuDisable(GU_DEPTH_TEST);
  sceGuDisable(GU_CULL_FACE);
  sceGuDisable(GU_FOG);
  sceGuDisable(GU_ALPHA_TEST); 
  sceGuEnable(GU_BLEND);
  sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
  sceGuEnable(GU_TEXTURE_2D);

  // Crosshair
  if (m_iconsTex.data) {
    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 16.0f / 256.0f;
    float v1 = 16.0f / 256.0f;
    float size = 16.0f;
    drawTexturedRect((screenWidth - size) / 2.0f, (screenHeight - size) / 2.0f, size, size, u0, v0, u1, v1, 0xFFFFFFFF, m_iconsTex);
  }

  // Hotbar
  if (m_guiTex.data) {
    float scale = 1.5f;
    float hbW = 182.0f * scale;
    float hbH = 22.0f * scale;
    float hbX = (screenWidth - hbW) / 2.0f;
    float hbY = screenHeight - hbH - 30.0f; 
    
    uint32_t guiColor = 0xC0FFFFFF; // 75% opacity

    drawTexturedRect(hbX, hbY, hbW, hbH, 0, 0, 182.0f / 256.0f, 22.0f / 256.0f, guiColor, m_guiTex);
    
    int localIdx = selectedIndex % 9; 
    
    float selSize = 24.0f * scale;
    float selX = hbX - (1.0f * scale) + localIdx * (20.0f * scale);
    float selY = hbY - (1.0f * scale);
    drawTexturedRect(selX, selY, selSize, selSize, 0, 22.0f / 256.0f, 24.0f / 256.0f, (22.0f + 24.0f) / 256.0f, guiColor, m_guiTex);

    // Hotbar Items
    if (m_terrainTex.data && hotbarItems) {
      for (int i = 0; i < 9 && i < hotbarCount; i++) {
        uint8_t bid = hotbarItems[i];
        if (bid == BLOCK_AIR) continue;

        float itemSize = 16.0f * scale;
        float itemX = hbX + (11.0f * scale) + i * (20.0f * scale);
        float itemY = hbY + (11.0f * scale);

        drawBlockIcon(itemX, itemY - 1.0f, itemSize * .9f, bid);
      }
    }
  }

  // Restore states
  sceGuDisable(GU_BLEND);
  sceGuEnable(GU_DEPTH_TEST);
  sceGuEnable(GU_CULL_FACE);
  sceGuEnable(GU_FOG);
  sceGuEnable(GU_ALPHA_TEST);

  sceGumMatrixMode(GU_MODEL);
  sceGumPopMatrix();
  sceGumMatrixMode(GU_VIEW);
  sceGumPopMatrix();
  sceGumMatrixMode(GU_PROJECTION);
  sceGumPopMatrix();
  sceGumMatrixMode(GU_MODEL);
}
