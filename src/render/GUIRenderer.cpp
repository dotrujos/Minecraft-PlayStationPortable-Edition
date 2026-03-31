#include "GUIRenderer.h"
#include <malloc.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspkernel.h>
#include <string.h>

GUIRenderer::GUIRenderer() {
  m_iconsTex.load("res/icons.png");
  m_vertices = (SkyVertex *)memalign(16, 6 * sizeof(SkyVertex));
}

GUIRenderer::~GUIRenderer() {
  if (m_vertices)
    free(m_vertices);
}

void GUIRenderer::render(int screenWidth, int screenHeight) {
  if (!m_iconsTex.data) return;

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

  // Crosshair is at (0,0) in icons.png, size 16x16
  float u0 = 0.0f;
  float v0 = 0.0f;
  float u1 = 16.0f / 256.0f;
  float v1 = 16.0f / 256.0f;

  float size = 16.0f;
  float x0 = (screenWidth - size) / 2.0f;
  float y0 = (screenHeight - size) / 2.0f;
  float x1 = x0 + size;
  float y1 = y0 + size;

  m_vertices[0] = {u0, v0, 0xFFFFFFFF, x0, y0, 0.0f};
  m_vertices[1] = {u1, v0, 0xFFFFFFFF, x1, y0, 0.0f};
  m_vertices[2] = {u0, v1, 0xFFFFFFFF, x0, y1, 0.0f};

  m_vertices[3] = {u1, v0, 0xFFFFFFFF, x1, y0, 0.0f};
  m_vertices[4] = {u1, v1, 0xFFFFFFFF, x1, y1, 0.0f};
  m_vertices[5] = {u0, v1, 0xFFFFFFFF, x0, y1, 0.0f};

  sceKernelDcacheWritebackInvalidateRange(m_vertices, 6 * sizeof(SkyVertex));

  sceGuDisable(GU_DEPTH_TEST);
  sceGuDisable(GU_CULL_FACE);
  sceGuDisable(GU_FOG);
  sceGuDisable(GU_ALPHA_TEST); // Crosshair might have low alpha
  
  sceGuEnable(GU_BLEND);
  // Simple alpha blend for now
  sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);

  sceGuEnable(GU_TEXTURE_2D);
  m_iconsTex.bind();

  sceGumDrawArray(GU_TRIANGLES,
                  GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_3D,
                  6, 0, m_vertices);

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
