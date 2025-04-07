#pragma once

#include <array>
#include <utility>

#include "common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/tiny_ecs.hpp"

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem
{
  /**
   * The following arrays store the assets the game will use. They are loaded
   * at initialization and are assumed to not be modified by the render loop.
   *
   * Whenever possible, add to these lists instead of creating dynamic state
   * it is easier to debug and faster to execute for the computer.
   */
  std::array<GLuint, texture_count> texture_gl_handles;
  std::array<ivec2, texture_count> texture_dimensions;

  // Make sure these paths remain in sync with the associated enumerators.
  // Associated id with .obj path
  const std::vector<std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths = {
      std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::LEGACY_CHICKEN, mesh_path("chicken.obj"))
      // specify meshes of other assets here
  };

  // Make sure these paths remain in sync with the associated enumerators (see TEXTURE_ASSET_ID).
  const std::array<std::string, texture_count> texture_paths = {
      // Numbers
      textures_path("numbers/0.png"),
      textures_path("numbers/1.png"),
      textures_path("numbers/2.png"),
      textures_path("numbers/3.png"),
      textures_path("numbers/4.png"),
      textures_path("numbers/5.png"),
      textures_path("numbers/6.png"),
      textures_path("numbers/7.png"),
      textures_path("numbers/8.png"),
      textures_path("numbers/9.png"),
      textures_path("numbers/colon.png"),

      // Ramster
      textures_path("player/run_0.png"),
      textures_path("player/run_1.png"),
      textures_path("player/run_2.png"),
      textures_path("player/run_3.png"),
      textures_path("player/run_4.png"),
      textures_path("player/run_5.png"),
      textures_path("player/run_6.png"),
      textures_path("player/run_7.png"),
      textures_path("player/idle_0.png"),
      textures_path("player/idle_1.png"),
      textures_path("player/idle_2.png"),
      textures_path("player/idle_3.png"),
      textures_path("player/idle_4.png"),
      textures_path("player/idle_5.png"),
      textures_path("projectiles/glass-front.png"),
      textures_path("projectiles/glass-back.png"),
      textures_path("projectiles/glass-wall.png"),

      // Grapple
      textures_path("projectiles/grapple_point.png"),
      textures_path("projectiles/grapple_outline.png"),


      // Screen Elements
      textures_path("screenElements/menu_title.png"),
      textures_path("screenElements/pause_title.png"),
      textures_path("screenElements/victory_title.png"),
      textures_path("screenElements/defeat_title.png"),
      textures_path("screenElements/menu_text.png"),
      textures_path("screenElements/pause_text.png"),
      textures_path("screenElements/gameover_text.png"),

      textures_path("screenElements/button_lvlUp.png"),
      textures_path("screenElements/button_lvlDown.png"),
      textures_path("screenElements/button_start.png"),
      textures_path("screenElements/button_resume.png"),
      textures_path("screenElements/button_restart.png"),
      textures_path("screenElements/button_mainMenu.png"),
      textures_path("screenElements/button_exit.png"),

      textures_path("screenElements/button_lvl1.png"),
      textures_path("screenElements/button_lvl2.png"),
      textures_path("screenElements/button_lvl3.png"),
      textures_path("screenElements/button_lvl4.png"),
      textures_path("screenElements/button_lvl5.png"),
      textures_path("screenElements/button_lvl6.png"),
      textures_path("screenElements/button_lvl7.png"),
      textures_path("screenElements/button_lvl8.png"),
      textures_path("screenElements/button_lvl9.png"),
      textures_path("screenElements/button_lvl10.png"),
      textures_path("screenElements/button_lvl11.png"),
      textures_path("screenElements/button_lvl12.png"),

      // Screens
      textures_path("screens/main_menu.png"),
      textures_path("screens/test_screen.png"), // This one's the playing screen that we will never use.
      textures_path("screens/pause.png"),
      textures_path("screens/game_ended_screen.png"),

      // Invaders
      textures_path("invaders/common_1.png"),
      textures_path("invaders/common_2.png"),
      textures_path("invaders/common_3.png"),
      textures_path("invaders/common_4.png"),
      textures_path("invaders/common_5.png"),

      textures_path("invaders/swarm_1.png"),
      textures_path("invaders/swarm_2.png"),
      textures_path("invaders/swarm_3.png"),
      textures_path("invaders/swarm_4.png"),

      textures_path("invaders/obstacle_1.png"),
      textures_path("invaders/obstacle_2.png"),
      textures_path("invaders/obstacle_3.png"),
      textures_path("invaders/obstacle_4.png"),

      // Levels
      textures_path("levels/level1.png"),
      textures_path("levels/level2.png"),
      textures_path("levels/level3.png"),
      textures_path("levels/level4.png"),
      textures_path("levels/level5.png"),
      textures_path("levels/level6.png"),
      textures_path("levels/tutorial.png"),
      textures_path("levels/tower.png"),
      textures_path("levels/lab.png"),
      textures_path("levels/under.png"),
      textures_path("levels/snake.png"),
      textures_path("levels/tunnelsmall.png"),

      // Parallax
      textures_path("levels/background_0.png"),
      textures_path("levels/background_1.png"),
      textures_path("levels/background_2.png"),
      textures_path("levels/background_3.png"),
      textures_path("levels/background_4.png"),
      textures_path("levels/background_5.png"),
      textures_path("levels/background_6.png"),
      textures_path("levels/background_7.png"),

      // Fireball
      textures_path("fireball_effect/frame_00_delay-0.06s.png"),
      textures_path("fireball_effect/frame_01_delay-0.06s.png"),
      textures_path("fireball_effect/frame_02_delay-0.06s.png"),
      textures_path("fireball_effect/frame_03_delay-0.06s.png"),
      textures_path("fireball_effect/frame_04_delay-0.06s.png"),
      textures_path("fireball_effect/frame_05_delay-0.06s.png"),
      textures_path("fireball_effect/frame_06_delay-0.06s.png"),
      textures_path("fireball_effect/frame_07_delay-0.06s.png"),
      textures_path("fireball_effect/frame_08_delay-0.06s.png"),
      textures_path("fireball_effect/frame_09_delay-0.06s.png"),
      textures_path("fireball_effect/frame_10_delay-0.06s.png"),
      textures_path("fireball_effect/frame_11_delay-0.06s.png"),

      // Story Slides
      textures_path("storyFrames/intro_1.png"),
      textures_path("storyFrames/intro_2.png"),
      textures_path("storyFrames/intro_3.png"),
      textures_path("storyFrames/intro_4.png"),
      textures_path("storyFrames/conclusion_1.png"),
      textures_path("storyFrames/conclusion_2.png"),
      textures_path("storyFrames/conclusion_3.png"),

      // Confetti
      textures_path("victory_confetti/frame_00_delay-0.03s.png"),
      textures_path("victory_confetti/frame_01_delay-0.03s.png"),
      textures_path("victory_confetti/frame_02_delay-0.03s.png"),
      textures_path("victory_confetti/frame_03_delay-0.03s.png"),
      textures_path("victory_confetti/frame_04_delay-0.03s.png"),
      textures_path("victory_confetti/frame_05_delay-0.03s.png"),
      textures_path("victory_confetti/frame_06_delay-0.03s.png"),
      textures_path("victory_confetti/frame_07_delay-0.03s.png"),
      textures_path("victory_confetti/frame_08_delay-0.03s.png"),
      textures_path("victory_confetti/frame_09_delay-0.03s.png"),
      textures_path("victory_confetti/frame_10_delay-0.03s.png"),
      textures_path("victory_confetti/frame_11_delay-0.03s.png"),
      textures_path("victory_confetti/frame_12_delay-0.03s.png"),
      textures_path("victory_confetti/frame_13_delay-0.03s.png"),
      textures_path("victory_confetti/frame_14_delay-0.03s.png"),
      textures_path("victory_confetti/frame_15_delay-0.03s.png"),
      textures_path("victory_confetti/frame_16_delay-0.03s.png"),
      textures_path("victory_confetti/frame_17_delay-0.03s.png"),
      textures_path("victory_confetti/frame_18_delay-0.03s.png"),
      textures_path("victory_confetti/frame_19_delay-0.03s.png"),
      textures_path("victory_confetti/frame_20_delay-0.03s.png"),
      textures_path("victory_confetti/frame_21_delay-0.03s.png"),
      textures_path("victory_confetti/frame_22_delay-0.03s.png"),
      textures_path("victory_confetti/frame_23_delay-0.03s.png"),
      textures_path("victory_confetti/frame_24_delay-0.03s.png"),
      textures_path("victory_confetti/frame_25_delay-0.03s.png"),
      textures_path("victory_confetti/frame_26_delay-0.03s.png"),
      textures_path("victory_confetti/frame_27_delay-0.03s.png"),
      textures_path("victory_confetti/frame_28_delay-0.03s.png"),
      textures_path("victory_confetti/frame_29_delay-0.03s.png"),
      textures_path("victory_confetti/frame_30_delay-0.03s.png"),
      textures_path("victory_confetti/frame_31_delay-0.03s.png"),
      textures_path("victory_confetti/frame_32_delay-0.03s.png"),
      textures_path("victory_confetti/frame_33_delay-0.03s.png"),
      textures_path("victory_confetti/frame_34_delay-0.03s.png"),
      textures_path("victory_confetti/frame_35_delay-0.03s.png"),
      textures_path("victory_confetti/frame_36_delay-0.03s.png"),
      textures_path("victory_confetti/frame_37_delay-0.03s.png"),
      textures_path("victory_confetti/frame_38_delay-0.03s.png"),
      textures_path("victory_confetti/frame_39_delay-0.03s.png"),
      textures_path("victory_confetti/frame_40_delay-0.03s.png"),
      textures_path("victory_confetti/frame_41_delay-0.03s.png"),
      textures_path("victory_confetti/frame_42_delay-0.03s.png"),
      textures_path("victory_confetti/frame_43_delay-0.03s.png"),
      textures_path("victory_confetti/frame_44_delay-0.03s.png"),
      textures_path("victory_confetti/frame_45_delay-0.03s.png"),
      textures_path("victory_confetti/frame_46_delay-0.03s.png"),
      textures_path("victory_confetti/frame_47_delay-0.03s.png"),
      textures_path("victory_confetti/frame_48_delay-0.03s.png"),
      textures_path("victory_confetti/frame_49_delay-0.03s.png"),
      textures_path("victory_confetti/frame_50_delay-0.03s.png"),
      textures_path("victory_confetti/frame_51_delay-0.03s.png"),
      textures_path("victory_confetti/frame_52_delay-0.03s.png"),
      textures_path("victory_confetti/frame_53_delay-0.03s.png"),
      textures_path("victory_confetti/frame_54_delay-0.03s.png"),
      textures_path("victory_confetti/frame_55_delay-0.03s.png"),
      textures_path("victory_confetti/frame_56_delay-0.03s.png"),
      textures_path("victory_confetti/frame_57_delay-0.03s.png"),
      textures_path("victory_confetti/frame_58_delay-0.03s.png"),
  };

  std::array<GLuint, effect_count> effects;
  // Make sure these paths remain in sync with the associated enumerators.
  const std::array<std::string, effect_count> effect_paths = {
      shader_path("egg"),
      shader_path("chicken"),
      shader_path("textured"),
      shader_path("vignette"),
      shader_path("parallax"),
      shader_path("translucent"),
      shader_path("fireball"),
      shader_path("player"),
  };

  std::array<GLuint, geometry_count> vertex_buffers;
  std::array<GLuint, geometry_count> index_buffers;
  std::array<Mesh, geometry_count> meshes;

public:
  // Initialize the window
  bool init(GLFWwindow *window);

  template <class T>
  void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);

  void initializeGlTextures();

  void initializeGlEffects();

  void initializeGlMeshes();

  Mesh &getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };

  void initializeGlGeometryBuffers();

  // Initialize the screen texture used as intermediate render target
  // The draw loop first renders to this texture, then it is used for the vignette shader
  bool initScreenTexture();

  // Destroy resources associated to one or all entities created by the system
  ~RenderSystem();

  // Draw all entities
  void draw(float elapsed_ms, bool game_active);

  mat3 createProjectionMatrix();

  Entity get_screen_state_entity() { return screen_state_entity; }

  // Window resizing
  void resizeScreenTexture(int width, int height);

  int screen_viewport_x = 0, screen_viewport_y = 0;
  int screen_viewport_w = 1200, screen_viewport_h = 900;

private:
  // Internal drawing functions for each entity type
  void drawGridLine(Entity entity, const mat3 &projection);
  void drawLine(Entity entity, const mat3 &projection);
  void drawTexturedMesh(Entity entity, const mat3 &projection, float elapsed_ms, bool game_active);
  void drawToScreen();

  // Window handle
  GLFWwindow *window;

  // Screen texture handles
  GLuint frame_buffer;
  GLuint off_screen_render_buffer_color;
  GLuint off_screen_render_buffer_depth;

  Entity screen_state_entity;
};

bool loadEffectFromFile(
    const std::string &vs_path, const std::string &fs_path, GLuint &out_program);
